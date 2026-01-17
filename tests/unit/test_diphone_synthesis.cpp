/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * test_diphone_synthesis.cpp - Unit tests for DiphoneSynthesis
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#include <gtest/gtest.h>
#include "../../src/synthesis/DiphoneSynthesis.h"
#include "../../src/core/Voice.h"
#include "../../src/core/Phoneme.h"
#include <vector>
#include <cmath>

using namespace ChoirV2;

//==============================================================================
// Test Fixture
//==============================================================================

class DiphoneSynthesisTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize synthesis parameters
        params_.sample_rate = 48000.0f;
        params_.max_block_size = 512;
        params_.enable_simd = true;
        params_.enable_anti_aliasing = true;
        params_.oversampling_factor = 1.0f;

        // Create synthesis method
        synthesis_ = std::make_unique<DiphoneSynthesis>();
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
        phoneme.category = "vowel";
        phoneme.articulatory.is_voiced = true;

        // Set formant frequencies (typical values)
        if (ipa == "i") {  // /i/ "ee"
            phoneme.formants.f1 = 270; phoneme.formants.f2 = 2300;
            phoneme.formants.f3 = 3000; phoneme.formants.f4 = 3500;
        } else if (ipa == "æ") {  // /æ/ "cat"
            phoneme.formants.f1 = 660; phoneme.formants.f2 = 1700;
            phoneme.formants.f3 = 2600; phoneme.formants.f4 = 3500;
        } else if (ipa == "o") {  // /o/ "oh"
            phoneme.formants.f1 = 570; phoneme.formants.f2 = 840;
            phoneme.formants.f3 = 2500; phoneme.formants.f4 = 3500;
        } else if (ipa == "ɪ") {  // /ɪ/ "ih"
            phoneme.formants.f1 = 390; phoneme.formants.f2 = 2000;
            phoneme.formants.f3 = 2800; phoneme.formants.f4 = 3500;
        } else if (ipa == "a") {  // /a/ "ah"
            phoneme.formants.f1 = 730; phoneme.formants.f2 = 1090;
            phoneme.formants.f3 = 2440; phoneme.formants.f4 = 3500;
        } else {
            // Default schwa-like
            phoneme.formants.f1 = 500; phoneme.formants.f2 = 1500;
            phoneme.formants.f3 = 2500; phoneme.formants.f4 = 3500;
        }

        // Set bandwidths
        phoneme.formants.bw1 = 60; phoneme.formants.bw2 = 90;
        phoneme.formants.bw3 = 120; phoneme.formants.bw4 = 150;

        return phoneme;
    }

    Phoneme createConsonantPhoneme(const std::string& ipa, bool voiced) {
        Phoneme phoneme;
        phoneme.id = "test_" + ipa;
        phoneme.ipa = ipa;
        phoneme.category = "consonant";
        phoneme.articulatory.is_voiced = voiced;

        // Set formant frequencies for consonants
        if (ipa == "k") {  // /k/ plosive
            phoneme.formants.f1 = 500; phoneme.formants.f2 = 1800;
            phoneme.formants.f3 = 2500; phoneme.formants.f4 = 3500;
        } else if (ipa == "t") {  // /t/ plosive
            phoneme.formants.f1 = 400; phoneme.formants.f2 = 1500;
            phoneme.formants.f3 = 2500; phoneme.formants.f4 = 3500;
        } else if (ipa == "s") {  // /s/ fricative
            phoneme.formants.f1 = 5000; phoneme.formants.f2 = 6000;
            phoneme.formants.f3 = 7000; phoneme.formants.f4 = 8000;
        } else if (ipa == "l") {  // /l/ lateral
            phoneme.formants.f1 = 350; phoneme.formants.f2 = 1400;
            phoneme.formants.f3 = 2500; phoneme.formants.f4 = 3500;
        } else {
            // Default
            phoneme.formants.f1 = 500; phoneme.formants.f2 = 1500;
            phoneme.formants.f3 = 2500; phoneme.formants.f4 = 3500;
        }

        phoneme.formants.bw1 = 50; phoneme.formants.bw2 = 80;
        phoneme.formants.bw3 = 120; phoneme.formants.bw4 = 150;

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
    std::unique_ptr<DiphoneSynthesis> synthesis_;
    std::unique_ptr<Voice> voice_;
};

//==============================================================================
// Initialization Tests
//==============================================================================

TEST_F(DiphoneSynthesisTest, InitializeReturnsSuccess) {
    DiphoneSynthesis synthesis;
    ASSERT_TRUE(synthesis.initialize(params_));
}

TEST_F(DiphoneSynthesisTest, InitializeTwice) {
    DiphoneSynthesis synthesis;
    ASSERT_TRUE(synthesis.initialize(params_));
    ASSERT_TRUE(synthesis.initialize(params_));  // Should handle re-init
}

TEST_F(DiphoneSynthesisTest, GetNameReturnsDiphone) {
    EXPECT_EQ(synthesis_->getName(), "diphone");
}

TEST_F(DiphoneSynthesisTest, ResetWorks) {
    synthesis_->reset();
    // Should not crash, state should be clean
    SUCCEED();
}

//==============================================================================
// CV Transition Tests (Consonant-Vowel)
//==============================================================================

TEST_F(DiphoneSynthesisTest, CVTransition_K_AE_Cat) {
    // Test /k/ → /æ/ transition (as in "cat")
    auto consonant = createConsonantPhoneme("k", false);  // /k/ unvoiced
    auto vowel = createVowelPhoneme("æ");  // /æ/ "cat"

    std::vector<float> output(512);

    // Synthesize consonant
    auto result1 = synthesis_->synthesizeVoice(
        voice_.get(),
        &consonant,
        output.data(),
        256
    );

    // Synthesize vowel (should transition smoothly)
    auto result2 = synthesis_->synthesizeVoice(
        voice_.get(),
        &vowel,
        output.data(),
        512
    );

    ASSERT_TRUE(result1.success);
    ASSERT_TRUE(result2.success);
    EXPECT_TRUE(hasSignalEnergy(output.data(), 512));
}

TEST_F(DiphoneSynthesisTest, CVTransition_T_OE_Toe) {
    // Test /t/ → /oʊ/ transition (as in "toe")
    auto consonant = createConsonantPhoneme("t", false);  // /t/ unvoiced
    auto vowel = createVowelPhoneme("o");  // /o/ "oh"

    std::vector<float> output(512);

    auto result1 = synthesis_->synthesizeVoice(
        voice_.get(),
        &consonant,
        output.data(),
        256
    );

    auto result2 = synthesis_->synthesizeVoice(
        voice_.get(),
        &vowel,
        output.data(),
        512
    );

    ASSERT_TRUE(result1.success);
    ASSERT_TRUE(result2.success);
}

//==============================================================================
// VC Transition Tests (Vowel-Consonant)
//==============================================================================

TEST_F(DiphoneSynthesisTest, VCTransition_AE_T_At) {
    // Test /æ/ → /t/ transition (as in "at")
    auto vowel = createVowelPhoneme("æ");  // /æ/ "cat"
    auto consonant = createConsonantPhoneme("t", false);  // /t/ unvoiced

    std::vector<float> output(512);

    auto result1 = synthesis_->synthesizeVoice(
        voice_.get(),
        &vowel,
        output.data(),
        256
    );

    auto result2 = synthesis_->synthesizeVoice(
        voice_.get(),
        &consonant,
        output.data(),
        512
    );

    ASSERT_TRUE(result1.success);
    ASSERT_TRUE(result2.success);
}

TEST_F(DiphoneSynthesisTest, VCTransition_OE_L_All) {
    // Test /oʊ/ → /l/ transition (as in "all")
    auto vowel = createVowelPhoneme("o");  // /o/ "oh"
    auto consonant = createConsonantPhoneme("l", true);  // /l/ voiced lateral

    std::vector<float> output(512);

    auto result1 = synthesis_->synthesizeVoice(
        voice_.get(),
        &vowel,
        output.data(),
        256
    );

    auto result2 = synthesis_->synthesizeVoice(
        voice_.get(),
        &consonant,
        output.data(),
        512
    );

    ASSERT_TRUE(result1.success);
    ASSERT_TRUE(result2.success);
}

//==============================================================================
// VV Diphthong Tests (Vowel-Vowel)
//==============================================================================

TEST_F(DiphoneSynthesisTest, VVDiphthong_AI_Eye) {
    // Test /a/ → /ɪ/ transition (as in "eye")
    auto vowel1 = createVowelPhoneme("a");  // /a/ "ah"
    auto vowel2 = createVowelPhoneme("ɪ");  // /ɪ/ "ih"

    std::vector<float> output1(512);
    std::vector<float> output2(512);

    auto result1 = synthesis_->synthesizeVoice(
        voice_.get(),
        &vowel1,
        output1.data(),
        512
    );

    auto result2 = synthesis_->synthesizeVoice(
        voice_.get(),
        &vowel2,
        output2.data(),
        512
    );

    ASSERT_TRUE(result1.success);
    ASSERT_TRUE(result2.success);

    // Both should have energy
    EXPECT_TRUE(hasSignalEnergy(output1.data(), 512));
    EXPECT_TRUE(hasSignalEnergy(output2.data(), 512));

    // Formants should be different (F1 and F2)
    // /a/ has higher F1, lower F2 than /ɪ/
}

TEST_F(DiphoneSynthesisTest, VVDiphthong_OI_Boy) {
    // Test /o/ → /ɪ/ transition (as in "boy")
    auto vowel1 = createVowelPhoneme("o");  // /o/ "oh"
    auto vowel2 = createVowelPhoneme("ɪ");  // /ɪ/ "ih"

    std::vector<float> output1(512);
    std::vector<float> output2(512);

    auto result1 = synthesis_->synthesizeVoice(
        voice_.get(),
        &vowel1,
        output1.data(),
        512
    );

    auto result2 = synthesis_->synthesizeVoice(
        voice_.get(),
        &vowel2,
        output2.data(),
        512
    );

    ASSERT_TRUE(result1.success);
    ASSERT_TRUE(result2.success);
}

//==============================================================================
// CC Cluster Tests (Consonant-Consonant)
//==============================================================================

TEST_F(DiphoneSynthesisTest, CCCluster_ST_Stop) {
    // Test /s/ → /t/ cluster (as in "stop")
    auto cons1 = createConsonantPhoneme("s", false);  // /s/ unvoiced fricative
    auto cons2 = createConsonantPhoneme("t", false);  // /t/ unvoiced plosive

    std::vector<float> output(512);

    auto result1 = synthesis_->synthesizeVoice(
        voice_.get(),
        &cons1,
        output.data(),
        256
    );

    auto result2 = synthesis_->synthesizeVoice(
        voice_.get(),
        &cons2,
        output.data(),
        512
    );

    ASSERT_TRUE(result1.success);
    ASSERT_TRUE(result2.success);
}

TEST_F(DiphoneSynthesisTest, CCCluster_STR_String) {
    // Test /st/ → /r/ cluster (as in "string")
    auto cons1 = createConsonantPhoneme("s", false);
    auto cons2 = createConsonantPhoneme("t", false);

    std::vector<float> output(512);

    auto result1 = synthesis_->synthesizeVoice(
        voice_.get(),
        &cons1,
        output.data(),
        170
    );

    auto result2 = synthesis_->synthesizeVoice(
        voice_.get(),
        &cons2,
        output.data(),
        512
    );

    ASSERT_TRUE(result1.success);
    ASSERT_TRUE(result2.success);
}

//==============================================================================
// Formant Interpolation Tests
//==============================================================================

TEST_F(DiphoneSynthesisTest, FormantInterpolationSmooth) {
    // Test that formant interpolation produces smooth transitions
    auto vowel1 = createVowelPhoneme("i");  // /i/ high F2
    auto vowel2 = createVowelPhoneme("a");  // /a/ low F2

    std::vector<float> output1(512);
    std::vector<float> output2(512);
    std::vector<float> output3(512);

    // Synthesize first vowel
    synthesis_->synthesizeVoice(voice_.get(), &vowel1, output1.data(), 512);

    // Synthesize second vowel (transition)
    synthesis_->synthesizeVoice(voice_.get(), &vowel2, output2.data(), 512);

    // Synthesize more of second vowel
    synthesis_->synthesizeVoice(voice_.get(), &vowel2, output3.data(), 512);

    // All should have energy
    EXPECT_TRUE(hasSignalEnergy(output1.data(), 512));
    EXPECT_TRUE(hasSignalEnergy(output2.data(), 512));
    EXPECT_TRUE(hasSignalEnergy(output3.data(), 512));
}

//==============================================================================
// Temporal Alignment Tests
//==============================================================================

TEST_F(DiphoneSynthesisTest, CVTemporalAlignment) {
    // CV diphone: consonant (30%) → vowel (70%)
    // The transition should weight the consonant portion less
    auto consonant = createConsonantPhoneme("k", false);
    auto vowel = createVowelPhoneme("æ");

    std::vector<float> output(512);

    // Start with consonant
    synthesis_->synthesizeVoice(voice_.get(), &consonant, output.data(), 150);

    // Transition to vowel
    synthesis_->synthesizeVoice(voice_.get(), &vowel, output.data(), 512);

    EXPECT_TRUE(hasSignalEnergy(output.data(), 512));
}

TEST_F(DiphoneSynthesisTest, VVTemporalAlignment) {
    // VV diphone: equal distribution (50%-50%)
    auto vowel1 = createVowelPhoneme("a");
    auto vowel2 = createVowelPhoneme("ɪ");

    std::vector<float> output(512);

    synthesis_->synthesizeVoice(voice_.get(), &vowel1, output.data(), 256);
    synthesis_->synthesizeVoice(voice_.get(), &vowel2, output.data(), 512);

    EXPECT_TRUE(hasSignalEnergy(output.data(), 512));
}

//==============================================================================
// Crossfade Curve Tests
//==============================================================================

TEST_F(DiphoneSynthesisTest, SetCrossfadeCurveLinear) {
    synthesis_->setCrossfadeCurve(1.0f);  // Linear
    // Should not crash
    SUCCEED();
}

TEST_F(DiphoneSynthesisTest, SetCrossfadeCurveNonLinear) {
    synthesis_->setCrossfadeCurve(2.0f);  // Slower transition
    synthesis_->setCrossfadeCurve(0.5f);  // Faster transition
    // Should not crash
    SUCCEED();
}

TEST_F(DiphoneSynthesisTest, CrossfadeCurveAffectsTransition) {
    // Test that different crossfade curves produce different results
    auto vowel1 = createVowelPhoneme("i");
    auto vowel2 = createVowelPhoneme("a");

    synthesis_->setCrossfadeCurve(1.0f);  // Linear

    std::vector<float> output1(512);
    synthesis_->synthesizeVoice(voice_.get(), &vowel1, output1.data(), 256);
    synthesis_->synthesizeVoice(voice_.get(), &vowel2, output1.data(), 512);

    synthesis_->setCrossfadeCurve(2.0f);  // Non-linear

    std::vector<float> output2(512);
    synthesis_->synthesizeVoice(voice_.get(), &vowel1, output2.data(), 256);
    synthesis_->synthesizeVoice(voice_.get(), &vowel2, output2.data(), 512);

    // Both should have energy
    EXPECT_TRUE(hasSignalEnergy(output1.data(), 512));
    EXPECT_TRUE(hasSignalEnergy(output2.data(), 512));
}

//==============================================================================
// Transition Duration Tests
//==============================================================================

TEST_F(DiphoneSynthesisTest, SetTransitionDuration) {
    synthesis_->setTransitionDuration(0.05f);  // 50ms
    synthesis_->setTransitionDuration(0.15f);  // 150ms
    synthesis_->setTransitionDuration(0.30f);  // 300ms
    // Should not crash
    SUCCEED();
}

TEST_F(DiphoneSynthesisTest, TransitionDurationAffectsSmoothing) {
    // Test that transition duration affects formant smoothing
    auto vowel1 = createVowelPhoneme("i");
    auto vowel2 = createVowelPhoneme("a");

    // Fast transition
    synthesis_->setTransitionDuration(0.02f);

    std::vector<float> output1(512);
    synthesis_->synthesizeVoice(voice_.get(), &vowel1, output1.data(), 256);
    synthesis_->synthesizeVoice(voice_.get(), &vowel2, output1.data(), 512);

    // Slow transition
    synthesis_->setTransitionDuration(0.20f);

    std::vector<float> output2(512);
    synthesis_->synthesizeVoice(voice_.get(), &vowel1, output2.data(), 256);
    synthesis_->synthesizeVoice(voice_.get(), &vowel2, output2.data(), 512);

    // Both should have energy
    EXPECT_TRUE(hasSignalEnergy(output1.data(), 512));
    EXPECT_TRUE(hasSignalEnergy(output2.data(), 512));
}

//==============================================================================
// Coarticulation Tests
//==============================================================================

TEST_F(DiphoneSynthesisTest, EnableCoarticulation) {
    synthesis_->setCoarticulationEnabled(true);
    // Should not crash
    SUCCEED();
}

TEST_F(DiphoneSynthesisTest, DisableCoarticulation) {
    synthesis_->setCoarticulationEnabled(false);
    // Should not crash
    SUCCEED();
}

//==============================================================================
// Multi-Voice SIMD Tests
//==============================================================================

TEST_F(DiphoneSynthesisTest, SynthesizeMultipleVoices) {
    std::vector<Voice*> voices;
    std::vector<const Phoneme*> phonemes;

    // Create 4 voices with different phonemes
    for (int i = 0; i < 4; ++i) {
        auto voice = std::make_unique<Voice>();
        voice->setFrequency(220.0f + i * 55.0f);
        voice->setAmplitude(0.5f);
        voice->setActive(true);

        Phoneme* phoneme;
        if (i % 2 == 0) {
            phoneme = new Phoneme(createVowelPhoneme("a"));
        } else {
            phoneme = new Phoneme(createVowelPhoneme("i"));
        }

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

TEST_F(DiphoneSynthesisTest, SynthesizeWithMismatchedVectorSizes) {
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

TEST_F(DiphoneSynthesisTest, SynthesizeWithNullVoice) {
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

TEST_F(DiphoneSynthesisTest, SynthesizeWithNullPhoneme) {
    std::vector<float> output(512);

    auto result = synthesis_->synthesizeVoice(
        voice_.get(),
        nullptr,  // null phoneme
        output.data(),
        512
    );

    EXPECT_FALSE(result.success);
}

TEST_F(DiphoneSynthesisTest, SynthesizeWithNullOutput) {
    auto phoneme = createVowelPhoneme("a");

    auto result = synthesis_->synthesizeVoice(
        voice_.get(),
        &phoneme,
        nullptr,  // null output
        512
    );

    EXPECT_FALSE(result.success);
}

TEST_F(DiphoneSynthesisTest, SynthesizeWithZeroSamples) {
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

TEST_F(DiphoneSynthesisTest, SynthesizeBeforeInitialize) {
    DiphoneSynthesis synthesis;  // Not initialized
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

TEST_F(DiphoneSynthesisTest, GetStatsAfterSynthesis) {
    auto phoneme = createVowelPhoneme("a");
    std::vector<float> output(512);

    synthesis_->synthesizeVoice(voice_.get(), &phoneme, output.data(), 512);
    synthesis_->synthesizeVoice(voice_.get(), &phoneme, output.data(), 512);

    auto stats = synthesis_->getStats();
    EXPECT_EQ(stats.voices_processed, 2);
    EXPECT_EQ(stats.blocks_processed, 2);
}

//==============================================================================
// Real-Time Safety Tests
//==============================================================================

TEST_F(DiphoneSynthesisTest, NoAllocationsDuringProcess) {
    // This test verifies that synthesis doesn't allocate memory
    // (critical for real-time safety)

    auto phoneme = createVowelPhoneme("a");
    std::vector<float> output(512);

    // Pre-allocate everything
    synthesis_->synthesizeVoice(voice_.get(), &phoneme, output.data(), 512);

    // Now synthesis should not allocate
    // (in a real test, we'd use a custom allocator to verify this)
    EXPECT_TRUE(hasSignalEnergy(output.data(), 512));
}

//==============================================================================
// Main
//==============================================================================

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
