/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * FormantSynthesisMethod.cpp - PureDSP formant synthesis implementation
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#include "dsp/synthesis/FormantSynthesisMethod.h"
#include <algorithm>
#include <cstring>

namespace DSP {

//==============================================================================
// FormantSynthesisMethod Implementation
//==============================================================================

FormantSynthesisMethod::FormantSynthesisMethod()
    : stats_{0.0f, 0, 0}
{
}

bool FormantSynthesisMethod::prepare(const SynthesisParams& params)
{
    sample_rate_ = params.sample_rate;

    // Initialize formant resonators
    for (int32_t i = 0; i < 5; ++i) {
        formants_[i].setParameters(500.0f + i * 1000.0f, 100.0f, params.sample_rate);
        formants_[i].reset();

        // Initialize smoothers with transition time
        formant_smoothers_[i].setTimeConstant(transition_time_, params.sample_rate);
        formant_smoothers_[i].reset();
    }

    // Initialize formant targets to default
    current_formants_ = getDefaultFormants();
    target_formants_ = getDefaultFormants();

    // Initialize smoothers to default frequencies
    formant_smoothers_[0].setTargetImmediate(current_formants_.f1);
    formant_smoothers_[1].setTargetImmediate(current_formants_.f2);
    formant_smoothers_[2].setTargetImmediate(current_formants_.f3);
    formant_smoothers_[3].setTargetImmediate(current_formants_.f4);
    formant_smoothers_[4].setTargetImmediate(current_formants_.f5);

    // Reset state
    phase_ = 0.0f;
    vibrato_phase_ = 0.0f;
    prepared_ = true;

    return true;
}

SynthesisResult FormantSynthesisMethod::process(
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
    int32_t vowel_index = phoneme_data ? *static_cast<const int32_t*>(phoneme_data) : 10; // Default to schwa

    // Update formant targets for new phoneme
    updateFormantTargets(vowel_index);

    // Determine excitation type
    ExcitationType excitation_type = getExcitationType(vowel_index);

    // Generate excitation signal
    generateExcitation(output, num_samples, frequency, excitation_type);

    // Process through formant filter bank (series connection)
    processFormants(output, num_samples);

    // Apply amplitude
    for (int32_t i = 0; i < num_samples; ++i) {
        output[i] *= amplitude;
    }

    // Update statistics
    stats_.voices_processed++;
    stats_.blocks_processed++;

    return {true, "", 0.01f};  // Approximate CPU usage
}

SynthesisResult FormantSynthesisMethod::processSIMD(
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
    // NOTE: For true SIMD optimization, we would process multiple voices
    // in parallel using SIMD instructions. This is a simplified implementation.
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

void FormantSynthesisMethod::reset()
{
    // Reset all formant resonators
    for (auto& formant : formants_) {
        formant.reset();
    }

    // Reset all smoothers
    for (auto& smoother : formant_smoothers_) {
        smoother.reset();
    }

    // Reset phase accumulators
    phase_ = 0.0f;
    vibrato_phase_ = 0.0f;

    // Reset formants to default
    current_formants_ = getDefaultFormants();
    target_formants_ = getDefaultFormants();
}

FormantSynthesisMethod::MethodStats FormantSynthesisMethod::getStats() const
{
    return stats_;
}

void FormantSynthesisMethod::setTransitionTime(float time_ms)
{
    transition_time_ = time_ms / 1000.0f;  // Convert ms to seconds

    // Update smoothers
    for (auto& smoother : formant_smoothers_) {
        smoother.setTimeConstant(transition_time_, sample_rate_);
    }
}

void FormantSynthesisMethod::setExcitationMix(float pulse_mix)
{
    pulse_mix_ = std::clamp(pulse_mix, 0.0f, 1.0f);
}

void FormantSynthesisMethod::setVibratoParams(const VibratoParams& params)
{
    vibrato_params_ = params;
}

void FormantSynthesisMethod::getFormantFrequencies(int32_t vowel_index, FormantDef& formants) const
{
    formants = getVowelFormants(vowel_index);
}

//==============================================================================
// Internal Processing Methods
//==============================================================================

void FormantSynthesisMethod::generateExcitation(
    float* output,
    int32_t num_samples,
    float frequency,
    ExcitationType type
)
{
    for (int32_t i = 0; i < num_samples; ++i) {
        float sample = 0.0f;

        switch (type) {
            case ExcitationType::Pulse:
                // Pure sawtooth pulse train
                sample = generatePulseSample(frequency);
                break;

            case ExcitationType::Noise:
                // Filtered noise (for fricatives)
                sample = generateNoiseSample() * noise_gain_;
                break;

            case ExcitationType::Mixed:
                // Mixed pulse and noise (breathy voice)
                {
                    float pulse = generatePulseSample(frequency);
                    float noise = generateNoiseSample() * noise_gain_;
                    sample = pulse * pulse_mix_ + noise * (1.0f - pulse_mix_);
                }
                break;

            case ExcitationType::Burst:
                // Impulse burst (for plosives)
                // Generate a short burst at the beginning
                if (i < static_cast<int32_t>(sample_rate_ * 0.01)) {  // 10ms burst
                    sample = generateNoiseSample() * 2.0f;
                } else {
                    sample = 0.0f;
                }
                break;
        }

        output[i] = sample;
    }
}

void FormantSynthesisMethod::processFormants(float* audio, int32_t num_samples)
{
    for (int32_t i = 0; i < num_samples; ++i) {
        // Update smoothed formant frequencies
        float f1 = formant_smoothers_[0].process();
        float f2 = formant_smoothers_[1].process();
        float f3 = formant_smoothers_[2].process();
        float f4 = formant_smoothers_[3].process();
        float f5 = formant_smoothers_[4].process();

        // Apply vibrato if enabled
        if (vibrato_params_.enabled) {
            applyVibrato(i, f1, f2);
        }

        // Update formant resonator parameters
        formants_[0].setParameters(f1, current_formants_.bandwidth1, sample_rate_);
        formants_[1].setParameters(f2, current_formants_.bandwidth2, sample_rate_);
        formants_[2].setParameters(f3, current_formants_.bandwidth3, sample_rate_);
        formants_[3].setParameters(f4, current_formants_.bandwidth4, sample_rate_);
        formants_[4].setParameters(f5, current_formants_.bandwidth5, sample_rate_);

        // Process through formant filters in series
        float sample = audio[i];
        sample = formants_[0].process(sample);
        sample = formants_[1].process(sample);
        sample = formants_[2].process(sample);
        sample = formants_[3].process(sample);
        sample = formants_[4].process(sample);

        audio[i] = sample;
    }
}

void FormantSynthesisMethod::updateFormantTargets(int32_t vowel_index)
{
    // Get target formants for phoneme
    FormantDef target_formants = getVowelFormants(vowel_index);

    // Update target formants
    target_formants_ = target_formants;

    // Set smoother targets
    formant_smoothers_[0].setTarget(target_formants_.f1);
    formant_smoothers_[1].setTarget(target_formants_.f2);
    formant_smoothers_[2].setTarget(target_formants_.f3);
    formant_smoothers_[3].setTarget(target_formants_.f4);
    formant_smoothers_[4].setTarget(target_formants_.f5);
}

void FormantSynthesisMethod::applyVibrato(int32_t /* sample_index */, float& f1, float& f2)
{
    // Update vibrato LFO phase
    float phase_increment = (2.0f * M_PI * vibrato_params_.rate) / sample_rate_;
    vibrato_phase_ += phase_increment;
    if (vibrato_phase_ > 2.0f * M_PI) {
        vibrato_phase_ -= 2.0f * M_PI;
    }

    // Calculate vibrato modulation
    float vibrato = std::sin(vibrato_phase_) * vibrato_params_.depth;

    // Apply to F1 and F2 (in semitones, convert to frequency ratio)
    float f1_ratio = std::pow(2.0f, vibrato / 12.0f);
    float f2_ratio = std::pow(2.0f, vibrato / 12.0f);

    f1 *= f1_ratio;
    f2 *= f2_ratio;
}

float FormantSynthesisMethod::generateNoiseSample()
{
    // Simple linear congruential generator for white noise
    // Using a fixed seed for reproducibility
    noise_seed_ = noise_seed_ * 1103515245 + 12345;
    return (static_cast<float>(noise_seed_ & 0x7FFF) / 16384.0f) - 1.0f;
}

float FormantSynthesisMethod::generatePulseSample(float frequency)
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

ExcitationType FormantSynthesisMethod::getExcitationType(int32_t vowel_index) const
{
    // Simple mapping: all vowels use pulse excitation
    // In a full implementation, this would check phoneme type
    (void)vowel_index;
    return ExcitationType::Pulse;
}

//==============================================================================
// Formant Data Lookup
//==============================================================================

FormantDef FormantSynthesisMethod::getVowelFormants(int32_t vowel_index)
{
    // Map vowel index to formant frequencies
    // Using Peterson & Barney (1952) data for adult male speakers

    switch (vowel_index) {
        case 0: return VowelFormants::I_EE;     // /i/ "ee"
        case 1: return VowelFormants::I_IH;     // /I/ "ih"
        case 2: return VowelFormants::E_EH;     // /e/ "eh"
        case 3: return VowelFormants::E_AE;     // /E/ "ae"
        case 4: return VowelFormants::A_AH;     // /a/ "ah"
        case 5: return VowelFormants::A_AW;     // /A/ "ah"
        case 6: return VowelFormants::O_OH;     // /o/ "oh"
        case 7: return VowelFormants::O_AW;     // /O/ "aw"
        case 8: return VowelFormants::U_OO;     // /u/ "oo"
        case 9: return VowelFormants::U_UH;     // /U/ "uh"
        case 10: return VowelFormants::SCHWA;   // /ə/ "uh"
        default: return VowelFormants::SCHWA;   // Default to schwa
    }
}

FormantDef FormantSynthesisMethod::getDefaultFormants()
{
    // Neutral vowel (schwa-like)
    return VowelFormants::SCHWA;
}

} // namespace DSP
