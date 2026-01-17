/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * SubharmonicSynthesis.cpp - Subharmonic synthesis implementation
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#include "SubharmonicSynthesis.h"
#include "../dsp/SubharmonicGenerator.h"
#include "../dsp/FormantResonator.h"
#include "../dsp/GlottalSource.h"
#include "../dsp/LinearSmoother.h"
#include "../dsp/SpectralEnhancer.h"
#include <cstring>
#include <cmath>
#include <algorithm>

namespace ChoirV2 {

// Static member initialization
std::map<std::string, ThroatSingingPreset> SubharmonicSynthesis::presets_;
bool SubharmonicSynthesis::presets_initialized_ = false;

SubharmonicSynthesis::SubharmonicSynthesis()
    : formant_filtering_enabled_(true)
    , spectral_enhancement_enabled_(true)
    , current_preset_()
    , stats_{0.0f, 0, 0}
{
    // Initialize presets if not already done
    if (!presets_initialized_) {
        initializePresets();
        presets_initialized_ = true;
    }
}

SubharmonicSynthesis::~SubharmonicSynthesis() {
    // Smart pointers handle cleanup
}

bool SubharmonicSynthesis::initialize(const SynthesisParams& params) {
    params_ = params;

    // Create glottal source for fundamental
    glottal_source_ = std::make_unique<GlottalSource>();
    glottal_source_->setSampleRate(params.sample_rate);
    glottal_source_->setModel(GlottalModel::Rosenberg);
    glottal_source_->setPulseShape(0.7f, 0.3f, 0.1f);

    // Create subharmonic generator with PLL
    subharmonic_generator_ = std::make_unique<SubharmonicGenerator>();
    subharmonic_generator_->setRatio(0.5f);  // Default: octave down
    subharmonic_generator_->setMix(0.5f);

    // Create formant resonators (4 formants)
    formant_resonators_.clear();
    for (int i = 0; i < 4; ++i) {
        auto resonator = std::make_unique<FormantResonator>();
        resonator->setParameters(500.0f + i * 1000.0f, 50.0f + i * 30.0f, params.sample_rate);
        formant_resonators_.push_back(std::move(resonator));
    }

    // Create parameter smoothers
    smoothers_.clear();
    for (int i = 0; i < 4; ++i) {
        auto smoother = std::make_unique<LinearSmoother>();
        smoother->setTimeConstant(0.005f, params.sample_rate);  // 5ms smoothing
        smoothers_.push_back(std::move(smoother));
    }

    // Create spectral enhancer
    spectral_enhancer_ = std::make_unique<SpectralEnhancer>();
    int fft_size = 2048;
    if (!spectral_enhancer_->initialize(params.sample_rate, fft_size)) {
        return false;
    }
    spectral_enhancer_->setEnhancementAmount(0.5f);

    return true;
}

SynthesisResult SubharmonicSynthesis::synthesizeVoice(
    Voice* voice,
    const Phoneme* phoneme,
    float* output,
    int num_samples
) {
    SynthesisResult result;
    result.success = true;
    result.cpu_usage = 0.0f;

    if (!voice || !phoneme || !output || num_samples <= 0) {
        result.success = false;
        result.error_message = "Invalid parameters";
        return result;
    }

    // Get or create voice state
    if (voice_states_.size() <= 0) {
        voice_states_.resize(1);
    }
    VoiceState& state = voice_states_[0];

    // Update targets from phoneme
    updateSubharmonicTargets(phoneme, state);
    updateFormantTargets(phoneme, state);

    // Update voice frequency
    state.target_f0 = voice->getFrequency();

    // Allocate temporary buffers (REAL-TIME SAFE: preallocate in production)
    // For now, use stack allocation for small buffers
    const int max_block_size = 256;
    float* fundamental = nullptr;
    float* subharmonic = nullptr;

    if (num_samples <= max_block_size) {
        // Use stack allocation for small blocks (real-time safe)
        float fundamental_stack[max_block_size];
        float subharmonic_stack[max_block_size];
        fundamental = fundamental_stack;
        subharmonic = subharmonic_stack;
    } else {
        // For large blocks, we'd need preallocated buffers
        // This is a limitation - in production, use a memory pool
        result.success = false;
        result.error_message = "Block size too large (requires preallocation)";
        return result;
    }

    // Clear output buffer
    std::memset(output, 0, num_samples * sizeof(float));
    std::memset(fundamental, 0, num_samples * sizeof(float));
    std::memset(subharmonic, 0, num_samples * sizeof(float));

    // Process fundamental
    processFundamental(fundamental, num_samples, state);

    // Process subharmonic
    processSubharmonic(subharmonic, num_samples, state);

    // Apply formant filtering to subharmonic
    if (formant_filtering_enabled_) {
        applyFormantFiltering(subharmonic, num_samples, state);
    }

    // Blend fundamental and subharmonic
    blendSignals(fundamental, subharmonic, output, num_samples, state);

    // Apply spectral enhancement
    if (spectral_enhancement_enabled_) {
        spectral_enhancer_->process(output, num_samples);
    }

    // Update stats
    stats_.voices_processed++;
    stats_.blocks_processed++;
    stats_.average_cpu_usage = 0.1f;  // Placeholder: should measure actual CPU

    return result;
}

SynthesisResult SubharmonicSynthesis::synthesizeVoicesSIMD(
    const std::vector<Voice*>& voices,
    const std::vector<const Phoneme*>& phonemes,
    float* output,
    int num_samples
) {
    SynthesisResult result;
    result.success = true;
    result.error_message = "";

    if (voices.size() != phonemes.size()) {
        result.success = false;
        result.error_message = "Voice and phoneme count mismatch";
        return result;
    }

    // Clear output buffer
    std::memset(output, 0, num_samples * sizeof(float) * 2);  // Stereo

    // Process each voice and mix to output
    for (size_t i = 0; i < voices.size(); ++i) {
        // Allocate temporary buffer for this voice
        const int max_block_size = 256;
        if (num_samples > max_block_size) {
            result.success = false;
            result.error_message = "Block size too large for SIMD processing";
            return result;
        }

        float voice_buffer[max_block_size];

        // Synthesize single voice
        SynthesisResult voice_result = synthesizeVoice(
            voices[i],
            phonemes[i],
            voice_buffer,
            num_samples
        );

        if (!voice_result.success) {
            return voice_result;
        }

        // Mix to stereo output
        float pan = voices[i]->getPan();  // -1 to 1
        float left_gain = 1.0f - (pan + 1.0f) * 0.5f;
        float right_gain = (pan + 1.0f) * 0.5f;

        for (int j = 0; j < num_samples; ++j) {
            output[j * 2] += voice_buffer[j] * left_gain;
            output[j * 2 + 1] += voice_buffer[j] * right_gain;
        }
    }

    stats_.voices_processed += voices.size();
    stats_.blocks_processed++;

    return result;
}

void SubharmonicSynthesis::reset() {
    if (glottal_source_) {
        glottal_source_->reset();
    }
    if (subharmonic_generator_) {
        subharmonic_generator_->reset();
    }
    for (auto& resonator : formant_resonators_) {
        resonator->reset();
    }
    for (auto& smoother : smoothers_) {
        smoother->reset();
    }
    if (spectral_enhancer_) {
        spectral_enhancer_->reset();
    }

    voice_states_.clear();
}

ISynthesisMethod::MethodStats SubharmonicSynthesis::getStats() const {
    return stats_;
}

bool SubharmonicSynthesis::setPreset(const std::string& preset) {
    auto it = presets_.find(preset);
    if (it != presets_.end()) {
        current_preset_ = it->second;
        return true;
    }
    return false;
}

void SubharmonicSynthesis::setSubharmonicRatio(float ratio) {
    if (subharmonic_generator_) {
        // Invert ratio (0.5 means divide by 2, so ratio is 0.5)
        subharmonic_generator_->setRatio(ratio);
    }
}

void SubharmonicSynthesis::setSubharmonicMix(float mix) {
    if (subharmonic_generator_) {
        subharmonic_generator_->setMix(mix);
    }
}

void SubharmonicSynthesis::setFormantFiltering(bool enable) {
    formant_filtering_enabled_ = enable;
}

void SubharmonicSynthesis::setSpectralEnhancement(bool enable) {
    spectral_enhancement_enabled_ = enable;
}

std::vector<std::string> SubharmonicSynthesis::getAvailablePresets() {
    if (!presets_initialized_) {
        initializePresets();
        presets_initialized_ = true;
    }

    std::vector<std::string> preset_names;
    for (const auto& pair : presets_) {
        preset_names.push_back(pair.first);
    }
    return preset_names;
}

void SubharmonicSynthesis::initializePresets() {
    presets_["tibetan_sygyt"] = createTibetanSygytPreset();
    presets_["tuva_kargyraa"] = createTuvaKargyraaPreset();
    presets_["inuit_katajjaq"] = createInuitKatajjaqPreset();
    presets_["sardinian_cantu_a_tenore"] = createSardinianCantuATenorePreset();
    presets_["subhuman_deep"] = createSubhumanDeepPreset();
    presets_["basso_profondo"] = createBassoProfondoPreset();
}

void SubharmonicSynthesis::loadPresetsFromJSON() {
    // TODO: Load from languages/throat_singing.json
    // For now, use hardcoded presets
}

void SubharmonicSynthesis::applyPresetToVoice(const ThroatSingingPreset& preset, VoiceState& state) {
    state.subharmonic_ratio = preset.subharmonic_ratio;
    state.fundamental_amplitude = 1.0f - preset.subharmonic_amplitude;
    state.subharmonic_amplitude = preset.subharmonic_amplitude;
    state.formant_frequencies[0] = preset.melody_formant_freq;
    state.formant_bandwidths[0] = preset.melody_formant_bandwidth;
}

void SubharmonicSynthesis::processFundamental(float* output, int num_samples, VoiceState& state) {
    if (!glottal_source_) {
        return;
    }

    // Update glottal source frequency
    glottal_source_->setFrequency(state.current_f0);

    // Generate fundamental
    glottal_source_->processBlock(output, num_samples);

    // Apply amplitude
    for (int i = 0; i < num_samples; ++i) {
        output[i] *= state.fundamental_amplitude;
    }
}

void SubharmonicSynthesis::processSubharmonic(float* output, int num_samples, VoiceState& state) {
    if (!subharmonic_generator_) {
        return;
    }

    // Update subharmonic generator parameters
    subharmonic_generator_->setRatio(1.0f / state.subharmonic_ratio);
    subharmonic_generator_->setMix(state.subharmonic_amplitude);

    // Generate subharmonic using PLL
    for (int i = 0; i < num_samples; ++i) {
        output[i] = subharmonic_generator_->generate(state.current_f0, params_.sample_rate);
    }
}

void SubharmonicSynthesis::applyFormantFiltering(float* audio, int num_samples, VoiceState& state) {
    // Apply formant resonators in series
    for (size_t i = 0; i < formant_resonators_.size(); ++i) {
        // Update formant frequency from smoothed value
        formant_resonators_[i]->setParameters(
            state.formant_frequencies[i],
            state.formant_bandwidths[i],
            params_.sample_rate
        );

        // Process through this resonator
        formant_resonators_[i]->processBlock(audio, audio, num_samples);
    }
}

void SubharmonicSynthesis::blendSignals(
    float* fundamental,
    float* subharmonic,
    float* output,
    int num_samples,
    VoiceState& state
) {
    // Blend fundamental and subharmonic
    for (int i = 0; i < num_samples; ++i) {
        output[i] = fundamental[i] + subharmonic[i];

        // Soft clip to prevent overload
        if (output[i] > 1.0f) output[i] = 1.0f;
        if (output[i] < -1.0f) output[i] = -1.0f;
    }
}

void SubharmonicSynthesis::updateSubharmonicTargets(const Phoneme* phoneme, VoiceState& state) {
    if (!phoneme) {
        return;
    }

    // Extract subharmonic parameters from phoneme
    const SubharmonicParams& params = phoneme->subharmonic_params;

    // Update subharmonic ratio (convert from integer ratio to float)
    // ratio 2.0 means divide by 2 (octave down)
    if (params.subharmonic_ratio > 0) {
        state.subharmonic_ratio = params.subharmonic_ratio;
    }

    // Update amplitudes
    state.fundamental_amplitude = 1.0f - params.subharmonic_amplitude;
    state.subharmonic_amplitude = params.subharmonic_amplitude;

    // Update fundamental frequency target
    if (params.fundamental_freq > 0) {
        state.target_f0 = params.fundamental_freq;
    }
}

void SubharmonicSynthesis::updateFormantTargets(const Phoneme* phoneme, VoiceState& state) {
    if (!phoneme) {
        return;
    }

    // Update formant frequencies from phoneme
    for (int i = 0; i < 4; ++i) {
        state.formant_frequencies[i] = phoneme->getFormantFrequency(i);
        state.formant_bandwidths[i] = phoneme->getFormantBandwidth(i);
    }
}

// Preset factory methods

ThroatSingingPreset SubharmonicSynthesis::createTibetanSygytPreset() {
    ThroatSingingPreset preset;
    preset.name = "Tibetan Sygyt";
    preset.description = "High-pitched whistle-like melody over drone";
    preset.fundamental_freq = 110.0f;
    preset.subharmonic_ratio = 2.0f;
    preset.subharmonic_amplitude = 0.4f;
    preset.melody_formant_freq = 1800.0f;
    preset.melody_formant_bandwidth = 80.0f;
    preset.melody_formant_amplitude = 0.85f;
    preset.sharp_resonance = true;
    return preset;
}

ThroatSingingPreset SubharmonicSynthesis::createTuvaKargyraaPreset() {
    ThroatSingingPreset preset;
    preset.name = "Tuva Kargyraa";
    preset.description = "Deep sub-bass with 3:1 subharmonic";
    preset.fundamental_freq = 110.0f;
    preset.subharmonic_ratio = 3.0f;
    preset.subharmonic_amplitude = 0.7f;
    preset.melody_formant_freq = 600.0f;
    preset.melody_formant_bandwidth = 150.0f;
    preset.melody_formant_amplitude = 0.5f;
    preset.ventricular_folds = true;
    preset.chest_voice = true;
    return preset;
}

ThroatSingingPreset SubharmonicSynthesis::createInuitKatajjaqPreset() {
    ThroatSingingPreset preset;
    preset.name = "Inuit Katajjaq";
    preset.description = "Rhythmic breathing patterns";
    preset.fundamental_freq = 147.0f;
    preset.subharmonic_ratio = 2.0f;
    preset.subharmonic_amplitude = 0.5f;
    preset.melody_formant_freq = 1200.0f;
    preset.melody_formant_bandwidth = 120.0f;
    preset.melody_formant_amplitude = 0.6f;
    preset.pulse_rate = 6.0f;
    preset.pulse_depth = 0.5f;
    preset.formant_modulation = true;
    return preset;
}

ThroatSingingPreset SubharmonicSynthesis::createSardinianCantuATenorePreset() {
    ThroatSingingPreset preset;
    preset.name = "Sardinian Cantu a Tenore";
    preset.description = "Four-voice polyphony";
    preset.fundamental_freq = 98.0f;
    preset.subharmonic_ratio = 2.0f;
    preset.subharmonic_amplitude = 0.3f;
    preset.melody_formant_freq = 1000.0f;
    preset.melody_formant_bandwidth = 100.0f;
    preset.melody_formant_amplitude = 0.7f;
    return preset;
}

ThroatSingingPreset SubharmonicSynthesis::createSubhumanDeepPreset() {
    ThroatSingingPreset preset;
    preset.name = "Sub-human Deep";
    preset.description = "Extreme sub-bass with 4:1 subharmonic (20-40 Hz)";
    preset.fundamental_freq = 82.0f;
    preset.subharmonic_ratio = 4.0f;
    preset.subharmonic_amplitude = 0.8f;
    preset.melody_formant_freq = 400.0f;
    preset.melody_formant_bandwidth = 200.0f;
    preset.melody_formant_amplitude = 0.4f;
    preset.ventricular_folds = true;
    preset.chest_voice = true;
    return preset;
}

ThroatSingingPreset SubharmonicSynthesis::createBassoProfondoPreset() {
    ThroatSingingPreset preset;
    preset.name = "Basso Profondo";
    preset.description = "Extreme bass (40-80 Hz)";
    preset.fundamental_freq = 65.0f;
    preset.subharmonic_ratio = 2.0f;
    preset.subharmonic_amplitude = 0.6f;
    preset.melody_formant_freq = 500.0f;
    preset.melody_formant_bandwidth = 150.0f;
    preset.melody_formant_amplitude = 0.5f;
    preset.chest_voice = true;
    return preset;
}

} // namespace ChoirV2
