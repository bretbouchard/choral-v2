# Choir V2.0 Usage Examples

## Table of Contents

- [Quick Start](#quick-start)
- [Basic Synthesis](#basic-synthesis)
- [Language Selection](#language-selection)
- [Synthesis Methods](#synthesis-methods)
- [Parameter Control](#parameter-control)
- [MIDI Integration](#midi-integration)
- [Plugin Integration](#plugin-integration)
- [Custom Languages](#custom-languages)
- [Performance Optimization](#performance-optimization)
- [Troubleshooting](#troubleshooting)

---

## Quick Start

### Standalone Application

```bash
# Build standalone application
./build_plugin.sh --standalone

# Run
./build/artifacts/standalone/ChoirV2
```

### Plugin in DAW

```bash
# Build all plugin formats
./build_plugin.sh --all

# Install (macOS)
./build_plugin.sh --all --install

# Open in DAW
open -a "Logic Pro"
# or
open -a "Ableton Live"
# or
open -a "Reaper"
```

---

## Basic Synthesis

### Simple Choir

```cpp
#include "ChoirV2Engine.h"

// Create engine with 44.1kHz sample rate
ChoirV2Engine engine(44100.0f);

// Initialize
engine.initialize();

// Set up basic choir
engine.setLanguage("en");           // English
engine.setSynthesisMethod(SynthesisMethod::Formant);
engine.setNumVoices(20);
engine.setMasterGain(-6.0f);        // -6 dB

// Play a chord
engine.noteOn(60, 0.8f);  // C
engine.noteOn(64, 0.8f);  // E
engine.noteOn(67, 0.8f);  // G

// Process audio
float* output[2];
float left[256], right[256];
output[0] = left;
output[1] = right;
engine.processAudio(output, 2, 256);
```

### Latin Choir

```cpp
// Set language to Latin
engine.setLanguage("la");

// Set lyrics (for future G2P integration)
engine.setLyrics("Agnus Dei");

// Use formant synthesis
engine.setSynthesisMethod(SynthesisMethod::Formant);

// Large ensemble
engine.setNumVoices(40);
engine.setMasterGain(-3.0f);
```

---

## Language Selection

### English

```cpp
// American English
engine.setLanguage("en");
engine.setLyrics("The quick brown fox");

// British English
engine.setLanguage("en-GB");
engine.setLyrics("Colour and flavour");

// Available English dialects:
// - en (US)
// - en-GB (UK)
// - en-AU (Australian)
// - en-IN (Indian)
```

### Latin

```cpp
// Ecclesiastical Latin
engine.setLanguage("la-ecc");
engine.setLyrics("Kyrie eleison");

// Classical Latin
engine.setLanguage("la-class");
engine.setLyrics("Alea iacta est");
```

### Klingon

```cpp
// Klingon (tlhIngan Hol)
engine.setLanguage("tlh");
engine.setLyrics("nuqneH"); // "Hello"

// Pronunciation: /nuːxnɛx/
```

### Throat Singing

```cpp
// Throat singing techniques
engine.setLanguage("throat-singing");
engine.setSynthesisMethod(SynthesisMethod::Subharmonic);

// Subharmonic mix
// Note: This would use parameter control (see below)
setParameter("subharmonicMix", 0.8f); // 80% subharmonic
```

---

## Synthesis Methods

### Formant Synthesis

**Best for**: Classic choir sounds, clear pronunciation

```cpp
// Set formant synthesis
engine.setSynthesisMethod(SynthesisMethod::Formant);

// Adjust formant mix
setParameter("formantMix", 0.9f); // 90% formant emphasis

// Example: Chamber choir
engine.setNumVoices(16);
engine.setMasterGain(-12.0f);
setParameter("formantMix", 0.8f);
setParameter("stereoWidth", 0.5f);
```

### Diphone Synthesis

**Best for**: Natural speech, diphthongs

```cpp
// Set diphone synthesis
engine.setSynthesisMethod(SynthesisMethod::Diphone);

// Requires diphone database
engine.setLanguage("en");

// Example: Narrator
engine.setNumVoices(1);
engine.setMasterGain(-6.0f);
engine.setLyrics("Once upon a time");
```

### Subharmonic Synthesis

**Best for**: Throat singing, bass effects

```cpp
// Set subharmonic synthesis
engine.setSynthesisMethod(SynthesisMethod::Subharmonic);

// High subharmonic mix
setParameter("subharmonicMix", 0.8f);

// Example: Throat singing
engine.setNumVoices(4);
engine.setMasterGain(-3.0f);
setParameter("subharmonicMix", 0.9f);
setParameter("formantMix", 0.3f);
```

---

## Parameter Control

### Real-Time Parameter Changes

```cpp
// All parameters are smoothed to prevent clicks

// Voice count (1-60)
setParameter("numVoices", 40);

// Master gain (-60 to 0 dB)
setParameter("masterGain", -6.0f);

// Formant mix (0-100%)
setParameter("formantMix", 0.7f);

// Subharmonic mix (0-100%)
setParameter("subharmonicMix", 0.5f);

// Stereo width (0-100%)
setParameter("stereoWidth", 0.8f);

// Vibrato rate (0-10 Hz)
setParameter("vibratoRate", 5.0f);

// Vibrato depth (0-100%)
setParameter("vibratoDepth", 0.3f);

// Reverb mix (0-100%)
setParameter("reverbMix", 0.4f);

// Reverb size (0-100%)
setParameter("reverbSize", 0.6f);

// Attack time (1-500 ms)
setParameter("attackTime", 50.0f);

// Release time (10-2000 ms)
setParameter("releaseTime", 500.0f);
```

### Automation Example

```cpp
// Smooth crossfade between formant and subharmonic
for (int i = 0; i < 100; ++i) {
    float mix = static_cast<float>(i) / 100.0f;
    setParameter("formantMix", 1.0f - mix);
    setParameter("subharmonicMix", mix);

    // Process audio block
    engine.processAudio(output, 2, 256);
}
```

---

## MIDI Integration

### Basic MIDI Input

```cpp
// In your audio processor
void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    // Parse MIDI messages
    for (const auto metadata : midiMessages) {
        auto message = metadata.getMessage();
        auto time = metadata.samplePosition;

        if (message.isNoteOn()) {
            int midiNote = message.getNoteNumber();
            float velocity = message.getVelocity() / 127.0f;
            int channel = message.getChannel();

            engine.noteOn(midiNote, velocity, channel);
        }
        else if (message.isNoteOff()) {
            int midiNote = message.getNoteNumber();
            int channel = message.getChannel();

            engine.noteOff(midiNote, channel);
        }
    }

    // Process audio
    float* output[2];
    output[0] = buffer.getWritePointer(0);
    output[1] = buffer.getWritePointer(1);
    engine.processAudio(output, buffer.getNumChannels(), buffer.getNumSamples());
}
```

### MIDI CC Mapping

```cpp
// Map MIDI CC to parameters
void handleController(juce::MidiMessage& message) {
    int ccNumber = message.getControllerNumber();
    float ccValue = message.getControllerValue() / 127.0f;

    switch (ccNumber) {
        case 1:  // Modulation wheel → Vibrato depth
            setParameter("vibratoDepth", ccValue);
            break;

        case 11: // Expression → Master gain
            setParameter("masterGain", juce::jmap(ccValue, 0.0f, 1.0f, -60.0f, 0.0f));
            break;

        case 74: // Brightness → Formant mix
            setParameter("formantMix", ccValue);
            break;

        default:
            break;
    }
}
```

---

## Plugin Integration

### Loading Presets

```cpp
// Load factory preset
engine.loadPreset("Chamber Choir");

// Load custom preset
engine.loadPreset("/path/to/custom.choirv2");

// Save current state as preset
engine.savePreset("/path/to/my_preset.choirv2");
```

### State Management

```cpp
// Get current state as XML
std::string state = engine.getStateInformation();

// Save to file
std::ofstream file("choir_state.xml");
file << state;
file.close();

// Load state from file
std::ifstream file("choir_state.xml");
std::string state((std::istreambuf_iterator<char>(file)),
                   std::istreambuf_iterator<char>());
file.close();

// Restore state
engine.setStateInformation(state);
```

---

## Custom Languages

### Creating a Language Definition

```json
{
  "name": "Custom Language",
  "code": "custom",
  "phonemes": [
    {
      "symbol": "a",
      "ipa": "a",
      "voiced": true,
      "place": "central",
      "manner": "open",
      "formants": [750.0, 1400.0, 2500.0, 3200.0],
      "bandwidths": [80.0, 100.0, 120.0, 150.0]
    },
    {
      "symbol": "b",
      "ipa": "b",
      "voiced": true,
      "place": "bilabial",
      "manner": "plosive",
      "formants": [200.0, 1200.0, 2200.0, 3000.0],
      "bandwidths": [60.0, 90.0, 110.0, 130.0]
    }
  ],
  "phonotactics": [
    {
      "pattern": "CVCV",
      "description": "Consonant-vowel alternation"
    }
  ]
}
```

### Loading Custom Language

```cpp
// Load language from JSON
LanguageLoader loader;
if (loader.loadFromJSON("/path/to/custom.json")) {
    Language language = loader.getLanguage();
    engine.setLanguage(language);
}
```

---

## Performance Optimization

### CPU Monitoring

```cpp
// Monitor CPU load
float cpuLoad = engine.getCPULoad();
int activeVoices = engine.getActiveVoiceCount();

// Print to console
std::cout << "CPU: " << (cpuLoad * 100.0f) << "% | Voices: " << activeVoices << std::endl;

// Auto-adjust voice count if CPU too high
if (cpuLoad > 0.3f) { // 30% threshold
    int newNumVoices = engine.getNumVoices() - 5;
    engine.setNumVoices(std::max(1, newNumVoices));
}
```

### Voice Stealing

```cpp
// Enable voice stealing (automatic when CPU limit exceeded)
engine.enableVoiceStealing(true);

// Set CPU threshold for stealing
engine.setVoiceStealingThreshold(0.3f); // 30%
```

### SIMD Optimization

```cpp
// Enable SIMD batch processing (automatic if available)
engine.enableSIMD(true);

// Check SIMD availability
if (engine.hasSIMD()) {
    std::cout << "SIMD optimization enabled" << std::endl;
}
```

---

## Preset Examples

### Chamber Choir

```cpp
engine.setLanguage("en");
engine.setSynthesisMethod(SynthesisMethod::Formant);
setParameter("numVoices", 16);
setParameter("masterGain", -12.0f);
setParameter("formantMix", 0.8f);
setParameter("subharmonicMix", 0.0f);
setParameter("stereoWidth", 0.5f);
setParameter("vibratoRate", 5.0f);
setParameter("vibratoDepth", 0.3f);
setParameter("reverbMix", 0.3f);
setParameter("reverbSize", 0.4f);
setParameter("attackTime", 50.0f);
setParameter("releaseTime", 300.0f);
```

### Large Ensemble

```cpp
engine.setLanguage("la");
engine.setSynthesisMethod(SynthesisMethod::Formant);
setParameter("numVoices", 50);
setParameter("masterGain", -3.0f);
setParameter("formantMix", 0.7f);
setParameter("subharmonicMix", 0.1f);
setParameter("stereoWidth", 0.8f);
setParameter("vibratoRate", 6.0f);
setParameter("vibratoDepth", 0.4f);
setParameter("reverbMix", 0.5f);
setParameter("reverbSize", 0.7f);
setParameter("attackTime", 80.0f);
setParameter("releaseTime", 500.0f);
```

### Throat Singing

```cpp
engine.setLanguage("throat-singing");
engine.setSynthesisMethod(SynthesisMethod::Subharmonic);
setParameter("numVoices", 4);
setParameter("masterGain", -3.0f);
setParameter("formantMix", 0.3f);
setParameter("subharmonicMix", 0.9f);
setParameter("stereoWidth", 0.6f);
setParameter("vibratoRate", 4.0f);
setParameter("vibratoDepth", 0.2f);
setParameter("reverbMix", 0.2f);
setParameter("reverbSize", 0.5f);
setParameter("attackTime", 20.0f);
setParameter("releaseTime", 200.0f);
```

### Ethereal Pad

```cpp
engine.setLanguage("en");
engine.setSynthesisMethod(SynthesisMethod::Formant);
setParameter("numVoices", 30);
setParameter("masterGain", -9.0f);
setParameter("formantMix", 0.6f);
setParameter("subharmonicMix", 0.2f);
setParameter("stereoWidth", 1.0f);
setParameter("vibratoRate", 3.0f);
setParameter("vibratoDepth", 0.5f);
setParameter("reverbMix", 0.7f);
setParameter("reverbSize", 0.9f);
setParameter("attackTime", 200.0f);
setParameter("releaseTime", 1000.0f);
```

---

## Troubleshooting

### No Sound Output

```cpp
// Check if voices are active
int activeVoices = engine.getActiveVoiceCount();
if (activeVoices == 0) {
    std::cerr << "No active voices - check note-on events" << std::endl;
}

// Check master gain
float masterGain = getParameter("masterGain");
if (masterGain < -50.0f) {
    std::cerr << "Master gain too low: " << masterGain << " dB" << std::endl;
}

// Check initialization
if (!engine.isInitialized()) {
    std::cerr << "Engine not initialized" << std::endl;
}
```

### Clicks and Pops

```cpp
// Ensure parameter smoothing is enabled
engine.enableParameterSmoothing(true);

// Increase attack/release times
setParameter("attackTime", 100.0f);
setParameter("releaseTime", 500.0f);

// Check for denormal numbers
engine.enableDenormalProtection(true);
```

### High CPU Usage

```cpp
// Reduce number of voices
setParameter("numVoices", 20);

// Disable reverb
setParameter("reverbMix", 0.0f);

// Enable voice stealing
engine.enableVoiceStealing(true);

// Check SIMD optimization
if (!engine.hasSIMD()) {
    std::cerr << "SIMD not available - CPU usage will be higher" << std::endl;
}
```

---

## Advanced Examples

### Real-Time Lyric Synchronization

```cpp
// Set lyrics with timestamps (future feature)
std::vector<LyricEvent> lyrics = {
    {0.0f, "Hello"},
    {1.0f, "world"},
    {2.0f, "from"},
    {2.5f, "Choir"},
    {3.0f, "V2.0"}
};

engine.setLyricsWithTiming(lyrics);
```

### MPE (MIDI Polyphonic Expression) Support

```cpp
// Future feature: MPE support
engine.enableMPE(true);

// Per-note pressure
engine.setNotePressure(60, 0.5f);

// Per-note slide
engine.setNoteSlide(60, 64); // Slide from C to E
```

### Custom DSP Chain

```cpp
// Create custom formant filter
FormantResonator customResonator;
customResonator.setFrequency(800.0f, 44100.0f);
customResonator.setBandwidth(100.0f, 44100.0f);

// Process audio through custom filter
float input[256];
float output[256];
customResonator.processBlock(input, output, 256);
```

---

**Examples Version**: 2.0.0
**Last Updated**: 2026-01-17
**Author**: Bret Bouchard
