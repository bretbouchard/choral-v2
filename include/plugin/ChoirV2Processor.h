#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "dsp/ChoirV2PureDSP.h"

class ChoirV2Processor  : public juce::AudioProcessor,
                            public juce::AudioProcessorValueTreeState::Listener,
                            private juce::Timer
{
public:
    ChoirV2Processor();
    ~ChoirV2Processor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    //==============================================================================
    const juce::String getName() const override { return "Choir V2.0"; }

    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }

    double getTailLengthSeconds() const override { return 2.0; }

    //==============================================================================
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int index) override {}

    const juce::String getProgramName(int index) override { return {}; }
    void changeProgramName(int index, const juce::String& name) override {}

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    //==============================================================================
    void parameterChanged(const juce::String& parameterID, float newValue) override;

    //==============================================================================
    juce::AudioProcessorValueTreeState& getValueTreeState() { return parameters; }

private:
    //==============================================================================
    // Convert MIDI messages to PureDSP ScheduledEvents
    void processMidiEvents(juce::AudioBuffer<float>& buffer,
                          const juce::MidiBuffer& midiMessages);

    // Create all parameters
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Timer callback for UI updates
    void timerCallback() override;

    //==============================================================================
    DSP::ChoirV2PureDSP pureDSP_;
    juce::AudioProcessorValueTreeState parameters;

    // Lock-free queue for parameter changes from audio thread
    juce::AbstractFifo parameterUpdateQueue { 1024 };

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChoirV2Processor)
};
