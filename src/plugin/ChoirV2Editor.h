/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * ChoirV2Editor.h - JUCE audio processor editor
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "ChoirV2Processor.h"

namespace ChoirV2 {

/**
 * @brief JUCE Audio Processor Editor for Choir V2.0
 *
 * Provides a comprehensive UI for the Choir V2.0 synthesizer.
 *
 * Features:
 * - Language selection dropdown
 * - Lyrics text editor
 * - Voice count slider
 * - Master gain knob
 * - Formant/subharmonic mix controls
 * - Reverb section (mix, size)
 * - ADSR envelope controls (attack, release)
 * - Visual feedback (active voices, CPU usage)
 *
 * Design:
 * - Clean, intuitive layout
 * - Real-time parameter updates
 * - Performance monitoring
 * - Preset management
 */
class ChoirV2Editor : public juce::AudioProcessorEditor,
                      public juce::Timer,
                      private juce::AudioProcessorValueTreeState::Listener
{
public:
    explicit ChoirV2Editor(ChoirV2Processor&);
    ~ChoirV2Editor() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    // Reference to processor
    ChoirV2Processor& processorRef;

    // APVTS for parameter attachments
    juce::AudioProcessorValueTreeState& parameters;

    // Parameter attachments (keep alive for callbacks)
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> languageAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> numVoicesAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> masterGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> formantMixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> subharmonicMixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> stereoWidthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> vibratoRateAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> vibratoDepthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> reverbMixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> reverbSizeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackTimeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> releaseTimeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::TextAttachment> lyricsAttachment;

    // UI Components - Language Section
    juce::GroupBox languageGroup;
    juce::Label languageLabel;
    juce::ComboBox languageComboBox;

    // UI Components - Lyrics Section
    juce::GroupBox lyricsGroup;
    juce::Label lyricsLabel;
    juce::TextEditor lyricsEditor;

    // UI Components - Voice Configuration
    juce::GroupBox voiceConfigGroup;
    juce::Label numVoicesLabel;
    juce::Slider numVoicesSlider;
    juce::Label numVoicesValueLabel;

    // UI Components - Master Section
    juce::GroupBox masterGroup;
    juce::Label masterGainLabel;
    juce::Slider masterGainKnob;
    juce::Label masterGainValueLabel;

    // UI Components - Synthesis Mix
    juce::GroupBox synthesisMixGroup;
    juce::Label formantMixLabel;
    juce::Slider formantMixSlider;
    juce::Label formantMixValueLabel;
    juce::Label subharmonicMixLabel;
    juce::Slider subharmonicMixSlider;
    juce::Label subharmonicMixValueLabel;
    juce::Label stereoWidthLabel;
    juce::Slider stereoWidthSlider;
    juce::Label stereoWidthValueLabel;

    // UI Components - Vibrato Section
    juce::GroupBox vibratoGroup;
    juce::Label vibratoRateLabel;
    juce::Slider vibratoRateSlider;
    juce::Label vibratoRateValueLabel;
    juce::Label vibratoDepthLabel;
    juce::Slider vibratoDepthSlider;
    juce::Label vibratoDepthValueLabel;

    // UI Components - Reverb Section
    juce::GroupBox reverbGroup;
    juce::Label reverbMixLabel;
    juce::Slider reverbMixSlider;
    juce::Label reverbMixValueLabel;
    juce::Label reverbSizeLabel;
    juce::Slider reverbSizeSlider;
    juce::Label reverbSizeValueLabel;

    // UI Components - Envelope Section
    juce::GroupBox envelopeGroup;
    juce::Label attackTimeLabel;
    juce::Slider attackTimeSlider;
    juce::Label attackTimeValueLabel;
    juce::Label releaseTimeLabel;
    juce::Slider releaseTimeSlider;
    juce::Label releaseTimeValueLabel;

    // UI Components - Performance Monitor
    juce::GroupBox performanceGroup;
    juce::Label activeVoicesLabel;
    juce::Label activeVoicesValueLabel;
    juce::Label cpuUsageLabel;
    juce::Label cpuUsageValueLabel;
    juce::Label cpuUsageBar;

    // Helper methods
    void setupUI();
    void setupLookAndFeel();
    void updatePerformanceDisplay();

    // Timer callback for performance updates
    void timerCallback() override;

    // JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChoirV2Editor)
};

} // namespace ChoirV2
