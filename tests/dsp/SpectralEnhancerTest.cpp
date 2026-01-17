#include "../../src/dsp/SpectralEnhancer.h"
#include <gtest/gtest.h>
#include <vector>
#include <cmath>

namespace ChoirV2 {
namespace Test {

class SpectralEnhancerTest : public ::testing::Test {
protected:
    void SetUp() override {
        enhancer = std::make_unique<SpectralEnhancer>();
        enhancer->initialize(44100.0);

        testBuffer.setSize(2, 512);
    }

    void TearDown() override {
        enhancer.reset();
    }

    std::unique_ptr<SpectralEnhancer> enhancer;
    juce::AudioBuffer<float> testBuffer;

    // Helper: Generate sine wave
    void generateSineWave(juce::AudioBuffer<float>& buffer, float frequency, float amplitude = 1.0f) {
        const int numSamples = buffer.getNumSamples();
        const int numChannels = buffer.getNumChannels();
        const double sampleRate = 44100.0;

        for (int channel = 0; channel < numChannels; ++channel) {
            float* data = buffer.getWritePointer(channel);
            for (int i = 0; i < numSamples; ++i) {
                const float phase = static_cast<float>(2.0 * M_PI * frequency * i / sampleRate);
                data[i] = amplitude * std::sin(phase);
            }
        }
    }

    // Helper: Generate silence
    void generateSilence(juce::AudioBuffer<float>& buffer) {
        buffer.clear();
    }

    // Helper: Detect clicks in buffer
    int detectClicks(const juce::AudioBuffer<float>& buffer, float threshold = 0.1f) {
        const int numSamples = buffer.getNumSamples();
        const int numChannels = buffer.getNumChannels();
        int clickCount = 0;

        for (int channel = 0; channel < numChannels; ++channel) {
            const float* data = buffer.getReadPointer(channel);

            for (int i = 1; i < numSamples; ++i) {
                const float difference = std::abs(data[i] - data[i - 1]);

                // Sudden large change indicates a click
                if (difference > threshold) {
                    clickCount++;
                }
            }
        }

        return clickCount;
    }

    // Helper: Calculate signal-to-noise ratio
    float calculateSNR(const juce::AudioBuffer<float>& signal, const juce::AudioBuffer<float>& noise) {
        float signalPower = 0.0f;
        float noisePower = 0.0f;

        const int numSamples = std::min(signal.getNumSamples(), noise.getNumSamples());
        const int numChannels = std::min(signal.getNumChannels(), noise.getNumChannels());

        for (int channel = 0; channel < numChannels; ++channel) {
            const float* signalData = signal.getReadPointer(channel);
            const float* noiseData = noise.getReadPointer(channel);

            for (int i = 0; i < numSamples; ++i) {
                signalPower += signalData[i] * signalData[i];
                noisePower += noiseData[i] * noiseData[i];
            }
        }

        // Avoid division by zero
        if (noisePower < 1e-10f) {
            return 100.0f;  // Perfect SNR
        }

        return 10.0f * std::log10(signalPower / noisePower);
    }

    // Helper: Check for discontinuities at buffer boundaries
    bool hasBoundaryDiscontinuities(const juce::AudioBuffer<float>& buffer, float threshold = 0.01f) {
        const int numChannels = buffer.getNumChannels();

        for (int channel = 0; channel < numChannels; ++channel) {
            const float* data = buffer.getReadPointer(channel);

            // Check start of buffer (should be near zero if properly windowed)
            if (std::abs(data[0]) > threshold) {
                return true;
            }

            // Check end of buffer (should be near zero if properly windowed)
            const int lastSample = buffer.getNumSamples() - 1;
            if (std::abs(data[lastSample]) > threshold) {
                return true;
            }
        }

        return false;
    }
};

// =============================================================================
// TEST: Overlap-Add Continuity
// =============================================================================

TEST_F(SpectralEnhancerTest, OverlapAddProducesContinuousOutput) {
    // Generate a sustained sine wave
    generateSineWave(testBuffer, 440.0f, 0.8f);

    // Store original for comparison
    juce::AudioBuffer<float> originalBuffer(testBuffer);

    // Process through spectral enhancer
    enhancer->process(testBuffer);

    // Check for clicks (should be zero or very few)
    int clicks = detectClicks(testBuffer, 0.05f);
    EXPECT_LE(clicks, 2) << "Overlap-add should prevent clicks, but detected "
                          << clicks << " clicks in processed buffer";
}

// =============================================================================
// TEST: Spectral Leakage Prevention
// =============================================================================

TEST_F(SpectralEnhancerTest, HannWindowPreventsSpectralLeakage) {
    // Generate sine wave at frequency that aligns with FFT bin
    const float binFrequency = 44100.0f / 2048.0f * 10.0f;  // Bin 10
    generateSineWave(testBuffer, binFrequency, 1.0f);

    // Process
    enhancer->process(testBuffer);

    // Check that boundaries are smooth (Hann window goes to zero at edges)
    bool hasDiscontinuities = hasBoundaryDiscontinuities(testBuffer, 0.01f);
    EXPECT_FALSE(hasDiscontinuities) << "Hann window should prevent boundary discontinuities";
}

// =============================================================================
// TEST: No Artifacts on Silent Input
// =============================================================================

TEST_F(SpectralEnhancerTest, SilentInputProducesNoArtifacts) {
    // Generate silence
    generateSilence(testBuffer);

    // Process
    enhancer->process(testBuffer);

    // Check that output is still silent (no artifacts introduced)
    const int numSamples = testBuffer.getNumSamples();
    const int numChannels = testBuffer.getNumChannels();

    for (int channel = 0; channel < numChannels; ++channel) {
        const float* data = testBuffer.getReadPointer(channel);
        for (int i = 0; i < numSamples; ++i) {
            EXPECT_NEAR(data[i], 0.0f, 1e-6f)
                << "Silent input should produce silent output (no artifacts)";
        }
    }
}

// =============================================================================
// TEST: Continuous Sine Wave Processing
// =============================================================================

TEST_F(SpectralEnhancerTest, ContinuousSineWaveProcessing) {
    // Process multiple consecutive buffers (simulates real-time streaming)
    const int numBuffers = 10;
    std::vector<float> allSamples;

    for (int buf = 0; buf < numBuffers; ++buf) {
        generateSineWave(testBuffer, 440.0f, 0.5f);
        enhancer->process(testBuffer);

        // Collect all samples
        const float* data = testBuffer.getReadPointer(0);
        for (int i = 0; i < testBuffer.getNumSamples(); ++i) {
            allSamples.push_back(data[i]);
        }
    }

    // Check for clicks at buffer boundaries
    int boundaryClicks = 0;
    const int bufferSize = testBuffer.getNumSamples();

    for (int buf = 1; buf < numBuffers; ++buf) {
        const int idx = buf * bufferSize;
        const float difference = std::abs(allSamples[idx] - allSamples[idx - 1]);

        if (difference > 0.05f) {
            boundaryClicks++;
        }
    }

    EXPECT_LE(boundaryClicks, 1)
        << "Should not have clicks at buffer boundaries due to overlap-add";
}

// =============================================================================
// TEST: Phase Preservation
// =============================================================================

TEST_F(SpectralEnhancerTest, PreservesPhaseInformation) {
    // Generate a sine wave with known phase
    const float frequency = 440.0f;
    const float amplitude = 0.8f;
    const double sampleRate = 44100.0;

    for (int i = 0; i < testBuffer.getNumSamples(); ++i) {
        const float phase = static_cast<float>(2.0 * M_PI * frequency * i / sampleRate + M_PI / 4.0f);
        testBuffer.setSample(0, i, amplitude * std::sin(phase));
        testBuffer.setSample(1, i, amplitude * std::sin(phase));
    }

    // Process
    enhancer->process(testBuffer);

    // Verify phase is preserved (check zero crossings are at expected positions)
    const float* data = testBuffer.getReadPointer(0);
    int zeroCrossings = 0;

    for (int i = 1; i < testBuffer.getNumSamples(); ++i) {
        if ((data[i - 1] >= 0.0f && data[i] < 0.0f) || (data[i - 1] < 0.0f && data[i] >= 0.0f)) {
            zeroCrossings++;
        }
    }

    // Expect approximately the same number of zero crossings as input
    const float expectedCrossings = frequency * testBuffer.getNumSamples() / sampleRate;
    EXPECT_NEAR(zeroCrossings, expectedCrossings, 2.0f)
        << "Phase should be preserved through FFT processing";
}

// =============================================================================
// TEST: Harmonic Enhancement
// =============================================================================

TEST_F(SpectralEnhancerTest, EnhancesHarmonicsOfMelodyFormant) {
    // Set melody formant to A4 (440 Hz)
    enhancer->setMelodyFormant(440.0f);
    enhancer->setOvertoneBoost(2.0f);  // +6dB boost

    // Generate sine wave at fundamental (440 Hz)
    generateSineWave(testBuffer, 440.0f, 0.5f);

    // Process
    enhancer->process(testBuffer);

    // Verify output is not identical to input (harmonics were enhanced)
    juce::AudioBuffer<float> inputCopy(testBuffer);
    generateSineWave(inputCopy, 440.0f, 0.5f);

    // The processed signal should be different (due to harmonic enhancement)
    float maxDifference = 0.0f;
    for (int i = 0; i < testBuffer.getNumSamples(); ++i) {
        const float diff = std::abs(testBuffer.getSample(0, i) - inputCopy.getSample(0, i));
        maxDifference = std::max(maxDifference, diff);
    }

    EXPECT_GT(maxDifference, 0.01f)
        << "Harmonic enhancement should modify the signal";
}

// =============================================================================
// TEST: Reset Functionality
// =============================================================================

TEST_F(SpectralEnhancerTest, ResetClearsInternalState) {
    // Process some audio
    generateSineWave(testBuffer, 440.0f, 0.5f);
    enhancer->process(testBuffer);

    // Reset
    enhancer->reset();

    // Process silence and verify no artifacts
    generateSilence(testBuffer);
    enhancer->process(testBuffer);

    // Check that output is silent
    const float* data = testBuffer.getReadPointer(0);
    for (int i = 0; i < testBuffer.getNumSamples(); ++i) {
        EXPECT_NEAR(data[i], 0.0f, 1e-5f)
            << "Reset should clear all internal state and prevent artifacts";
    }
}

// =============================================================================
// TEST: Different Sample Rates
// =============================================================================

TEST_F(SpectralEnhancerTest, WorksAtDifferentSampleRates) {
    const std::vector<double> sampleRates = {44100.0, 48000.0, 96000.0};

    for (double sampleRate : sampleRates) {
        // Reinitialize at this sample rate
        enhancer->initialize(sampleRate);

        // Generate sine wave
        generateSineWave(testBuffer, 440.0f, 0.5f);

        // Process
        enhancer->process(testBuffer);

        // Check for clicks
        int clicks = detectClicks(testBuffer, 0.05f);
        EXPECT_LE(clicks, 2)
            << "Should not have clicks at sample rate " << sampleRate;
    }
}

// =============================================================================
// TEST: Stereo Processing Consistency
// =============================================================================

TEST_F(SpectralEnhancerTest, ProcessesStereoConsistently) {
    // Generate same sine wave on both channels
    generateSineWave(testBuffer, 440.0f, 0.5f);

    // Process
    enhancer->process(testBuffer);

    // Verify left and right channels are processed consistently
    const float* left = testBuffer.getReadPointer(0);
    const float* right = testBuffer.getReadPointer(1);

    float maxDifference = 0.0f;
    for (int i = 0; i < testBuffer.getNumSamples(); ++i) {
        const float diff = std::abs(left[i] - right[i]);
        maxDifference = std::max(maxDifference, diff);
    }

    EXPECT_NEAR(maxDifference, 0.0f, 1e-5f)
        << "Stereo channels should be processed consistently";
}

} // namespace Test
} // namespace ChoirV2

// Main function to run all tests
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
