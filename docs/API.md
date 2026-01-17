# Choir V2.0 API Documentation

## Overview

Choir V2.0 provides a C++ API for vocal synthesis, supporting multiple languages, synthesis methods, and real-time audio processing.

## Table of Contents

- [Core API](#core-api)
- [DSP Components](#dsp-components)
- [Synthesis Methods](#synthesis-methods)
- [Plugin API](#plugin-api)
- [Data Structures](#data-structures)
- [Examples](#examples)

---

## Core API

### ChoirV2Engine

Main synthesis engine coordinating all components.

**Location**: `src/core/ChoirV2Engine.h`

```cpp
class ChoirV2Engine {
public:
    /**
     * Constructor
     * @param sampleRate Audio sample rate in Hz
     */
    ChoirV2Engine(float sampleRate);

    /**
     * Initialize the engine
     * @return true if successful
     */
    bool initialize();

    /**
     * Process audio buffer
     * @param output Output buffer [numChannels][numSamples]
     * @param numChannels Number of audio channels (2 for stereo)
     * @param numSamples Number of samples per channel
     */
    void processAudio(float** output, int numChannels, int numSamples);

    /**
     * Note-on event
     * @param midiNote MIDI note number (0-127)
     * @param velocity Note velocity (0.0-1.0)
     * @param midiChannel MIDI channel (0-15)
     */
    void noteOn(int midiNote, float velocity, int midiChannel = 0);

    /**
     * Note-off event
     * @param midiNote MIDI note number (0-127)
     * @param midiChannel MIDI channel (0-15)
     */
    void noteOff(int midiNote, int midiChannel = 0);

    /**
     * Set language
     * @param languageCode ISO 639-1 code (e.g., "en", "la", "tlh")
     * @return true if language loaded successfully
     */
    bool setLanguage(const std::string& languageCode);

    /**
     * Set synthesis method
     * @param method Synthesis method (formant, diphone, subharmonic)
     */
    void setSynthesisMethod(SynthesisMethod method);

    /**
     * Set lyrics for synthesis
     * @param lyrics Text to synthesize
     */
    void setLyrics(const std::string& lyrics);

    /**
     * Set number of voices
     * @param numVoices Number of voices (1-60)
     */
    void setNumVoices(int numVoices);

    /**
     * Set master gain
     * @param gainDB Gain in decibels (-60 to 0)
     */
    void setMasterGain(float gainDB);

    /**
     * Get CPU load percentage
     * @return CPU load (0.0-1.0)
     */
    float getCPULoad() const;

    /**
     * Get active voice count
     * @return Number of active voices
     */
    int getActiveVoiceCount() const;

    /**
     * Prepare for playback (call after sample rate change)
     * @param sampleRate New sample rate
     * @param samplesPerBlock Expected block size
     */
    void prepareToPlay(float sampleRate, int samplesPerBlock);

private:
    // Internal implementation
};
```

---

## DSP Components

### FormantResonator

Real-time formant filter using biquad implementation.

**Location**: `src/dsp/FormantResonator.h`

```cpp
class FormantResonator {
public:
    /**
     * Constructor
     */
    FormantResonator();

    /**
     * Set formant frequency
     * @param frequency Formant frequency in Hz
     * @param sampleRate Sample rate in Hz
     */
    void setFrequency(float frequency, float sampleRate);

    /**
     * Set formant bandwidth
     * @param bandwidth Bandwidth in Hz
     * @param sampleRate Sample rate in Hz
     */
    void setBandwidth(float bandwidth, float sampleRate);

    /**
     * Set resonance
     * @param resonance Resonance factor (0.0-1.0)
     */
    void setResonance(float resonance);

    /**
     * Process single sample
     * @param input Input sample
     * @return Filtered sample
     */
    float process(float input);

    /**
     * Process block of samples
     * @param input Input buffer
     * @param output Output buffer
     * @param numSamples Number of samples
     */
    void processBlock(const float* input, float* output, int numSamples);

    /**
     * Reset filter state
     */
    void reset();

private:
    // Coefficients
    float b0_, b1_, b2_;
    float a1_, a2_;

    // State
    float x1_, x2_;  // Input delays
    float y1_, y2_;  // Output delays
};
```

### SubharmonicGenerator

Generate subharmonic frequencies using PLL-based tracking.

**Location**: `src/dsp/SubharmonicGenerator.h`

```cpp
class SubharmonicGenerator {
public:
    /**
     * Constructor
     */
    SubharmonicGenerator();

    /**
     * Set subharmonic ratio
     * @param ratio Ratio (e.g., 2.0 for octave below)
     */
    void setRatio(float ratio);

    /**
     * Set PLL parameters
     * @param kp Proportional gain
     * @param ki Integral gain
     */
    void setPLLParameters(double kp, double ki);

    /**
     * Process single sample
     * @param input Input sample (fundamental frequency)
     * @return Subharmonic sample
     */
    float process(float input);

    /**
     * Process block of samples
     * @param input Input buffer
     * @param output Output buffer
     * @param numSamples Number of samples
     */
    void processBlock(const float* input, float* output, int numSamples);

    /**
     * Reset phase accumulator
     */
    void reset();

private:
    double phase_;
    double integrator_;
    double kp_, ki_;
    float ratio_;
};
```

### LinearSmoother

Smooth parameter transitions to prevent clicks.

**Location**: `src/dsp/LinearSmoother.h`

```cpp
class LinearSmoother {
public:
    /**
     * Constructor
     */
    LinearSmoother();

    /**
     * Set time constant
     * @param timeConstant Time constant in seconds
     * @param sampleRate Sample rate in Hz
     */
    void setTimeConstant(float timeConstant, float sampleRate);

    /**
     * Set target value
     * @param target Target value
     */
    void setTarget(float target);

    /**
     * Set current value immediately (no smoothing)
     * @param value Value to set
     */
    void setCurrentValue(float value);

    /**
     * Process single sample
     * @return Smoothed value
     */
    float process();

    /**
     * Process block of samples
     * @param output Output buffer
     * @param numSamples Number of samples
     */
    void processBlock(float* output, int numSamples);

    /**
     * Check if smoothing is complete
     * @return true if target reached
     */
    bool isSmoothing() const;

private:
    float current_;
    float target_;
    float coefficient_;
};
```

### SpectralEnhancer

Enhance spectral envelope using overlap-add FFT.

**Location**: `src/dsp/SpectralEnhancer.h`

```cpp
class SpectralEnhancer {
public:
    /**
     * Constructor
     */
    SpectralEnhancer();

    /**
     * Set enhancement amount
     * @param amount Enhancement amount (0.0-1.0)
     */
    void setEnhancementAmount(float amount);

    /**
     * Set FFT size
     * @param fftSize FFT size (must be power of 2)
     */
    void setFFTSize(int fftSize);

    /**
     * Process single sample
     * @param input Input sample
     * @return Enhanced sample
     */
    float process(float input);

    /**
     * Process block of samples
     * @param input Input buffer
     * @param output Output buffer
     * @param numSamples Number of samples
     */
    void processBlock(const float* input, float* output, int numSamples);

    /**
     * Reset processing state
     */
    void reset();

private:
    // FFT implementation
    std::unique_ptr<FFT> fft_;
    std::vector<float> window_;
    std::vector<float> overlapBuffer_;
};
```

---

## Synthesis Methods

### FormantSynthesis

Classic formant-based vocal synthesis.

**Location**: `src/synthesis/FormantSynthesis.h`

```cpp
class FormantSynthesis {
public:
    /**
     * Constructor
     */
    FormantSynthesis();

    /**
     * Initialize synthesis
     * @param sampleRate Sample rate in Hz
     * @return true if successful
     */
    bool initialize(float sampleRate);

    /**
     * Set phoneme to synthesize
     * @param phoneme Phoneme definition
     */
    void setPhoneme(const Phoneme& phoneme);

    /**
     * Set fundamental frequency
     * @param frequency Frequency in Hz
     */
    void setFrequency(float frequency);

    /**
     * Set vibrato parameters
     * @param rate Vibrato rate in Hz
     * @param depth Vibrato depth (0.0-1.0)
     */
    void setVibrato(float rate, float depth);

    /**
     * Generate audio
     * @param output Output buffer
     * @param numSamples Number of samples to generate
     */
    void generate(float* output, int numSamples);

    /**
     * Reset synthesis state
     */
    void reset();

private:
    std::unique_ptr<GlottalSource> glottalSource_;
    std::array<std::unique_ptr<FormantResonator>, 4> formants_;
    std::unique_ptr<SpectralEnhancer> spectralEnhancer_;
    float frequency_;
    float vibratoRate_;
    float vibratoDepth_;
};
```

### DiphoneSynthesis

Unit concatenation synthesis.

**Location**: `src/synthesis/DiphoneSynthesis.h`

```cpp
class DiphoneSynthesis {
public:
    /**
     * Constructor
     */
    DiphoneSynthesis();

    /**
     * Initialize synthesis
     * @param sampleRate Sample rate in Hz
     * @param diphoneDatabasePath Path to diphone database
     * @return true if successful
     */
    bool initialize(float sampleRate, const std::string& diphoneDatabasePath);

    /**
     * Set phoneme sequence
     * @param phonemes Phoneme sequence to synthesize
     */
    void setPhonemes(const std::vector<Phoneme>& phonemes);

    /**
     * Set fundamental frequency contour
     * @param frequencies Frequency contour (Hz per sample)
     */
    void setFrequencyContour(const std::vector<float>& frequencies);

    /**
     * Generate audio
     * @param output Output buffer
     * @param numSamples Number of samples to generate
     */
    void generate(float* output, int numSamples);

    /**
     * Reset synthesis state
     */
    void reset();

private:
    std::unique_ptr<DiphoneDatabase> diphoneDatabase_;
    std::unique_ptr<PSOLAProcessor> psolaProcessor_;
    std::unique_ptr<UnitConcatenator> unitConcatenator_;
};
```

### SubharmonicSynthesis

Throat singing synthesis.

**Location**: `src/synthesis/SubharmonicSynthesis.h`

```cpp
class SubharmonicSynthesis {
public:
    /**
     * Constructor
     */
    SubharmonicSynthesis();

    /**
     * Initialize synthesis
     * @param sampleRate Sample rate in Hz
     * @return true if successful
     */
    bool initialize(float sampleRate);

    /**
     * Set phoneme to synthesize
     * @param phoneme Phoneme definition
     */
    void setPhoneme(const Phoneme& phoneme);

    /**
     * Set fundamental frequency
     * @param frequency Frequency in Hz
     */
    void setFrequency(float frequency);

    /**
     * Set subharmonic ratio
     * @param ratio Subharmonic ratio (e.g., 2.0 for octave)
     */
    void setSubharmonicRatio(float ratio);

    /**
     * Set subharmonic mix
     * @param mix Mix amount (0.0-1.0)
     */
    void setSubharmonicMix(float mix);

    /**
     * Generate audio
     * @param output Output buffer
     * @param numSamples Number of samples to generate
     */
    void generate(float* output, int numSamples);

    /**
     * Reset synthesis state
     */
    void reset();

private:
    std::unique_ptr<GlottalSource> glottalSource_;
    std::unique_ptr<SubharmonicGenerator> subharmonicGenerator_;
    std::array<std::unique_ptr<FormantResonator>, 4> formants_;
    float frequency_;
    float subharmonicRatio_;
    float subharmonicMix_;
};
```

---

## Plugin API

### ChoirV2Processor

JUCE audio processor for plugin integration.

**Location**: `src/plugin/ChoirV2Processor.h`

```cpp
class ChoirV2Processor : public juce::AudioProcessor {
public:
    /**
     * Constructor
     */
    ChoirV2Processor();

    /**
     * Destructor
     */
    ~ChoirV2Processor() override;

    /**
     * Prepare to play
     * @param sampleRate Sample rate
     * @param samplesPerBlock Samples per block
     */
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;

    /**
     * Release resources
     */
    void releaseResources() override;

    /**
     * Process audio block
     * @param buffer Audio buffer
     */
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

    /**
     * Get editor
     * @return Plugin editor
     */
    juce::AudioProcessorEditor* createEditor() override;

    /**
     * Has editor
     * @return true (has UI)
     */
    bool hasEditor() const override;

    /**
     * Get processor name
     * @return Plugin name
     */
    const juce::String getName() const override;

    /**
     * Accepts MIDI
     * @return true (accepts MIDI input)
     */
    bool acceptsMidi() const override;

    /**
     * Produces MIDI
     * @return false (audio only)
     */
    bool producesMidi() const override;

    /**
     * Get parameter count
     * @return Number of parameters
     */
    int getNumParameters() override;

    /**
     * Get parameter name
     * @param index Parameter index
     * @return Parameter name
     */
    const juce::String getParameterName(int index) override;

    /**
     * Get parameter
     * @param index Parameter index
     * @return Parameter value
     */
    float getParameter(int index) override;

    /**
     * Set parameter
     * @param index Parameter index
     * @param newValue Parameter value
     */
    void setParameter(int index, float newValue) override;

    /**
     * Get state information
     * @return State as XML
     */
    juce::String getStateInformation() override;

    /**
     * Set state information
     * @param state State XML
     */
    void setStateInformation(const juce::String& state) override;

private:
    // Engine
    std::unique_ptr<ChoirV2Engine> engine_;

    // Parameters
    juce::AudioProcessorValueTreeState parameters_;

    // Smoothers
    std::array<juce::LinearSmoothedValue<float>, 13> parameterSmoothers_;

    // Reverb
    juce::dsp::Reverb reverb_;

    // Performance monitoring
    float cpuLoad_;
    int activeVoices_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChoirV2Processor)
};
```

---

## Data Structures

### Phoneme

Phoneme definition.

```cpp
struct Phoneme {
    std::string symbol;              // IPA or X-SAMPA symbol
    std::string ipa;                 // IPA notation
    bool voiced;                     // Voiced/unvoiced
    PlaceOfArticulation place;       // Bilabial, labiodental, etc.
    Manner manner;                   // Plosive, fricative, etc.
    std::vector<float> formants;     // F1, F2, F3, F4 frequencies
    std::vector<float> bandwidths;   // B1, B2, B3, B4 bandwidths
};
```

### Language

Language definition.

```cpp
struct Language {
    std::string name;                // Language name
    std::string code;                // ISO 639-1 code
    std::vector<Phoneme> phonemes;   // Phoneme inventory
    std::vector<PhonotacticRule> phonotactics; // Phonotactic rules
    std::vector<ProsodyRule> prosody; // Prosody rules
};
```

### SynthesisMethod

Synthesis method enum.

```cpp
enum class SynthesisMethod {
    Formant,         // Formant synthesis
    Diphone,         // Diphone synthesis
    Subharmonic      // Subharmonic synthesis
};
```

---

## Examples

### Basic Usage

```cpp
#include "ChoirV2Engine.h"

// Create engine
ChoirV2Engine engine(44100.0f);

// Initialize
if (!engine.initialize()) {
    std::cerr << "Failed to initialize engine" << std::endl;
    return 1;
}

// Set language
engine.setLanguage("en");

// Set lyrics
engine.setLyrics("Hello world");

// Set synthesis method
engine.setSynthesisMethod(SynthesisMethod::Formant);

// Set number of voices
engine.setNumVoices(40);

// Set master gain
engine.setMasterGain(-6.0f);

// Note-on
engine.noteOn(60, 0.8f); // Middle C, velocity 0.8

// Process audio
float* output[2];
float leftBuffer[256];
float rightBuffer[256];
output[0] = leftBuffer;
output[1] = rightBuffer;

engine.processAudio(output, 2, 256);

// Note-off
engine.noteOff(60);
```

### Custom Phoneme

```cpp
#include "PhonemeDatabase.h"

// Create custom phoneme
Phoneme phoneme;
phoneme.symbol = "æ";
phoneme.ipa = "æ";
phoneme.voiced = true;
phoneme.place = PlaceOfArticulation::Front;
phoneme.manner = Manner::Open;
phoneme.formants = {750.0f, 1400.0f, 2500.0f, 3200.0f};
phoneme.bandwidths = {80.0f, 100.0f, 120.0f, 150.0f};

// Add to database
PhonemeDatabase db;
db.addPhoneme(phoneme);
```

### Real-Time Parameter Smoothing

```cpp
#include "LinearSmoother.h"

// Create smoother
LinearSmoother smoother;

// Set time constant (50ms)
smoother.setTimeConstant(0.05f, 44100.0f);

// Set target
smoother.setTarget(1.0f);

// Process block
float output[256];
smoother.processBlock(output, 256);

// Use smoothed values
for (int i = 0; i < 256; ++i) {
    float smoothed = output[i];
    // Use smoothed value in DSP
}
```

---

## Thread Safety

### Audio Thread

**Real-Time Safe Operations**:
- `noteOn()` / `noteOff()`
- `processAudio()`
- `getActiveVoiceCount()`

**Restrictions**:
- No memory allocation
- No locks/mutexes
- No file I/O
- Bounded execution time

### UI Thread

**Non-Real-Time Operations**:
- `setLanguage()`
- `setLyrics()`
- `setSynthesisMethod()`
- `setParameter()` (uses smoothing)

---

## Error Handling

### Return Values

- `true` = Success
- `false` = Failure

### Exceptions

No exceptions thrown in audio thread (real-time safety).

### Logging

Use standard logging for diagnostics:

```cpp
// In non-audio thread
std::cerr << "Error: Failed to load language" << std::endl;

// In audio thread (use lock-free logging)
juce::Logger::writeToLog("Error in audio thread");
```

---

**API Version**: 2.0.0
**Last Updated**: 2026-01-17
**Author**: Bret Bouchard
