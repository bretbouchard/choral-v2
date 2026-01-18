#pragma once

#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>

namespace DSP {

/**
 * ChoirV2PureDSP - Pure DSP engine for Choir V2.0 instrument
 *
 * Implements advanced choral synthesis with:
 * - Formant synthesis with vowel space navigation
 * - Subharmonic generation for rich harmonics
 * - Spectral enhancement and harmonic boosting
 * - Multi-voice ensemble with configurable sections
 * - Built-in reverb and effects
 * - Diphone phoneme system with crossfading
 */
class ChoirV2PureDSP
{
public:
    //==============================================================================
    // Parameter IDs (must match JUCE AudioProcessorValueTreeState)
    struct ParameterID {
        static constexpr const char* masterVolume = "masterVolume";
        static constexpr const char* polyphony = "polyphony";
        static constexpr const char* textInput = "textInput";
        static constexpr const char* phonemeDisplay = "phonemeDisplay";

        // Vowel space navigation (3D)
        static constexpr const char* vowelX = "vowelX";
        static constexpr const char* vowelY = "vowelY";
        static constexpr const char* vowelZ = "vowelZ";

        // Formant controls
        static constexpr const char* formantScale = "formantScale";
        static constexpr const char* formantShift = "formantShift";

        // Breath and air
        static constexpr const char* breathMix = "breathMix";
        static constexpr const char* breathColor = "breathColor";

        // Vibrato
        static constexpr const char* vibratoRate = "vibratoRate";
        static constexpr const char* vibratoDepth = "vibratoDepth";
        static constexpr const char* vibratoDelay = "vibratoDelay";

        // Ensemble
        static constexpr const char* tightness = "tightness";
        static constexpr const char* ensembleSize = "ensembleSize";
        static constexpr const char* voiceSpread = "voiceSpread";

        // ADSR envelope
        static constexpr const char* attack = "attack";
        static constexpr const char* decay = "decay";
        static constexpr const char* sustain = "sustain";
        static constexpr const char* release = "release";

        // Section levels
        static constexpr const char* sopranoLevel = "sopranoLevel";
        static constexpr const char* altoLevel = "altoLevel";
        static constexpr const char* tenorLevel = "tenorLevel";
        static constexpr const char* bassLevel = "bassLevel";

        // Reverb
        static constexpr const char* reverbMix = "reverbMix";
        static constexpr const char* reverbDecay = "reverbDecay";
        static constexpr const char* reverbPredelay = "reverbPredelay";

        // Enhancement
        static constexpr const char* spectralEnhancement = "spectralEnhancement";
        static constexpr const char* harmonicsBoost = "harmonicsBoost";

        // Subharmonic
        static constexpr const char* subharmonicMix = "subharmonicMix";
        static constexpr const char* subharmonicDepth = "subharmonicDepth";
        static constexpr const char* subharmonicRatio = "subharmonicRatio";

        // Diphone system
        static constexpr const char* diphoneCrossfadeDuration = "diphoneCrossfadeDuration";
        static constexpr const char* diphoneFormantSmoothing = "diphoneFormantSmoothing";

        // Synthesis
        static constexpr const char* synthesisMethod = "synthesisMethod";
    };

    //==============================================================================
    // Scheduled event for MIDI and parameter changes
    struct ScheduledEvent {
        // Forward declare data structures
        struct NoteOnData { int noteNumber; float velocity; };
        struct PitchBendData { int pitchBendValue; };
        struct AftertouchData { float aftertouchValue; };
        struct ParameterData { const char* parameterId; float value; };

        enum Type {
            NoteOn,
            NoteOff,
            PitchBend,
            Aftertouch,
            ParameterChange
        };

        Type type;
        int sampleOffset;
        union {
            NoteOnData noteOn;
            PitchBendData pitchBend;
            AftertouchData aftertouch;
            ParameterData parameter;
        };

        ScheduledEvent() : sampleOffset(0) {}
    };

    //==============================================================================
    ChoirV2PureDSP();
    ~ChoirV2PureDSP();

    //==============================================================================
    // Initialization
    void prepare(double sampleRate, int samplesPerBlock);
    void reset();

    //==============================================================================
    // Processing
    void process(juce::AudioBuffer<float>& buffer,
                 const juce::MidiBuffer& midiMessages);

    //==============================================================================
    // Event handling
    void handleEvent(const ScheduledEvent& event);

    //==============================================================================
    // Parameter control
    void setParameter(const juce::String& parameterID, float value);
    float getParameter(const juce::String& parameterID) const;

    //==============================================================================
    // Preset management
    juce::String savePreset() const;
    bool loadPreset(const juce::String& presetJson);

    //==============================================================================
    // State
    bool isActive() const { return activeNotes.size() > 0; }
    int getActiveNoteCount() const { return activeNotes.size(); }

private:
    //==============================================================================
    // Internal DSP processing
    void processStereo(juce::AudioBuffer<float>& buffer);
    void processNoteOn(int noteNumber, float velocity);
    void processNoteOff(int noteNumber);
    void processPitchBend(int pitchBendValue);
    void processAftertouch(float aftertouchValue);

    //==============================================================================
    // Voice management
    struct Voice {
        int noteNumber;
        float velocity;
        float age;
        bool active;
        float phase[4]; // Formant phases
        float lfoPhase;
    };

    juce::OwnedArray<Voice> voices;
    int maxPolyphony = 64;

    //==============================================================================
    // Parameter storage (normalized 0-1 range)
    juce::HashMap<juce::String, float> parameters;

    // Active note tracking
    juce::Array<int> activeNotes;

    // Current pitch bend (normalized -1 to 1)
    float currentPitchBend = 0.0f;

    // Aftertouch (0-1)
    float currentAftertouch = 0.0f;

    //==============================================================================
    // Synthesis state
    double sampleRate = 44100.0;
    int samplesPerBlock = 512;

    //==============================================================================
    // Formant synthesis
    struct Formant {
        float f1, f2, f3, f4; // Formant frequencies
        float b1, b2, b3, b4; // Formant bandwidths
    };

    Formant calculateFormants(float vowelX, float vowelY, float vowelZ) const;
    float synthesizeFormant(float input, float frequency, float bandwidth, float& phase);

    //==============================================================================
    // Effects
    void processReverb(juce::AudioBuffer<float>& buffer);
    void processSubharmonic(juce::AudioBuffer<float>& buffer);
    void processSpectralEnhancement(juce::AudioBuffer<float>& buffer);

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChoirV2PureDSP)
};

} // namespace DSP
