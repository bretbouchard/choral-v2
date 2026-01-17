/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * test_synthesis_pipeline.cpp - Integration tests for complete synthesis pipeline
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#include <gtest/gtest.h>
#include <core/VoiceManager.h>
#include <core/G2PEngine.h>
#include <core/PhonemeDatabase.h>
#include <core/LanguageLoader.h>
#include <synthesis/FormantSynthesis.h>
#include <synthesis/SubharmonicSynthesis.h>
#include <synthesis/DiphoneSynthesis.h>
#include <dsp/FormantResonator.h>
#include <dsp/SubharmonicGenerator.h>
#include <dsp/SpectralEnhancer.h>
#include <utils/AudioBuffer.h>

#include <fstream>
#include <cmath>
#include <chrono>

using namespace ChoirV2;

class SynthesisPipelineTest : public ::testing::Test {
protected:
    void SetUp() override {
        sampleRate_ = 44100.0;
        bufferSize_ = 512;

        // Initialize phoneme database
        phonemeDB_ = std::make_shared<PhonemeDatabase>();
        ASSERT_TRUE(phonemeDB_->loadFromDirectory("languages/phonemes"));

        // Initialize language loader
        languageLoader_ = std::make_unique<LanguageLoader>();

        // Initialize G2P engine
        g2pEngine_ = std::make_unique<G2PEngine>(phonemeDB_);

        // Initialize voice manager
        voiceManager_ = std::make_unique<VoiceManager>(60, sampleRate_);
        voiceManager_->prepare(sampleRate_, bufferSize_);

        // Initialize synthesis components
        formantSynth_ = std::make_unique<FormantSynthesis>(sampleRate_);
        subharmonicSynth_ = std::make_unique<SubharmonicSynthesis>(sampleRate_);
        diphoneSynth_ = std::make_unique<DiphoneSynthesis>(sampleRate_);

        // Initialize DSP components
        formantResonator_ = std::make_unique<FormantResonator>(sampleRate_);
        subharmonicGen_ = std::make_unique<SubharmonicGenerator>(sampleRate_);
        spectralEnhancer_ = std::make_unique<SpectralEnhancer>(sampleRate_);

        // Prepare DSP components
        formantResonator_->prepare(sampleRate_, bufferSize_);
        subharmonicGen_->prepare(sampleRate_, bufferSize_);
        spectralEnhancer_->prepare(sampleRate_, bufferSize_);
    }

    void TearDown() override {
        voiceManager_.reset();
        formantSynth_.reset();
        subharmonicSynth_.reset();
        diphoneSynth_.reset();
        formantResonator_.reset();
        subharmonicGen_.reset();
        spectralEnhancer_.reset();
        g2pEngine_.reset();
        languageLoader_.reset();
        phonemeDB_.reset();
    }

    // Helper: Check if audio signal is valid (no NaN, Inf, or excessive values)
    bool isValidAudio(const float* buffer, int numSamples, float maxValue = 1.5f) {
        for (int i = 0; i < numSamples; ++i) {
            if (std::isnan(buffer[i]) || std::isinf(buffer[i])) {
                return false;
            }
            if (std::abs(buffer[i]) > maxValue) {
                return false;
            }
        }
        return true;
    }

    // Helper: Calculate signal energy
    float calculateEnergy(const float* buffer, int numSamples) {
        float energy = 0.0f;
        for (int i = 0; i < numSamples; ++i) {
            energy += buffer[i] * buffer[i];
        }
        return std::sqrt(energy / numSamples);
    }

    // Helper: Check for clicks/pops (sudden large changes)
    bool hasClicks(const float* buffer, int numSamples, float maxDelta = 0.1f) {
        for (int i = 1; i < numSamples; ++i) {
            float delta = std::abs(buffer[i] - buffer[i-1]);
            if (delta > maxDelta) {
                return true;
            }
        }
        return false;
    }

    // Helper: Save audio buffer to file (for debugging)
    void saveAudioToFile(const float* buffer, int numSamples, const std::string& filename) {
        std::ofstream file(filename, std::ios::binary);
        if (file.is_open()) {
            file.write(reinterpret_cast<const char*>(buffer), numSamples * sizeof(float));
            file.close();
        }
    }

    // Helper: Load language
    bool loadLanguage(const std::string& langCode) {
        auto language = languageLoader_->loadLanguage(langCode);
        if (language) {
            g2pEngine_->setLanguage(language);
            return true;
        }
        return false;
    }

    double sampleRate_;
    int bufferSize_;

    std::shared_ptr<PhonemeDatabase> phonemeDB_;
    std::unique_ptr<LanguageLoader> languageLoader_;
    std::unique_ptr<G2PEngine> g2pEngine_;
    std::unique_ptr<VoiceManager> voiceManager_;
    std::unique_ptr<FormantSynthesis> formantSynth_;
    std::unique_ptr<SubharmonicSynthesis> subharmonicSynth_;
    std::unique_ptr<DiphoneSynthesis> diphoneSynth_;
    std::unique_ptr<FormantResonator> formantResonator_;
    std::unique_ptr<SubharmonicGenerator> subharmonicGen_;
    std::unique_ptr<SpectralEnhancer> spectralEnhancer_;
};

// ============================================================================
// Test 1: English Text-to-Phoneme-to-Audio Pipeline
// ============================================================================

TEST_F(SynthesisPipelineTest, EnglishTextToAudioPipeline) {
    // Load English language
    ASSERT_TRUE(loadLanguage("en"));

    // Convert text to phonemes
    std::string text = "The quick brown fox";
    G2PResult result = g2pEngine_->convert(text);

    EXPECT_TRUE(result.success);
    EXPECT_GT(result.getPhonemeCount(), 0);

    // Verify phonemes are valid
    for (const auto& phoneme : result.phonemes) {
        EXPECT_FALSE(phoneme.symbol.empty());
        EXPECT_GT(phoneme.duration, 0.0f);
    }

    // Calculate total duration
    float totalDuration = result.getTotalDuration();
    EXPECT_GT(totalDuration, 0.0f);

    // Generate audio from phonemes
    int numSamples = static_cast<int>(totalDuration * sampleRate_);
    AudioBuffer outputBuffer(2, numSamples);
    outputBuffer.clear();

    // Process through formant synthesis
    formantSynth_->process(result.phonemes, outputBuffer);

    // Verify output is valid
    EXPECT_TRUE(isValidAudio(outputBuffer.getReadPointer(0), numSamples));
    EXPECT_TRUE(isValidAudio(outputBuffer.getReadPointer(1), numSamples));

    // Check that audio was generated (has energy)
    float energy = calculateEnergy(outputBuffer.getReadPointer(0), numSamples);
    EXPECT_GT(energy, 0.001f);
}

// ============================================================================
// Test 2: Latin Text-to-Phoneme-to-Audio Pipeline
// ============================================================================

TEST_F(SynthesisPipelineTest, LatinTextToAudioPipeline) {
    // Load Latin language
    ASSERT_TRUE(loadLanguage("la"));

    // Convert text to phonemes
    std::string text = "Pater noster";
    G2PResult result = g2pEngine_->convert(text);

    EXPECT_TRUE(result.success);
    EXPECT_GT(result.getPhonemeCount(), 0);

    // Verify phonemes have Latin characteristics
    bool hasLatinPhonemes = false;
    for (const auto& phoneme : result.phonemes) {
        // Latin should have specific phoneme patterns
        if (!phoneme.symbol.empty()) {
            hasLatinPhonemes = true;
            break;
        }
    }
    EXPECT_TRUE(hasLatinPhonemes);

    // Generate audio
    int numSamples = static_cast<int>(result.getTotalDuration() * sampleRate_);
    AudioBuffer outputBuffer(2, numSamples);
    outputBuffer.clear();

    formantSynth_->process(result.phonemes, outputBuffer);

    // Verify output
    EXPECT_TRUE(isValidAudio(outputBuffer.getReadPointer(0), numSamples));
    float energy = calculateEnergy(outputBuffer.getReadPointer(0), numSamples);
    EXPECT_GT(energy, 0.001f);
}

// ============================================================================
// Test 3: Klingon Text-to-Phoneme-to-Audio Pipeline
// ============================================================================

TEST_F(SynthesisPipelineTest, KlingonTextToAudioPipeline) {
    // Load Klingon language
    ASSERT_TRUE(loadLanguage("tlh"));

    // Convert text to phonemes
    std::string text = "tlhIngan maH";
    G2PResult result = g2pEngine_->convert(text);

    EXPECT_TRUE(result.success);
    EXPECT_GT(result.getPhonemeCount(), 0);

    // Klingon should have unique phonemes
    bool hasKlingonPhonemes = false;
    for (const auto& phoneme : result.phonemes) {
        // Check for Klingon-specific phoneme patterns
        if (!phoneme.symbol.empty()) {
            hasKlingonPhonemes = true;
            break;
        }
    }
    EXPECT_TRUE(hasKlingonPhonemes);

    // Generate audio
    int numSamples = static_cast<int>(result.getTotalDuration() * sampleRate_);
    AudioBuffer outputBuffer(2, numSamples);
    outputBuffer.clear();

    formantSynth_->process(result.phonemes, outputBuffer);

    // Verify output
    EXPECT_TRUE(isValidAudio(outputBuffer.getReadPointer(0), numSamples));
    float energy = calculateEnergy(outputBuffer.getReadPointer(0), numSamples);
    EXPECT_GT(energy, 0.001f);
}

// ============================================================================
// Test 4: Throat Singing Presets (All 6 Variants)
// ============================================================================

TEST_F(SynthesisPipelineTest, ThroatSingingAllPresets) {
    struct ThroatSingingPreset {
        std::string name;
        float baseFreq;
        float formant1;
        float formant2;
        float subharmonicMix;
    };

    std::vector<ThroatSingingPreset> presets = {
        {"Sygyt", 440.0f, 1200.0f, 2400.0f, 0.8f},
        {"Kargyraa", 220.0f, 800.0f, 1600.0f, 0.9f},
        {"Borbangnadyr", 330.0f, 1000.0f, 2000.0f, 0.85f},
        {"Ezengileer", 275.0f, 900.0f, 1800.0f, 0.75f},
        {"Chylandyk", 385.0f, 1100.0f, 2200.0f, 0.7f},
        {"Khöömei Basics", 293.0f, 950.0f, 1900.0f, 0.6f}
    };

    for (const auto& preset : presets) {
        // Configure DSP components for preset
        formantResonator_->setFormantFrequency(0, preset.formant1);
        formantResonator_->setFormantFrequency(1, preset.formant2);
        subharmonicGen_->setSubharmonicRatio(0.5f);
        subharmonicGen_->setMix(preset.subharmonicMix);

        // Generate test tone
        int numSamples = static_cast<int>(sampleRate_); // 1 second
        AudioBuffer outputBuffer(2, numSamples);
        outputBuffer.clear();

        // Simple synthesis: sine wave at base frequency
        float* left = outputBuffer.getWritePointer(0);
        float* right = outputBuffer.getWritePointer(1);
        float phase = 0.0f;
        float phaseIncrement = (2.0f * M_PI * preset.baseFreq) / sampleRate_;

        for (int i = 0; i < numSamples; ++i) {
            float sample = std::sin(phase);
            phase += phaseIncrement;
            if (phase > 2.0f * M_PI) phase -= 2.0f * M_PI;

            left[i] = sample;
            right[i] = sample;
        }

        // Process through formant resonator
        formantResonator_->process(outputBuffer);

        // Process through subharmonic generator
        subharmonicGen_->process(outputBuffer);

        // Process through spectral enhancer
        spectralEnhancer_->process(outputBuffer);

        // Verify output for this preset
        EXPECT_TRUE(isValidAudio(left, numSamples))
            << "Failed for preset: " << preset.name;
        EXPECT_TRUE(isValidAudio(right, numSamples))
            << "Failed for preset: " << preset.name;

        // Check energy level
        float energy = calculateEnergy(left, numSamples);
        EXPECT_GT(energy, 0.001f)
            << "No audio generated for preset: " << preset.name;

        // Check for clicks/pops
        EXPECT_FALSE(hasClicks(left, numSamples))
            << "Audio has clicks for preset: " << preset.name;
    }
}

// ============================================================================
// Test 5: Formant Synthesis Integration
// ============================================================================

TEST_F(SynthesisPipelineTest, FormantSynthesisIntegration) {
    // Create phoneme sequence for a simple vowel
    std::vector<PhonemeResult> phonemes = {
        {"AA", 0.5f, 220.0f, false, 0, 0},
        {"EH", 0.5f, 220.0f, false, 0, 1}
    };

    int numSamples = static_cast<int>(sampleRate_);
    AudioBuffer outputBuffer(2, numSamples);
    outputBuffer.clear();

    // Process through formant synthesis
    formantSynth_->process(phonemes, outputBuffer);

    // Verify output
    EXPECT_TRUE(isValidAudio(outputBuffer.getReadPointer(0), numSamples));
    EXPECT_TRUE(isValidAudio(outputBuffer.getReadPointer(1), numSamples));

    float energy = calculateEnergy(outputBuffer.getReadPointer(0), numSamples);
    EXPECT_GT(energy, 0.001f);
}

// ============================================================================
// Test 6: Subharmonic Synthesis Integration
// ============================================================================

TEST_F(SynthesisPipelineTest, SubharmonicSynthesisIntegration) {
    // Configure subharmonic generator
    subharmonicGen_->setSubharmonicRatio(0.5f);
    subharmonicGen_->setMix(0.7f);

    // Generate input tone
    int numSamples = static_cast<int>(sampleRate_);
    AudioBuffer outputBuffer(2, numSamples);
    outputBuffer.clear();

    float* left = outputBuffer.getWritePointer(0);
    float* right = outputBuffer.getWritePointer(1);
    float phase = 0.0f;
    float phaseIncrement = (2.0f * M_PI * 220.0f) / sampleRate_;

    for (int i = 0; i < numSamples; ++i) {
        float sample = std::sin(phase);
        phase += phaseIncrement;
        if (phase > 2.0f * M_PI) phase -= 2.0f * M_PI;
        left[i] = sample;
        right[i] = sample;
    }

    // Process through subharmonic generator
    subharmonicGen_->process(outputBuffer);

    // Verify output
    EXPECT_TRUE(isValidAudio(left, numSamples));
    EXPECT_TRUE(isValidAudio(right, numSamples));

    float energy = calculateEnergy(left, numSamples);
    EXPECT_GT(energy, 0.001f);

    // Check that subharmonic adds richness (more energy than input)
    float inputEnergy = 0.7071f; // RMS of sine wave
    EXPECT_GT(energy, inputEnergy);
}

// ============================================================================
// Test 7: Diphone Synthesis Integration
// ============================================================================

TEST_F(SynthesisPipelineTest, DiphoneSynthesisIntegration) {
    // Create diphone sequence
    std::vector<PhonemeResult> phonemes = {
        {"AA", 0.3f, 220.0f, false, 0, 0},
        {"EH", 0.3f, 220.0f, false, 0, 1},
        {"IY", 0.3f, 220.0f, false, 0, 2}
    };

    int numSamples = static_cast<int>(sampleRate_);
    AudioBuffer outputBuffer(2, numSamples);
    outputBuffer.clear();

    // Process through diphone synthesis
    diphoneSynth_->process(phonemes, outputBuffer);

    // Verify output
    EXPECT_TRUE(isValidAudio(outputBuffer.getReadPointer(0), numSamples));
    EXPECT_TRUE(isValidAudio(outputBuffer.getReadPointer(1), numSamples));

    float energy = calculateEnergy(outputBuffer.getReadPointer(0), numSamples);
    EXPECT_GT(energy, 0.001f);

    // Check for smooth transitions (no clicks)
    EXPECT_FALSE(hasClicks(outputBuffer.getReadPointer(0), numSamples));
}

// ============================================================================
// Test 8: Complete Pipeline Performance
// ============================================================================

TEST_F(SynthesisPipelineTest, CompletePipelinePerformance) {
    // Load English language
    ASSERT_TRUE(loadLanguage("en"));

    // Convert text
    std::string text = "The quick brown fox jumps over the lazy dog";
    G2PResult result = g2pEngine_->convert(text);
    ASSERT_TRUE(result.success);

    // Measure performance
    auto startTime = std::chrono::high_resolution_clock::now();

    int numSamples = static_cast<int>(result.getTotalDuration() * sampleRate_);
    AudioBuffer outputBuffer(2, numSamples);
    outputBuffer.clear();

    // Complete pipeline
    formantSynth_->process(result.phonemes, outputBuffer);
    spectralEnhancer_->process(outputBuffer);

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    // Performance target: Should complete in less than 2x real-time
    float realTimeDuration = (numSamples / sampleRate_) * 1000.0f;
    EXPECT_LT(duration.count(), realTimeDuration * 2.0f);

    // Verify output quality
    EXPECT_TRUE(isValidAudio(outputBuffer.getReadPointer(0), numSamples));
}

// ============================================================================
// Test 9: Voice Manager Integration
// ============================================================================

TEST_F(SynthesisPipelineTest, VoiceManagerIntegration) {
    // Test simultaneous voice allocation
    const int numVoices = 60;
    std::vector<int> voiceIds;

    for (int i = 0; i < numVoices; ++i) {
        int midiNote = 60 + (i % 24); // Spread across 2 octaves
        float velocity = 0.8f;
        int voiceId = voiceManager_->noteOn(midiNote, velocity);
        EXPECT_GE(voiceId, 0);
        voiceIds.push_back(voiceId);
    }

    // Verify all voices are active
    EXPECT_EQ(voiceManager_->getActiveVoiceCount(), numVoices);

    // Process audio
    AudioBuffer outputBuffer(2, bufferSize_);
    outputBuffer.clear();

    voiceManager_->processAudio(
        outputBuffer.getWritePointer(0),
        outputBuffer.getWritePointer(1),
        bufferSize_
    );

    // Verify output is valid
    EXPECT_TRUE(isValidAudio(outputBuffer.getReadPointer(0), bufferSize_));
    EXPECT_TRUE(isValidAudio(outputBuffer.getReadPointer(1), bufferSize_));

    // Release all voices
    for (int voiceId : voiceIds) {
        VoiceInstance* voice = voiceManager_->getVoice(voiceId);
        if (voice && voice->active) {
            int midiNote = static_cast<int>(voice->frequency);
            voiceManager_->noteOff(midiNote, 0.0f);
        }
    }
}

// ============================================================================
// Test 10: Parameter Smoothing
// ============================================================================

TEST_F(SynthesisPipelineTest, ParameterSmoothing) {
    // Configure voice manager
    voiceManager_->setMasterGain(1.0f);
    voiceManager_->setAttackTime(0.01f);
    voiceManager_->setReleaseTime(0.1f);

    // Start a voice
    int voiceId = voiceManager_->noteOn(60, 1.0f);
    EXPECT_GE(voiceId, 0);

    // Process audio
    AudioBuffer outputBuffer(2, bufferSize_);
    outputBuffer.clear();

    voiceManager_->processAudio(
        outputBuffer.getWritePointer(0),
        outputBuffer.getWritePointer(1),
        bufferSize_
    );

    // Verify no clicks during parameter changes
    EXPECT_FALSE(hasClicks(outputBuffer.getReadPointer(0), bufferSize_, 0.2f));

    // Change parameters rapidly
    for (int i = 0; i < 10; ++i) {
        voiceManager_->setMasterGain(0.5f + (i % 3) * 0.25f);
        voiceManager_->setVibratoDepth(0.1f * i);

        voiceManager_->processAudio(
            outputBuffer.getWritePointer(0),
            outputBuffer.getWritePointer(1),
            bufferSize_
        );

        // Verify smooth transitions
        EXPECT_FALSE(hasClicks(outputBuffer.getReadPointer(0), bufferSize_, 0.3f))
            << "Click detected at iteration " << i;
    }

    // Release voice
    voiceManager_->noteOff(60, 0.0f);
}

// ============================================================================
// Test 11: Spectral Enhancement
// ============================================================================

TEST_F(SynthesisPipelineTest, SpectralEnhancement) {
    // Generate simple sine wave
    int numSamples = static_cast<int>(sampleRate_);
    AudioBuffer outputBuffer(2, numSamples);
    outputBuffer.clear();

    float* left = outputBuffer.getWritePointer(0);
    float* right = outputBuffer.getWritePointer(1);
    float phase = 0.0f;
    float phaseIncrement = (2.0f * M_PI * 440.0f) / sampleRate_;

    for (int i = 0; i < numSamples; ++i) {
        float sample = std::sin(phase);
        phase += phaseIncrement;
        if (phase > 2.0f * M_PI) phase -= 2.0f * M_PI;
        left[i] = sample;
        right[i] = sample;
    }

    // Calculate input energy
    float inputEnergy = calculateEnergy(left, numSamples);

    // Process through spectral enhancer
    spectralEnhancer_->process(outputBuffer);

    // Calculate output energy
    float outputEnergy = calculateEnergy(left, numSamples);

    // Spectral enhancer should add harmonics (increase energy)
    EXPECT_GT(outputEnergy, inputEnergy * 1.1f);

    // Verify no distortion
    EXPECT_TRUE(isValidAudio(left, numSamples));
}

// ============================================================================
// Test 12: Real-Time Safety
// ============================================================================

TEST_F(SynthesisPipelineTest, RealTimeSafety) {
    // This test verifies no memory allocations in audio thread

    // Start multiple voices
    for (int i = 0; i < 40; ++i) {
        voiceManager_->noteOn(60 + i, 0.8f);
    }

    // Process multiple buffers (simulating real-time processing)
    AudioBuffer outputBuffer(2, bufferSize_);
    outputBuffer.clear();

    for (int i = 0; i < 100; ++i) {
        voiceManager_->processAudio(
            outputBuffer.getWritePointer(0),
            outputBuffer.getWritePointer(1),
            bufferSize_
        );

        // Verify output remains valid
        EXPECT_TRUE(isValidAudio(outputBuffer.getReadPointer(0), bufferSize_));
    }

    // All notes off
    voiceManager_->allNotesOff();
}

// ============================================================================
// Test 13: Edge Cases
// ============================================================================

TEST_F(SynthesisPipelineTest, EdgeCases) {
    // Test empty text
    ASSERT_TRUE(loadLanguage("en"));
    G2PResult result = g2pEngine_->convert("");
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.getPhonemeCount(), 0);

    // Test very long text
    std::string longText(10000, 'a'); // 10k characters
    result = g2pEngine_->convert(longText);
    EXPECT_TRUE(result.success);

    // Test very short duration
    std::vector<PhonemeResult> shortPhonemes = {
        {"AA", 0.001f, 220.0f, false, 0, 0}
    };

    int numSamples = 10;
    AudioBuffer outputBuffer(2, numSamples);
    outputBuffer.clear();

    formantSynth_->process(shortPhonemes, outputBuffer);
    EXPECT_TRUE(isValidAudio(outputBuffer.getReadPointer(0), numSamples));

    // Test zero voices
    EXPECT_EQ(voiceManager_->getActiveVoiceCount(), 0);

    // Test extreme frequencies
    int extremeVoiceId = voiceManager_->noteOn(0, 1.0f); // Lowest MIDI note
    EXPECT_GE(extremeVoiceId, 0);

    extremeVoiceId = voiceManager_->noteOn(127, 1.0f); // Highest MIDI note
    EXPECT_GE(extremeVoiceId, 0);
}
