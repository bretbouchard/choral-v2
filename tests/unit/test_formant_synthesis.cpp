/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * test_formant_synthesis.cpp - Unit tests for FormantSynthesis
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#include <gtest/gtest.h>
#include "../../src/synthesis/FormantSynthesis.h"
#include "../../src/core/Voice.h"
#include "../../src/core/Phoneme.h"
#include <vector>
#include <cmath>

using namespace ChoirV2;

//==============================================================================
// Test Fixture
//==============================================================================

class FormantSynthesisTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize synthesis parameters
        params_.sample_rate = 48000.0f;
        params_.max_block_size = 512;
        params_.enable_simd = true;
        params_.enable_anti_aliasing = true;
        params_.oversampling_factor = 1.0f;

        // Create synthesis method
        synthesis_ = std::make_unique<FormantSynthesis>();
        ASSERT_TRUE(synthesis_->initialize(params_));

        // Create test voice
        voice_ = std::make_unique<Voice>();
        voice_->setFrequency(220.0f);  // A3
        voice_->setAmplitude(0.7f);
        voice_->setActive(true);
    }

    void TearDown() override {
        synthesis_.reset();
        voice_.reset();
    }

    // Helper: Create test phoneme
    Phoneme createVowelPhoneme(const std::string& ipa) {
        Phoneme phoneme;
        phoneme.id = "test_" + ipa;
        phoneme.ipa = ipa;
        phoneme.category = PhonemeCategory::Vowel;
        phoneme.articulatory.is_voiced = true;
        return phoneme;
    }

    Phoneme createConsonantPhoneme(const std::string& ipa, bool voiced) {
        Phoneme phoneme;
        phoneme.id = "test_" + ipa;
        phoneme.ipa = ipa;
        phoneme.category = PhonemeCategory::Consonant;
        phoneme.articulatory.is_voiced = voiced;
        return phoneme;
    }

    // Helper: Check if signal has energy
    bool hasSignalEnergy(const float* buffer, int num_samples, float threshold = 0.001f) {
        float sum = 0.0f;
        for (int i = 0; i < num_samples; ++i) {
            sum += std::abs(buffer[i]);
        }
        float rms = std::sqrt(sum / num_samples);
        return rms > threshold;
    }

    // Helper: Count zero crossings (estimate frequency)
    int countZeroCrossings(const float* buffer, int num_samples) {
        int crossings = 0;
        for (int i = 1; i < num_samples; ++i) {
            if ((buffer[i-1] >= 0.0f && buffer[i] < 0.0f) ||
                (buffer[i-1] < 0.0f && buffer[i] >= 0.0f)) {
                crossings++;
            }
        }
        return crossings;
    }

    // Test parameters
    SynthesisParams params_;
    std::unique_ptr<FormantSynthesis> synthesis_;
    std::unique_ptr<Voice> voice_;
};

//==============================================================================
// Initialization Tests
//==============================================================================

TEST_F(FormantSynthesisTest, InitializeReturnsSuccess) {
    FormantSynthesis synthesis;
    ASSERT_TRUE(synthesis.initialize(params_));
}

TEST_F(FormantSynthesisTest, InitializeTwice) {
    FormantSynthesis synthesis;
    ASSERT_TRUE(synthesis.initialize(params_));
    ASSERT_TRUE(synthesis.initialize(params_));  // Should handle re-init
}

TEST_F(FormantSynthesisTest, GetNameReturnsFormant) {
    EXPECT_EQ(synthesis_->getName(), "formant");
}

TEST_F(FormantSynthesisTest, ResetWorks) {
    synthesis_->reset();
    // Should not crash, state should be clean
    SUCCEED();
}

//==============================================================================
// Vowel Synthesis Tests
//==============================================================================

TEST_F(FormantSynthesisTest, SynthesizeVowel_I_EE) {
    auto phoneme = createVowelPhoneme("i");  // /i/ "ee"

    std::vector<float> output(512);
    auto result = synthesis_->synthesizeVoice(
        voice_.get(),
        &phoneme,
        output.data(),
        512
    );

    ASSERT_TRUE(result.success);
    EXPECT_TRUE(hasSignalEnergy(output.data(), 512));
}

TEST_F(FormantSynthesisTest, SynthesizeVowel_A_AH) {
    auto phoneme = createVowelPhoneme("a");  // /a/ "ah"

    std::vector<float> output(512);
    auto result = synthesis_->synthesizeVoice(
        voice_.get(),
        &phoneme,
        output.data(),
        512
    );

    ASSERT_TRUE(result.success);
    EXPECT_TRUE(hasSignalEnergy(output.data(), 512));
}

TEST_F(FormantSynthesisTest, SynthesizeVowel_U_OO) {
    auto phoneme = createVowelPhoneme("u");  // /u/ "oo"

    std::vector<float> output(512);
    auto result = synthesis_->synthesizeVoice(
        voice_.get(),
        &phoneme,
        output.data(),
        512
    );

    ASSERT_TRUE(result.success);
    EXPECT_TRUE(hasSignalEnergy(output.data(), 512));
}

TEST_F(FormantSynthesisTest, SynthesizeVowel_O_OH) {
    auto phoneme = createVowelPhoneme("o");  // /o/ "oh"

    std::vector<float> output(512);
    auto result = synthesis_->synthesizeVoice(
        voice_.get(),
        &phoneme,
        output.data(),
        512
    );

    ASSERT_TRUE(result.success);
    EXPECT_TRUE(hasSignalEnergy(output.data(), 512));
}

TEST_F(FormantSynthesisTest, SynthesizeVowel_Schwa) {
    auto phoneme = createVowelPhoneme("ə");  // /ə/ schwa

    std::vector<float> output(512);
    auto result = synthesis_->synthesizeVoice(
        voice_.get(),
        &phoneme,
        output.data(),
        512
    );

    ASSERT_TRUE(result.success);
    EXPECT_TRUE(hasSignalEnergy(output.data(), 512));
}

//==============================================================================
// Diphthong Tests (Formant Transitions)
//==============================================================================

TEST_F(FormantSynthesisTest, DiphthongTransition_OI_OY) {
    // Test formant transition from /o/ to /ɪ/ (oy sound)
    auto phoneme_o = createVowelPhoneme("o");
    auto phoneme_i = createVowelPhoneme("ɪ");

    std::vector<float> output1(512);
    std::vector<float> output2(512);

    // Synthesize first vowel
    auto result1 = synthesis_->synthesizeVoice(
        voice_.get(),
        &phoneme_o,
        output1.data(),
        512
    );

    // Synthesize second vowel (should transition smoothly)
    auto result2 = synthesis_->synthesizeVoice(
        voice_.get(),
        &phoneme_i,
        output2.data(),
        512
    );

    ASSERT_TRUE(result1.success);
    ASSERT_TRUE(result2.success);
    EXPECT_TRUE(hasSignalEnergy(output1.data(), 512));
    EXPECT_TRUE(hasSignalEnergy(output2.data(), 512));
}

TEST_F(FormantSynthesisTest, DiphthongTransition_AI_EYE) {
    // Test formant transition from /a/ to /ɪ/ (eye sound)
    auto phoneme_a = createVowelPhoneme("a");
    auto phoneme_i = createVowelPhoneme("ɪ");

    std::vector<float> output1(512);
    std::vector<float> output2(512);

    auto result1 = synthesis_->synthesizeVoice(
        voice_.get(),
        &phoneme_a,
        output1.data(),
        512
    );

    auto result2 = synthesis_->synthesizeVoice(
        voice_.get(),
        &phoneme_i,
        output2.data(),
        512
    );

    ASSERT_TRUE(result1.success);
    ASSERT_TRUE(result2.success);
}

//==============================================================================
// Consonant Tests
//==============================================================================

TEST_F(FormantSynthesisTest, SynthesizeConsonant_S_Fricative) {
    auto phoneme = createConsonantPhoneme("s", false);  // /s/ unvoiced fricative

    std::vector<float> output(512);
    auto result = synthesis_->synthesizeVoice(
        voice_.get(),
        &phoneme,
        output.data(),
        512
    );

    ASSERT_TRUE(result.success);
    // /s/ should have high-frequency noise
    EXPECT_TRUE(hasSignalEnergy(output.data(), 512));
}

TEST_F(FormantSynthesisTest, SynthesizeConsonant_M_Nasal) {
    auto phoneme = createConsonantPhoneme("m", true);  // /m/ voiced nasal

    std::vector<float> output(512);
    auto result = synthesis_->synthesizeVoice(
        voice_.get(),
        &phoneme,
        output.data(),
        512
    );

    ASSERT_TRUE(result.success);
    // /m/ should have harmonic content (voiced)
    EXPECT_TRUE(hasSignalEnergy(output.data(), 512));
}

TEST_F(FormantSynthesisTest, SynthesizeConsonant_T_Plosive) {
    auto phoneme = createConsonantPhoneme("t", false);  // /t/ unvoiced plosive

    std::vector<float> output(512);
    auto result = synthesis_->synthesizeVoice(
        voice_.get(),
        &phoneme,
        output.data(),
        512
    );

    ASSERT_TRUE(result.success);
    // /t/ should have a burst at the beginning
    // After the burst, signal should decay
}

//==============================================================================
// Formant Frequency Tests
//==============================================================================

TEST_F(FormantSynthesisTest, GetFormantFrequenciesForVowel) {
    auto phoneme = createVowelPhoneme("i");
    FormantDef formants;
    synthesis_->getFormantFrequencies(&phoneme, formants);

    // /i/ has high F2 and F3
    EXPECT_GT(formants.f2, 2000.0f);
    EXPECT_GT(formants.f3, 2500.0f);
}

TEST_F(FormantSynthesisTest, GetFormantFrequenciesForConsonant) {
    auto phoneme = createConsonantPhoneme("s", false);
    FormantDef formants;
    synthesis_->getFormantFrequencies(&phoneme, formants);

    // /s/ has very high formant frequencies
    EXPECT_GT(formants.f1, 4000.0f);
    EXPECT_GT(formants.f2, 5000.0f);
}

//==============================================================================
// Excitation Type Tests
//==============================================================================

TEST_F(FormantSynthesisTest, VoicedPhonemeUsesPulseExcitation) {
    auto phoneme = createVowelPhoneme("a");
    phoneme.articulatory.is_voiced = true;

    std::vector<float> output(512);
    synthesis_->synthesizeVoice(voice_.get(), &phoneme, output.data(), 512);

    // Should have harmonic content (zero crossings related to pitch)
    int crossings = countZeroCrossings(output.data(), 512);
    // For 220 Hz at 48 kHz, expect ~2-3 crossings per sample block
    EXPECT_GT(crossings, 0);
}

TEST_F(FormantSynthesisTest, UnvoicedPhonemeUsesNoiseExcitation) {
    auto phoneme = createConsonantPhoneme("s", false);
    phoneme.articulatory.is_voiced = false;

    std::vector<float> output(512);
    synthesis_->synthesizeVoice(voice_.get(), &phoneme, output.data(), 512);

    // Should have noise (many zero crossings)
    int crossings = countZeroCrossings(output.data(), 512);
    // Noise has many zero crossings
    EXPECT_GT(crossings, 100);
}

//==============================================================================
// Parameter Tests
//==============================================================================

TEST_F(FormantSynthesisTest, SetTransitionTime) {
    synthesis_->setTransitionTime(100.0f);  // 100ms
    // Should not crash
    SUCCEED();
}

TEST_F(FormantSynthesisTest, SetExcitationMix) {
    synthesis_->setExcitationMix(0.7f);  // 70% pulse, 30% noise
    // Should not crash
    SUCCEED();
}

TEST_F(FormantSynthesisTest, SetVibratoParams) {
    VibratoParams params;
    params.rate = 6.0f;
    params.depth = 1.5f;
    params.enabled = true;

    synthesis_->setVibratoParams(params);
    // Should not crash
    SUCCEED();
}

//==============================================================================
// Multi-Voice SIMD Tests
//==============================================================================

TEST_F(FormantSynthesisTest, SynthesizeMultipleVoices) {
    std::vector<Voice*> voices;
    std::vector<const Phoneme*> phonemes;

    // Create 4 voices
    for (int i = 0; i < 4; ++i) {
        auto voice = std::make_unique<Voice>();
        voice->setFrequency(220.0f + i * 55.0f);  // Different pitches
        voice->setAmplitude(0.5f);
        voice->setActive(true);

        auto phoneme = new Phoneme(createVowelPhoneme("a"));
        voices.push_back(voice.get());
        phonemes.push_back(phoneme);
    }

    std::vector<float> output(512);
    auto result = synthesis_->synthesizeVoicesSIMD(
        voices,
        phonemes,
        output.data(),
        512
    );

    ASSERT_TRUE(result.success);
    EXPECT_TRUE(hasSignalEnergy(output.data(), 512));

    // Cleanup
    for (auto phoneme : phonemes) {
        delete phoneme;
    }
}

TEST_F(FormantSynthesisTest, SynthesizeWithMismatchedVectorSizes) {
    std::vector<Voice*> voices;
    std::vector<const Phoneme*> phonemes;

    auto voice = std::make_unique<Voice>();
    voice->setActive(true);
    voices.push_back(voice.get());

    // Empty phonemes vector (mismatched)
    auto result = synthesis_->synthesizeVoicesSIMD(
        voices,
        phonemes,
        nullptr,
        512
    );

    EXPECT_FALSE(result.success);
}

//==============================================================================
// Edge Case Tests
//==============================================================================

TEST_F(FormantSynthesisTest, SynthesizeWithNullVoice) {
    auto phoneme = createVowelPhoneme("a");
    std::vector<float> output(512);

    auto result = synthesis_->synthesizeVoice(
        nullptr,  // null voice
        &phoneme,
        output.data(),
        512
    );

    EXPECT_FALSE(result.success);
}

TEST_F(FormantSynthesisTest, SynthesizeWithNullPhoneme) {
    std::vector<float> output(512);

    auto result = synthesis_->synthesizeVoice(
        voice_.get(),
        nullptr,  // null phoneme
        output.data(),
        512
    );

    EXPECT_FALSE(result.success);
}

TEST_F(FormantSynthesisTest, SynthesizeWithNullOutput) {
    auto phoneme = createVowelPhoneme("a");

    auto result = synthesis_->synthesizeVoice(
        voice_.get(),
        &phoneme,
        nullptr,  // null output
        512
    );

    EXPECT_FALSE(result.success);
}

TEST_F(FormantSynthesisTest, SynthesizeWithZeroSamples) {
    auto phoneme = createVowelPhoneme("a");
    std::vector<float> output(512);

    auto result = synthesis_->synthesizeVoice(
        voice_.get(),
        &phoneme,
        output.data(),
        0  // zero samples
    );

    EXPECT_FALSE(result.success);
}

TEST_F(FormantSynthesisTest, SynthesizeBeforeInitialize) {
    FormantSynthesis synthesis;  // Not initialized
    auto phoneme = createVowelPhoneme("a");
    std::vector<float> output(512);

    auto result = synthesis.synthesizeVoice(
        voice_.get(),
        &phoneme,
        output.data(),
        512
    );

    EXPECT_FALSE(result.success);
}

//==============================================================================
// Statistics Tests
//==============================================================================

TEST_F(FormantSynthesisTest, GetStatsAfterSynthesis) {
    auto phoneme = createVowelPhoneme("a");
    std::vector<float> output(512);

    synthesis_->synthesizeVoice(voice_.get(), &phoneme, output.data(), 512);
    synthesis_->synthesizeVoice(voice_.get(), &phoneme, output.data(), 512);

    auto stats = synthesis_->getStats();
    EXPECT_EQ(stats.voices_processed, 2);
    EXPECT_EQ(stats.blocks_processed, 2);
}

//==============================================================================
// Main
//==============================================================================

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
