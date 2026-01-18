#include "plugin/ChoirV2Processor.h"
#include "plugin/ChoirV2Editor.h"

//==============================================================================
ChoirV2Processor::ChoirV2Processor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
      parameters(*this, nullptr, juce::Identifier("ChoirV2"),
                 createParameterLayout())
#endif
{
    // Initialize PureDSP engine with default parameters
    pureDSP_.prepare(44100.0, 512);

    // Listen to all parameter changes
    auto& params = parameters;
    for (auto* param : params)
    {
        param->addListener(this);
    }

    // Start timer for UI updates (30Hz)
    startTimerHz(30);
}

ChoirV2Processor::~ChoirV2Processor()
{
    // Remove parameter listeners
    auto& params = parameters;
    for (auto* param : params)
    {
        param->removeListener(this);
    }
}

//==============================================================================
void ChoirV2Processor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Prepare PureDSP engine
    pureDSP_.prepare(sampleRate, samplesPerBlock);
}

void ChoirV2Processor::releaseResources()
{
    // PureDSP will handle cleanup
}

void ChoirV2Processor::processBlock(juce::AudioBuffer<float>& buffer,
                                   juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear any output channels that don't contain input data
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Process MIDI events
    processMidiEvents(buffer, midiMessages);

    // Process audio through PureDSP
    pureDSP_.process(buffer, midiMessages);
}

//==============================================================================
void ChoirV2Processor::processMidiEvents(juce::AudioBuffer<float>& buffer,
                                        const juce::MidiBuffer& midiMessages)
{
    // Convert MIDI messages to PureDSP ScheduledEvents
    for (const auto metadata : midiMessages)
    {
        const auto message = metadata.getMessage();
        const int sampleOffset = metadata.samplePosition;

        DSP::ChoirV2PureDSP::ScheduledEvent event;
        event.sampleOffset = sampleOffset;

        if (message.isNoteOn())
        {
            event.type = DSP::ChoirV2PureDSP::ScheduledEvent::NoteOn;
            event.noteNumber = message.getNoteNumber();
            event.velocity = message.getVelocity() / 127.0f;
            pureDSP_.handleEvent(event);
        }
        else if (message.isNoteOff())
        {
            event.type = DSP::ChoirV2PureDSP::ScheduledEvent::NoteOff;
            event.noteNumber = message.getNoteNumber();
            pureDSP_.handleEvent(event);
        }
        else if (message.isPitchBend())
        {
            event.type = DSP::ChoirV2PureDSP::ScheduledEvent::PitchBend;
            event.pitchBendValue = message.getPitchBendValue();
            pureDSP_.handleEvent(event);
        }
        else if (message.isAftertouch())
        {
            event.type = DSP::ChoirV2PureDSP::ScheduledEvent::Aftertouch;
            event.aftertouchValue = message.getAfterTouchValue() / 127.0f;
            pureDSP_.handleEvent(event);
        }
    }
}

//==============================================================================
juce::AudioProcessorEditor* ChoirV2Processor::createEditor()
{
    return new ChoirV2Editor(*this);
}

//==============================================================================
void ChoirV2Processor::getStateInformation(juce::MemoryBlock& destData)
{
    // Save PureDSP preset as JSON
    juce::String presetJson = pureDSP_.savePreset();

    // Create state object
    juce::ValueTree state("ChoirV2State");

    // Store parameters
    state.setProperty("parameters", parameters.copyState().toXmlString(), nullptr);

    // Store PureDSP preset
    state.setProperty("preset", presetJson, nullptr);

    // Store version
    state.setProperty("version", "1.0.0", nullptr);

    // Convert to XML and save
    auto xml = state.createXml();
    copyXmlToBinary(*xml, destData);
}

void ChoirV2Processor::setStateInformation(const void* data, int sizeInBytes)
{
    // Load from XML
    auto xml = getXmlFromBinary(data, sizeInBytes);

    if (xml != nullptr)
    {
        juce::ValueTree state = juce::ValueTree::fromXml(*xml);

        // Restore parameters
        if (state.hasProperty("parameters"))
        {
            auto paramXml = xml->getChildElement(0);
            if (paramXml != nullptr)
            {
                parameters.replaceState(juce::ValueTree::fromXml(*paramXml));
            }
        }

        // Restore PureDSP preset
        if (state.hasProperty("preset"))
        {
            juce::String presetJson = state.getProperty("preset").toString();
            pureDSP_.loadPreset(presetJson);
        }
    }
}

//==============================================================================
void ChoirV2Processor::parameterChanged(const juce::String& parameterID,
                                       float newValue)
{
    // Forward parameter changes to PureDSP
    pureDSP_.setParameter(parameterID, newValue);
}

//==============================================================================
void ChoirV2Processor::timerCallback()
{
    // Trigger async updates for editor
    // This is useful for updating visualizations like:
    // - Active note count
    // - Waveform displays
    // - Spectrum analyzers
    // - VU meters
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout
ChoirV2Processor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Helper macro for creating parameters
    auto makeParam = [](const char* id, const char* name,
                       float min, float max, float def,
                       const char* label = "")
    -> std::unique_ptr<juce::AudioParameterFloat>
    {
        return std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(id, 1),
            name,
            juce::NormalisableRange<float>(min, max),
            def,
            juce::AudioParameterFloatAttributes()
                .withLabel(label)
                .withCategory(juce::AudioParameterFloat::Category::genericParameter)
        );
    };

    // Master
    params.push_back(makeParam(
        DSP::ChoirV2PureDSP::ParameterID::masterVolume,
        "Master Volume", 0.0f, 1.0f, 0.8f
    ));
    params.push_back(makeParam(
        DSP::ChoirV2PureDSP::ParameterID::polyphony,
        "Polyphony", 1.0f, 64.0f, 32.0f
    ));

    // Text input (string parameter)
    params.push_back(std::make_unique<juce::AudioParameterString>(
        juce::ParameterID(DSP::ChoirV2PureDSP::ParameterID::textInput, 1),
        "Text Input",
        ""
    ));

    // Phoneme display (read-only)
    params.push_back(std::make_unique<juce::AudioParameterString>(
        juce::ParameterID(DSP::ChoirV2PureDSP::ParameterID::phonemeDisplay, 1),
        "Phoneme Display",
        "",
        juce::AudioParameterStringAttributes().withAutomatable(false)
    ));

    // Vowel space navigation (3D)
    params.push_back(makeParam(
        DSP::ChoirV2PureDSP::ParameterID::vowelX,
        "Vowel X", -1.0f, 1.0f, 0.0f
    ));
    params.push_back(makeParam(
        DSP::ChoirV2PureDSP::ParameterID::vowelY,
        "Vowel Y", -1.0f, 1.0f, 0.0f
    ));
    params.push_back(makeParam(
        DSP::ChoirV2PureDSP::ParameterID::vowelZ,
        "Vowel Z", -1.0f, 1.0f, 0.0f
    ));

    // Formant controls
    params.push_back(makeParam(
        DSP::ChoirV2PureDSP::ParameterID::formantScale,
        "Formant Scale", 0.5f, 2.0f, 1.0f
    ));
    params.push_back(makeParam(
        DSP::ChoirV2PureDSP::ParameterID::formantShift,
        "Formant Shift", -12.0f, 12.0f, 0.0f
    ));

    // Breath and air
    params.push_back(makeParam(
        DSP::ChoirV2PureDSP::ParameterID::breathMix,
        "Breath Mix", 0.0f, 1.0f, 0.0f
    ));
    params.push_back(makeParam(
        DSP::ChoirV2PureDSP::ParameterID::breathColor,
        "Breath Color", 0.0f, 1.0f, 0.5f
    ));

    // Vibrato
    params.push_back(makeParam(
        DSP::ChoirV2PureDSP::ParameterID::vibratoRate,
        "Vibrato Rate", 0.0f, 15.0f, 5.0f, "Hz"
    ));
    params.push_back(makeParam(
        DSP::ChoirV2PureDSP::ParameterID::vibratoDepth,
        "Vibrato Depth", 0.0f, 1.0f, 0.1f
    ));
    params.push_back(makeParam(
        DSP::ChoirV2PureDSP::ParameterID::vibratoDelay,
        "Vibrato Delay", 0.0f, 2.0f, 0.5f, "s"
    ));

    // Ensemble
    params.push_back(makeParam(
        DSP::ChoirV2PureDSP::ParameterID::tightness,
        "Tightness", 0.0f, 1.0f, 0.5f
    ));
    params.push_back(makeParam(
        DSP::ChoirV2PureDSP::ParameterID::ensembleSize,
        "Ensemble Size", 1.0f, 100.0f, 32.0f
    ));
    params.push_back(makeParam(
        DSP::ChoirV2PureDSP::ParameterID::voiceSpread,
        "Voice Spread", 0.0f, 1.0f, 0.3f
    ));

    // ADSR envelope
    params.push_back(makeParam(
        DSP::ChoirV2PureDSP::ParameterID::attack,
        "Attack", 0.001f, 2.0f, 0.05f, "s"
    ));
    params.push_back(makeParam(
        DSP::ChoirV2PureDSP::ParameterID::decay,
        "Decay", 0.001f, 2.0f, 0.2f, "s"
    ));
    params.push_back(makeParam(
        DSP::ChoirV2PureDSP::ParameterID::sustain,
        "Sustain", 0.0f, 1.0f, 0.7f
    ));
    params.push_back(makeParam(
        DSP::ChoirV2PureDSP::ParameterID::release,
        "Release", 0.01f, 5.0f, 0.3f, "s"
    ));

    // Section levels
    params.push_back(makeParam(
        DSP::ChoirV2PureDSP::ParameterID::sopranoLevel,
        "Soprano Level", 0.0f, 1.0f, 0.8f
    ));
    params.push_back(makeParam(
        DSP::ChoirV2PureDSP::ParameterID::altoLevel,
        "Alto Level", 0.0f, 1.0f, 0.8f
    ));
    params.push_back(makeParam(
        DSP::ChoirV2PureDSP::ParameterID::tenorLevel,
        "Tenor Level", 0.0f, 1.0f, 0.8f
    ));
    params.push_back(makeParam(
        DSP::ChoirV2PureDSP::ParameterID::bassLevel,
        "Bass Level", 0.0f, 1.0f, 0.8f
    ));

    // Reverb
    params.push_back(makeParam(
        DSP::ChoirV2PureDSP::ParameterID::reverbMix,
        "Reverb Mix", 0.0f, 1.0f, 0.3f
    ));
    params.push_back(makeParam(
        DSP::ChoirV2PureDSP::ParameterID::reverbDecay,
        "Reverb Decay", 0.1f, 10.0f, 2.5f, "s"
    ));
    params.push_back(makeParam(
        DSP::ChoirV2PureDSP::ParameterID::reverbPredelay,
        "Reverb Predelay", 0.0f, 0.1f, 0.02f, "s"
    ));

    // Enhancement
    params.push_back(makeParam(
        DSP::ChoirV2PureDSP::ParameterID::spectralEnhancement,
        "Spectral Enhancement", 0.0f, 1.0f, 0.5f
    ));
    params.push_back(makeParam(
        DSP::ChoirV2PureDSP::ParameterID::harmonicsBoost,
        "Harmonics Boost", 0.0f, 1.0f, 0.3f
    ));

    // Subharmonic
    params.push_back(makeParam(
        DSP::ChoirV2PureDSP::ParameterID::subharmonicMix,
        "Subharmonic Mix", 0.0f, 1.0f, 0.0f
    ));
    params.push_back(makeParam(
        DSP::ChoirV2PureDSP::ParameterID::subharmonicDepth,
        "Subharmonic Depth", 0.0f, 1.0f, 0.5f
    ));
    params.push_back(makeParam(
        DSP::ChoirV2PureDSP::ParameterID::subharmonicRatio,
        "Subharmonic Ratio", 0.5f, 2.0f, 1.0f
    ));

    // Diphone system
    params.push_back(makeParam(
        DSP::ChoirV2PureDSP::ParameterID::diphoneCrossfadeDuration,
        "Diphone Crossfade", 0.01f, 0.5f, 0.1f, "s"
    ));
    params.push_back(makeParam(
        DSP::ChoirV2PureDSP::ParameterID::diphoneFormantSmoothing,
        "Formant Smoothing", 0.0f, 1.0f, 0.5f
    ));

    // Synthesis method (choice parameter)
    auto synthesisChoices = juce::StringArray{
        "Formant Synthesis",
        "Additive Synthesis",
        "Concatenative",
        "Hybrid"
    };

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID(DSP::ChoirV2PureDSP::ParameterID::synthesisMethod, 1),
        "Synthesis Method",
        synthesisChoices,
        0
    ));

    return { params.begin(), params.end() };
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ChoirV2Processor();
}
