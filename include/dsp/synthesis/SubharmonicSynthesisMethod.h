/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * SubharmonicSynthesisMethod.h - PureDSP subharmonic synthesis
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#pragma once

#include "ISynthesisMethod.h"
#include "../../src/dsp/SubharmonicGenerator.h"
#include "../../src/dsp/FormantResonator.h"
#include "../../src/dsp/GlottalSource.h"
#include "../../src/dsp/LinearSmoother.h"
#include "../../src/dsp/SpectralEnhancer.h"
#include <memory>
#include <vector>
#include <array>
#include <map>
#include <string>

namespace DSP {

/**
 * @brief Throat singing preset
 */
struct ThroatSingingPreset {
    const char* name;
    const char* description;

    // Fundamental parameters
    float fundamental_freq = 110.0f;
    float subharmonic_ratio = 2.0f;
    float subharmonic_amplitude = 0.5f;

    // Formant parameters
    float melody_formant_freq = 1200.0f;
    float melody_formant_bandwidth = 150.0f;
    float melody_formant_amplitude = 0.7f;

    // Character
    bool ventricular_folds = false;
    bool chest_voice = false;
    bool sharp_resonance = false;

    // Modulation (for pulsed styles)
    float pulse_rate = 0.0f;
    float pulse_depth = 0.0f;
    bool formant_modulation = false;
};

/**
 * @brief PureDSP Subharmonic synthesis method
 *
 * Implements subharmonic synthesis for throat singing and similar techniques.
 * Refactored from ChoirV2::SubharmonicSynthesis to PureDSP namespace.
 *
 * Processing chain:
 * 1. Generate fundamental frequency (glottal source)
 * 2. Generate subharmonic using PLL (SubharmonicGenerator)
 * 3. Apply formant filtering to subharmonic
 * 4. Blend fundamental + subharmonic
 * 5. Add spectral enhancement for brightness
 *
 * Supports:
 * - Multiple subharmonic ratios (0.5, 0.33, 0.25)
 * - Real-time safe (deterministic timing)
 * - Throat singing presets (Tibetan, Tuva, Inuit, etc.)
 * - Ultra-low frequencies (20-80 Hz sub-bass)
 *
 * CRITICAL: Real-time safe - NO allocations in process()
 *
 * Changes from ChoirV2 version:
 * - Uses DSP:: namespace instead of ChoirV2::
 * - prepare() instead of initialize()
 * - process() instead of synthesizeVoice()
 * - No Voice* or Phoneme* dependencies
 * - Standard C++ types only
 * - Uses ChoirV2:: DSP components (FormantResonator, etc.)
 */
class SubharmonicSynthesisMethod : public ISynthesisMethod {
public:
    SubharmonicSynthesisMethod();
    ~SubharmonicSynthesisMethod() override;

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
    const char* getName() const override { return "subharmonic"; }
    MethodStats getStats() const override;

    /**
     * @brief Set throat singing preset
     * @param preset Preset name ("tibetan_sygyt", "tuva_kargyraa", etc.)
     */
    bool setPreset(const std::string& preset);

    /**
     * @brief Set subharmonic ratio
     * @param ratio Division ratio (0.5 = octave down, 0.33 = third down, etc.)
     */
    void setSubharmonicRatio(float ratio);

    /**
     * @brief Set subharmonic mix
     * @param mix Mix amount (0.0-1.0)
     */
    void setSubharmonicMix(float mix);

    /**
     * @brief Enable/disable formant filtering on subharmonic
     * @param enable true to enable
     */
    void setFormantFiltering(bool enable);

    /**
     * @brief Enable/disable spectral enhancement
     * @param enable true to enable
     */
    void setSpectralEnhancement(bool enable);

    /**
     * @brief Get list of available presets
     */
    static std::vector<std::string> getAvailablePresets();

private:
    SynthesisParams params_;

    // DSP components (using ChoirV2:: components)
    std::unique_ptr<ChoirV2::GlottalSource> glottal_source_;
    std::unique_ptr<ChoirV2::SubharmonicGenerator> subharmonic_generator_;
    std::vector<std::unique_ptr<ChoirV2::FormantResonator>> formant_resonators_;
    std::vector<std::unique_ptr<ChoirV2::LinearSmoother>> smoothers_;
    std::unique_ptr<ChoirV2::SpectralEnhancer> spectral_enhancer_;

    // Per-voice state
    struct VoiceState {
        float current_f0;
        float target_f0;
        float subharmonic_ratio;
        float subharmonic_mix;
        float fundamental_amplitude;
        float subharmonic_amplitude;
        std::array<float, 4> formant_frequencies;
        std::array<float, 4> formant_bandwidths;
    };
    std::vector<VoiceState> voice_states_;

    // Internal state
    bool formant_filtering_enabled_;
    bool spectral_enhancement_enabled_;
    ThroatSingingPreset current_preset_;
    MethodStats stats_;

    // Preset management
    static std::map<std::string, ThroatSingingPreset> presets_;
    static bool presets_initialized_;
    static void initializePresets();

    // Helper methods
    void loadPresetsFromJSON();
    void applyPresetToVoice(const ThroatSingingPreset& preset, VoiceState& state);
    void processFundamental(float* output, int32_t num_samples, VoiceState& state);
    void processSubharmonic(float* output, int32_t num_samples, VoiceState& state);
    void applyFormantFiltering(float* audio, int32_t num_samples, VoiceState& state);
    void blendSignals(float* fundamental, float* subharmonic, float* output, int32_t num_samples, VoiceState& state);
    void updateSubharmonicTargets(const void* phoneme_data, VoiceState& state);
    void updateFormantTargets(const void* phoneme_data, VoiceState& state);

    // Preset factory methods
    static ThroatSingingPreset createTibetanSygytPreset();
    static ThroatSingingPreset createTuvaKargyraaPreset();
    static ThroatSingingPreset createInuitKatajjaqPreset();
    static ThroatSingingPreset createSardinianCantuATenorePreset();
    static ThroatSingingPreset createSubhumanDeepPreset();
    static ThroatSingingPreset createBassoProfondoPreset();
};

} // namespace DSP
