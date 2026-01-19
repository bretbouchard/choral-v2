#include "dsp/ChoirV2PureDSP.h"
#include <juce_core/juce_core.h>
#include <juce_dsp/juce_dsp.h>

namespace DSP {

//==============================================================================
ChoirV2PureDSP::ChoirV2PureDSP()
{
    // Initialize default parameters
    parameters.set(juce::String(ParameterID::masterVolume), 0.8f);
    parameters.set(juce::String(ParameterID::polyphony), 40.0f);  // Updated to realistic target
    parameters.set(juce::String(ParameterID::vowelX), 0.0f);
    parameters.set(juce::String(ParameterID::vowelY), 0.0f);
    parameters.set(juce::String(ParameterID::vowelZ), 0.0f);
    parameters.set(juce::String(ParameterID::formantScale), 1.0f);
    parameters.set(juce::String(ParameterID::formantShift), 0.0f);
    parameters.set(juce::String(ParameterID::breathMix), 0.0f);
    parameters.set(juce::String(ParameterID::breathColor), 0.5f);
    parameters.set(juce::String(ParameterID::vibratoRate), 5.0f);
    parameters.set(juce::String(ParameterID::vibratoDepth), 0.1f);
    parameters.set(juce::String(ParameterID::vibratoDelay), 0.5f);
    parameters.set(juce::String(ParameterID::tightness), 0.5f);
    parameters.set(juce::String(ParameterID::ensembleSize), 40.0f);  // Updated to match maxPolyphony
    parameters.set(juce::String(ParameterID::voiceSpread), 0.3f);
    parameters.set(juce::String(ParameterID::attack), 0.05f);
    parameters.set(juce::String(ParameterID::decay), 0.2f);
    parameters.set(juce::String(ParameterID::sustain), 0.7f);
    parameters.set(juce::String(ParameterID::release), 0.3f);
    parameters.set(juce::String(ParameterID::sopranoLevel), 0.8f);
    parameters.set(juce::String(ParameterID::altoLevel), 0.8f);
    parameters.set(juce::String(ParameterID::tenorLevel), 0.8f);
    parameters.set(juce::String(ParameterID::bassLevel), 0.8f);
    parameters.set(juce::String(ParameterID::reverbMix), 0.3f);
    parameters.set(juce::String(ParameterID::reverbDecay), 2.5f);
    parameters.set(juce::String(ParameterID::reverbPredelay), 0.02f);
    parameters.set(juce::String(ParameterID::spectralEnhancement), 0.5f);
    parameters.set(juce::String(ParameterID::harmonicsBoost), 0.3f);
    parameters.set(juce::String(ParameterID::subharmonicMix), 0.0f);
    parameters.set(juce::String(ParameterID::subharmonicDepth), 0.5f);
    parameters.set(juce::String(ParameterID::subharmonicRatio), 1.0f);
    parameters.set(juce::String(ParameterID::diphoneCrossfadeDuration), 0.1f);
    parameters.set(juce::String(ParameterID::diphoneFormantSmoothing), 0.5f);
    parameters.set(juce::String(ParameterID::synthesisMethod), 0.0f);
}

ChoirV2PureDSP::~ChoirV2PureDSP()
{
    voices.clear();
}

//==============================================================================
void ChoirV2PureDSP::prepare(double newSampleRate, int newSamplesPerBlock)
{
    sampleRate = newSampleRate;
    samplesPerBlock = newSamplesPerBlock;

    // Initialize DSP modules with corrected implementations
    formantSynthesis = std::make_unique<FormantSynthesis>(sampleRate);
    subharmonicGenerator = std::make_unique<SubharmonicGenerator>(sampleRate);
    spectralEnhancer = std::make_unique<SpectralEnhancer>(sampleRate);

    // Initialize parameter smoothers (10ms smoothing time)
    vowelXSmoother = std::make_unique<LinearSmoother>();
    vowelYSmoother = std::make_unique<LinearSmoother>();
    vowelZSmoother = std::make_unique<LinearSmoother>();
    formantScaleSmoother = std::make_unique<LinearSmoother>();

    vowelXSmoother->setup(sampleRate, 0.01f);
    vowelYSmoother->setup(sampleRate, 0.01f);
    vowelZSmoother->setup(sampleRate, 0.01f);
    formantScaleSmoother->setup(sampleRate, 0.01f);

    // Initialize voice pool with single-threaded SIMD processing
    voices.clear();
    for (int i = 0; i < maxPolyphony; ++i)
    {
        auto* voice = new Voice();
        voice->active = false;
        voice->noteNumber = -1;
        voice->velocity = 0.0f;
        voice->age = 0.0f;
        voice->phase = 0.0f;
        voice->lfoPhase = 0.0f;
        voice->pan = (i % 2 == 0) ? -0.3f : 0.3f;  // Alternate pan positions
        voice->detune = (float(i % 8) - 4.0f) * 0.5f;  // Spread detune

        voices.add(voice);
    }
}

void ChoirV2PureDSP::reset()
{
    activeNotes.clear();
    currentPitchBend = 0.0f;
    currentAftertouch = 0.0f;

    // Reset DSP modules
    if (formantSynthesis) formantSynthesis->reset();
    if (subharmonicGenerator) subharmonicGenerator->reset();
    if (spectralEnhancer) spectralEnhancer->reset();

    // Reset parameter smoothers
    if (vowelXSmoother) vowelXSmoother->reset();
    if (vowelYSmoother) vowelYSmoother->reset();
    if (vowelZSmoother) vowelZSmoother->reset();
    if (formantScaleSmoother) formantScaleSmoother->reset();

    // Reset all voices
    for (auto* voice : voices)
    {
        voice->active = false;
        voice->noteNumber = -1;
        voice->age = 0.0f;
        voice->phase = 0.0f;
        voice->lfoPhase = 0.0f;
    }
}

//==============================================================================
void ChoirV2PureDSP::process(juce::AudioBuffer<float>& buffer,
                            const juce::MidiBuffer& midiMessages)
{
    (void)midiMessages; // Unused until MIDI processing is implemented

    // Clear output buffer
    buffer.clear();

    // Process stereo output
    processStereo(buffer);
}

//==============================================================================
void ChoirV2PureDSP::handleEvent(const ScheduledEvent& event)
{
    switch (event.type)
    {
        case ScheduledEvent::NoteOn:
            processNoteOn(event.noteOn.noteNumber, event.noteOn.velocity);
            break;

        case ScheduledEvent::NoteOff:
            processNoteOff(event.noteOn.noteNumber);
            break;

        case ScheduledEvent::PitchBend:
            processPitchBend(event.pitchBend.pitchBendValue);
            break;

        case ScheduledEvent::Aftertouch:
            processAftertouch(event.aftertouch.aftertouchValue);
            break;

        default:
            break;
    }
}

//==============================================================================
void ChoirV2PureDSP::setParameter(const juce::String& parameterID, float value)
{
    parameters.set(parameterID, value);

    // Trigger parameter smoothing for critical parameters
    if (parameterID == juce::String(ParameterID::vowelX) && vowelXSmoother)
    {
        vowelXSmoother->setTargetValue(value);
    }
    else if (parameterID == juce::String(ParameterID::vowelY) && vowelYSmoother)
    {
        vowelYSmoother->setTargetValue(value);
    }
    else if (parameterID == juce::String(ParameterID::vowelZ) && vowelZSmoother)
    {
        vowelZSmoother->setTargetValue(value);
    }
    else if (parameterID == juce::String(ParameterID::formantScale) && formantScaleSmoother)
    {
        formantScaleSmoother->setTargetValue(value);
    }
}

float ChoirV2PureDSP::getParameter(const juce::String& parameterID) const
{
    return parameters[parameterID];
}

//==============================================================================
juce::String ChoirV2PureDSP::savePreset() const
{
    // Create JSON object with all parameters
    juce::DynamicObject::Ptr json = new juce::DynamicObject();

    // Add all parameters
    for (juce::HashMap<juce::String, float>::Iterator it(parameters);
         it.next();)
    {
        json->setProperty(it.getKey(), it.getValue());
    }

    // Convert to JSON string
    return juce::JSON::toString(juce::var(json));
}

bool ChoirV2PureDSP::loadPreset(const juce::String& presetJson)
{
    // Parse JSON
    auto json = juce::JSON::parse(presetJson);

    if (!json.isObject())
        return false;

    auto* obj = json.getDynamicObject();
    if (obj == nullptr)
        return false;

    // Load all parameters
    for (int i = 0; i < obj->getProperties().size(); ++i)
    {
        auto name = obj->getProperties().getName(i).toString();
        auto value = (float)obj->getProperties().getValueAt(i);

        parameters.set(name, value);
    }

    return true;
}

//==============================================================================
void ChoirV2PureDSP::processStereo(juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    // Get smoothed parameters
    const float vowelX = vowelXSmoother->getNextValue();
    const float vowelY = vowelYSmoother->getNextValue();
    const float vowelZ = vowelZSmoother->getNextValue();
    const float formantScale = formantScaleSmoother->getNextValue();

    // Update formant synthesis based on smoothed parameters
    // Map 3D vowel space to vowel index (0-9)
    int vowelIndex = juce::jlimit(0, 9, int(vowelX * 9.0f));
    formantSynthesis->setCurrentVowel(vowelIndex);
    formantSynthesis->setVibratoRate(parameters[juce::String(ParameterID::vibratoRate)]);
    formantSynthesis->setVibratoDepth(parameters[juce::String(ParameterID::vibratoDepth)]);

    // Update subharmonic generator
    subharmonicGenerator->setSubharmonicMix(parameters[juce::String(ParameterID::subharmonicMix)]);
    subharmonicGenerator->enablePll(true);  // Enable PLL for phase-locked tracking

    // Update spectral enhancer
    spectralEnhancer->setEnhancementAmount(parameters[juce::String(ParameterID::spectralEnhancement)]);
    spectralEnhancer->setHarmonicFocus(parameters[juce::String(ParameterID::harmonicsBoost)]);

    // Create temporary buffers for processing
    juce::AudioBuffer<float> monoBuffer(numSamples, 1);
    juce::AudioBuffer<float> formantBuffer(numSamples, 1);
    juce::AudioBuffer<float> subharmonicBuffer(numSamples, 1);
    juce::AudioBuffer<float> enhancedBuffer(numSamples, 1);

    monoBuffer.clear();
    formantBuffer.clear();
    subharmonicBuffer.clear();
    enhancedBuffer.clear();

    auto* mono = monoBuffer.getWritePointer(0);
    auto* formant = formantBuffer.getWritePointer(0);
    auto* subharmonic = subharmonicBuffer.getWritePointer(0);
    auto* enhanced = enhancedBuffer.getWritePointer(0);

    // Process all active voices (single-threaded SIMD)
    for (auto* voice : voices)
    {
        if (!voice->active)
            continue;

        // Calculate frequency with detune and pitch bend
        float baseFreq = 440.0f * std::pow(2.0f, (voice->noteNumber - 69) / 12.0f);
        float detuneCents = voice->detune + currentPitchBend * 200.0f;  // ±200 cents
        float frequency = baseFreq * std::pow(2.0f, detuneCents / 1200.0f);

        // Generate waveform (simple sawtooth for now)
        for (int i = 0; i < numSamples; ++i)
        {
            float phaseIncrement = frequency / static_cast<float>(sampleRate);
            voice->phase += phaseIncrement;
            if (voice->phase >= 1.0f)
                voice->phase -= 1.0f;

            // Sawtooth oscillator
            float sample = 2.0f * voice->phase - 1.0f;

            // Apply velocity and envelope
            float amplitude = voice->velocity * 0.5f;
            sample *= amplitude;

            // Add to mono mix
            mono[i] += sample;
        }
    }

    // Apply formant synthesis
    if (formantSynthesis)
    {
        formantSynthesis->process(formant, mono, numSamples);
    }

    // Apply subharmonic generation
    if (subharmonicGenerator)
    {
        subharmonicGenerator->process(subharmonic, formant, numSamples);
    }

    // Apply spectral enhancement
    if (spectralEnhancer)
    {
        spectralEnhancer->process(enhanced, subharmonic, numSamples);
    }

    // Apply master volume and mix to stereo outputs
    const float masterVolume = parameters[juce::String(ParameterID::masterVolume)];

    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto* output = buffer.getWritePointer(channel);

        for (int i = 0; i < numSamples; ++i)
        {
            // Simple stereo spread (pan based on voice index)
            float pan = (channel == 0) ? -0.5f : 0.5f;
            float sample = enhanced[i] * masterVolume;

            output[i] = sample * (1.0f - std::abs(pan) * 0.3f);  // 30% stereo spread
        }
    }
}

//==============================================================================
void ChoirV2PureDSP::processNoteOn(int noteNumber, float velocity)
{
    // Find free voice
    for (auto* voice : voices)
    {
        if (!voice->active)
        {
            voice->active = true;
            voice->noteNumber = noteNumber;
            voice->velocity = velocity;
            voice->age = 0.0f;
            voice->phase = 0.0f;
            voice->lfoPhase = 0.0f;

            activeNotes.addIfNotAlreadyThere(noteNumber);
            break;
        }
    }
}

void ChoirV2PureDSP::processNoteOff(int noteNumber)
{
    // Find voice and deactivate
    for (auto* voice : voices)
    {
        if (voice->active && voice->noteNumber == noteNumber)
        {
            voice->active = false;
            voice->noteNumber = -1;
            break;
        }
    }

    activeNotes.removeAllInstancesOf(noteNumber);
}

void ChoirV2PureDSP::processPitchBend(int pitchBendValue)
{
    // Convert to -1 to 1 range
    currentPitchBend = (float)pitchBendValue / 8192.0f;
    currentPitchBend = juce::jlimit(-1.0f, 1.0f, currentPitchBend);
}

void ChoirV2PureDSP::processAftertouch(float aftertouchValue)
{
    currentAftertouch = juce::jlimit(0.0f, 1.0f, aftertouchValue);
}

//==============================================================================
ChoirV2PureDSP::Formant ChoirV2PureDSP::calculateFormants(float vowelX,
                                                          float vowelY,
                                                          float vowelZ) const
{
    (void)vowelX; (void)vowelY; (void)vowelZ; // Unused until implementation

    // TODO: Implement formant calculation based on vowel space position
    // This maps 3D vowel space coordinates to formant frequencies

    Formant formant;

    // Default vowels (example)
    formant.f1 = 800.0f;
    formant.f2 = 1150.0f;
    formant.f3 = 2900.0f;
    formant.f4 = 3900.0f;

    formant.b1 = 80.0f;
    formant.b2 = 90.0f;
    formant.b3 = 120.0f;
    formant.b4 = 130.0f;

    return formant;
}

float ChoirV2PureDSP::synthesizeFormant(float input, float frequency,
                                        float bandwidth, float& phase)
{
    (void)frequency; (void)bandwidth; (void)phase; // Unused until implementation

    // TODO: Implement formant resonance using biquad filter
    // This is a stub that returns the input unchanged
    return input;
}

//==============================================================================
void ChoirV2PureDSP::processReverb(juce::AudioBuffer<float>& buffer)
{
    (void)buffer; // Unused until implementation
    // TODO: Implement reverb effect
    // Use JUCE's ReverbAudioSource or custom algorithm
}

void ChoirV2PureDSP::processSubharmonic(juce::AudioBuffer<float>& buffer)
{
    (void)buffer; // Unused until implementation
    // TODO: Implement subharmonic generation
    // Add frequency-divided content at configurable ratio
}

void ChoirV2PureDSP::processSpectralEnhancement(juce::AudioBuffer<float>& buffer)
{
    (void)buffer; // Unused until implementation
    // TODO: Implement spectral enhancement
    // Boost harmonics using FFT or phase vocoder
}

} // namespace DSP
