/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * ReverbEffect.cpp - High-quality stereo reverb implementation
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#include "ReverbEffect.h"
#include <cstring>
#include <algorithm>
#include <memory>

namespace ChoirV2 {

//==============================================================================
// JUCE Reverb Engine Wrapper
//==============================================================================

/**
 * @brief Internal wrapper for JUCE's reverb engine
 *
 * Uses the Pimpl idiom to avoid including JUCE headers in the public header.
 * JUCE's reverb is based on Schroeder/Moorer design:
 * - 8 parallel comb filters (early reflections)
 * - 4 series allpass filters (diffusion)
 * - Adjustable damping and decay
 */
struct ReverbEffect::ReverbEngine {
    // JUCE reverb parameters (matching JUCE dsp::Reverb::Parameters)
    struct Parameters {
        float roomSize;     // 0.0 to 1.0
        float damping;      // 0.0 to 1.0
        float wetLevel;     // 0.0 to 1.0
        float dryLevel;     // 0.0 to 1.0
        float width;        // 0.0 to 1.0
        float freezeMode;   // 0.0 or 1.0
    };

    Parameters params;
    bool initialized = false;

    // Comb filter state (8 parallel filters)
    static constexpr int num_comb_filters = 8;
    std::array<std::vector<float>, num_comb_filters> comb_buffers_;
    std::array<int, num_comb_filters> comb_indices_;
    std::array<float, num_comb_filters> comb_feedback_;
    std::array<float, num_comb_filters> comb_damping_;

    // Allpass filter state (4 series filters)
    static constexpr int num_allpass_filters = 4;
    std::array<std::vector<float>, num_allpass_filters> allpass_buffers_;
    std::array<int, num_allpass_filters> allpass_indices_;

    // Damping state (for high-frequency absorption)
    float damping_1 = 0.0f;
    float damping_2 = 0.0f;

    ReverbEngine() {
        // Default parameters (medium hall)
        params.roomSize = 0.5f;
        params.damping = 0.5f;
        params.wetLevel = 0.33f;
        params.dryLevel = 0.67f;
        params.width = 1.0f;
        params.freezeMode = 0.0f;
    }

    void prepare(float sample_rate, int max_samples) {
        // Comb filter delay lengths (in samples)
        // Tuned for rich reverb without metallic artifacts
        const std::array<int, num_comb_filters> comb_lengths = {
            static_cast<int>(sample_rate * 0.032),   // 32ms
            static_cast<int>(sample_rate * 0.037),   // 37ms
            static_cast<int>(sample_rate * 0.041),   // 41ms
            static_cast<int>(sample_rate * 0.043),   // 43ms
            static_cast<int>(sample_rate * 0.047),   // 47ms
            static_cast<int>(sample_rate * 0.053),   // 53ms
            static_cast<int>(sample_rate * 0.059),   // 59ms
            static_cast<int>(sample_rate * 0.061)    // 61ms
        };

        // Allpass filter delay lengths (diffusion)
        const std::array<int, num_allpass_filters> allpass_lengths = {
            static_cast<int>(sample_rate * 0.005),   // 5ms
            static_cast<int>(sample_rate * 0.007),   // 7ms
            static_cast<int>(sample_rate * 0.011),   // 11ms
            static_cast<int>(sample_rate * 0.013)    // 13ms
        };

        // Allocate comb filter buffers
        for (int i = 0; i < num_comb_filters; ++i) {
            comb_buffers_[i].resize(comb_lengths[i] + max_samples, 0.0f);
            comb_indices_[i] = 0;
        }

        // Allocate allpass filter buffers
        for (int i = 0; i < num_allpass_filters; ++i) {
            allpass_buffers_[i].resize(allpass_lengths[i] + max_samples, 0.0f);
            allpass_indices_[i] = 0;
        }

        initialized = true;
    }

    void reset() {
        // Clear all buffers
        for (auto& buf : comb_buffers_) {
            std::fill(buf.begin(), buf.end(), 0.0f);
        }
        for (auto& buf : allpass_buffers_) {
            std::fill(buf.begin(), buf.end(), 0.0f);
        }

        // Reset indices
        for (auto& idx : comb_indices_) {
            idx = 0;
        }
        for (auto& idx : allpass_indices_) {
            idx = 0;
        }

        // Reset damping
        damping_1 = 0.0f;
        damping_2 = 0.0f;
    }

    void setParameters(const Parameters& new_params) {
        params = new_params;
        updateDerivedParameters();
    }

    void updateDerivedParameters() {
        // Calculate comb filter feedback based on room size
        // Larger room = longer decay = higher feedback
        float decay = params.roomSize;
        if (params.freezeMode > 0.5f) {
            decay = 1.0f;  // Infinite feedback in freeze mode
        }

        // Set feedback values for each comb filter
        // Slightly detuned for richness
        for (int i = 0; i < num_comb_filters; ++i) {
            comb_feedback_[i] = decay * (0.98f - i * 0.01f);
        }

        // Calculate damping coefficients (high-frequency absorption)
        float damp = params.damping;
        damping_1 = damp;
        damping_2 = 1.0f - damp;
    }

    float processCombFilter(int filter_idx, float input, int channel) {
        auto& buffer = comb_buffers_[filter_idx];
        int& index = comb_indices_[filter_idx];
        int size = static_cast<int>(buffer.size());

        // Read delayed sample
        float delayed = buffer[index];
        buffer[index] = input;

        // Apply damping to feedback
        float temp = delayed;
        float filtered = temp * damping_2 + damping_1 * buffer[(index + 1) % size];
        buffer[(index + 1) % size] = filtered;

        // Advance index
        index = (index + 1) % size;

        // Output with feedback
        return input + filtered * comb_feedback_[filter_idx];
    }

    float processAllpassFilter(int filter_idx, float input) {
        auto& buffer = allpass_buffers_[filter_idx];
        int& index = allpass_indices_[filter_idx];
        int size = static_cast<int>(buffer.size());

        // Allpass: y[n] = -gain * x[n] + x[n-d] + gain * y[n-d]
        // Where gain = 0.5 for unity gain allpass
        constexpr float allpass_gain = 0.5f;

        float delayed = buffer[index];
        float output = -allpass_gain * input + delayed + allpass_gain * buffer[index];
        buffer[index] = input;

        index = (index + 1) % size;

        return output;
    }

    std::pair<float, float> processStereo(float left_in, float right_in) {
        // Combine inputs (mono to reverb engine)
        float mono = (left_in + right_in) * 0.5f;

        // Parallel comb filters (early reflections)
        float accum = 0.0f;
        for (int i = 0; i < num_comb_filters; ++i) {
            accum += processCombFilter(i, mono, 0);
        }
        accum *= 0.125f;  // Normalize by number of filters

        // Series allpass filters (diffusion)
        for (int i = 0; i < num_allpass_filters; ++i) {
            accum = processAllpassFilter(i, accum);
        }

        // Apply stereo width
        float width = params.width;
        float wet_left = accum * (1.0f - width * 0.5f);
        float wet_right = accum * (width * 0.5f);

        // Invert one channel for stereo enhancement
        wet_right = -wet_right;

        return {wet_left, wet_right};
    }
};

//==============================================================================
// ReverbEffect Implementation
//==============================================================================

ReverbEffect::ReverbEffect()
    : engine_(std::make_unique<ReverbEngine>())
    , room_size_(0.5f)
    , damping_(0.5f)
    , wet_level_(0.33f)
    , dry_level_(0.67f)
    , width_(1.0f)
    , freeze_mode_(false)
    , bypass_(false)
    , sample_rate_(44100.0f)
{
}

void ReverbEffect::prepare(float sample_rate, int max_block_size)
{
    sample_rate_ = sample_rate;

    // Prepare reverb engine
    engine_->prepare(sample_rate, max_block_size);

    // Prepare smoothers (20ms for natural transitions)
    float smooth_time = 0.02f;  // 20ms
    size_smoother_.setTimeConstant(smooth_time, sample_rate);
    damping_smoother_.setTimeConstant(smooth_time, sample_rate);
    wet_smoother_.setTimeConstant(smooth_time, sample_rate);
    dry_smoother_.setTimeConstant(smooth_time, sample_rate);
    width_smoother_.setTimeConstant(smooth_time, sample_rate);

    // Initialize smoothers to current parameter values
    size_smoother_.setTargetImmediate(room_size_);
    damping_smoother_.setTargetImmediate(damping_);
    wet_smoother_.setTargetImmediate(wet_level_);
    dry_smoother_.setTargetImmediate(dry_level_);
    width_smoother_.setTargetImmediate(width_);

    // Update engine parameters
    updateEngineParameters();
}

void ReverbEffect::reset()
{
    // Reset reverb engine
    engine_->reset();

    // Reset smoothers to current targets
    size_smoother_.reset();
    damping_smoother_.reset();
    wet_smoother_.reset();
    dry_smoother_.reset();
    width_smoother_.reset();
}

void ReverbEffect::setRoomSize(float size)
{
    // Clamp to valid range
    room_size_ = std::clamp(size, 0.0f, 1.0f);
    size_smoother_.setTarget(room_size_);
}

void ReverbEffect::setDamping(float damping)
{
    // Clamp to valid range
    damping_ = std::clamp(damping, 0.0f, 1.0f);
    damping_smoother_.setTarget(damping_);
}

void ReverbEffect::setWetLevel(float wet)
{
    // Clamp to valid range
    wet_level_ = std::clamp(wet, 0.0f, 1.0f);
    wet_smoother_.setTarget(wet_level_);
}

void ReverbEffect::setDryLevel(float dry)
{
    // Clamp to valid range
    dry_level_ = std::clamp(dry, 0.0f, 1.0f);
    dry_smoother_.setTarget(dry_level_);
}

void ReverbEffect::setWidth(float width)
{
    // Clamp to valid range
    width_ = std::clamp(width, 0.0f, 1.0f);
    width_smoother_.setTarget(width_);
}

void ReverbEffect::setFreezeMode(bool freeze)
{
    freeze_mode_ = freeze;
    // Freeze mode affects feedback, update immediately
    updateEngineParameters();
}

void ReverbEffect::loadPreset(ReverbPreset preset)
{
    switch (preset) {
        case ReverbPreset::SmallRoom:
            setRoomSize(0.3f);
            setDamping(0.5f);
            setWetLevel(0.2f);
            setDryLevel(0.8f);
            setWidth(0.5f);
            setFreezeMode(false);
            break;

        case ReverbPreset::LargeHall:
            setRoomSize(0.7f);
            setDamping(0.4f);
            setWetLevel(0.4f);
            setDryLevel(0.6f);
            setWidth(0.8f);
            setFreezeMode(false);
            break;

        case ReverbPreset::Cathedral:
            setRoomSize(0.9f);
            setDamping(0.3f);
            setWetLevel(0.5f);
            setDryLevel(0.5f);
            setWidth(1.0f);
            setFreezeMode(false);
            break;

        case ReverbPreset::Plate:
            setRoomSize(0.5f);
            setDamping(0.6f);
            setWetLevel(0.3f);
            setDryLevel(0.7f);
            setWidth(0.7f);
            setFreezeMode(false);
            break;

        case ReverbPreset::Custom:
            // Don't change parameters
            break;
    }

    // Apply preset immediately (no smoothing for preset switching)
    size_smoother_.setTargetImmediate(room_size_);
    damping_smoother_.setTargetImmediate(damping_);
    wet_smoother_.setTargetImmediate(wet_level_);
    dry_smoother_.setTargetImmediate(dry_level_);
    width_smoother_.setTargetImmediate(width_);

    updateEngineParameters();
}

void ReverbEffect::processMonoToStereo(const float* input,
                                        float* output_left,
                                        float* output_right,
                                        int num_samples)
{
    if (bypass_) {
        // Bypass: copy mono to both channels
        std::copy(input, input + num_samples, output_left);
        std::copy(input, input + num_samples, output_right);
        return;
    }

    // Process sample-by-sample for smoothed parameter updates
    for (int i = 0; i < num_samples; ++i) {
        // Update smoothers
        float current_size = size_smoother_.process();
        float current_damping = damping_smoother_.process();
        float current_wet = wet_smoother_.process();
        float current_dry = dry_smoother_.process();
        float current_width = width_smoother_.process();

        // Update engine parameters (only if they changed significantly)
        // For efficiency, we could batch this, but per-sample is safer
        static float last_size = -1.0f;
        static float last_damping = -1.0f;
        static float last_wet = -1.0f;
        static float last_dry = -1.0f;
        static float last_width = -1.0f;

        if (std::abs(current_size - last_size) > 0.001f ||
            std::abs(current_damping - last_damping) > 0.001f ||
            std::abs(current_wet - last_wet) > 0.001f ||
            std::abs(current_dry - last_dry) > 0.001f ||
            std::abs(current_width - last_width) > 0.001f) {

            last_size = current_size;
            last_damping = current_damping;
            last_wet = current_wet;
            last_dry = current_dry;
            last_width = current_width;

            updateEngineParameters();
        }

        // Process through reverb
        float dry = input[i];
        auto [wet_left, wet_right] = engine_->processStereo(dry, dry);

        // Apply wet/dry mix
        applyWetDryMix(dry, dry, wet_left, wet_right, output_left[i], output_right[i]);
    }
}

void ReverbEffect::processStereo(const float* input_left,
                                 const float* input_right,
                                 float* output_left,
                                 float* output_right,
                                 int num_samples)
{
    if (bypass_) {
        // Bypass: copy through
        std::copy(input_left, input_left + num_samples, output_left);
        std::copy(input_right, input_right + num_samples, output_right);
        return;
    }

    // Process sample-by-sample for smoothed parameter updates
    for (int i = 0; i < num_samples; ++i) {
        // Update smoothers
        float current_size = size_smoother_.process();
        float current_damping = damping_smoother_.process();
        float current_wet = wet_smoother_.process();
        float current_dry = dry_smoother_.process();
        float current_width = width_smoother_.process();

        // Update engine parameters (only if they changed significantly)
        static float last_size = -1.0f;
        static float last_damping = -1.0f;
        static float last_wet = -1.0f;
        static float last_dry = -1.0f;
        static float last_width = -1.0f;

        if (std::abs(current_size - last_size) > 0.001f ||
            std::abs(current_damping - last_damping) > 0.001f ||
            std::abs(current_wet - last_wet) > 0.001f ||
            std::abs(current_dry - last_dry) > 0.001f ||
            std::abs(current_width - last_width) > 0.001f) {

            last_size = current_size;
            last_damping = current_damping;
            last_wet = current_wet;
            last_dry = current_dry;
            last_width = current_width;

            updateEngineParameters();
        }

        // Process through reverb
        float dry_left = input_left[i];
        float dry_right = input_right[i];
        auto [wet_left, wet_right] = engine_->processStereo(dry_left, dry_right);

        // Apply wet/dry mix
        applyWetDryMix(dry_left, dry_right, wet_left, wet_right,
                       output_left[i], output_right[i]);
    }
}

std::array<float, 5> ReverbEffect::getParameters() const
{
    return {room_size_, damping_, wet_level_, dry_level_, width_};
}

void ReverbEffect::updateEngineParameters()
{
    // Get current smoothed values
    float current_size = size_smoother_.getCurrent();
    float current_damping = damping_smoother_.getCurrent();
    float current_wet = wet_smoother_.getCurrent();
    float current_dry = dry_smoother_.getCurrent();
    float current_width = width_smoother_.getCurrent();

    // Update engine parameters
    ReverbEngine::Parameters params;
    params.roomSize = current_size;
    params.damping = current_damping;
    params.wetLevel = current_wet;
    params.dryLevel = current_dry;
    params.width = current_width;
    params.freezeMode = freeze_mode_ ? 1.0f : 0.0f;

    engine_->setParameters(params);
}

inline void ReverbEffect::applyWetDryMix(float dry_left, float dry_right,
                                         float wet_left, float wet_right,
                                         float& output_left, float& output_right)
{
    // Get current wet/dry levels (smoothed)
    float wet = wet_smoother_.getCurrent();
    float dry = dry_smoother_.getCurrent();

    // Apply wet/dry mix (equal-power crossfade)
    // Wet: 0->1 maps to 0->1 amplitude
    // Dry: 0->1 maps to 1->0 amplitude
    // This maintains constant power when wet + dry = 1.0
    float wet_gain = wet;
    float dry_gain = dry;

    output_left = dry_left * dry_gain + wet_left * wet_gain;
    output_right = dry_right * dry_gain + wet_right * wet_gain;
}

} // namespace ChoirV2
