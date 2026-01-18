/*
  ==============================================================================

    ChoirV2PureDSP.h
    Created: January 18, 2026
    Author:  Bret Bouchard

    Pure DSP implementation of Choir V2 - Advanced Vocal Synthesizer

    Architecture Overview:
    ----------------------
    - Inherits from DSP::InstrumentDSP (no JUCE dependencies)
    - Headless operation (no GUI coupling)
    - G2P (Grapheme-to-Phoneme) text-to-speech engine
    - Formant-based synthesis with 4-formant filter bank
    - Subharmonic synthesis for rich bass voices
    - Diphone-based transitions for realistic coarticulation
    - Polyphonic voice bank (up to 128 active voices)
    - SATB voicing support with intelligent voice allocation
    - Vibrato, breath noise, spectral enhancement
    - Reverb effect with early reflections
    - JSON preset save/load system
    - SIMD-optimized processing (ARM NEON / x86 AVX2)

    Design Principles:
    -----------------
    - Pure C++20 (no runtime allocation in audio thread)
    - Deterministic output (same input = same output)
    - tvOS-safe (no file I/O, no threads)
    - Real-time safe (all audio path operations)

  ==============================================================================
*/

#pragma once

#include "dsp/InstrumentDSP.h"
#include "dsp/synthesis/ISynthesisMethod.h"
#include "dsp/synthesis/FormantSynthesisMethod.h"
#include "dsp/synthesis/SubharmonicSynthesisMethod.h"
#include "dsp/synthesis/DiphoneSynthesisMethod.h"
#include "dsp/core/PhonemeDatabase.h"
#include "dsp/core/LanguageLoader.h"
#include "dsp/core/G2PEngine.h"
#include "../../src/core/VoiceManager.h"
#include <vector>
#include <array>
#include <memory>
#include <cmath>
#include <random>
#include <algorithm>
#include <string>

namespace DSP {

//==============================================================================
// Parameter IDs (const char* for PureDSP compatibility)
//==============================================================================

namespace ChoirV2Parameters {
    // Master
    constexpr const char* MASTER_VOLUME = "masterVolume";
    constexpr const char* STEREO_WIDTH = "stereoWidth";
    constexpr const char* POLPHONY = "polyphony";

    // Text Input
    constexpr const char* TEXT_INPUT = "textInput";
    constexpr const char* PHONEME_DISPLAY = "phonemeDisplay";

    // Vowel (3D control)
    constexpr const char* VOWEL_X = "vowelX";
    constexpr const char* VOWEL_Y = "vowelY";
    constexpr const char* VOWEL_Z = "vowelZ";

    // Formants
    constexpr const char* FORMANT_SCALE = "formantScale";
    constexpr const char* FORMANT_SHIFT = "formantShift";

    // Breath
    constexpr const char* BREATH_MIX = "breathMix";
    constexpr const char* BREATH_COLOR = "breathColor";

    // Vibrato
    constexpr const char* VIBRATO_RATE = "vibratoRate";
    constexpr const char* VIBRATO_DEPTH = "vibratoDepth";
    constexpr const char* VIBRATO_DELAY = "vibratoDelay";

    // Ensemble
    constexpr const char* TIGHTNESS = "tightness";
    constexpr const char* ENSEMBLE_SIZE = "ensembleSize";
    constexpr const char* VOICE_SPREAD = "voiceSpread";

    // Envelope (ADSR)
    constexpr const char* ATTACK = "attack";
    constexpr const char* DECAY = "decay";
    constexpr const char* SUSTAIN = "sustain";
    constexpr const char* RELEASE = "release";

    // SATB blend
    constexpr const char* SOPRANO_LEVEL = "sopranoLevel";
    constexpr const char* ALTO_LEVEL = "altoLevel";
    constexpr const char* TENOR_LEVEL = "tenorLevel";
    constexpr const char* BASS_LEVEL = "bassLevel";

    // Effects
    constexpr const char* REVERB_MIX = "reverbMix";
    constexpr const char* REVERB_DECAY = "reverbDecay";
    constexpr const char* REVERB_PREDELAY = "reverbPredelay";

    // Spectral Enhancement
    constexpr const char* SPECTRAL_ENHANCEMENT = "spectralEnhancement";
    constexpr const char* HARMONICS_BOOST = "harmonicsBoost";

    // Subharmonic Generation
    constexpr const char* SUBHARMONIC_MIX = "subharmonicMix";
    constexpr const char* SUBHARMONIC_DEPTH = "subharmonicDepth";

    // Diphone/Coarticulation
    constexpr const char* COARTICULATION_AMOUNT = "coarticulationAmount";
    constexpr const char* TRANSITION_SPEED = "transitionSpeed";
}

//==============================================================================
// Main Choir V2 PureDSP Engine
//==============================================================================

/**
 * @brief Choir V2 PureDSP implementation
 *
 * Advanced vocal synthesizer with text-to-speech capabilities,
 * multiple synthesis methods, and intelligent voice management.
 */
class ChoirV2PureDSP : public InstrumentDSP {
public:
    //==========================================================================
    // Construction/Destruction
    //==========================================================================

    ChoirV2PureDSP();
    ~ChoirV2PureDSP() override;

    //==========================================================================
    // InstrumentDSP Interface Implementation
    //==========================================================================

    /**
     * @brief Prepare Choir V2 for audio processing
     *
     * Initializes all synthesis engines, allocates memory pools,
     * prepares DSP components, loads phoneme database.
     *
     * @param sampleRate Sample rate in Hz (e.g., 48000.0, 96000.0)
     * @param blockSize Maximum samples per process() call (power of 2)
     * @return true if preparation succeeded, false on error
     */
    bool prepare(double sampleRate, int blockSize) override;

    /**
     * @brief Reset all internal state
     *
     * Resets all voices, envelopes, oscillators, filters to initial state.
     * Clears phoneme queue and resets G2P engine state.
     */
    void reset() override;

    /**
     * @brief Process audio and generate output
     *
     * Main audio processing loop. Generates vocal synthesis output
     * by mixing all active voices through the formant filter bank.
     *
     * @param outputs Output buffers [numChannels][numSamples]
     * @param numChannels Number of output channels (typically 2 for stereo)
     * @param numSamples Number of samples in this buffer (typically 128-512)
     */
    void process(float** outputs, int numChannels, int numSamples) override;

    /**
     * @brief Handle a scheduled event
     *
     * Processes MIDI events (note on/off, pitch bend, CC) and
     * parameter changes. Schedules text input for G2P processing.
     *
     * @param event The event to handle
     */
    void handleEvent(const ScheduledEvent& event) override;

    //==========================================================================
    // Parameter Management
    //==========================================================================

    /**
     * @brief Get parameter value by ID
     *
     * Thread-safe parameter access with atomic reads.
     *
     * @param paramId Null-terminated parameter identifier string
     * @return Current parameter value (normalized 0.0 to 1.0, or raw value)
     */
    float getParameter(const char* paramId) const override;

    /**
     * @brief Set parameter value by ID
     *
     * Thread-safe parameter update with smoothing applied in audio thread.
     *
     * @param paramId Null-terminated parameter identifier string
     * @param value New parameter value (normalized 0.0 to 1.0, or raw value)
     */
    void setParameter(const char* paramId, float value) override;

    //==========================================================================
    // Preset Management
    //==========================================================================

    /**
     * @brief Save current state as JSON preset
     *
     * Serializes all parameters, phoneme database state, and voice settings.
     *
     * @param jsonBuffer Output buffer for JSON string (caller-allocated)
     * @param jsonBufferSize Size of jsonBuffer in bytes
     * @return true if save succeeded, false if buffer too small
     */
    bool savePreset(char* jsonBuffer, int jsonBufferSize) const override;

    /**
     * @brief Load state from JSON preset
     *
     * Deserializes JSON and updates all parameters, phonemes, and settings.
     *
     * @param jsonData Null-terminated JSON string
     * @return true if load succeeded, false on parse error
     */
    bool loadPreset(const char* jsonData) override;

    //==========================================================================
    // Voice Management
    //==========================================================================

    /**
     * @brief Get number of active voices
     *
     * Returns currently sounding voices (envelope > 0.001).
     *
     * @return Number of currently active voices
     */
    int getActiveVoiceCount() const override;

    /**
     * @brief Get maximum polyphony
     *
     * Returns the maximum number of simultaneous voices.
     *
     * @return Maximum polyphony (configurable, default 64)
     */
    int getMaxPolyphony() const override;

    //==========================================================================
    // Instrument Identification
    //==========================================================================

    /**
     * @brief Get instrument name
     *
     * @return Instrument name string
     */
    const char* getInstrumentName() const override { return "Choir V2"; }

    /**
     * @brief Get instrument version
     *
     * @return Version string (e.g., "2.0.0")
     */
    const char* getInstrumentVersion() const override { return "2.0.0-PureDSP"; }

    //==========================================================================
    // Choir V2-Specific Methods
    //==========================================================================

    /**
     * @brief Set text input for G2P processing
     *
     * Converts text string to phoneme sequence for synthesis.
     *
     * @param text Null-terminated text string (English lyrics)
     */
    void setTextInput(const char* text);

    /**
     * @brief Get current phoneme sequence
     *
     * Returns the phoneme sequence generated from text input.
     *
     * @return Phoneme sequence as string (e.g., "S AH L OW T OW")
     */
    const char* getPhonemeSequence() const;

    /**
     * @brief Set SATB blend levels
     *
     * Controls the mix of soprano, alto, tenor, and bass sections.
     *
     * @param soprano Soprano level (0.0 to 1.0)
     * @param alto Alto level (0.0 to 1.0)
     * @param tenor Tenor level (0.0 to 1.0)
     * @param bass Bass level (0.0 to 1.0)
     */
    void setSATBBlend(float soprano, float alto, float tenor, float bass);

    /**
     * @brief Set ensemble size
     *
     * Controls the number of voices per note (4 to 32).
     *
     * @param size Ensemble size (0.0 = 4 voices, 1.0 = 32 voices)
     */
    void setEnsembleSize(float size);

    /**
     * @brief Set stereo width
     *
     * Controls stereo imaging of the choir.
     *
     * @param width Stereo width (0.0 = mono, 1.0 = wide stereo)
     */
    void setStereoWidth(float width);

    /**
     * @brief Select synthesis method
     *
     * Chooses between formant, subharmonic, or diphone synthesis.
     *
     * @param method Synthesis method (0 = formant, 1 = subharmonic, 2 = diphone)
     */
    void setSynthesisMethod(int method);

    /**
     * @brief Load language database
     *
     * Loads phoneme database for specific language (e.g., "en-US", "es-ES").
     *
     * @param language Language code (ISO 639-1)
     * @return true if load succeeded, false if language not found
     */
    bool loadLanguage(const char* language);

private:
    //==========================================================================
    // Synthesis Engines
    //==========================================================================

    std::unique_ptr<FormantSynthesisMethod> formantSynth_;
    std::unique_ptr<SubharmonicSynthesisMethod> subharmonicSynth_;
    std::unique_ptr<DiphoneSynthesisMethod> diphoneSynth_;

    //==========================================================================
    // DSP Components
    //==========================================================================

    // TODO: Uncomment when DSP components are fully implemented
    // std::vector<std::unique_ptr<FormantResonator>> formantResonators_;
    // std::unique_ptr<GlottalSource> glottalSource_;
    // std::unique_ptr<SubharmonicGenerator> subharmonicGen_;
    // std::unique_ptr<SpectralEnhancer> spectralEnhancer_;
    // std::vector<std::unique_ptr<LinearSmoother>> smoothers_;

    // Reverb effect
    struct Reverb {
        std::array<std::array<float, 8192>, 2> buffer;
        std::array<int, 8> indices;
        std::array<float, 8> gains;
        int writeIndex = 0;
        float mix = 0.0f;
        float feedback = 0.3f;
        float predelay = 0.02f;

        void prepare(double sampleRate);
        void reset();
        void process(float& left, float& right);
    } reverb_;

    //==========================================================================
    // Core Components
    //==========================================================================

    std::shared_ptr<PhonemeDatabase> phonemeDB_;
    std::unique_ptr<LanguageLoader> languageLoader_;
    std::unique_ptr<G2PEngine> g2pEngine_;
    std::unique_ptr<ChoirV2::VoiceManager> voiceManager_;

    //==========================================================================
    // Parameters
    //==========================================================================

    struct Parameters {
        // Master
        float masterVolume = 0.7f;
        float stereoWidth = 0.5f;
        int polyphony = 64;

        // Vowel (3D control)
        float vowelX = 0.5f;  // Front/Back
        float vowelY = 0.5f;  // Open/Closed
        float vowelZ = 0.5f;  // Rounded/Spread

        // Formants
        float formantScale = 1.0f;
        float formantShift = 0.0f;

        // Breath
        float breathMix = 0.3f;
        float breathColor = 0.5f;

        // Vibrato
        float vibratoRate = 5.0f;   // Hz
        float vibratoDepth = 0.5f;
        float vibratoDelay = 0.1f;  // seconds

        // Ensemble
        float tightness = 0.5f;      // How tight the ensemble is
        float ensembleSize = 0.5f;   // 4-32 voices
        float voiceSpread = 0.5f;    // Stereo spread

        // Envelope (ADSR)
        float attack = 0.1f;   // seconds
        float decay = 0.2f;    // seconds
        float sustain = 0.7f;  // level
        float release = 0.3f;  // seconds

        // SATB blend
        float sopranoLevel = 1.0f;
        float altoLevel = 0.8f;
        float tenorLevel = 0.6f;
        float bassLevel = 0.4f;

        // Effects
        float reverbMix = 0.3f;
        float reverbDecay = 2.0f;     // seconds
        float reverbPredelay = 0.02f; // seconds

        // Spectral Enhancement
        float spectralEnhancement = 0.5f;
        float harmonicsBoost = 0.5f;

        // Subharmonic
        float subharmonicMix = 0.0f;
        float subharmonicDepth = 0.5f;

        // Diphone/Coarticulation
        float coarticulationAmount = 0.5f;
        float transitionSpeed = 0.5f;

        // Synthesis method (0 = formant, 1 = subharmonic, 2 = diphone)
        int synthesisMethod = 0;
    } params_;

    //==========================================================================
    // State
    //==========================================================================

    bool prepared_ = false;
    double sampleRate_ = 48000.0;
    int blockSize_ = 512;

    // Current phoneme sequence
    std::string currentPhonemes_;
    std::string currentText_;

    // Active synthesis method
    int currentSynthMethod_ = 0;

    //==========================================================================
    // Helper Methods
    //==========================================================================

    /**
     * @brief Apply parameter smoothing
     *
     * Smooths parameter changes to prevent clicks and artifacts.
     */
    void applyParameterSmoothing();

    /**
     * @brief Calculate frequency from MIDI note number
     *
     * Converts MIDI note to frequency in Hz using equal temperament.
     *
     * @param midiNote MIDI note number (0-127)
     * @return Frequency in Hz
     */
    float calculateFrequency(int midiNote) const;

    /**
     * @brief Process formant synthesis
     *
     * Generates sound using formant filter bank synthesis.
     */
    void processFormantSynthesis(float** outputs, int numChannels, int numSamples);

    /**
     * @brief Process subharmonic synthesis
     *
     * Generates rich bass using subharmonic generation.
     */
    void processSubharmonicSynthesis(float** outputs, int numChannels, int numSamples);

    /**
     * @brief Process diphone synthesis
     *
     * Generates realistic coarticulation using diphone transitions.
     */
    void processDiphoneSynthesis(float** outputs, int numChannels, int numSamples);

    /**
     * @brief Write JSON parameter to buffer
     *
     * Helper for JSON serialization.
     */
    bool writeJsonParameter(const char* name, double value,
                           char* buffer, int& offset, int bufferSize) const;

    /**
     * @brief Parse JSON parameter from string
     *
     * Helper for JSON deserialization.
     */
    bool parseJsonParameter(const char* json, const char* param, double& value) const;

    /**
     * @brief SIMD-optimized stereo processing
     *
     * Processes stereo output with SIMD acceleration.
     */
    void processStereoOutput(float** outputs, int numChannels, int numSamples);
};

} // namespace DSP
