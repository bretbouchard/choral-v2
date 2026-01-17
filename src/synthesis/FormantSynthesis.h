/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * FormantSynthesis.h - Formant-based vocal synthesis method
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#pragma once

#include "ISynthesisMethod.h"
#include "../dsp/FormantResonator.h"
#include "../dsp/LinearSmoother.h"
#include <array>
#include <memory>
#include <vector>

namespace ChoirV2 {

/**
 * @brief Formant definition for vowel synthesis (5 formants)
 */
struct FormantDef {
    float f1 = 500.0f;         // First formant (Hz)
    float f2 = 1500.0f;        // Second formant (Hz)
    float f3 = 2500.0f;        // Third formant (Hz)
    float f4 = 3500.0f;        // Fourth formant (Hz)
    float f5 = 4500.0f;        // Fifth formant (Hz)
    float bandwidth1 = 60.0f;  // F1 bandwidth (Hz)
    float bandwidth2 = 90.0f;  // F2 bandwidth (Hz)
    float bandwidth3 = 120.0f; // F3 bandwidth (Hz)
    float bandwidth4 = 130.0f; // F4 bandwidth (Hz)
    float bandwidth5 = 140.0f; // F5 bandwidth (Hz)
};

/**
 * @brief Excitation source type
 */
enum class ExcitationType {
    Pulse,      // Glottal pulse (voiced)
    Noise,      // Noise (unvoiced/fricatives)
    Mixed,      // Pulse + noise (mixed voicing)
    Burst       // Impulse burst (plosives)
};

/**
 * @brief Vibrato parameters
 */
struct VibratoParams {
    float rate = 6.0f;          // LFO rate (Hz) - typically 5-7 Hz
    float depth = 1.0f;         // Depth in semitones - typically 0.5-2 semitones
    bool enabled = false;
};

/**
 * @brief Formant-based vocal synthesis
 *
 * Implements traditional formant synthesis using parallel resonators.
 *
 * Synthesis pipeline:
 * 1. Generate excitation (sawtooth pulse + filtered noise)
 * 2. Apply formant filters in series (F1 → F2 → F3 → F4 → F5)
 * 3. Mix output with appropriate gain
 * 4. Handle consonant bursts (noise bursts for plosives)
 * 5. Support vibrato (LFO on F1/F2, 5-7 Hz)
 *
 * CRITICAL: Real-time safe - NO allocations in process()
 */
class FormantSynthesis : public ISynthesisMethod {
public:
    FormantSynthesis();
    ~FormantSynthesis() override = default;

    // ISynthesisMethod interface
    bool initialize(const SynthesisParams& params) override;
    SynthesisResult synthesizeVoice(
        Voice* voice,
        const Phoneme* phoneme,
        float* output,
        int num_samples
    ) override;
    SynthesisResult synthesizeVoicesSIMD(
        const std::vector<Voice*>& voices,
        const std::vector<const Phoneme*>& phonemes,
        float* output,
        int num_samples
    ) override;
    void reset() override;
    std::string getName() const override { return "formant"; }
    MethodStats getStats() const override;

    // Formant synthesis specific
    void setTransitionTime(float time_ms);
    void setExcitationMix(float pulse_mix);
    void setVibratoParams(const VibratoParams& params);
    void getFormantFrequencies(const Phoneme* phoneme, FormantDef& formants) const;

private:
    // DSP components - 5 parallel formant resonators (F1-F5)
    std::array<FormantResonator, 5> formants_;           // F1-F5 resonators
    std::array<LinearSmoother, 5> formant_smoothers_;    // Formant frequency smoothing

    // Excitation generation state
    float phase_ = 0.0f;              // Sawtooth oscillator phase
    float vibrato_phase_ = 0.0f;      // Vibrato LFO phase
    unsigned int noise_seed_ = 12345; // Noise seed (for reproducibility)

    // Parameters
    float transition_time_ = 0.05f;   // Formant transition smoothing (seconds)
    float pulse_mix_ = 0.5f;          // Pulse vs noise mix (0-1)
    float noise_gain_ = 0.3f;         // Noise component gain
    VibratoParams vibrato_params_;
    FormantDef current_formants_;
    FormantDef target_formants_;

    // State
    bool initialized_ = false;
    MethodStats stats_;

    // Internal processing methods
    void generateExcitation(float* output, int num_samples, float frequency, ExcitationType type);
    void processFormants(float* audio, int num_samples);
    void updateFormantTargets(const Phoneme* phoneme);
    void applyVibrato(int sample_index, float& f1, float& f2);
    float generateNoiseSample();
    float generatePulseSample(float frequency);
    ExcitationType getExcitationType(const Phoneme* phoneme) const;

    // Formant data lookup
    static FormantDef getVowelFormants(const std::string& ipa_symbol);
    static FormantDef getConsonantFormants(const std::string& ipa_symbol);
    static FormantDef getDefaultFormants();
};

/**
 * @brief Formant frequencies for common vowels (Peterson & Barney, 1952)
 */
namespace VowelFormants {
    constexpr FormantDef i = { 270, 2300, 3000, 3500, 4500, 60, 90, 120, 130, 140 };  // /i/ "ee"
    constexpr FormantDef I = { 390, 2000, 2800, 3500, 4500, 50, 80, 120, 130, 140 };  // /I/ "ih"
    constexpr FormantDef e = { 530, 1800, 2500, 3500, 4500, 50, 80, 120, 130, 140 };  // /e/ "eh"
    constexpr FormantDef E = { 660, 1700, 2600, 3500, 4500, 60, 90, 120, 130, 140 };  // /E/ "ae"
    constexpr FormantDef a = { 730, 1090, 2440, 3500, 4500, 80, 100, 120, 130, 140 }; // /a/ "ah"
    constexpr FormantDef A = { 570, 1200, 2500, 3500, 4500, 70, 100, 120, 130, 140 }; // /A/ "ah"
    constexpr FormantDef o = { 570, 840, 2500, 3500, 4500, 50, 80, 120, 130, 140 };   // /o/ "oh"
    constexpr FormantDef O = { 440, 1020, 2500, 3500, 4500, 50, 80, 120, 130, 140 };  // /O/ "aw"
    constexpr FormantDef u = { 300, 870, 2250, 3500, 4500, 50, 80, 120, 130, 140 };  // /u/ "oo"
    constexpr FormantDef U = { 440, 1020, 2500, 3500, 4500, 50, 80, 120, 130, 140 };  // /U/ "uh"
    constexpr FormantDef schwa = { 500, 1500, 2500, 3500, 4500, 60, 90, 120, 130, 140 }; // /ə/ "uh"
}

/**
 * @brief Consonant formant transitions
 */
namespace ConsonantFormants {
    constexpr FormantDef s = { 5000, 6000, 7000, 8000, 9000, 1000, 1000, 1000, 1000, 1000 }; // /s/
    constexpr FormantDef sh = { 3000, 4000, 5000, 6000, 7000, 1000, 1000, 1000, 1000, 1000 }; // /ʃ/
    constexpr FormantDef f = { 4000, 5000, 6000, 7000, 8000, 1000, 1000, 1000, 1000, 1000 }; // /f/
    constexpr FormantDef m = { 300, 1200, 2500, 3500, 4500, 50, 100, 120, 130, 140 }; // /m/
    constexpr FormantDef n = { 350, 1400, 2500, 3500, 4500, 50, 100, 120, 130, 140 }; // /n/
    constexpr FormantDef p = { 300, 1200, 2500, 3500, 4500, 50, 80, 120, 130, 140 };  // /p/
    constexpr FormantDef t = { 400, 1500, 2500, 3500, 4500, 50, 80, 120, 130, 140 };  // /t/
    constexpr FormantDef k = { 500, 1800, 2500, 3500, 4500, 50, 80, 120, 130, 140 };  // /k/
}

} // namespace ChoirV2
