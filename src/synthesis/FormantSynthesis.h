/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * FormantSynthesis.h - Formant-based vocal synthesis
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#pragma once

#include "ISynthesisMethod.h"
#include <memory>

namespace ChoirV2 {

// Forward declarations
class FormantResonator;
class GlottalSource;
class LinearSmoother;
class SpectralEnhancer;

/**
 * @brief Formant synthesis method
 *
 * Traditional formant synthesis using:
 * - Glottal source (pulse train)
 * - Formant resonators (filter bank)
 * - Parameter smoothing (prevents clicks)
 * - Spectral enhancement (improves quality)
 *
 * This is the most mature and reliable synthesis method.
 */
class FormantSynthesis : public ISynthesisMethod {
public:
    FormantSynthesis();
    ~FormantSynthesis() override;

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

    /**
     * @brief Set formant transition smoothing time
     * @param time Smoothing time (seconds)
     */
    void setTransitionSmoothing(float time);

    /**
     * @brief Enable/disable spectral enhancement
     * @param enable true to enable
     */
    void setSpectralEnhancement(bool enable);

private:
    SynthesisParams params_;

    // DSP components
    std::unique_ptr<GlottalSource> glottal_source_;
    std::vector<std::unique_ptr<FormantResonator>> formant_resonators_;
    std::vector<std::unique_ptr<LinearSmoother>> smoothers_;
    std::unique_ptr<SpectralEnhancer> spectral_enhancer_;

    // Per-voice state (SIMD batch)
    struct VoiceState {
        std::array<float, 4> formant_frequencies;  // F1, F2, F3, F4
        std::array<float, 4> formant_bandwidths;   // B1, B2, B3, B4
        float current_f0;                          // Current fundamental frequency
        float target_f0;                           // Target fundamental frequency
        float amplitude;                           // Current amplitude
    };
    std::vector<VoiceState> voice_states_;

    // Internal state
    bool spectral_enhancement_enabled_;
    float transition_smoothing_time_;
    MethodStats stats_;

    // Helper methods
    void updateFormantTargets(
        const Phoneme* phoneme,
        VoiceState& state
    );
    void processFormantTransition(
        VoiceState& state,
        float delta_time
    );
    void applySpectralEnhancement(
        float* audio,
        int num_samples
    );
};

} // namespace ChoirV2
