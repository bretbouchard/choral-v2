/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * ChoirV2Processor.h - JUCE audio processor
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>

#include "core/ChoirV2Engine.h"
#include "core/VoiceManager.h"

namespace ChoirV2 {

/**
 * @brief JUCE Audio Processor for Choir V2.0
 *
 * Wraps ChoirV2Engine in a JUCE AudioProcessor interface.
 * Supports all 7 plugin formats: VST3, AU, CLAP, LV2, AUv3, Standalone
 *
 * Features:
 * - MIDI note input triggering synthesis
 * - Real-time parameter smoothing
 * - State management (presets, XML serialization)
 * - Multi-format plugin support
 * - 128 voice polyphony
 */
class ChoirV2Processor : public juce::AudioProcessor,
                         public juce::AudioProcessorValueTreeState::Listener
{
public:
    ChoirV2Processor();
    ~ChoirV2Processor() override;

    // AudioProcessor interface
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;
    void processBlockBypassed(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Choir V2.0"; }

    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }

    double getTailLengthSeconds() const override { return 0.0; }

    // Program support
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram([[maybe_unused]] int index) override {}
    const juce::String getProgramName([[maybe_unused]] int index) override { return {}; }
    void changeProgramName([[maybe_unused]] int index, [[maybe_unused]] const juce::String& newName) override {}

    // State management
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Buses layout
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    // AudioProcessorValueTreeState::Listener
    void parameterChanged(const juce::String& parameterID, float newValue) override;

    // Accessors for editor
    juce::AudioProcessorValueTreeState& getAPVTS() { return parameters; }
    VoiceManager* getVoiceManager() { return voiceManager_.get(); }
    const ChoirV2Engine::PerfStats& getPerformanceStats() const { return perfStats_; }

    // Parameter IDs (public for editor access)
    static const juce::String PARAM_LANGUAGE;
    static const juce::String PARAM_LYRICS;
    static const juce::String PARAM_NUM_VOICES;
    static const juce::String PARAM_MASTER_GAIN;
    static const juce::String PARAM_FORMANT_MIX;
    static const juce::String PARAM_SUBHARMONIC_MIX;
    static const juce::String PARAM_STEREO_WIDTH;
    static const juce::String PARAM_VIBRATO_RATE;
    static const juce::String PARAM_VIBRATO_DEPTH;
    static const juce::String PARAM_REVERB_MIX;
    static const juce::String PARAM_REVERB_SIZE;
    static const juce::String PARAM_ATTACK_TIME;
    static const juce::String PARAM_RELEASE_TIME;

private:
    // Choir V2 Engine components
    std::unique_ptr<ChoirV2Engine> engine_;
    std::unique_ptr<VoiceManager> voiceManager_;

    // JUCE parameter management
    juce::AudioProcessorValueTreeState parameters;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Parameter smoothing (LinearSmoother for real-time safety)
    struct SmoothedParams {
        juce::LinearSmoothedValue<float> masterGain;
        juce::LinearSmoothedValue<float> formantMix;
        juce::LinearSmoothedValue<float> subharmonicMix;
        juce::LinearSmoothedValue<float> stereoWidth;
        juce::LinearSmoothedValue<float> vibratoRate;
        juce::LinearSmoothedValue<float> vibratoDepth;
        juce::LinearSmoothedValue<float> reverbMix;
        juce::LinearSmoothedValue<float> reverbSize;
        juce::LinearSmoothedValue<float> attackTime;
        juce::LinearSmoothedValue<float> releaseTime;

        void reset(double sampleRate, float rampLengthSeconds = 0.05)
        {
            masterGain.reset(sampleRate, rampLengthSeconds);
            formantMix.reset(sampleRate, rampLengthSeconds);
            subharmonicMix.reset(sampleRate, rampLengthSeconds);
            stereoWidth.reset(sampleRate, rampLengthSeconds);
            vibratoRate.reset(sampleRate, rampLengthSeconds);
            vibratoDepth.reset(sampleRate, rampLengthSeconds);
            reverbMix.reset(sampleRate, rampLengthSeconds);
            reverbSize.reset(sampleRate, rampLengthSeconds);
            attackTime.reset(sampleRate, rampLengthSeconds);
            releaseTime.reset(sampleRate, rampLengthSeconds);
        }
    };
    SmoothedParams smoothedParams_;

    // Reverb (built-in JUCE reverb for space)
    juce::dsp::Reverb reverb_;
    juce::dsp::Reverb::Parameters reverbParams_;
    bool reverbPrepared_;

    // Performance tracking
    ChoirV2Engine::PerfStats perfStats_;
    juce::Atomic<float> cpuUsage_;
    int activeVoiceCount_;

    // Language and lyrics state
    juce::String currentLanguage_;
    juce::String currentLyrics_;

    // Helper methods
    void updateParameters();
    void processMidi(juce::MidiBuffer& midiMessages);
    void applyReverb(juce::AudioBuffer<float>& buffer);
    void updatePerformanceStats();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChoirV2Processor)
};

} // namespace ChoirV2
