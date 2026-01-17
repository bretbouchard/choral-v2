/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * DiphoneSynthesis.cpp - Diphone-based vocal synthesis implementation
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#include "DiphoneSynthesis.h"
#include <cmath>
#include <algorithm>
#include <cstring>

namespace ChoirV2 {

//==============================================================================
// DiphoneSynthesis Implementation
//==============================================================================

DiphoneSynthesis::DiphoneSynthesis()
    : stats_{0.0f, 0, 0}
{
}

bool DiphoneSynthesis::initialize(const SynthesisParams& params)
{
    sample_rate_ = params.sample_rate;

    // Initialize formant resonators (F1-F4)
    for (int i = 0; i < 4; ++i) {
        float freq = 500.0f + i * 1000.0f;
        float bw = 50.0f + i * 30.0f;
        formants_[i].setParameters(freq, bw, sample_rate_);
        formants_[i].reset();

        // Initialize smoothers with transition time
        formant_smoothers_[i].setTimeConstant(params_.transition_duration, sample_rate_);
        formant_smoothers_[i].reset();
    }

    // Initialize diphone states (max 60 voices)
    diphone_states_.resize(60);
    for (auto& state : diphone_states_) {
        state = DiphoneState{};
    }

    // Reset state
    phase_ = 0.0f;
    initialized_ = true;

    return true;
}

SynthesisResult DiphoneSynthesis::synthesizeVoice(
    Voice* voice,
    const Phoneme* phoneme,
    float* output,
    int num_samples
)
{
    if (!initialized_ || !voice || !phoneme || !output || num_samples <= 0) {
        return {false, "Invalid parameters or not initialized", 0.0f};
    }

    // Get voice parameters
    float frequency = voice->getFrequency();
    float amplitude = voice->getAmplitude();

    // Generate excitation signal
    generateExcitation(output, num_samples, frequency, phoneme, 0.0f);

    // Process through formant filter bank (series connection)
    processFormants(output, num_samples);

    // Apply amplitude
    for (int i = 0; i < num_samples; ++i) {
        output[i] *= amplitude;
    }

    // Update statistics
    stats_.voices_processed++;
    stats_.blocks_processed++;

    return {true, "", 0.015f};  // Approximate CPU usage
}

SynthesisResult DiphoneSynthesis::synthesizeVoicesSIMD(
    const std::vector<Voice*>& voices,
    const std::vector<const Phoneme*>& phonemes,
    float* output,
    int num_samples
)
{
    if (!initialized_ || voices.empty() || phonemes.empty() || !output || num_samples <= 0) {
        return {false, "Invalid parameters or not initialized", 0.0f};
    }

    if (voices.size() != phonemes.size()) {
        return {false, "Voice and phoneme vectors must have same size", 0.0f};
    }

    // Clear output buffer
    std::memset(output, 0, num_samples * sizeof(float));

    // Process each voice and mix to output
    std::vector<float> voice_buffer(num_samples);

    for (size_t i = 0; i < voices.size(); ++i) {
        auto result = synthesizeVoice(voices[i], phonemes[i], voice_buffer.data(), num_samples);
        if (!result.success) {
            return result;
        }

        // Mix to output
        for (int j = 0; j < num_samples; ++j) {
            output[j] += voice_buffer[j];
        }
    }

    // Normalize by number of voices to prevent clipping
    float gain = 1.0f / static_cast<float>(voices.size());
    for (int i = 0; i < num_samples; ++i) {
        output[i] *= gain;
    }

    return {true, "", 0.02f * voices.size()};  // Approximate CPU usage
}

void DiphoneSynthesis::reset()
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

std::string DiphoneSynthesis::getName() const
{
    return "diphone";
}

MethodStats DiphoneSynthesis::getStats() const
{
    return stats_;
}

void DiphoneSynthesis::setTransitionDuration(float duration_seconds)
{
    params_.transition_duration = std::clamp(duration_seconds, 0.01f, 1.0f);

    // Update smoothers
    for (auto& smoother : formant_smoothers_) {
        smoother.setTimeConstant(params_.transition_duration, sample_rate_);
    }
}

void DiphoneSynthesis::setCrossfadeCurve(float curve_power)
{
    params_.crossfade_curve = std::clamp(curve_power, 0.1f, 3.0f);
}

void DiphoneSynthesis::setCoarticulationEnabled(bool enabled)
{
    params_.enable_coarticulation = enabled;
}

void DiphoneSynthesis::startDiphoneTransition(Voice* voice, const Phoneme* target)
{
    // TODO: Implement voice tracking for diphone transitions
    // This requires a voice ID system to track which voice is transitioning
    // For now, transitions are handled per-sample in synthesizeVoice
}

//==============================================================================
// Internal Processing Methods
//==============================================================================

void DiphoneSynthesis::generateExcitation(
    float* output,
    int num_samples,
    float frequency,
    const Phoneme* phoneme,
    float transition_pos
)
{
    // Determine excitation type based on phoneme
    bool voiced = isVoiced(phoneme);
    bool fricative = isFricative(phoneme);
    bool plosive = isPlosive(phoneme);

    for (int i = 0; i < num_samples; ++i) {
        float sample = 0.0f;

        if (plosive && i < static_cast<int>(sample_rate_ * 0.01)) {
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

void DiphoneSynthesis::processFormants(float* audio, int num_samples)
{
    for (int i = 0; i < num_samples; ++i) {
        // Update smoothed formant frequencies
        float f1 = formant_smoothers_[0].process();
        float f2 = formant_smoothers_[1].process();
        float f3 = formant_smoothers_[2].process();
        float f4 = formant_smoothers_[3].process();

        // Update formant resonator parameters (bandwidths from current phoneme)
        // NOTE: In a full implementation, these would come from interpolated formant data
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

void DiphoneSynthesis::updateFormantTargets(
    const Phoneme* source,
    const Phoneme* target,
    float t
)
{
    // Interpolate formant frequencies
    FormantData interpolated;
    interpolateFormants(source->formants, target->formants, t, interpolated);

    // Set smoother targets
    formant_smoothers_[0].setTarget(interpolated.f1);
    formant_smoothers_[1].setTarget(interpolated.f2);
    formant_smoothers_[2].setTarget(interpolated.f3);
    formant_smoothers_[3].setTarget(interpolated.f4);
}

DiphoneType DiphoneSynthesis::determineDiphoneType(
    const Phoneme* source,
    const Phoneme* target
) const
{
    // Simple heuristic based on phoneme category
    // In a full implementation, this would use the PhonemeDatabase

    bool source_is_vowel = (source->category == "vowel");
    bool target_is_vowel = (target->category == "vowel");

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

float DiphoneSynthesis::getTransitionRatio(DiphoneType type, float t) const
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

float DiphoneSynthesis::generateNoiseSample()
{
    // Simple linear congruential generator for white noise
    noise_seed_ = noise_seed_ * 1103515245 + 12345;
    return (static_cast<float>(noise_seed_ & 0x7FFF) / 16384.0f) - 1.0f;
}

float DiphoneSynthesis::generatePulseSample(float frequency)
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

bool DiphoneSynthesis::isVoiced(const Phoneme* phoneme) const
{
    // Use articulatory features
    return phoneme->articulatory.is_voiced;
}

bool DiphoneSynthesis::isFricative(const Phoneme* phoneme) const
{
    // Check IPA symbol for fricatives
    const std::string& ipa = phoneme->ipa;
    return (ipa == "s" || ipa == "ʃ" || ipa == "f" || ipa == "v" ||
            ipa == "z" || ipa == "ʒ" || ipa == "θ" || ipa == "ð" ||
            ipa == "h" || ipa == "x");
}

bool DiphoneSynthesis::isPlosive(const Phoneme* phoneme) const
{
    // Check IPA symbol for plosives
    const std::string& ipa = phoneme->ipa;
    return (ipa == "p" || ipa == "t" || ipa == "k" ||
            ipa == "b" || ipa == "d" || ipa == "g");
}

//==============================================================================
// Formant Interpolation
//==============================================================================

void DiphoneSynthesis::interpolateFormants(
    const FormantData& source,
    const FormantData& target,
    float t,
    FormantData& result
) const
{
    // Linear interpolation of formant frequencies
    result.f1 = crossfade(source.f1, target.f1, t, params_.crossfade_curve);
    result.f2 = crossfade(source.f2, target.f2, t, params_.crossfade_curve);
    result.f3 = crossfade(source.f3, target.f3, t, params_.crossfade_curve);
    result.f4 = crossfade(source.f4, target.f4, t, params_.crossfade_curve);

    // Interpolate bandwidths
    result.bw1 = crossfade(source.bw1, target.bw1, t, params_.crossfade_curve);
    result.bw2 = crossfade(source.bw2, target.bw2, t, params_.crossfade_curve);
    result.bw3 = crossfade(source.bw3, target.bw3, t, params_.crossfade_curve);
    result.bw4 = crossfade(source.bw4, target.bw4, t, params_.crossfade_curve);
}

float DiphoneSynthesis::crossfade(float a, float b, float t, float curve) const
{
    // Power curve for non-linear crossfade
    // curve = 1.0: linear
    // curve > 1.0: slower transition, more abrupt change
    // curve < 1.0: faster transition, smoother change
    float t_curve = std::pow(t, curve);
    return a * (1.0f - t_curve) + b * t_curve;
}

} // namespace ChoirV2
