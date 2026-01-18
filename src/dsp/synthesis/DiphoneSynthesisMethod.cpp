/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * DiphoneSynthesisMethod.cpp - PureDSP diphone synthesis implementation
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#include "dsp/synthesis/DiphoneSynthesisMethod.h"
#include <algorithm>
#include <cstring>

namespace DSP {

//==============================================================================
// DiphoneSynthesisMethod Implementation
//==============================================================================

DiphoneSynthesisMethod::DiphoneSynthesisMethod()
    : stats_{0.0f, 0, 0}
{
}

bool DiphoneSynthesisMethod::prepare(const SynthesisParams& params)
{
    sample_rate_ = params.sample_rate;
    max_voices_ = params.max_block_size > 0 ? params.max_block_size : 60;

    // Initialize formant resonators (F1-F4)
    for (int32_t i = 0; i < 4; ++i) {
        float freq = 500.0f + i * 1000.0f;
        float bw = 50.0f + i * 30.0f;
        formants_[i].setParameters(freq, bw, sample_rate_);
        formants_[i].reset();

        // Initialize smoothers with transition time
        formant_smoothers_[i].setTimeConstant(params_.transition_duration, sample_rate_);
        formant_smoothers_[i].reset();
    }

    // Initialize diphone states
    diphone_states_.resize(max_voices_);
    for (auto& state : diphone_states_) {
        state = DiphoneState{};
    }

    // Reset state
    phase_ = 0.0f;
    prepared_ = true;

    return true;
}

SynthesisResult DiphoneSynthesisMethod::process(
    float frequency,
    float amplitude,
    const void* phoneme_data,
    float* output,
    int32_t num_samples
)
{
    if (!prepared_ || !output || num_samples <= 0) {
        return {false, "Invalid parameters or not prepared", 0.0f};
    }

    // Extract phoneme index from opaque data
    int32_t phoneme_index = phoneme_data ? *static_cast<const int32_t*>(phoneme_data) : 0;

    // Generate excitation signal
    generateExcitation(output, num_samples, frequency, phoneme_index, 0.0f);

    // Process through formant filter bank (series connection)
    processFormants(output, num_samples);

    // Apply amplitude
    for (int32_t i = 0; i < num_samples; ++i) {
        output[i] *= amplitude;
    }

    // Update statistics
    stats_.voices_processed++;
    stats_.blocks_processed++;

    return {true, "", 0.015f};  // Approximate CPU usage
}

SynthesisResult DiphoneSynthesisMethod::processSIMD(
    const float* frequencies,
    const float* amplitudes,
    const void* const* phoneme_data_array,
    int32_t num_voices,
    float* output,
    int32_t num_samples
)
{
    if (!prepared_ || !frequencies || !amplitudes || !phoneme_data_array || !output || num_samples <= 0) {
        return {false, "Invalid parameters or not prepared", 0.0f};
    }

    if (num_voices <= 0) {
        return {false, "No voices to process", 0.0f};
    }

    // Clear output buffer
    std::memset(output, 0, num_samples * sizeof(float));

    // Process each voice and mix to output
    std::vector<float> voice_buffer(num_samples);

    for (int32_t i = 0; i < num_voices; ++i) {
        auto result = process(frequencies[i], amplitudes[i], phoneme_data_array[i],
                            voice_buffer.data(), num_samples);
        if (!result.success) {
            return result;
        }

        // Mix to output
        for (int32_t j = 0; j < num_samples; ++j) {
            output[j] += voice_buffer[j];
        }
    }

    // Normalize by number of voices to prevent clipping
    float gain = 1.0f / static_cast<float>(num_voices);
    for (int32_t i = 0; i < num_samples; ++i) {
        output[i] *= gain;
    }

    return {true, "", 0.02f * num_voices};  // Approximate CPU usage
}

void DiphoneSynthesisMethod::reset()
{
    // Reset all formant resonators
    for (auto& formant : formants_) {
        formant.reset();
    }

    // Reset all smoothers
    for (auto& smoother : formant_smoothers_) {
        smoother.reset();
    }

    // Reset phase accumulator
    phase_ = 0.0f;

    // Reset diphone states
    for (auto& state : diphone_states_) {
        state = DiphoneState{};
    }
}

DiphoneSynthesisMethod::MethodStats DiphoneSynthesisMethod::getStats() const
{
    return stats_;
}

void DiphoneSynthesisMethod::setTransitionDuration(float duration_seconds)
{
    params_.transition_duration = std::clamp(duration_seconds, 0.01f, 1.0f);

    // Update smoothers
    for (auto& smoother : formant_smoothers_) {
        smoother.setTimeConstant(params_.transition_duration, sample_rate_);
    }
}

void DiphoneSynthesisMethod::setCrossfadeCurve(float curve_power)
{
    params_.crossfade_curve = std::clamp(curve_power, 0.1f, 3.0f);
}

void DiphoneSynthesisMethod::setCoarticulationEnabled(bool enabled)
{
    params_.enable_coarticulation = enabled;
}

void DiphoneSynthesisMethod::startDiphoneTransition(int32_t voice_index, int32_t target_phoneme)
{
    if (voice_index >= 0 && voice_index < static_cast<int32_t>(diphone_states_.size())) {
        auto& state = diphone_states_[voice_index];
        state.target_phoneme_index = target_phoneme;
        state.is_transitioning = true;
        state.position = 0.0f;
    }
}

//==============================================================================
// Internal Processing Methods
//==============================================================================

void DiphoneSynthesisMethod::generateExcitation(
    float* output,
    int32_t num_samples,
    float frequency,
    int32_t phoneme_type,
    float transition_pos
)
{
    (void)transition_pos;  // Unused for now

    // Determine excitation type based on phoneme
    bool voiced = isVoiced(phoneme_type);
    bool fricative = isFricative(phoneme_type);
    bool plosive = isPlosive(phoneme_type);

    for (int32_t i = 0; i < num_samples; ++i) {
        float sample = 0.0f;

        if (plosive && i < static_cast<int32_t>(sample_rate_ * 0.01)) {
            // Plosive burst (first 10ms)
            sample = generateNoiseSample() * 2.0f;
        } else if (fricative) {
            // Fricative: noise only
            sample = generateNoiseSample() * 0.5f;
        } else if (voiced) {
            // Voiced: sawtooth pulse
            sample = generatePulseSample(frequency);
        } else {
            // Unvoiced: noise
            sample = generateNoiseSample() * 0.3f;
        }

        output[i] = sample;
    }
}

void DiphoneSynthesisMethod::processFormants(float* audio, int32_t num_samples)
{
    for (int32_t i = 0; i < num_samples; ++i) {
        // Update smoothed formant frequencies
        float f1 = formant_smoothers_[0].process();
        float f2 = formant_smoothers_[1].process();
        float f3 = formant_smoothers_[2].process();
        float f4 = formant_smoothers_[3].process();

        // Update formant resonator parameters
        formants_[0].setParameters(f1, 60.0f, sample_rate_);
        formants_[1].setParameters(f2, 90.0f, sample_rate_);
        formants_[2].setParameters(f3, 120.0f, sample_rate_);
        formants_[3].setParameters(f4, 150.0f, sample_rate_);

        // Process through formant filters in series
        float sample = audio[i];
        sample = formants_[0].process(sample);
        sample = formants_[1].process(sample);
        sample = formants_[2].process(sample);
        sample = formants_[3].process(sample);

        audio[i] = sample;
    }
}

void DiphoneSynthesisMethod::updateFormantTargets(
    int32_t source_index,
    int32_t target_index,
    float t
)
{
    // Get formant data for source and target
    FormantData source = getPhonemeFormants(source_index);
    FormantData target = getPhonemeFormants(target_index);

    // Interpolate formant frequencies
    FormantData interpolated;
    interpolateFormants(source, target, t, interpolated);

    // Set smoother targets
    formant_smoothers_[0].setTarget(interpolated.frequencies[0]);
    formant_smoothers_[1].setTarget(interpolated.frequencies[1]);
    formant_smoothers_[2].setTarget(interpolated.frequencies[2]);
    formant_smoothers_[3].setTarget(interpolated.frequencies[3]);
}

DiphoneType DiphoneSynthesisMethod::determineDiphoneType(int32_t source, int32_t target) const
{
    // Simple heuristic based on phoneme index
    // 0-10 = vowels, 11+ = consonants
    bool source_is_vowel = (source >= 0 && source <= 10);
    bool target_is_vowel = (target >= 0 && target <= 10);

    if (source_is_vowel && target_is_vowel) {
        return DiphoneType::VV;
    } else if (source_is_vowel && !target_is_vowel) {
        return DiphoneType::VC;
    } else if (!source_is_vowel && target_is_vowel) {
        return DiphoneType::CV;
    } else {
        return DiphoneType::CC;
    }
}

float DiphoneSynthesisMethod::getTransitionRatio(DiphoneType type, float t) const
{
    // Apply temporal alignment based on diphone type
    switch (type) {
        case DiphoneType::CV:
            // Consonant (30%) → Vowel (70%)
            if (t < params_.consonant_ratio) {
                return t / params_.consonant_ratio * 0.5f;  // First half
            } else {
                return 0.5f + (t - params_.consonant_ratio) / params_.vowel_ratio * 0.5f;
            }

        case DiphoneType::VC:
            // Vowel (70%) → Consonant (30%)
            if (t < params_.vowel_ratio) {
                return t / params_.vowel_ratio * 0.5f;  // First half
            } else {
                return 0.5f + (t - params_.vowel_ratio) / params_.consonant_ratio * 0.5f;
            }

        case DiphoneType::VV:
            // Equal distribution
            return t;

        case DiphoneType::CC:
            // Equal distribution
            return t;

        default:
            return t;
    }
}

//==============================================================================
// Excitation Helpers
//==============================================================================

float DiphoneSynthesisMethod::generateNoiseSample()
{
    // Simple linear congruential generator for white noise
    noise_seed_ = noise_seed_ * 1103515245 + 12345;
    return (static_cast<float>(noise_seed_ & 0x7FFF) / 16384.0f) - 1.0f;
}

float DiphoneSynthesisMethod::generatePulseSample(float frequency)
{
    // Generate sawtooth wave
    float phase_increment = frequency / sample_rate_;
    phase_ += phase_increment;
    if (phase_ > 1.0f) {
        phase_ -= 1.0f;
    }

    // Sawtooth: -1 to +1
    return 2.0f * phase_ - 1.0f;
}

bool DiphoneSynthesisMethod::isVoiced(int32_t phoneme_type) const
{
    // Vowels are voiced
    return (phoneme_type >= 0 && phoneme_type <= 10);
}

bool DiphoneSynthesisMethod::isFricative(int32_t phoneme_type) const
{
    // Simple mapping: some consonants are fricatives
    return (phoneme_type == 11 || phoneme_type == 12 || phoneme_type == 13);
}

bool DiphoneSynthesisMethod::isPlosive(int32_t phoneme_type) const
{
    // Simple mapping: some consonants are plosives
    return (phoneme_type == 14 || phoneme_type == 15 || phoneme_type == 16);
}

//==============================================================================
// Formant Interpolation
//==============================================================================

void DiphoneSynthesisMethod::interpolateFormants(
    const FormantData& source,
    const FormantData& target,
    float t,
    FormantData& result
) const
{
    // Linear interpolation of formant frequencies
    result.frequencies[0] = crossfade(source.frequencies[0], target.frequencies[0], t, params_.crossfade_curve);
    result.frequencies[1] = crossfade(source.frequencies[1], target.frequencies[1], t, params_.crossfade_curve);
    result.frequencies[2] = crossfade(source.frequencies[2], target.frequencies[2], t, params_.crossfade_curve);
    result.frequencies[3] = crossfade(source.frequencies[3], target.frequencies[3], t, params_.crossfade_curve);

    // Interpolate bandwidths
    result.bandwidths[0] = crossfade(source.bandwidths[0], target.bandwidths[0], t, params_.crossfade_curve);
    result.bandwidths[1] = crossfade(source.bandwidths[1], target.bandwidths[1], t, params_.crossfade_curve);
    result.bandwidths[2] = crossfade(source.bandwidths[2], target.bandwidths[2], t, params_.crossfade_curve);
    result.bandwidths[3] = crossfade(source.bandwidths[3], target.bandwidths[3], t, params_.crossfade_curve);
}

float DiphoneSynthesisMethod::crossfade(float a, float b, float t, float curve) const
{
    // Power curve for non-linear crossfade
    float t_curve = std::pow(t, curve);
    return a * (1.0f - t_curve) + b * t_curve;
}

//==============================================================================
// Formant Data Lookup
//==============================================================================

FormantData DiphoneSynthesisMethod::getPhonemeFormants(int32_t phoneme_index)
{
    FormantData data;

    // Default formants for vowels (0-10)
    if (phoneme_index >= 0 && phoneme_index <= 10) {
        // Use simplified formant data
        data.frequencies[0] = 500.0f + phoneme_index * 100.0f;
        data.frequencies[1] = 1500.0f + phoneme_index * 50.0f;
        data.frequencies[2] = 2500.0f;
        data.frequencies[3] = 3500.0f;
        data.bandwidths[0] = 60.0f;
        data.bandwidths[1] = 90.0f;
        data.bandwidths[2] = 120.0f;
        data.bandwidths[3] = 130.0f;
    } else {
        // Consonants
        data.frequencies[0] = 5000.0f;
        data.frequencies[1] = 6000.0f;
        data.frequencies[2] = 7000.0f;
        data.frequencies[3] = 8000.0f;
        data.bandwidths[0] = 1000.0f;
        data.bandwidths[1] = 1000.0f;
        data.bandwidths[2] = 1000.0f;
        data.bandwidths[3] = 1000.0f;
    }

    return data;
}

} // namespace DSP
