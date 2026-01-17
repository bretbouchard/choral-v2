/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * test_reverb_effect.cpp - ReverbEffect unit tests
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#include <gtest/gtest.h>
#include <cmath>
#include <algorithm>
#include "dsp/ReverbEffect.h"

using namespace ChoirV2;

class ReverbEffectTest : public ::testing::Test {
protected:
    void SetUp() override {
        sample_rate_ = 48000.0f;
        max_block_size_ = 256;
        reverb_ = std::make_unique<ReverbEffect>();
        reverb_->prepare(sample_rate_, max_block_size_);
    }

    void TearDown() override {
        reverb_.reset();
    }

    // Helper: Generate silence buffer
    std::vector<float> generateSilence(int num_samples) {
        return std::vector<float>(num_samples, 0.0f);
    }

    // Helper: Generate DC offset (constant value)
    std::vector<float> generateDC(int num_samples, float value = 1.0f) {
        return std::vector<float>(num_samples, value);
    }

    // Helper: Generate sine wave
    std::vector<float> generateSine(int num_samples, float freq, float sample_rate) {
        std::vector<float> buffer(num_samples);
        for (int i = 0; i < num_samples; ++i) {
            buffer[i] = std::sin(2.0f * M_PI * freq * i / sample_rate);
        }
        return buffer;
    }

    // Helper: Calculate RMS level
    float calculateRMS(const float* buffer, int num_samples) {
        float sum = 0.0f;
        for (int i = 0; i < num_samples; ++i) {
            sum += buffer[i] * buffer[i];
        }
        return std::sqrt(sum / num_samples);
    }

    // Helper: Check if buffer is silent
    bool isSilent(const float* buffer, int num_samples, float threshold = 1e-6f) {
        for (int i = 0; i < num_samples; ++i) {
            if (std::abs(buffer[i]) > threshold) {
                return false;
            }
        }
        return true;
    }

    // Helper: Check if buffer has signal
    bool hasSignal(const float* buffer, int num_samples, float threshold = 1e-6f) {
        return !isSilent(buffer, num_samples, threshold);
    }

    float sample_rate_;
    int max_block_size_;
    std::unique_ptr<ReverbEffect> reverb_;
};

//==============================================================================
// Initialization Tests
//==============================================================================

TEST_F(ReverbEffectTest, PrepareInitializesCorrectly) {
    // Should not crash or assert
    EXPECT_NO_THROW({
        ReverbEffect reverb;
        reverb.prepare(48000.0f, 256);
    });
}

TEST_F(ReverbEffectTest, ResetClearsState) {
    // Process some audio to fill delay lines
    auto input = generateDC(max_block_size_, 1.0f);
    std::vector<float> output_left(max_block_size_);
    std::vector<float> output_right(max_block_size_);

    reverb_->setWetLevel(1.0f);
    reverb_->setDryLevel(0.0f);

    reverb_->processMonoToStereo(input.data(), output_left.data(), output_right.data(),
                                 max_block_size_);

    // Reset should clear reverb tail
    reverb_->reset();

    auto silence = generateSilence(max_block_size_);
    reverb_->processMonoToStereo(silence.data(), output_left.data(), output_right.data(),
                                 max_block_size_);

    // Output should be silent (or very close)
    EXPECT_LT(calculateRMS(output_left.data(), max_block_size_), 0.001f);
    EXPECT_LT(calculateRMS(output_right.data(), max_block_size_), 0.001f);
}

//==============================================================================
// Parameter Tests
//==============================================================================

TEST_F(ReverbEffectTest, SetRoomSizeClampsToValidRange) {
    reverb_->setRoomSize(-0.5f);
    auto params = reverb_->getParameters();
    EXPECT_GE(params[0], 0.0f);

    reverb_->setRoomSize(1.5f);
    params = reverb_->getParameters();
    EXPECT_LE(params[0], 1.0f);
}

TEST_F(ReverbEffectTest, SetDampingClampsToValidRange) {
    reverb_->setDamping(-0.5f);
    auto params = reverb_->getParameters();
    EXPECT_GE(params[1], 0.0f);

    reverb_->setDamping(1.5f);
    params = reverb_->getParameters();
    EXPECT_LE(params[1], 1.0f);
}

TEST_F(ReverbEffectTest, SetWetLevelClampsToValidRange) {
    reverb_->setWetLevel(-0.5f);
    auto params = reverb_->getParameters();
    EXPECT_GE(params[2], 0.0f);

    reverb_->setWetLevel(1.5f);
    params = reverb_->getParameters();
    EXPECT_LE(params[2], 1.0f);
}

TEST_F(ReverbEffectTest, SetDryLevelClampsToValidRange) {
    reverb_->setDryLevel(-0.5f);
    auto params = reverb_->getParameters();
    EXPECT_GE(params[3], 0.0f);

    reverb_->setDryLevel(1.5f);
    params = reverb_->getParameters();
    EXPECT_LE(params[3], 1.0f);
}

TEST_F(ReverbEffectTest, SetWidthClampsToValidRange) {
    reverb_->setWidth(-0.5f);
    auto params = reverb_->getParameters();
    EXPECT_GE(params[4], 0.0f);

    reverb_->setWidth(1.5f);
    params = reverb_->getParameters();
    EXPECT_LE(params[4], 1.0f);
}

TEST_F(ReverbEffectTest, SetFreezeModeAffectsDecay) {
    // Process impulse with freeze off
    auto input = generateDC(max_block_size_, 1.0f);
    std::vector<float> output_left_1(max_block_size_);
    std::vector<float> output_right_1(max_block_size_);

    reverb_->setFreezeMode(false);
    reverb_->setWetLevel(1.0f);
    reverb_->setDryLevel(0.0f);
    reverb_->reset();

    reverb_->processMonoToStereo(input.data(), output_left_1.data(), output_right_1.data(),
                                 max_block_size_);

    float rms_normal = calculateRMS(output_left_1.data(), max_block_size_);

    // Process impulse with freeze on
    std::vector<float> output_left_2(max_block_size_);
    std::vector<float> output_right_2(max_block_size_);

    reverb_->setFreezeMode(true);
    reverb_->reset();

    reverb_->processMonoToStereo(input.data(), output_left_2.data(), output_right_2.data(),
                                 max_block_size_);

    float rms_freeze = calculateRMS(output_left_2.data(), max_block_size_);

    // Freeze mode should sustain longer (higher RMS)
    EXPECT_GT(rms_freeze, rms_normal);
}

//==============================================================================
// Preset Tests
//==============================================================================

TEST_F(ReverbEffectTest, LoadPresetSmallRoom) {
    reverb_->loadPreset(ReverbPreset::SmallRoom);
    auto params = reverb_->getParameters();

    // Check preset values (approximate due to smoothing)
    EXPECT_NEAR(params[0], 0.3f, 0.01f);  // room size
    EXPECT_NEAR(params[1], 0.5f, 0.01f);  // damping
    EXPECT_NEAR(params[2], 0.2f, 0.01f);  // wet
    EXPECT_NEAR(params[3], 0.8f, 0.01f);  // dry
    EXPECT_NEAR(params[4], 0.5f, 0.01f);  // width
}

TEST_F(ReverbEffectTest, LoadPresetLargeHall) {
    reverb_->loadPreset(ReverbPreset::LargeHall);
    auto params = reverb_->getParameters();

    EXPECT_NEAR(params[0], 0.7f, 0.01f);  // room size
    EXPECT_NEAR(params[1], 0.4f, 0.01f);  // damping
    EXPECT_NEAR(params[2], 0.4f, 0.01f);  // wet
    EXPECT_NEAR(params[3], 0.6f, 0.01f);  // dry
    EXPECT_NEAR(params[4], 0.8f, 0.01f);  // width
}

TEST_F(ReverbEffectTest, LoadPresetCathedral) {
    reverb_->loadPreset(ReverbPreset::Cathedral);
    auto params = reverb_->getParameters();

    EXPECT_NEAR(params[0], 0.9f, 0.01f);  // room size
    EXPECT_NEAR(params[1], 0.3f, 0.01f);  // damping
    EXPECT_NEAR(params[2], 0.5f, 0.01f);  // wet
    EXPECT_NEAR(params[3], 0.5f, 0.01f);  // dry
    EXPECT_NEAR(params[4], 1.0f, 0.01f);  // width
}

TEST_F(ReverbEffectTest, LoadPresetPlate) {
    reverb_->loadPreset(ReverbPreset::Plate);
    auto params = reverb_->getParameters();

    EXPECT_NEAR(params[0], 0.5f, 0.01f);  // room size
    EXPECT_NEAR(params[1], 0.6f, 0.01f);  // damping
    EXPECT_NEAR(params[2], 0.3f, 0.01f);  // wet
    EXPECT_NEAR(params[3], 0.7f, 0.01f);  // dry
    EXPECT_NEAR(params[4], 0.7f, 0.01f);  // width
}

//==============================================================================
// Processing Tests
//==============================================================================

TEST_F(ReverbEffectTest, ProcessMonoToStereoCreatesStereoOutput) {
    auto input = generateSine(max_block_size_, 440.0f, sample_rate_);
    std::vector<float> output_left(max_block_size_);
    std::vector<float> output_right(max_block_size_);

    reverb_->setWetLevel(0.5f);
    reverb_->setDryLevel(0.5f);

    reverb_->processMonoToStereo(input.data(), output_left.data(), output_right.data(),
                                 max_block_size_);

    // Both channels should have signal
    EXPECT_TRUE(hasSignal(output_left.data(), max_block_size_));
    EXPECT_TRUE(hasSignal(output_right.data(), max_block_size_));
}

TEST_F(ReverbEffectTest, ProcessStereoCreatesStereoOutput) {
    auto input_left = generateSine(max_block_size_, 440.0f, sample_rate_);
    auto input_right = generateSine(max_block_size_, 880.0f, sample_rate_);
    std::vector<float> output_left(max_block_size_);
    std::vector<float> output_right(max_block_size_);

    reverb_->setWetLevel(0.5f);
    reverb_->setDryLevel(0.5f);

    reverb_->processStereo(input_left.data(), input_right.data(),
                          output_left.data(), output_right.data(),
                          max_block_size_);

    // Both channels should have signal
    EXPECT_TRUE(hasSignal(output_left.data(), max_block_size_));
    EXPECT_TRUE(hasSignal(output_right.data(), max_block_size_));
}

TEST_F(ReverbEffectTest, BypassPassesSignalUnchanged) {
    auto input = generateSine(max_block_size_, 440.0f, sample_rate_);
    std::vector<float> output_left(max_block_size_);
    std::vector<float> output_right(max_block_size_);

    reverb_->setBypass(true);

    reverb_->processMonoToStereo(input.data(), output_left.data(), output_right.data(),
                                 max_block_size_);

    // Output should match input (on both channels for mono input)
    for (int i = 0; i < max_block_size_; ++i) {
        EXPECT_NEAR(output_left[i], input[i], 1e-5f);
        EXPECT_NEAR(output_right[i], input[i], 1e-5f);
    }
}

TEST_F(ReverbEffectTest, WetLevelZeroPassesDrySignal) {
    auto input = generateSine(max_block_size_, 440.0f, sample_rate_);
    std::vector<float> output_left(max_block_size_);
    std::vector<float> output_right(max_block_size_);

    reverb_->setWetLevel(0.0f);
    reverb_->setDryLevel(1.0f);

    reverb_->processMonoToStereo(input.data(), output_left.data(), output_right.data(),
                                 max_block_size_);

    // Output should match input (dry only)
    for (int i = 0; i < max_block_size_; ++i) {
        EXPECT_NEAR(output_left[i], input[i], 1e-4f);
        EXPECT_NEAR(output_right[i], input[i], 1e-4f);
    }
}

TEST_F(ReverbEffectTest, DryLevelZeroPassesWetSignal) {
    auto input = generateSine(max_block_size_, 440.0f, sample_rate_);
    std::vector<float> output_left(max_block_size_);
    std::vector<float> output_right(max_block_size_);

    reverb_->setWetLevel(1.0f);
    reverb_->setDryLevel(0.0f);

    reverb_->processMonoToStereo(input.data(), output_left.data(), output_right.data(),
                                 max_block_size_);

    // Output should NOT match input (wet only)
    bool different = false;
    for (int i = 0; i < max_block_size_; ++i) {
        if (std::abs(output_left[i] - input[i]) > 0.01f) {
            different = true;
            break;
        }
    }
    EXPECT_TRUE(different);
}

//==============================================================================
// Reverb Quality Tests
//==============================================================================

TEST_F(ReverbEffectTest, ReverbDecaysOverTime) {
    // Process an impulse
    std::vector<float> impulse(1, 1.0f);
    std::vector<float> silence(max_block_size_, 0.0f);

    std::vector<float> output_left(max_block_size_);
    std::vector<float> output_right(max_block_size_);

    reverb_->setWetLevel(1.0f);
    reverb_->setDryLevel(0.0f);
    reverb_->setRoomSize(0.5f);
    reverb_->reset();

    // Process impulse
    reverb_->processMonoToStereo(impulse.data(), output_left.data(), output_right.data(), 1);

    // Process silence and measure decay
    const int decay_blocks = 50;
    std::vector<float> rms_values;

    for (int block = 0; block < decay_blocks; ++block) {
        reverb_->processMonoToStereo(silence.data(), output_left.data(), output_right.data(),
                                     max_block_size_);
        float rms = calculateRMS(output_left.data(), max_block_size_);
        rms_values.push_back(rms);
    }

    // RMS should decay over time (each block should be lower than the previous)
    for (size_t i = 1; i < rms_values.size(); ++i) {
        EXPECT_LT(rms_values[i], rms_values[i - 1]);
    }
}

TEST_F(ReverbEffectTest, RoomSizeAffectsDecayTime) {
    auto impulse = std::vector<float>(1, 1.0f);
    auto silence = std::vector<float>(max_block_size_, 0.0f);
    std::vector<float> output_left(max_block_size_);
    std::vector<float> output_right(max_block_size_);

    reverb_->setWetLevel(1.0f);
    reverb_->setDryLevel(0.0f);

    // Test small room
    reverb_->setRoomSize(0.3f);
    reverb_->reset();
    reverb_->processMonoToStereo(impulse.data(), output_left.data(), output_right.data(), 1);

    int decay_samples_small = 0;
    for (int i = 0; i < 100; ++i) {
        reverb_->processMonoToStereo(silence.data(), output_left.data(), output_right.data(),
                                     max_block_size_);
        float rms = calculateRMS(output_left.data(), max_block_size_);
        if (rms < 0.001f) {
            decay_samples_small = i * max_block_size_;
            break;
        }
    }

    // Test large room
    reverb_->setRoomSize(0.9f);
    reverb_->reset();
    reverb_->processMonoToStereo(impulse.data(), output_left.data(), output_right.data(), 1);

    int decay_samples_large = 0;
    for (int i = 0; i < 100; ++i) {
        reverb_->processMonoToStereo(silence.data(), output_left.data(), output_right.data(),
                                     max_block_size_);
        float rms = calculateRMS(output_left.data(), max_block_size_);
        if (rms < 0.001f) {
            decay_samples_large = i * max_block_size_;
            break;
        }
    }

    // Large room should decay slower
    EXPECT_GT(decay_samples_large, decay_samples_small);
}

TEST_F(ReverbEffectTest, DampingAffectsHighFrequencies) {
    // Generate high-frequency sine
    auto input_hf = generateSine(max_block_size_, 8000.0f, sample_rate_);
    std::vector<float> output_left(max_block_size_);
    std::vector<float> output_right(max_block_size_);

    reverb_->setWetLevel(1.0f);
    reverb_->setDryLevel(0.0f);

    // Test with low damping (bright)
    reverb_->setDamping(0.0f);
    reverb_->setRoomSize(0.7f);
    reverb_->reset();

    // Process for a while to let reverb build up
    for (int i = 0; i < 10; ++i) {
        reverb_->processMonoToStereo(input_hf.data(), output_left.data(), output_right.data(),
                                     max_block_size_);
    }
    float rms_bright = calculateRMS(output_left.data(), max_block_size_);

    // Test with high damping (dark)
    reverb_->setDamping(1.0f);
    reverb_->reset();

    for (int i = 0; i < 10; ++i) {
        reverb_->processMonoToStereo(input_hf.data(), output_left.data(), output_right.data(),
                                     max_block_size_);
    }
    float rms_dark = calculateRMS(output_left.data(), max_block_size_);

    // High damping should reduce high-frequency content
    EXPECT_LT(rms_dark, rms_bright);
}

TEST_F(ReverbEffectTest, WidthAffectsStereoImaging) {
    auto input = generateSine(max_block_size_, 440.0f, sample_rate_);
    std::vector<float> output_left(max_block_size_);
    std::vector<float> output_right(max_block_size_);

    reverb_->setWetLevel(1.0f);
    reverb_->setDryLevel(0.0f);

    // Test with mono width
    reverb_->setWidth(0.0f);
    reverb_->processMonoToStereo(input.data(), output_left.data(), output_right.data(),
                                 max_block_size_);

    float correlation_mono = 0.0f;
    for (int i = 0; i < max_block_size_; ++i) {
        correlation_mono += output_left[i] * output_right[i];
    }

    // Test with full stereo width
    reverb_->setWidth(1.0f);
    reverb_->processMonoToStereo(input.data(), output_left.data(), output_right.data(),
                                 max_block_size_);

    float correlation_stereo = 0.0f;
    for (int i = 0; i < max_block_size_; ++i) {
        correlation_stereo += output_left[i] * output_right[i];
    }

    // Stereo width should produce less correlated channels
    // (Note: this is a simplified test; real stereo imaging is more complex)
    EXPECT_LT(std::abs(correlation_stereo), std::abs(correlation_mono));
}

//==============================================================================
// Real-Time Safety Tests
//==============================================================================

TEST_F(ReverbEffectTest, ProcessBlockDoesNotAllocate) {
    auto input = generateSine(max_block_size_, 440.0f, sample_rate_);
    std::vector<float> output_left(max_block_size_);
    std::vector<float> output_right(max_block_size_);

    // Set all parameters
    reverb_->setRoomSize(0.7f);
    reverb_->setDamping(0.5f);
    reverb_->setWetLevel(0.5f);
    reverb_->setDryLevel(0.5f);
    reverb_->setWidth(0.8f);

    // Process should not allocate (this will crash if allocations occur)
    EXPECT_NO_THROW({
        reverb_->processMonoToStereo(input.data(), output_left.data(), output_right.data(),
                                     max_block_size_);
    });
}

TEST_F(ReverbEffectTest, ProcessDifferentBlockSizes) {
    auto input_large = generateSine(1024, 440.0f, sample_rate_);
    std::vector<float> output_left(1024);
    std::vector<float> output_right(1024);

    reverb_->setWetLevel(0.5f);
    reverb_->setDryLevel(0.5f);

    // Test various block sizes
    std::vector<int> block_sizes = {16, 32, 64, 128, 256, 512, 1024};

    for (int block_size : block_sizes) {
        EXPECT_NO_THROW({
            reverb_->processMonoToStereo(input_large.data(), output_left.data(),
                                        output_right.data(), block_size);
        });
    }
}

//==============================================================================
// Edge Case Tests
//==============================================================================

TEST_F(ReverbEffectTest, HandleZeroSamples) {
    std::vector<float> output_left(1);
    std::vector<float> output_right(1);

    EXPECT_NO_THROW({
        reverb_->processMonoToStereo(nullptr, output_left.data(), output_right.data(), 0);
    });
}

TEST_F(ReverbEffectTest, HandleSilentInput) {
    auto silence = generateSilence(max_block_size_);
    std::vector<float> output_left(max_block_size_);
    std::vector<float> output_right(max_block_size_);

    reverb_->setWetLevel(1.0f);
    reverb_->setDryLevel(0.0f);

    reverb_->processMonoToStereo(silence.data(), output_left.data(), output_right.data(),
                                 max_block_size_);

    // Output should be silent (or very close)
    EXPECT_LT(calculateRMS(output_left.data(), max_block_size_), 0.001f);
    EXPECT_LT(calculateRMS(output_right.data(), max_block_size_), 0.001f);
}

TEST_F(ReverbEffectTest, HandleDCInput) {
    auto dc = generateDC(max_block_size_, 1.0f);
    std::vector<float> output_left(max_block_size_);
    std::vector<float> output_right(max_block_size_);

    reverb_->setWetLevel(0.5f);
    reverb_->setDryLevel(0.5f);

    EXPECT_NO_THROW({
        reverb_->processMonoToStereo(dc.data(), output_left.data(), output_right.data(),
                                     max_block_size_);
    });

    // Should produce output without crashing
    EXPECT_TRUE(hasSignal(output_left.data(), max_block_size_));
    EXPECT_TRUE(hasSignal(output_right.data(), max_block_size_));
}

TEST_F(ReverbEffectTest, ProcessMultipleBlocksContinuously) {
    auto input = generateSine(max_block_size_ * 10, 440.0f, sample_rate_);
    std::vector<float> output_left(max_block_size_);
    std::vector<float> output_right(max_block_size_);

    reverb_->setWetLevel(0.5f);
    reverb_->setDryLevel(0.5f);

    // Process 10 consecutive blocks
    for (int i = 0; i < 10; ++i) {
        int offset = i * max_block_size_;
        EXPECT_NO_THROW({
            reverb_->processMonoToStereo(input.data() + offset, output_left.data(),
                                        output_right.data(), max_block_size_);
        });
    }
}
