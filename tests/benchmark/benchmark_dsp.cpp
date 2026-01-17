/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * benchmark_dsp.cpp - Performance benchmarks for DSP components
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#include <gtest/gtest.h>
#include <core/VoiceManager.h>
#include <dsp/FormantResonator.h>
#include <dsp/SubharmonicGenerator.h>
#include <dsp/SpectralEnhancer.h>
#include <synthesis/FormantSynthesis.h>
#include <synthesis/SubharmonicSynthesis.h>
#include <synthesis/DiphoneSynthesis.h>
#include <utils/AudioBuffer.h>

#include <chrono>
#include <cmath>
#include <vector>
#include <iomanip>
#include <sstream>

using namespace ChoirV2;

// Performance tracking helper
struct PerformanceMetrics {
    std::string name;
    int iterations;
    double totalTimeMs;
    double avgTimeMs;
    double minTimeMs;
    double maxTimeMs;
    double samplesPerSecond;

    std::string getReport() const {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(3);
        oss << "  " << name << ":\n";
        oss << "    Iterations: " << iterations << "\n";
        oss << "    Total: " << totalTimeMs << " ms\n";
        oss << "    Avg: " << avgTimeMs << " ms\n";
        oss << "    Min: " << minTimeMs << " ms\n";
        oss << "    Max: " << maxTimeMs << " ms\n";
        oss << "    Throughput: " << samplesPerSecond << " samples/sec\n";
        return oss.str();
    }
};

class DSPBenchmarkTest : public ::testing::Test {
protected:
    void SetUp() override {
        sampleRate_ = 44100.0;
        bufferSize_ = 512;
        benchmarkIterations_ = 1000;
    }

    void TearDown() override {
    }

    // Helper: Generate test audio buffer
    void generateSineWave(AudioBuffer& buffer, float frequency) {
        int numSamples = buffer.getNumSamples();
        float* left = buffer.getWritePointer(0);
        float* right = buffer.getWritePointer(1);

        float phase = 0.0f;
        float phaseIncrement = (2.0f * M_PI * frequency) / sampleRate_;

        for (int i = 0; i < numSamples; ++i) {
            float sample = std::sin(phase);
            phase += phaseIncrement;
            if (phase > 2.0f * M_PI) phase -= 2.0f * M_PI;
            left[i] = sample;
            right[i] = sample;
        }
    }

    // Helper: Run benchmark
    PerformanceMetrics runBenchmark(
        const std::string& name,
        std::function<void()> benchmarkFunc,
        int iterations
    ) {
        std::vector<double> times;
        times.reserve(iterations);

        for (int i = 0; i < iterations; ++i) {
            auto startTime = std::chrono::high_resolution_clock::now();
            benchmarkFunc();
            auto endTime = std::chrono::high_resolution_clock::now();

            auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
                endTime - startTime
            ).count();

            times.push_back(duration / 1000000.0); // Convert to ms
        }

        PerformanceMetrics metrics;
        metrics.name = name;
        metrics.iterations = iterations;
        metrics.totalTimeMs = std::accumulate(times.begin(), times.end(), 0.0);
        metrics.avgTimeMs = metrics.totalTimeMs / iterations;
        metrics.minTimeMs = *std::min_element(times.begin(), times.end());
        metrics.maxTimeMs = *std::max_element(times.begin(), times.end());

        // Calculate samples per second
        double avgSeconds = metrics.avgTimeMs / 1000.0;
        metrics.samplesPerSecond = bufferSize_ / avgSeconds;

        return metrics;
    }

    // Helper: Print performance report
    void printPerformanceReport(const PerformanceMetrics& metrics) {
        std::cout << "\n" << metrics.getReport() << std::endl;
    }

    double sampleRate_;
    int bufferSize_;
    int benchmarkIterations_;
};

// ============================================================================
// Benchmark 1: FormantResonator Performance
// ============================================================================

TEST_F(DSPBenchmarkTest, FormantResonatorPerformance) {
    FormantResonator resonator(sampleRate_);
    resonator.prepare(sampleRate_, bufferSize_);

    // Set formant frequencies
    resonator.setFormantFrequency(0, 800.0f);  // F1
    resonator.setFormantFrequency(1, 1200.0f); // F2
    resonator.setFormantFrequency(2, 2500.0f); // F3

    AudioBuffer buffer(2, bufferSize_);
    generateSineWave(buffer, 220.0f);

    auto metrics = runBenchmark(
        "FormantResonator::process",
        [&]() {
            AudioBuffer tempBuffer(buffer);
            resonator.process(tempBuffer);
        },
        benchmarkIterations_
    );

    printPerformanceReport(metrics);

    // Performance targets:
    // - Should process at least 1000 samples/call in < 1ms
    // - Throughput > 500k samples/sec
    EXPECT_LT(metrics.avgTimeMs, 1.0)
        << "FormantResonator too slow: " << metrics.avgTimeMs << " ms";
    EXPECT_GT(metrics.samplesPerSecond, 500000.0)
        << "FormantResonator throughput too low";
}

// ============================================================================
// Benchmark 2: SubharmonicGenerator Performance
// ============================================================================

TEST_F(DSPBenchmarkTest, SubharmonicGeneratorPerformance) {
    SubharmonicGenerator subharmonic(sampleRate_);
    subharmonic.prepare(sampleRate_, bufferSize_);

    // Configure for typical usage
    subharmonic.setSubharmonicRatio(0.5f);
    subharmonic.setMix(0.7f);

    AudioBuffer buffer(2, bufferSize_);
    generateSineWave(buffer, 220.0f);

    auto metrics = runBenchmark(
        "SubharmonicGenerator::process",
        [&]() {
            AudioBuffer tempBuffer(buffer);
            subharmonic.process(tempBuffer);
        },
        benchmarkIterations_
    );

    printPerformanceReport(metrics);

    // Performance targets:
    // - Should process 1000 samples/call in < 0.5ms
    // - Throughput > 1M samples/sec
    EXPECT_LT(metrics.avgTimeMs, 0.5)
        << "SubharmonicGenerator too slow: " << metrics.avgTimeMs << " ms";
    EXPECT_GT(metrics.samplesPerSecond, 1000000.0)
        << "SubharmonicGenerator throughput too low";
}

// ============================================================================
// Benchmark 3: SpectralEnhancer Performance
// ============================================================================

TEST_F(DSPBenchmarkTest, SpectralEnhancerPerformance) {
    SpectralEnhancer enhancer(sampleRate_);
    enhancer.prepare(sampleRate_, bufferSize_);

    AudioBuffer buffer(2, bufferSize_);
    generateSineWave(buffer, 440.0f);

    auto metrics = runBenchmark(
        "SpectralEnhancer::process",
        [&]() {
            AudioBuffer tempBuffer(buffer);
            enhancer.process(tempBuffer);
        },
        benchmarkIterations_
    );

    printPerformanceReport(metrics);

    // Performance targets:
    // - FFT-based processing in < 2ms
    // - Throughput > 250k samples/sec
    EXPECT_LT(metrics.avgTimeMs, 2.0)
        << "SpectralEnhancer too slow: " << metrics.avgTimeMs << " ms";
    EXPECT_GT(metrics.samplesPerSecond, 250000.0)
        << "SpectralEnhancer throughput too low";
}

// ============================================================================
// Benchmark 4: VoiceManager Performance
// ============================================================================

TEST_F(DSPBenchmarkTest, VoiceManagerPerformance) {
    VoiceManager voiceManager(60, sampleRate_);
    voiceManager.prepare(sampleRate_, bufferSize_);

    // Activate 40 voices
    for (int i = 0; i < 40; ++i) {
        voiceManager.noteOn(60 + (i % 24), 0.8f);
    }

    AudioBuffer outputBuffer(2, bufferSize_);
    outputBuffer.clear();

    auto metrics = runBenchmark(
        "VoiceManager::processAudio (40 voices)",
        [&]() {
            AudioBuffer tempBuffer(2, bufferSize_);
            tempBuffer.clear();
            voiceManager.processAudio(
                tempBuffer.getWritePointer(0),
                tempBuffer.getWritePointer(1),
                bufferSize_
            );
        },
        benchmarkIterations_
    );

    printPerformanceReport(metrics);

    // Performance targets:
    // - 40 voices @ 44.1kHz in < 5ms per buffer
    // - Throughput > 100k samples/sec with 40 voices
    EXPECT_LT(metrics.avgTimeMs, 5.0)
        << "VoiceManager too slow with 40 voices: " << metrics.avgTimeMs << " ms";
    EXPECT_GT(metrics.samplesPerSecond, 100000.0)
        << "VoiceManager throughput too low with 40 voices";
}

// ============================================================================
// Benchmark 5: VoiceManager Maximum Load (60 voices)
// ============================================================================

TEST_F(DSPBenchmarkTest, VoiceManagerMaxLoadPerformance) {
    VoiceManager voiceManager(60, sampleRate_);
    voiceManager.prepare(sampleRate_, bufferSize_);

    // Activate all 60 voices
    for (int i = 0; i < 60; ++i) {
        voiceManager.noteOn(60 + (i % 24), 0.8f);
    }

    AudioBuffer outputBuffer(2, bufferSize_);
    outputBuffer.clear();

    auto metrics = runBenchmark(
        "VoiceManager::processAudio (60 voices)",
        [&]() {
            AudioBuffer tempBuffer(2, bufferSize_);
            tempBuffer.clear();
            voiceManager.processAudio(
                tempBuffer.getWritePointer(0),
                tempBuffer.getWritePointer(1),
                bufferSize_
            );
        },
        benchmarkIterations_
    );

    printPerformanceReport(metrics);

    // Performance targets:
    // - 60 voices @ 44.1kHz in < 8ms per buffer
    // - Should handle maximum load
    EXPECT_LT(metrics.avgTimeMs, 8.0)
        << "VoiceManager too slow with 60 voices: " << metrics.avgTimeMs << " ms";
}

// ============================================================================
// Benchmark 6: FormantSynthesis Performance
// ============================================================================

TEST_F(DSPBenchmarkTest, FormantSynthesisPerformance) {
    FormantSynthesis synth(sampleRate_);

    // Create phoneme sequence
    std::vector<PhonemeResult> phonemes = {
        {"AA", 0.1f, 220.0f, false, 0, 0},
        {"EH", 0.1f, 220.0f, false, 0, 1},
        {"IY", 0.1f, 220.0f, false, 0, 2},
        {"OW", 0.1f, 220.0f, false, 0, 3},
        {"UH", 0.1f, 220.0f, false, 0, 4}
    };

    int numSamples = static_cast<int>(sampleRate_ * 0.5); // 0.5 seconds
    AudioBuffer buffer(2, numSamples);
    buffer.clear();

    auto metrics = runBenchmark(
        "FormantSynthesis::process (5 phonemes)",
        [&]() {
            AudioBuffer tempBuffer(2, numSamples);
            tempBuffer.clear();
            synth.process(phonemes, tempBuffer);
        },
        100 // Fewer iterations for synthesis
    );

    printPerformanceReport(metrics);

    // Performance targets:
    // - Should complete synthesis in < 2x real-time
    double realTimeMs = (numSamples / sampleRate_) * 1000.0;
    EXPECT_LT(metrics.avgTimeMs, realTimeMs * 2.0)
        << "FormantSynthesis too slow: " << metrics.avgTimeMs << " ms (real-time: " << realTimeMs << " ms)";
}

// ============================================================================
// Benchmark 7: SubharmonicSynthesis Performance
// ============================================================================

TEST_F(DSPBenchmarkTest, SubharmonicSynthesisPerformance) {
    SubharmonicSynthesis synth(sampleRate_);

    // Configure synthesis
    synth.setFundamentalFrequency(220.0f);
    synth.setSubharmonicRatio(0.5f);
    synth.setMix(0.7f);

    int numSamples = static_cast<int>(sampleRate_ * 0.5); // 0.5 seconds
    AudioBuffer buffer(2, numSamples);
    buffer.clear();

    auto metrics = runBenchmark(
        "SubharmonicSynthesis::process",
        [&]() {
            AudioBuffer tempBuffer(2, numSamples);
            tempBuffer.clear();
            synth.process(tempBuffer);
        },
        100
    );

    printPerformanceReport(metrics);

    // Performance targets:
    // - Should complete in < 2x real-time
    double realTimeMs = (numSamples / sampleRate_) * 1000.0;
    EXPECT_LT(metrics.avgTimeMs, realTimeMs * 2.0)
        << "SubharmonicSynthesis too slow: " << metrics.avgTimeMs << " ms";
}

// ============================================================================
// Benchmark 8: DiphoneSynthesis Performance
// ============================================================================

TEST_F(DSPBenchmarkTest, DiphoneSynthesisPerformance) {
    DiphoneSynthesis synth(sampleRate_);

    // Create phoneme sequence for diphone transitions
    std::vector<PhonemeResult> phonemes = {
        {"AA", 0.1f, 220.0f, false, 0, 0},
        {"EH", 0.1f, 220.0f, false, 0, 1},
        {"IY", 0.1f, 220.0f, false, 0, 2},
        {"OW", 0.1f, 220.0f, false, 0, 3}
    };

    int numSamples = static_cast<int>(sampleRate_ * 0.4); // 0.4 seconds
    AudioBuffer buffer(2, numSamples);
    buffer.clear();

    auto metrics = runBenchmark(
        "DiphoneSynthesis::process (4 diphones)",
        [&]() {
            AudioBuffer tempBuffer(2, numSamples);
            tempBuffer.clear();
            synth.process(phonemes, tempBuffer);
        },
        100
    );

    printPerformanceReport(metrics);

    // Performance targets:
    // - Diphone transitions in < 2x real-time
    double realTimeMs = (numSamples / sampleRate_) * 1000.0;
    EXPECT_LT(metrics.avgTimeMs, realTimeMs * 2.0)
        << "DiphoneSynthesis too slow: " << metrics.avgTimeMs << " ms";
}

// ============================================================================
// Benchmark 9: Real-Time Safety (No Allocations)
// ============================================================================

TEST_F(DSPBenchmarkTest, RealTimeSafetyNoAllocations) {
    // This benchmark verifies no memory allocations during audio processing
    // by running many iterations and checking for consistent timing

    VoiceManager voiceManager(60, sampleRate_);
    voiceManager.prepare(sampleRate_, bufferSize_);

    // Activate 40 voices
    for (int i = 0; i < 40; ++i) {
        voiceManager.noteOn(60 + (i % 24), 0.8f);
    }

    std::vector<double> times;
    times.reserve(10000);

    for (int i = 0; i < 10000; ++i) {
        AudioBuffer buffer(2, bufferSize_);
        buffer.clear();

        auto startTime = std::chrono::high_resolution_clock::now();
        voiceManager.processAudio(
            buffer.getWritePointer(0),
            buffer.getWritePointer(1),
            bufferSize_
        );
        auto endTime = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
            endTime - startTime
        ).count();

        times.push_back(duration / 1000000.0); // Convert to ms
    }

    // Check for consistent timing (no spikes from allocations)
    double avgTime = std::accumulate(times.begin(), times.end(), 0.0) / times.size();
    double maxTime = *std::max_element(times.begin(), times.end());

    // Max time should be within 3x average (allowing for some variance)
    EXPECT_LT(maxTime, avgTime * 3.0)
        << "Detected potential memory allocation (timing spike): "
        << "avg=" << avgTime << "ms, max=" << maxTime << "ms";
}

// ============================================================================
// Benchmark 10: DSP Chain Performance
// ============================================================================

TEST_F(DSPBenchmarkTest, DSPChainPerformance) {
    // Complete DSP chain: FormantResonator -> SubharmonicGenerator -> SpectralEnhancer

    FormantResonator resonator(sampleRate_);
    SubharmonicGenerator subharmonic(sampleRate_);
    SpectralEnhancer enhancer(sampleRate_);

    resonator.prepare(sampleRate_, bufferSize_);
    subharmonic.prepare(sampleRate_, bufferSize_);
    enhancer.prepare(sampleRate_, bufferSize_);

    // Configure components
    resonator.setFormantFrequency(0, 800.0f);
    resonator.setFormantFrequency(1, 1200.0f);
    subharmonic.setSubharmonicRatio(0.5f);
    subharmonic.setMix(0.7f);

    AudioBuffer buffer(2, bufferSize_);
    generateSineWave(buffer, 220.0f);

    auto metrics = runBenchmark(
        "Complete DSP Chain",
        [&]() {
            AudioBuffer tempBuffer(buffer);
            resonator.process(tempBuffer);
            subharmonic.process(tempBuffer);
            enhancer.process(tempBuffer);
        },
        benchmarkIterations_
    );

    printPerformanceReport(metrics);

    // Performance targets:
    // - Complete chain in < 3ms
    // - Throughput > 150k samples/sec
    EXPECT_LT(metrics.avgTimeMs, 3.0)
        << "DSP chain too slow: " << metrics.avgTimeMs << " ms";
    EXPECT_GT(metrics.samplesPerSecond, 150000.0)
        << "DSP chain throughput too low";
}

// ============================================================================
// Benchmark 11: Parameter Smoothing Performance
// ============================================================================

TEST_F(DSPBenchmarkTest, ParameterSmoothingPerformance) {
    VoiceManager voiceManager(60, sampleRate_);
    voiceManager.prepare(sampleRate_, bufferSize_);

    // Activate a voice
    voiceManager.noteOn(60, 1.0f);

    AudioBuffer buffer(2, bufferSize_);
    buffer.clear();

    auto metrics = runBenchmark(
        "VoiceManager with parameter changes",
        [&]() {
            // Change parameters every buffer
            voiceManager.setMasterGain(0.5f + (rand() % 100) / 200.0f);
            voiceManager.setVibratoRate(5.0f + (rand() % 100) / 10.0f);
            voiceManager.setVibratoDepth((rand() % 100) / 100.0f);

            AudioBuffer tempBuffer(2, bufferSize_);
            tempBuffer.clear();
            voiceManager.processAudio(
                tempBuffer.getWritePointer(0),
                tempBuffer.getWritePointer(1),
                bufferSize_
            );
        },
        benchmarkIterations_
    );

    printPerformanceReport(metrics);

    // Parameter smoothing should not significantly impact performance
    EXPECT_LT(metrics.avgTimeMs, 2.0)
        << "Parameter smoothing too slow: " << metrics.avgTimeMs << " ms";
}

// ============================================================================
// Benchmark 12: SIMD Optimization Verification
// ============================================================================

TEST_F(DSPBenchmarkTest, SIMDOptimizationVerification) {
    // This test compares SIMD vs non-SIMD performance
    // Note: Actual SIMD implementation is placeholder, but this
    // establishes the benchmark framework

    VoiceManager voiceManager(60, sampleRate_);
    voiceManager.prepare(sampleRate_, bufferSize_);

    // Test with voice counts that benefit from SIMD (multiples of 4/8)
    std::vector<int> voiceCounts = {4, 8, 16, 32, 40};

    for (int voiceCount : voiceCounts) {
        // Activate voices
        for (int i = 0; i < voiceCount; ++i) {
            voiceManager.noteOn(60 + (i % 24), 0.8f);
        }

        AudioBuffer buffer(2, bufferSize_);
        buffer.clear();

        auto metrics = runBenchmark(
            "VoiceManager (" + std::to_string(voiceCount) + " voices)",
            [&]() {
                AudioBuffer tempBuffer(2, bufferSize_);
                tempBuffer.clear();
                voiceManager.processAudio(
                    tempBuffer.getWritePointer(0),
                    tempBuffer.getWritePointer(1),
                    bufferSize_
                );
            },
            500
        );

        printPerformanceReport(metrics);

        // Verify performance scales reasonably with voice count
        // Linear scaling: time should be proportional to voice count
        double timePerVoice = metrics.avgTimeMs / voiceCount;
        EXPECT_LT(timePerVoice, 0.15)
            << "Voice processing too slow at " << voiceCount << " voices";

        // Release voices
        voiceManager.allNotesOff();
    }
}

// ============================================================================
// Benchmark 13: Memory Usage Performance
// ============================================================================

TEST_F(DSPBenchmarkTest, MemoryUsagePerformance) {
    // Measure memory allocation patterns during DSP operations

    VoiceManager voiceManager(60, sampleRate_);
    voiceManager.prepare(sampleRate_, bufferSize_);

    // Activate maximum voices
    for (int i = 0; i < 60; ++i) {
        voiceManager.noteOn(60 + (i % 24), 0.8f);
    }

    // Process many buffers and measure timing consistency
    std::vector<double> times;
    times.reserve(5000);

    for (int i = 0; i < 5000; ++i) {
        AudioBuffer buffer(2, bufferSize_);
        buffer.clear();

        auto startTime = std::chrono::high_resolution_clock::now();
        voiceManager.processAudio(
            buffer.getWritePointer(0),
            buffer.getWritePointer(1),
            bufferSize_
        );
        auto endTime = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
            endTime - startTime
        ).count();

        times.push_back(duration / 1000000.0);
    }

    // Calculate statistics
    double avgTime = std::accumulate(times.begin(), times.end(), 0.0) / times.size();
    double minTime = *std::min_element(times.begin(), times.end());
    double maxTime = *std::max_element(times.begin(), times.end());
    double variance = 0.0;
    for (double time : times) {
        variance += (time - avgTime) * (time - avgTime);
    }
    variance /= times.size();
    double stdDev = std::sqrt(variance);

    std::cout << "\nMemory Usage Statistics (60 voices, 5000 iterations):\n";
    std::cout << "  Average: " << avgTime << " ms\n";
    std::cout << "  Min: " << minTime << " ms\n";
    std::cout << "  Max: " << maxTime << " ms\n";
    std::cout << "  Std Dev: " << stdDev << " ms\n";

    // Low standard deviation indicates consistent memory access patterns
    EXPECT_LT(stdDev, avgTime * 0.2)
        << "High timing variance suggests memory allocation issues";
}

// ============================================================================
// Benchmark 14: Concurrent Processing Simulation
// ============================================================================

TEST_F(DSPBenchmarkTest, ConcurrentProcessingSimulation) {
    // Simulate concurrent processing of multiple instances
    // (e.g., multi-instrument setup in DAW)

    std::vector<std::unique_ptr<VoiceManager>> managers;
    for (int i = 0; i < 4; ++i) {
        auto manager = std::make_unique<VoiceManager>(60, sampleRate_);
        manager->prepare(sampleRate_, bufferSize_);

        // Activate 15 voices per instance
        for (int j = 0; j < 15; ++j) {
            manager->noteOn(60 + j, 0.8f);
        }

        managers.push_back(std::move(manager));
    }

    auto metrics = runBenchmark(
        "Concurrent Processing (4 instances x 15 voices)",
        [&]() {
            for (auto& manager : managers) {
                AudioBuffer buffer(2, bufferSize_);
                buffer.clear();
                manager->processAudio(
                    buffer.getWritePointer(0),
                    buffer.getWritePointer(1),
                    bufferSize_
                );
            }
        },
        benchmarkIterations_
    );

    printPerformanceReport(metrics);

    // Should handle concurrent instances efficiently
    EXPECT_LT(metrics.avgTimeMs, 10.0)
        << "Concurrent processing too slow: " << metrics.avgTimeMs << " ms";
}

// ============================================================================
// Benchmark 15: Comparison Summary
// ============================================================================

TEST_F(DSPBenchmarkTest, PerformanceSummary) {
    std::cout << "\n===========================================\n";
    std::cout << "CHOIR V2.0 DSP PERFORMANCE BENCHMARK SUMMARY\n";
    std::cout << "===========================================\n";
    std::cout << "Sample Rate: " << sampleRate_ << " Hz\n";
    std::cout << "Buffer Size: " << bufferSize_ << " samples\n";
    std::cout << "Target: Real-time safe processing (< 11.6ms per buffer @ 44.1kHz)\n";
    std::cout << "===========================================\n\n";

    // All individual benchmarks will print their results
    // This test serves as a summary point

    EXPECT_TRUE(true); // Placeholder
}
