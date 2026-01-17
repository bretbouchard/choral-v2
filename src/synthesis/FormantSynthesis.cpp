/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * FormantSynthesis.cpp - Formant-based vocal synthesis implementation
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#include "FormantSynthesis.h"
#include "../core/Voice.h"
#include "../core/PhonemeDatabase.h"
#include <cmath>
#include <algorithm>
#include <cstring>

namespace ChoirV2 {

//==============================================================================
// FormantSynthesis Implementation
//==============================================================================

FormantSynthesis::FormantSynthesis()
    : stats_{0.0f, 0, 0}
{
}

bool FormantSynthesis::initialize(const SynthesisParams& params)
{
    // Initialize formant resonators
    for (int i = 0; i < 5; ++i) {
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
    initialized_ = true;

    return true;
}

SynthesisResult FormantSynthesis::synthesizeVoice(
    Voice* voice,
    const Phoneme* phoneme,
    float* output,
    int num_samples
)
{
    if (!initialized_ || !voice || !phoneme || !output || num_samples <= 0) {
        return {false, "Invalid parameters or not initialized", 0.0f};
    }

    // Update formant targets for new phoneme
    updateFormantTargets(phoneme);

    // Get voice parameters
    float frequency = voice->getFrequency();
    float amplitude = voice->getAmplitude();

    // Determine excitation type
    ExcitationType excitation_type = getExcitationType(phoneme);

    // Generate excitation signal
    generateExcitation(output, num_samples, frequency, excitation_type);

    // Process through formant filter bank (series connection)
    processFormants(output, num_samples);

    // Apply amplitude
    for (int i = 0; i < num_samples; ++i) {
        output[i] *= amplitude;
    }

    // Update statistics
    stats_.voices_processed++;
    stats_.blocks_processed++;

    return {true, "", 0.01f};  // Approximate CPU usage
}

SynthesisResult FormantSynthesis::synthesizeVoicesSIMD(
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
    // NOTE: For true SIMD optimization, we would process multiple voices
    // in parallel using SIMD instructions. This is a simplified implementation.
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

void FormantSynthesis::reset()
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

FormantSynthesis::MethodStats FormantSynthesis::getStats() const
{
    return stats_;
}

void FormantSynthesis::setTransitionTime(float time_ms)
{
    transition_time_ = time_ms / 1000.0f;  // Convert ms to seconds
}

void FormantSynthesis::setExcitationMix(float pulse_mix)
{
    pulse_mix_ = std::clamp(pulse_mix, 0.0f, 1.0f);
}

void FormantSynthesis::setVibratoParams(const VibratoParams& params)
{
    vibrato_params_ = params;
}

void FormantSynthesis::getFormantFrequencies(const Phoneme* phoneme, FormantDef& formants) const
{
    if (phoneme->category == PhonemeCategory::Vowel) {
        formants = getVowelFormants(phoneme->ipa);
    } else if (phoneme->category == PhonemeCategory::Consonant) {
        formants = getConsonantFormants(phoneme->ipa);
    } else {
        formants = getDefaultFormants();
    }
}

//==============================================================================
// Internal Processing Methods
//==============================================================================

void FormantSynthesis::generateExcitation(
    float* output,
    int num_samples,
    float frequency,
    ExcitationType type
)
{
    float sample_rate = 48000.0f;  // TODO: Get from params

    for (int i = 0; i < num_samples; ++i) {
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
                if (i < static_cast<int>(sample_rate * 0.01)) {  // 10ms burst
                    sample = generateNoiseSample() * 2.0f;
                } else {
                    sample = 0.0f;
                }
                break;
        }

        output[i] = sample;
    }
}

void FormantSynthesis::processFormants(float* audio, int num_samples)
{
    float sample_rate = 48000.0f;  // TODO: Get from params

    for (int i = 0; i < num_samples; ++i) {
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
        formants_[0].setParameters(f1, current_formants_.bandwidth1, sample_rate);
        formants_[1].setParameters(f2, current_formants_.bandwidth2, sample_rate);
        formants_[2].setParameters(f3, current_formants_.bandwidth3, sample_rate);
        formants_[3].setParameters(f4, current_formants_.bandwidth4, sample_rate);
        formants_[4].setParameters(f5, current_formants_.bandwidth5, sample_rate);

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

void FormantSynthesis::updateFormantTargets(const Phoneme* phoneme)
{
    // Get target formants for phoneme
    FormantDef target_formants;
    getFormantFrequencies(phoneme, target_formants);

    // Update target formants
    target_formants_ = target_formants;

    // Set smoother targets
    formant_smoothers_[0].setTarget(target_formants_.f1);
    formant_smoothers_[1].setTarget(target_formants_.f2);
    formant_smoothers_[2].setTarget(target_formants_.f3);
    formant_smoothers_[3].setTarget(target_formants_.f4);
    formant_smoothers_[4].setTarget(target_formants_.f5);
}

void FormantSynthesis::applyVibrato(int /* sample_index */, float& f1, float& f2)
{
    float sample_rate = 48000.0f;  // TODO: Get from params

    // Update vibrato LFO phase
    float phase_increment = (2.0f * M_PI * vibrato_params_.rate) / sample_rate;
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

float FormantSynthesis::generateNoiseSample()
{
    // Simple linear congruential generator for white noise
    // Using a fixed seed for reproducibility
    noise_seed_ = noise_seed_ * 1103515245 + 12345;
    return (static_cast<float>(noise_seed_ & 0x7FFF) / 16384.0f) - 1.0f;
}

float FormantSynthesis::generatePulseSample(float frequency)
{
    float sample_rate = 48000.0f;  // TODO: Get from params

    // Generate sawtooth wave
    float phase_increment = frequency / sample_rate;
    phase_ += phase_increment;
    if (phase_ > 1.0f) {
        phase_ -= 1.0f;
    }

    // Sawtooth: -1 to +1
    return 2.0f * phase_ - 1.0f;
}

ExcitationType FormantSynthesis::getExcitationType(const Phoneme* phoneme) const
{
    if (!phoneme) {
        return ExcitationType::Pulse;  // Default to pulse
    }

    // Use articulatory features to determine excitation type
    if (!phoneme->articulatory.is_voiced) {
        return ExcitationType::Noise;  // Unvoiced = noise only
    }

    // Check for specific phoneme types
    const std::string& ipa = phoneme->ipa;

    // Fricatives (mixed voicing)
    if (ipa == "s" || ipa == "ʃ" || ipa == "f" || ipa == "v" || ipa == "z" || ipa == "ʒ") {
        return ExcitationType::Mixed;
    }

    // Plosives
    if (ipa == "p" || ipa == "t" || ipa == "k" || ipa == "b" || ipa == "d" || ipa == "g") {
        return ExcitationType::Burst;
    }

    // Nasals
    if (ipa == "m" || ipa == "n" || ipa == "ŋ") {
        return ExcitationType::Pulse;  // Nasals use pulse excitation
    }

    // Default to pulse for vowels
    return ExcitationType::Pulse;
}

//==============================================================================
// Formant Data Lookup
//==============================================================================

FormantDef FormantSynthesis::getVowelFormants(const std::string& ipa_symbol)
{
    // Map IPA symbols to formant frequencies
    // Using Peterson & Barney (1952) data for adult male speakers

    if (ipa_symbol == "i") return VowelFormants::i;      // /i/ "ee"
    if (ipa_symbol == "ɪ") return VowelFormants::I;      // /I/ "ih"
    if (ipa_symbol == "e") return VowelFormants::e;      // /e/ "eh"
    if (ipa_symbol == "ɛ") return VowelFormants::E;      // /E/ "ae"
    if (ipa_symbol == "æ") return VowelFormants::E;      // /E/ "ae"
    if (ipa_symbol == "a") return VowelFormants::a;      // /a/ "ah"
    if (ipa_symbol == "ɑ") return VowelFormants::A;      // /A/ "ah"
    if (ipa_symbol == "o") return VowelFormants::o;      // /o/ "oh"
    if (ipa_symbol == "ɔ") return VowelFormants::O;      // /O/ "aw"
    if (ipa_symbol == "u") return VowelFormants::u;      // /u/ "oo"
    if (ipa_symbol == "ʊ") return VowelFormants::U;      // /U/ "uh"
    if (ipa_symbol == "ə") return VowelFormants::schwa;  // /ə/ "uh"

    // Default to schwa if not found
    return VowelFormants::schwa;
}

FormantDef FormantSynthesis::getConsonantFormants(const std::string& ipa_symbol)
{
    // Map IPA symbols to consonant formant transitions

    // Fricatives
    if (ipa_symbol == "s") return ConsonantFormants::s;
    if (ipa_symbol == "ʃ") return ConsonantFormants::sh;
    if (ipa_symbol == "f") return ConsonantFormants::f;

    // Nasals
    if (ipa_symbol == "m") return ConsonantFormants::m;
    if (ipa_symbol == "n") return ConsonantFormants::n;

    // Plosives
    if (ipa_symbol == "p") return ConsonantFormants::p;
    if (ipa_symbol == "t") return ConsonantFormants::t;
    if (ipa_symbol == "k") return ConsonantFormants::k;

    // Default to neutral
    return getDefaultFormants();
}

FormantDef FormantSynthesis::getDefaultFormants()
{
    // Neutral vowel (schwa-like)
    return VowelFormants::schwa;
}

} // namespace ChoirV2
