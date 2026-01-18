/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * DiphoneSynthesisMethod.h - PureDSP diphone-based vocal synthesis
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#pragma once

#include "ISynthesisMethod.h"
#include "../../src/dsp/FormantResonator.h"
#include "../../src/dsp/LinearSmoother.h"
#include <array>
#include <cmath>
#include <vector>

namespace DSP {

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
    int32_t source_phoneme_index = -1;   // Source phoneme index
    int32_t target_phoneme_index = -1;   // Target phoneme index
    DiphoneType type;                    // Transition type
    float position = 0.0f;               // Current position (0-1)
    float duration = 0.15f;              // Total duration (seconds)
    bool is_transitioning = false;       // Currently in transition?
};

/**
 * @brief Formant data structure (for interpolation)
 */
struct FormantData {
    std::array<float, 4> frequencies;    // F1-F4 frequencies
    std::array<float, 4> bandwidths;     // F1-F4 bandwidths
};

/**
 * @brief PureDSP Diphone-based vocal synthesis
 *
 * Implements concatenative synthesis using diphone units.
 * Refactored from ChoirV2::DiphoneSynthesis to PureDSP namespace.
 *
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
 *
 * Changes from ChoirV2 version:
 * - Uses DSP:: namespace instead of ChoirV2::
 * - prepare() instead of initialize()
 * - process() instead of synthesizeVoice()
 * - No Voice* or Phoneme* dependencies
 * - Standard C++ types only
 */
class DiphoneSynthesisMethod : public ISynthesisMethod {
public:
    DiphoneSynthesisMethod();
    ~DiphoneSynthesisMethod() override = default;

    // ISynthesisMethod interface
    bool prepare(const SynthesisParams& params) override;
    SynthesisResult process(
        float frequency,
        float amplitude,
        const void* phoneme_data,
        float* output,
        int32_t num_samples
    ) override;
    SynthesisResult processSIMD(
        const float* frequencies,
        const float* amplitudes,
        const void* const* phoneme_data_array,
        int32_t num_voices,
        float* output,
        int32_t num_samples
    ) override;
    void reset() override;
    const char* getName() const override { return "diphone"; }
    MethodStats getStats() const override;

    // Diphone synthesis specific
    void setTransitionDuration(float duration_seconds);
    void setCrossfadeCurve(float curve_power);
    void setCoarticulationEnabled(bool enabled);
    void startDiphoneTransition(int32_t voice_index, int32_t target_phoneme);

private:
    // DSP components - 4 formant resonators (F1-F4)
    std::array<ChoirV2::FormantResonator, 4> formants_;
    std::array<ChoirV2::LinearSmoother, 4> formant_smoothers_;

    // Excitation generation state
    float phase_ = 0.0f;              // Sawtooth oscillator phase
    uint32_t noise_seed_ = 12345;     // Noise seed

    // Diphone transition state (per-voice)
    std::vector<DiphoneState> diphone_states_;

    // Parameters
    DiphoneParams params_;
    float sample_rate_ = 48000.0f;
    int32_t max_voices_ = 60;
    MethodStats stats_;

    // State
    bool prepared_ = false;

    // Internal processing methods
    void generateExcitation(float* output, int32_t num_samples, float frequency,
                           int32_t phoneme_type, float transition_pos);
    void processFormants(float* audio, int32_t num_samples);
    void updateFormantTargets(int32_t source_index, int32_t target_index, float t);
    DiphoneType determineDiphoneType(int32_t source, int32_t target) const;
    float getTransitionRatio(DiphoneType type, float t) const;

    // Excitation helpers
    float generateNoiseSample();
    float generatePulseSample(float frequency);
    bool isVoiced(int32_t phoneme_type) const;
    bool isFricative(int32_t phoneme_type) const;
    bool isPlosive(int32_t phoneme_type) const;

    // Formant interpolation
    void interpolateFormants(const FormantData& source, const FormantData& target,
                            float t, FormantData& result) const;
    float crossfade(float a, float b, float t, float curve) const;

    // Formant data lookup
    static FormantData getPhonemeFormants(int32_t phoneme_index);
};

} // namespace DSP
