/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * DiphoneSynthesis.h - Diphone-based vocal synthesis method
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#pragma once

#include "ISynthesisMethod.h"
#include "../dsp/FormantResonator.h"
#include "../dsp/LinearSmoother.h"
#include "../core/Phoneme.h"
#include <array>
#include <memory>
#include <vector>

namespace ChoirV2 {

/**
 * @brief Diphone transition types
 */
enum class DiphoneType {
    CV,     // Consonant-Vowel (e.g., /k/ → /æ/ in "cat")
    VC,     // Vowel-Consonant (e.g., /æ/ → /t/ in "at")
    VV,     // Vowel-Vowel diphthong (e.g., /o/ → /ɪ/ in "boy")
    CC      // Consonant-Consonant cluster (e.g., /st/ in "stop")
};

/**
 * @brief Diphone transition parameters
 */
struct DiphoneParams {
    float transition_duration = 0.15f;    // Transition time (seconds)
    float consonant_ratio = 0.3f;         // Consonant portion in CV diphone
    float vowel_ratio = 0.7f;             // Vowel portion in CV diphone
    float crossfade_curve = 1.0f;         // Crossfade curve power (1.0 = linear)
    bool enable_coarticulation = true;    // Enable coarticulation effects
};

/**
 * @brief Diphone state for active transitions
 */
struct DiphoneState {
    const Phoneme* source_phoneme = nullptr;  // Source phoneme
    const Phoneme* target_phoneme = nullptr;  // Target phoneme
    DiphoneType type;                         // Transition type
    float position = 0.0f;                    // Current position (0-1)
    float duration = 0.15f;                   // Total duration (seconds)
    bool is_transitioning = false;            // Currently in transition?
};

/**
 * @brief Diphone-based vocal synthesis
 *
 * Implements concatenative synthesis using diphone units.
 * Creates smooth transitions between phoneme pairs through:
 * 1. Formant frequency interpolation (F1-F4)
 * 2. Crossfade between source and target
 * 3. Coarticulation modeling
 * 4. Temporal alignment for different diphone types
 *
 * Synthesis pipeline:
 * 1. Determine diphone type (CV, VC, VV, CC)
 * 2. Get source and target phonemes from database
 * 3. Create interpolated diphone using formant interpolation
 * 4. Apply formant crossfade over transition duration
 * 5. Generate excitation based on phoneme types
 * 6. Apply formant filtering with interpolated frequencies
 * 7. Handle temporal alignment (consonant before/after vowel)
 *
 * Transition timing:
 * - CV diphone: consonant (30%) → vowel (70%)
 * - VC diphone: vowel (70%) → consonant (30%)
 * - VV diphone: first vowel (50%) → second vowel (50%)
 * - CC cluster: equal distribution
 *
 * CRITICAL: Real-time safe - NO allocations in process()
 */
class DiphoneSynthesis : public ISynthesisMethod {
public:
    DiphoneSynthesis();
    ~DiphoneSynthesis() override = default;

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
    std::string getName() const override { return "diphone"; }
    MethodStats getStats() const override;

    // Diphone synthesis specific
    void setTransitionDuration(float duration_seconds);
    void setCrossfadeCurve(float curve_power);
    void setCoarticulationEnabled(bool enabled);
    void startDiphoneTransition(Voice* voice, const Phoneme* target);

private:
    // DSP components - 4 formant resonators (F1-F4)
    std::array<FormantResonator, 4> formants_;           // F1-F4 resonators
    std::array<LinearSmoother, 4> formant_smoothers_;    // Formant frequency smoothing

    // Excitation generation state
    float phase_ = 0.0f;              // Sawtooth oscillator phase
    unsigned int noise_seed_ = 12345; // Noise seed

    // Diphone transition state (per-voice)
    std::vector<DiphoneState> diphone_states_;

    // Parameters
    DiphoneParams params_;
    float sample_rate_ = 48000.0f;
    MethodStats stats_;

    // State
    bool initialized_ = false;

    // Internal processing methods
    void generateExcitation(float* output, int num_samples, float frequency,
                           const Phoneme* phoneme, float transition_pos);
    void processFormants(float* audio, int num_samples);
    void updateFormantTargets(const Phoneme* source, const Phoneme* target, float t);
    DiphoneType determineDiphoneType(const Phoneme* source, const Phoneme* target) const;
    float getTransitionRatio(DiphoneType type, float t) const;

    // Excitation helpers
    float generateNoiseSample();
    float generatePulseSample(float frequency);
    bool isVoiced(const Phoneme* phoneme) const;
    bool isFricative(const Phoneme* phoneme) const;
    bool isPlosive(const Phoneme* phoneme) const;

    // Formant interpolation
    void interpolateFormants(const FormantData& source, const FormantData& target,
                            float t, FormantData& result) const;
    float crossfade(float a, float b, float t, float curve) const;
};

} // namespace ChoirV2
