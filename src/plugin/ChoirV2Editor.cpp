/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * ChoirV2Editor.cpp - JUCE audio processor editor implementation
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#include "ChoirV2Editor.h"

namespace ChoirV2 {

//==============================================================================
ChoirV2Editor::ChoirV2Editor(ChoirV2Processor& p)
    : AudioProcessorEditor(&p)
    , processorRef(p)
    , parameters(processorRef.getAPVTS())
{
    setupLookAndFeel();
    setupUI();

    // Set initial size
    setSize(800, 900);

    // Start timer for performance updates (30 FPS)
    startTimerHz(30);
}

ChoirV2Editor::~ChoirV2Editor()
{
    stopTimer();
}

//==============================================================================
void ChoirV2Editor::setupLookAndFeel()
{
    // Custom look and feel can be added here
    // For now, using default JUCE look and feel
}

void ChoirV2Editor::setupUI()
{
    // Language Section
    addAndMakeVisible(languageGroup);
    languageGroup.setText("Language & Lyrics");
    languageGroup.setTextLabelPosition(juce::Justification::centred);

    addAndMakeVisible(languageLabel);
    languageLabel.setText("Language:", juce::dontSendNotification);

    addAndMakeVisible(languageComboBox);
    languageComboBox.addItem("English", 1);
    languageComboBox.addItem("Latin", 2);
    languageComboBox.addItem("Klingon", 3);
    languageComboBox.addItem("Throat Singing", 4);

    // Lyrics editor
    addAndMakeVisible(lyricsLabel);
    lyricsLabel.setText("Lyrics:", juce::dontSendNotification);

    addAndMakeVisible(lyricsEditor);
    lyricsEditor.setMultiLine(true);
    lyricsEditor.setReturnKeyStartsNewLine(true);
    lyricsEditor.setText("Ah");
    lyricsEditor.setReadOnly(false);
    lyricsEditor.setCaretVisible(true);

    // Voice Configuration Section
    addAndMakeVisible(voiceConfigGroup);
    voiceConfigGroup.setText("Voice Configuration");
    voiceConfigGroup.setTextLabelPosition(juce::Justification::centred);

    addAndMakeVisible(numVoicesLabel);
    numVoicesLabel.setText("Voices:", juce::dontSendNotification);

    addAndMakeVisible(numVoicesSlider);
    numVoicesSlider.setRange(1, 60, 1);
    numVoicesSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    numVoicesSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 25);

    addAndMakeVisible(numVoicesValueLabel);
    numVoicesValueLabel.setText("40", juce::dontSendNotification);

    // Master Section
    addAndMakeVisible(masterGroup);
    masterGroup.setText("Master");
    masterGroup.setTextLabelPosition(juce::Justification::centred);

    addAndMakeVisible(masterGainLabel);
    masterGainLabel.setText("Gain:", juce::dontSendNotification);

    addAndMakeVisible(masterGainKnob);
    masterGainKnob.setRange(-60.0, 0.0, 0.1);
    masterGainKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    masterGainKnob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 25);

    addAndMakeVisible(masterGainValueLabel);
    masterGainValueLabel.setText("-6.0 dB", juce::dontSendNotification);

    // Synthesis Mix Section
    addAndMakeVisible(synthesisMixGroup);
    synthesisMixGroup.setText("Synthesis Mix");
    synthesisMixGroup.setTextLabelPosition(juce::Justification::centred);

    addAndMakeVisible(formantMixLabel);
    formantMixLabel.setText("Formant:", juce::dontSendNotification);

    addAndMakeVisible(formantMixSlider);
    formantMixSlider.setRange(0.0, 100.0, 0.1);
    formantMixSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    formantMixSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 25);

    addAndMakeVisible(formantMixValueLabel);
    formantMixValueLabel.setText("100%", juce::dontSendNotification);

    addAndMakeVisible(subharmonicMixLabel);
    subharmonicMixLabel.setText("Subharmonic:", juce::dontSendNotification);

    addAndMakeVisible(subharmonicMixSlider);
    subharmonicMixSlider.setRange(0.0, 100.0, 0.1);
    subharmonicMixSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    subharmonicMixSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 25);

    addAndMakeVisible(subharmonicMixValueLabel);
    subharmonicMixValueLabel.setText("0%", juce::dontSendNotification);

    addAndMakeVisible(stereoWidthLabel);
    stereoWidthLabel.setText("Stereo Width:", juce::dontSendNotification);

    addAndMakeVisible(stereoWidthSlider);
    stereoWidthSlider.setRange(0.0, 100.0, 0.1);
    stereoWidthSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    stereoWidthSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 25);

    addAndMakeVisible(stereoWidthValueLabel);
    stereoWidthValueLabel.setText("100%", juce::dontSendNotification);

    // Vibrato Section
    addAndMakeVisible(vibratoGroup);
    vibratoGroup.setText("Vibrato");
    vibratoGroup.setTextLabelPosition(juce::Justification::centred);

    addAndMakeVisible(vibratoRateLabel);
    vibratoRateLabel.setText("Rate:", juce::dontSendNotification);

    addAndMakeVisible(vibratoRateSlider);
    vibratoRateSlider.setRange(0.0, 10.0, 0.1);
    vibratoRateSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    vibratoRateSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 25);

    addAndMakeVisible(vibratoRateValueLabel);
    vibratoRateValueLabel.setText("5.0 Hz", juce::dontSendNotification);

    addAndMakeVisible(vibratoDepthLabel);
    vibratoDepthLabel.setText("Depth:", juce::dontSendNotification);

    addAndMakeVisible(vibratoDepthSlider);
    vibratoDepthSlider.setRange(0.0, 100.0, 0.1);
    vibratoDepthSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    vibratoDepthSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 25);

    addAndMakeVisible(vibratoDepthValueLabel);
    vibratoDepthValueLabel.setText("0%", juce::dontSendNotification);

    // Reverb Section
    addAndMakeVisible(reverbGroup);
    reverbGroup.setText("Reverb");
    reverbGroup.setTextLabelPosition(juce::Justification::centred);

    addAndMakeVisible(reverbMixLabel);
    reverbMixLabel.setText("Mix:", juce::dontSendNotification);

    addAndMakeVisible(reverbMixSlider);
    reverbMixSlider.setRange(0.0, 100.0, 0.1);
    reverbMixSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    reverbMixSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 25);

    addAndMakeVisible(reverbMixValueLabel);
    reverbMixValueLabel.setText("20%", juce::dontSendNotification);

    addAndMakeVisible(reverbSizeLabel);
    reverbSizeLabel.setText("Size:", juce::dontSendNotification);

    addAndMakeVisible(reverbSizeSlider);
    reverbSizeSlider.setRange(0.0, 100.0, 0.1);
    reverbSizeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    reverbSizeSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 25);

    addAndMakeVisible(reverbSizeValueLabel);
    reverbSizeValueLabel.setText("50%", juce::dontSendNotification);

    // Envelope Section
    addAndMakeVisible(envelopeGroup);
    envelopeGroup.setText("Envelope");
    envelopeGroup.setTextLabelPosition(juce::Justification::centred);

    addAndMakeVisible(attackTimeLabel);
    attackTimeLabel.setText("Attack:", juce::dontSendNotification);

    addAndMakeVisible(attackTimeSlider);
    attackTimeSlider.setRange(1.0, 500.0, 1.0);
    attackTimeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    attackTimeSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 25);

    addAndMakeVisible(attackTimeValueLabel);
    attackTimeValueLabel.setText("10 ms", juce::dontSendNotification);

    addAndMakeVisible(releaseTimeLabel);
    releaseTimeLabel.setText("Release:", juce::dontSendNotification);

    addAndMakeVisible(releaseTimeSlider);
    releaseTimeSlider.setRange(10.0, 2000.0, 1.0);
    releaseTimeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    releaseTimeSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 25);

    addAndMakeVisible(releaseTimeValueLabel);
    releaseTimeValueLabel.setText("200 ms", juce::dontSendNotification);

    // Performance Monitor Section
    addAndMakeVisible(performanceGroup);
    performanceGroup.setText("Performance");
    performanceGroup.setTextLabelPosition(juce::Justification::centred);

    addAndMakeVisible(activeVoicesLabel);
    activeVoicesLabel.setText("Active Voices:", juce::dontSendNotification);

    addAndMakeVisible(activeVoicesValueLabel);
    activeVoicesValueLabel.setText("0 / 60", juce::dontSendNotification);
    activeVoicesValueLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    activeVoicesValueLabel.setColour(juce::Label::textColourId, juce::Colours::green);

    addAndMakeVisible(cpuUsageLabel);
    cpuUsageLabel.setText("CPU:", juce::dontSendNotification);

    addAndMakeVisible(cpuUsageValueLabel);
    cpuUsageValueLabel.setText("0%", juce::dontSendNotification);
    cpuUsageValueLabel.setFont(juce::Font(14.0f, juce::Font::bold));

    addAndMakeVisible(cpuUsageBar);
    cpuUsageBar.setText("", juce::dontSendNotification);
    cpuUsageBar.setColour(juce::Label::backgroundColourId, juce::Colours::darkgrey);

    // Create parameter attachments
    languageAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        parameters, ChoirV2Processor::PARAM_LANGUAGE, languageComboBox);

    numVoicesAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        parameters, ChoirV2Processor::PARAM_NUM_VOICES, numVoicesSlider);

    masterGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        parameters, ChoirV2Processor::PARAM_MASTER_GAIN, masterGainKnob);

    formantMixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        parameters, ChoirV2Processor::PARAM_FORMANT_MIX, formantMixSlider);

    subharmonicMixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        parameters, ChoirV2Processor::PARAM_SUBHARMONIC_MIX, subharmonicMixSlider);

    stereoWidthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        parameters, ChoirV2Processor::PARAM_STEREO_WIDTH, stereoWidthSlider);

    vibratoRateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        parameters, ChoirV2Processor::PARAM_VIBRATO_RATE, vibratoRateSlider);

    vibratoDepthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        parameters, ChoirV2Processor::PARAM_VIBRATO_DEPTH, vibratoDepthSlider);

    reverbMixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        parameters, ChoirV2Processor::PARAM_REVERB_MIX, reverbMixSlider);

    reverbSizeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        parameters, ChoirV2Processor::PARAM_REVERB_SIZE, reverbSizeSlider);

    attackTimeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        parameters, ChoirV2Processor::PARAM_ATTACK_TIME, attackTimeSlider);

    releaseTimeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        parameters, ChoirV2Processor::PARAM_RELEASE_TIME, releaseTimeSlider);

    lyricsAttachment = std::make_unique<juce::AudioProcessorValueTreeState::TextAttachment>(
        parameters, ChoirV2Processor::PARAM_LYRICS, lyricsEditor);

    // Add slider value change listeners
    numVoicesSlider.onValueChange = [this]() {
        numVoicesValueLabel.setText(juce::String(static_cast<int>(numVoicesSlider.getValue())),
                                    juce::dontSendNotification);
    };

    masterGainKnob.onValueChange = [this]() {
        masterGainValueLabel.setText(juce::String(masterGainKnob.getValue(), 1) + " dB",
                                     juce::dontSendNotification);
    };

    formantMixSlider.onValueChange = [this]() {
        formantMixValueLabel.setText(juce::String(formantMixSlider.getValue(), 1) + "%",
                                     juce::dontSendNotification);
    };

    subharmonicMixSlider.onValueChange = [this]() {
        subharmonicMixValueLabel.setText(juce::String(subharmonicMixSlider.getValue(), 1) + "%",
                                        juce::dontSendNotification);
    };

    stereoWidthSlider.onValueChange = [this]() {
        stereoWidthValueLabel.setText(juce::String(stereoWidthSlider.getValue(), 1) + "%",
                                      juce::dontSendNotification);
    };

    vibratoRateSlider.onValueChange = [this]() {
        vibratoRateValueLabel.setText(juce::String(vibratoRateSlider.getValue(), 1) + " Hz",
                                      juce::dontSendNotification);
    };

    vibratoDepthSlider.onValueChange = [this]() {
        vibratoDepthValueLabel.setText(juce::String(vibratoDepthSlider.getValue(), 1) + "%",
                                       juce::dontSendNotification);
    };

    reverbMixSlider.onValueChange = [this]() {
        reverbMixValueLabel.setText(juce::String(reverbMixSlider.getValue(), 1) + "%",
                                    juce::dontSendNotification);
    };

    reverbSizeSlider.onValueChange = [this]() {
        reverbSizeValueLabel.setText(juce::String(reverbSizeSlider.getValue(), 1) + "%",
                                     juce::dontSendNotification);
    };

    attackTimeSlider.onValueChange = [this]() {
        attackTimeValueLabel.setText(juce::String(static_cast<int>(attackTimeSlider.getValue())) + " ms",
                                     juce::dontSendNotification);
    };

    releaseTimeSlider.onValueChange = [this]() {
        releaseTimeValueLabel.setText(juce::String(static_cast<int>(releaseTimeSlider.getValue())) + " ms",
                                      juce::dontSendNotification);
    };
}

//==============================================================================
void ChoirV2Editor::paint(juce::Graphics& g)
{
    // Background
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    // Title
    g.setColour(juce::Colours::white);
    g.setFont(24.0f);
    g.drawText("Choir V2.0", getLocalBounds().removeFromTop(40), juce::Justification::centred);
}

void ChoirV2Editor::resized()
{
    auto area = getLocalBounds().reduced(10);
    area.removeFromTop(50); // Space for title

    int sectionHeight = 120;
    int sectionGap = 10;

    // Language & Lyrics Section
    auto languageArea = area.removeFromTop(sectionHeight);
    languageGroup.setBounds(languageArea);
    auto languageInner = languageArea.reduced(10, 20);

    auto languageRow = languageInner.removeFromTop(30);
    languageLabel.setBounds(languageRow.removeFromLeft(80));
    languageComboBox.setBounds(languageRow);

    lyricsLabel.setBounds(languageInner.removeFromTop(25));
    lyricsEditor.setBounds(languageInner.removeFromTop(60));

    area.removeFromTop(sectionGap);

    // Voice Configuration Section
    auto voiceConfigArea = area.removeFromTop(sectionHeight - 20);
    voiceConfigGroup.setBounds(voiceConfigArea);
    auto voiceConfigInner = voiceConfigArea.reduced(10, 20);

    auto voiceRow = voiceConfigInner.removeFromTop(30);
    numVoicesLabel.setBounds(voiceRow.removeFromLeft(80));
    numVoicesSlider.setBounds(voiceRow.removeFromLeft(200));
    numVoicesValueLabel.setBounds(voiceRow.removeFromLeft(60));

    area.removeFromTop(sectionGap);

    // Master Section
    auto masterArea = area.removeFromTop(sectionHeight - 20);
    masterGroup.setBounds(masterArea);
    auto masterInner = masterArea.reduced(10, 20);

    auto masterRow = masterInner.removeFromTop(30);
    masterGainLabel.setBounds(masterRow.removeFromLeft(80));
    masterGainKnob.setBounds(masterRow.removeFromLeft(80).reduced(0, 10));
    masterGainValueLabel.setBounds(masterRow.removeFromLeft(80));

    area.removeFromTop(sectionGap);

    // Synthesis Mix Section
    auto synthesisMixArea = area.removeFromTop(sectionHeight);
    synthesisMixGroup.setBounds(synthesisMixArea);
    auto synthesisMixInner = synthesisMixArea.reduced(10, 20);

    auto formantRow = synthesisMixInner.removeFromTop(25);
    formantMixLabel.setBounds(formantRow.removeFromLeft(80));
    formantMixSlider.setBounds(formantRow.removeFromLeft(200));
    formantMixValueLabel.setBounds(formantRow.removeFromLeft(60));

    auto subharmonicRow = synthesisMixInner.removeFromTop(25);
    subharmonicMixLabel.setBounds(subharmonicRow.removeFromLeft(80));
    subharmonicMixSlider.setBounds(subharmonicRow.removeFromLeft(200));
    subharmonicMixValueLabel.setBounds(subharmonicRow.removeFromLeft(60));

    auto stereoWidthRow = synthesisMixInner.removeFromTop(25);
    stereoWidthLabel.setBounds(stereoWidthRow.removeFromLeft(80));
    stereoWidthSlider.setBounds(stereoWidthRow.removeFromLeft(200));
    stereoWidthValueLabel.setBounds(stereoWidthRow.removeFromLeft(60));

    area.removeFromTop(sectionGap);

    // Vibrato Section
    auto vibratoArea = area.removeFromTop(sectionHeight - 40);
    vibratoGroup.setBounds(vibratoArea);
    auto vibratoInner = vibratoArea.reduced(10, 20);

    auto vibratoRateRow = vibratoInner.removeFromTop(25);
    vibratoRateLabel.setBounds(vibratoRateRow.removeFromLeft(80));
    vibratoRateSlider.setBounds(vibratoRateRow.removeFromLeft(200));
    vibratoRateValueLabel.setBounds(vibratoRateRow.removeFromLeft(60));

    auto vibratoDepthRow = vibratoInner.removeFromTop(25);
    vibratoDepthLabel.setBounds(vibratoDepthRow.removeFromLeft(80));
    vibratoDepthSlider.setBounds(vibratoDepthRow.removeFromLeft(200));
    vibratoDepthValueLabel.setBounds(vibratoDepthRow.removeFromLeft(60));

    area.removeFromTop(sectionGap);

    // Reverb Section
    auto reverbArea = area.removeFromTop(sectionHeight - 40);
    reverbGroup.setBounds(reverbArea);
    auto reverbInner = reverbArea.reduced(10, 20);

    auto reverbMixRow = reverbInner.removeFromTop(25);
    reverbMixLabel.setBounds(reverbMixRow.removeFromLeft(80));
    reverbMixSlider.setBounds(reverbMixRow.removeFromLeft(200));
    reverbMixValueLabel.setBounds(reverbMixRow.removeFromLeft(60));

    auto reverbSizeRow = reverbInner.removeFromTop(25);
    reverbSizeLabel.setBounds(reverbSizeRow.removeFromLeft(80));
    reverbSizeSlider.setBounds(reverbSizeRow.removeFromLeft(200));
    reverbSizeValueLabel.setBounds(reverbSizeRow.removeFromLeft(60));

    area.removeFromTop(sectionGap);

    // Envelope Section
    auto envelopeArea = area.removeFromTop(sectionHeight - 40);
    envelopeGroup.setBounds(envelopeArea);
    auto envelopeInner = envelopeArea.reduced(10, 20);

    auto attackRow = envelopeInner.removeFromTop(25);
    attackTimeLabel.setBounds(attackRow.removeFromLeft(80));
    attackTimeSlider.setBounds(attackRow.removeFromLeft(200));
    attackTimeValueLabel.setBounds(attackRow.removeFromLeft(60));

    auto releaseRow = envelopeInner.removeFromTop(25);
    releaseTimeLabel.setBounds(releaseRow.removeFromLeft(80));
    releaseTimeSlider.setBounds(releaseRow.removeFromLeft(200));
    releaseTimeValueLabel.setBounds(releaseRow.removeFromLeft(60));

    area.removeFromTop(sectionGap);

    // Performance Monitor Section
    auto performanceArea = area.removeFromTop(sectionHeight - 60);
    performanceGroup.setBounds(performanceArea);
    auto performanceInner = performanceArea.reduced(10, 20);

    auto activeVoicesRow = performanceInner.removeFromTop(25);
    activeVoicesLabel.setBounds(activeVoicesRow.removeFromLeft(100));
    activeVoicesValueLabel.setBounds(activeVoicesRow.removeFromLeft(100));

    auto cpuUsageRow = performanceInner.removeFromTop(25);
    cpuUsageLabel.setBounds(cpuUsageRow.removeFromLeft(100));
    cpuUsageValueLabel.setBounds(cpuUsageRow.removeFromLeft(80));
    cpuUsageBar.setBounds(cpuUsageRow.removeFromLeft(100).reduced(0, 5));
}

//==============================================================================
void ChoirV2Editor::timerCallback()
{
    updatePerformanceDisplay();
}

void ChoirV2Editor::updatePerformanceDisplay()
{
    const auto& perfStats = processorRef.getPerformanceStats();

    // Update active voices
    int activeVoices = perfStats.active_voices;
    int maxVoices = processorRef.getVoiceManager()->getMaxVoices();
    activeVoicesValueLabel.setText(juce::String(activeVoices) + " / " + juce::String(maxVoices),
                                   juce::dontSendNotification);

    // Update CPU usage
    float cpuUsage = perfStats.cpu_usage * 100.0f;
    cpuUsageValueLabel.setText(juce::String(cpuUsage, 1) + "%", juce::dontSendNotification);

    // Color-code CPU usage
    if (cpuUsage < 50.0f)
        cpuUsageValueLabel.setColour(juce::Label::textColourId, juce::Colours::green);
    else if (cpuUsage < 75.0f)
        cpuUsageValueLabel.setColour(juce::Label::textColourId, juce::Colours::orange);
    else
        cpuUsageValueLabel.setColour(juce::Label::textColourId, juce::Colours::red);

    // Update CPU usage bar
    cpuUsageBar.setColour(juce::Label::backgroundColourId,
                          cpuUsage < 50.0f ? juce::Colours::green :
                          cpuUsage < 75.0f ? juce::Colours::orange :
                                              juce::Colours::red);
}

//==============================================================================
} // namespace ChoirV2
