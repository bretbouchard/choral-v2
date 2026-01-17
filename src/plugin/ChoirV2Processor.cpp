/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * ChoirV2Processor.cpp - JUCE audio processor implementation
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#include "ChoirV2Processor.h"
#include "ChoirV2Editor.h"

namespace ChoirV2 {

// Parameter ID constants
const juce::String ChoirV2Processor::PARAM_LANGUAGE = "language";
const juce::String ChoirV2Processor::PARAM_LYRICS = "lyrics";
const juce::String ChoirV2Processor::PARAM_NUM_VOICES = "numVoices";
const juce::String ChoirV2Processor::PARAM_MASTER_GAIN = "masterGain";
const juce::String ChoirV2Processor::PARAM_FORMANT_MIX = "formantMix";
const juce::String ChoirV2Processor::PARAM_SUBHARMONIC_MIX = "subharmonicMix";
const juce::String ChoirV2Processor::PARAM_STEREO_WIDTH = "stereoWidth";
const juce::String ChoirV2Processor::PARAM_VIBRATO_RATE = "vibratoRate";
const juce::String ChoirV2Processor::PARAM_VIBRATO_DEPTH = "vibratoDepth";
const juce::String ChoirV2Processor::PARAM_REVERB_MIX = "reverbMix";
const juce::String ChoirV2Processor::PARAM_REVERB_SIZE = "reverbSize";
const juce::String ChoirV2Processor::PARAM_ATTACK_TIME = "attackTime";
const juce::String ChoirV2Processor::PARAM_RELEASE_TIME = "releaseTime";

//==============================================================================
ChoirV2Processor::ChoirV2Processor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), false)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true))
    , parameters(*this, nullptr, juce::Identifier("ChoirV2"), createParameterLayout())
    , reverbPrepared_(false)
    , cpuUsage_(0.0f)
    , activeVoiceCount_(0)
{
    // Initialize engine parameters
    ChoirV2Params params;
    params.num_voices = 60;
    params.sample_rate = 44100.0;
    params.max_block_size = 512;
    params.synthesis_method = "formant";
    params.enable_simd = true;
    params.cpu_limit = 0.8f;
    params.voice_stealing_threshold = 55;
    params.enable_anti_aliasing = true;
    params.enable_spectral_enhancement = true;
    params.oversampling_factor = 2.0f;

    // Create engine
    engine_ = std::make_unique<ChoirV2Engine>(params);
    engine_->initialize();

    // Create voice manager
    voiceManager_ = std::make_unique<VoiceManager>(60, 44100.0);

    // Initialize reverb parameters
    reverbParams_.roomSize = 0.5f;
    reverbParams_.damping = 0.5f;
    reverbParams_.wetLevel = 0.0f;
    reverbParams_.dryLevel = 1.0f;
    reverbParams_.width = 1.0f;
    reverbParams_.freezeMode = 0.0f;

    // Load default language
    currentLanguage_ = "English";
    currentLyrics_ = "Ah";

    // Register as parameter listener
    parameters.addParameterListener(PARAM_MASTER_GAIN, this);
    parameters.addParameterListener(PARAM_FORMANT_MIX, this);
    parameters.addParameterListener(PARAM_SUBHARMONIC_MIX, this);
    parameters.addParameterListener(PARAM_STEREO_WIDTH, this);
    parameters.addParameterListener(PARAM_VIBRATO_RATE, this);
    parameters.addParameterListener(PARAM_VIBRATO_DEPTH, this);
    parameters.addParameterListener(PARAM_REVERB_MIX, this);
    parameters.addParameterListener(PARAM_REVERB_SIZE, this);
    parameters.addParameterListener(PARAM_ATTACK_TIME, this);
    parameters.addParameterListener(PARAM_RELEASE_TIME, this);
}

ChoirV2Processor::~ChoirV2Processor()
{
    parameters.removeParameterListener(PARAM_MASTER_GAIN, this);
    parameters.removeParameterListener(PARAM_FORMANT_MIX, this);
    parameters.removeParameterListener(PARAM_SUBHARMONIC_MIX, this);
    parameters.removeParameterListener(PARAM_STEREO_WIDTH, this);
    parameters.removeParameterListener(PARAM_VIBRATO_RATE, this);
    parameters.removeParameterListener(PARAM_VIBRATO_DEPTH, this);
    parameters.removeParameterListener(PARAM_REVERB_MIX, this);
    parameters.removeParameterListener(PARAM_REVERB_SIZE, this);
    parameters.removeParameterListener(PARAM_ATTACK_TIME, this);
    parameters.removeParameterListener(PARAM_RELEASE_TIME, this);
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout
ChoirV2Processor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // Language (choice parameter)
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        PARAM_LANGUAGE,
        "Language",
        juce::StringArray{ "English", "Latin", "Klingon", "Throat Singing" },
        0
    ));

    // Lyrics (string parameter - using Float parameter as workaround for JUCE 8.0)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        PARAM_LYRICS,
        "Lyrics",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.0f,
        ""
    ));

    // Number of voices (integer: 1-60)
    layout.add(std::make_unique<juce::AudioParameterInt>(
        PARAM_NUM_VOICES,
        "Voices",
        1,
        60,
        40
    ));

    // Master gain (float: -60 to 0 dB)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        PARAM_MASTER_GAIN,
        "Master Gain",
        juce::NormalisableRange<float>(-60.0f, 0.0f, 0.1f),
        -6.0f,
        "dB"
    ));

    // Formant mix (float: 0-100%)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        PARAM_FORMANT_MIX,
        "Formant Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        100.0f,
        "%"
    ));

    // Subharmonic mix (float: 0-100%)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        PARAM_SUBHARMONIC_MIX,
        "Subharmonic Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        0.0f,
        "%"
    ));

    // Stereo width (float: 0-100%)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        PARAM_STEREO_WIDTH,
        "Stereo Width",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        100.0f,
        "%"
    ));

    // Vibrato rate (float: 0-10 Hz)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        PARAM_VIBRATO_RATE,
        "Vibrato Rate",
        juce::NormalisableRange<float>(0.0f, 10.0f, 0.1f),
        5.0f,
        "Hz"
    ));

    // Vibrato depth (float: 0-100%)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        PARAM_VIBRATO_DEPTH,
        "Vibrato Depth",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        0.0f,
        "%"
    ));

    // Reverb mix (float: 0-100%)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        PARAM_REVERB_MIX,
        "Reverb Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        20.0f,
        "%"
    ));

    // Reverb size (float: 0-100%)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        PARAM_REVERB_SIZE,
        "Reverb Size",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        50.0f,
        "%"
    ));

    // Attack time (float: 1-500 ms)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        PARAM_ATTACK_TIME,
        "Attack",
        juce::NormalisableRange<float>(1.0f, 500.0f, 1.0f),
        10.0f,
        "ms"
    ));

    // Release time (float: 10-2000 ms)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        PARAM_RELEASE_TIME,
        "Release",
        juce::NormalisableRange<float>(10.0f, 2000.0f, 1.0f),
        200.0f,
        "ms"
    ));

    return layout;
}

//==============================================================================
void ChoirV2Processor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Prepare voice manager
    voiceManager_->prepare(sampleRate, samplesPerBlock);

    // Initialize smoothed parameters
    smoothedParams_.reset(sampleRate, 0.05);

    // Set initial smoothed parameter values
    auto* masterGainParam = parameters.getRawParameterValue(PARAM_MASTER_GAIN);
    smoothedParams_.masterGain.setTargetValue(juce::Decibels::decibelsToGain(masterGainParam->load()));

    auto* formantMixParam = parameters.getRawParameterValue(PARAM_FORMANT_MIX);
    smoothedParams_.formantMix.setTargetValue(formantMixParam->load() / 100.0f);

    auto* subharmonicMixParam = parameters.getRawParameterValue(PARAM_SUBHARMONIC_MIX);
    smoothedParams_.subharmonicMix.setTargetValue(subharmonicMixParam->load() / 100.0f);

    auto* stereoWidthParam = parameters.getRawParameterValue(PARAM_STEREO_WIDTH);
    smoothedParams_.stereoWidth.setTargetValue(stereoWidthParam->load() / 100.0f);

    auto* vibratoRateParam = parameters.getRawParameterValue(PARAM_VIBRATO_RATE);
    smoothedParams_.vibratoRate.setTargetValue(vibratoRateParam->load());

    auto* vibratoDepthParam = parameters.getRawParameterValue(PARAM_VIBRATO_DEPTH);
    smoothedParams_.vibratoDepth.setTargetValue(vibratoDepthParam->load() / 100.0f);

    auto* reverbMixParam = parameters.getRawParameterValue(PARAM_REVERB_MIX);
    smoothedParams_.reverbMix.setTargetValue(reverbMixParam->load() / 100.0f);

    auto* reverbSizeParam = parameters.getRawParameterValue(PARAM_REVERB_SIZE);
    smoothedParams_.reverbSize.setTargetValue(reverbSizeParam->load() / 100.0f);

    auto* attackTimeParam = parameters.getRawParameterValue(PARAM_ATTACK_TIME);
    smoothedParams_.attackTime.setTargetValue(attackTimeParam->load() / 1000.0f);

    auto* releaseTimeParam = parameters.getRawParameterValue(PARAM_RELEASE_TIME);
    smoothedParams_.releaseTime.setTargetValue(releaseTimeParam->load() / 1000.0f);

    // Prepare reverb
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = 2;
    reverb_.prepare(spec);
    reverbPrepared_ = true;

    // Update engine sample rate
    auto& engineParams = const_cast<ChoirV2Params&>(engine_->getParams());
    engineParams.sample_rate = sampleRate;
    engineParams.max_block_size = samplesPerBlock;
}

void ChoirV2Processor::releaseResources()
{
    voiceManager_->resetStats();
    reverb_.reset();
    reverbPrepared_ = false;
}

//==============================================================================
void ChoirV2Processor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // Clear output buffer
    buffer.clear();

    // Process MIDI messages
    processMidi(midiMessages);

    // Update smoothed parameters
    smoothedParams_.masterGain.skip(buffer.getNumSamples());
    smoothedParams_.formantMix.skip(buffer.getNumSamples());
    smoothedParams_.subharmonicMix.skip(buffer.getNumSamples());
    smoothedParams_.stereoWidth.skip(buffer.getNumSamples());
    smoothedParams_.vibratoRate.skip(buffer.getNumSamples());
    smoothedParams_.vibratoDepth.skip(buffer.getNumSamples());
    smoothedParams_.reverbMix.skip(buffer.getNumSamples());
    smoothedParams_.reverbSize.skip(buffer.getNumSamples());
    smoothedParams_.attackTime.skip(buffer.getNumSamples());
    smoothedParams_.releaseTime.skip(buffer.getNumSamples());

    // Apply parameters to voice manager
    updateParameters();

    // Process audio through voice manager
    float* outputLeft = buffer.getWritePointer(0);
    float* outputRight = buffer.getWritePointer(1);
    voiceManager_->processAudio(outputLeft, outputRight, buffer.getNumSamples());

    // Apply master gain
    float currentGain = smoothedParams_.masterGain.getCurrentValue();
    buffer.applyGain(currentGain);

    // Apply reverb if enabled
    applyReverb(buffer);

    // Update performance stats
    updatePerformanceStats();
}

void ChoirV2Processor::processBlockBypassed(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    // Process MIDI but bypass audio
    processMidi(midiMessages);
    buffer.clear();
}

//==============================================================================
void ChoirV2Processor::processMidi(juce::MidiBuffer& midiMessages)
{
    for (const auto metadata : midiMessages)
    {
        const auto message = metadata.getMessage();
        [[maybe_unused]] const auto timestamp = metadata.samplePosition;

        if (message.isNoteOn())
        {
            int midiNote = message.getNoteNumber();
            float velocity = message.getVelocity();
            voiceManager_->noteOn(midiNote, velocity);
        }
        else if (message.isNoteOff())
        {
            int midiNote = message.getNoteNumber();
            float velocity = message.getVelocity();
            voiceManager_->noteOff(midiNote, velocity);
        }
        else if (message.isAllNotesOff())
        {
            voiceManager_->allNotesOff();
        }
    }
}

void ChoirV2Processor::updateParameters()
{
    // Update voice manager parameters (real-time safe)
    voiceManager_->setMasterGain(smoothedParams_.masterGain.getCurrentValue());
    voiceManager_->setAttackTime(smoothedParams_.attackTime.getCurrentValue());
    voiceManager_->setReleaseTime(smoothedParams_.releaseTime.getCurrentValue());
    voiceManager_->setVibratoRate(smoothedParams_.vibratoRate.getCurrentValue());
    voiceManager_->setVibratoDepth(smoothedParams_.vibratoDepth.getCurrentValue());
}

void ChoirV2Processor::applyReverb(juce::AudioBuffer<float>& buffer)
{
    float reverbMix = smoothedParams_.reverbMix.getCurrentValue();
    float reverbSize = smoothedParams_.reverbSize.getCurrentValue();

    if (reverbMix > 0.0f && reverbPrepared_)
    {
        // Update reverb parameters
        reverbParams_.roomSize = reverbSize;
        reverbParams_.wetLevel = reverbMix * 0.5f;
        reverbParams_.dryLevel = 1.0f - (reverbMix * 0.5f);
        reverb_.setParameters(reverbParams_);

        // Apply reverb using JUCE DSP
        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        reverb_.process(context);
    }
}

void ChoirV2Processor::updatePerformanceStats()
{
    activeVoiceCount_ = voiceManager_->getActiveVoiceCount();

    const auto& voiceStats = voiceManager_->getStats();
    perfStats_.active_voices = activeVoiceCount_;
    perfStats_.cpu_usage = voiceStats.averageCpuUsage;
    perfStats_.stolen_voices = voiceStats.stolenVoices;
    perfStats_.average_latency = voiceStats.averageCpuUsage * 1000.0f; // Convert to ms

    cpuUsage_ = voiceStats.averageCpuUsage;
}

//==============================================================================
void ChoirV2Processor::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (parameterID == PARAM_MASTER_GAIN)
    {
        smoothedParams_.masterGain.setTargetValue(juce::Decibels::decibelsToGain(newValue));
    }
    else if (parameterID == PARAM_FORMANT_MIX)
    {
        smoothedParams_.formantMix.setTargetValue(newValue / 100.0f);
    }
    else if (parameterID == PARAM_SUBHARMONIC_MIX)
    {
        smoothedParams_.subharmonicMix.setTargetValue(newValue / 100.0f);
    }
    else if (parameterID == PARAM_STEREO_WIDTH)
    {
        smoothedParams_.stereoWidth.setTargetValue(newValue / 100.0f);
    }
    else if (parameterID == PARAM_VIBRATO_RATE)
    {
        smoothedParams_.vibratoRate.setTargetValue(newValue);
    }
    else if (parameterID == PARAM_VIBRATO_DEPTH)
    {
        smoothedParams_.vibratoDepth.setTargetValue(newValue / 100.0f);
    }
    else if (parameterID == PARAM_REVERB_MIX)
    {
        smoothedParams_.reverbMix.setTargetValue(newValue / 100.0f);
    }
    else if (parameterID == PARAM_REVERB_SIZE)
    {
        smoothedParams_.reverbSize.setTargetValue(newValue / 100.0f);
    }
    else if (parameterID == PARAM_ATTACK_TIME)
    {
        smoothedParams_.attackTime.setTargetValue(newValue / 1000.0f);
    }
    else if (parameterID == PARAM_RELEASE_TIME)
    {
        smoothedParams_.releaseTime.setTargetValue(newValue / 1000.0f);
    }
}

//==============================================================================
bool ChoirV2Processor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    // Only accept stereo output
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // Input is optional (stereo if present)
    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo() &&
        layouts.getMainInputChannelSet() != juce::AudioChannelSet::disabled())
        return false;

    return true;
}

//==============================================================================
juce::AudioProcessorEditor* ChoirV2Processor::createEditor()
{
    return new ChoirV2Editor(*this);
}

//==============================================================================
void ChoirV2Processor::getStateInformation(juce::MemoryBlock& destData)
{
    // Save state to XML
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void ChoirV2Processor::setStateInformation(const void* data, int sizeInBytes)
{
    // Restore state from XML
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));

    if (xml && xml->hasTagName(parameters.state.getType()))
    {
        parameters.replaceState(juce::ValueTree::fromXml(*xml));
    }
}

//==============================================================================
} // namespace ChoirV2

// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ChoirV2::ChoirV2Processor();
}
