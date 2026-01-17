# Choir V2.0 Architecture

## System Overview

Choir V2.0 is a **universal vocal synthesis platform** designed to synthesize any human language (natural or constructed) and support any vocal technique, including sub-harmonic frequencies.

## Design Principles

1. **Data-Driven**: Language definitions in JSON, not hardcoded
2. **Pluggable**: Synthesis methods are interchangeable
3. **Real-Time Safe**: No allocations in audio thread
4. **SIMD-Optimized**: Use CPU vectorization for performance
5. **Testable**: Comprehensive unit and integration tests

## Architecture Diagram

```
┌─────────────────────────────────────────────────────────┐
│                    JUCE Plugin Layer                    │
│  ┌───────────────┐  ┌──────────────┐  ┌──────────────┐ │
│  │ Plugin Editor │  │ Plugin       │  │ Parameter    │ │
│  │               │  │ Processor    │  │ Manager      │ │
│  └───────────────┘  └──────────────┘  └──────────────┘ │
├─────────────────────────────────────────────────────────┤
│                    Choir V2.0 Engine                    │
│  ┌───────────────┐  ┌──────────────┐  ┌──────────────┐ │
│  │ Phoneme       │  │ Language     │  │ G2P          │ │
│  │ Database      │  │ Loader       │  │ Engine       │ │
│  └───────────────┘  └──────────────┘  └──────────────┘ │
│  ┌──────────────────────────────────────────────────┐  │
│  │              Voice Manager                        │  │
│  │  - Voice allocation                               │  │
│  │  - Voice stealing (CPU limit)                     │  │
│  │  - Priority management                            │  │
│  └──────────────────────────────────────────────────┘  │
│  ┌───────────────┐  ┌──────────────┐  ┌──────────────┐ │
│  │ Formant       │  │ Diphone      │  │ Subharmonic  │ │
│  │ Synthesis     │  │ Synthesis    │  │ Synthesis    │ │
│  └───────────────┘  └──────────────┘  └──────────────┘ │
├─────────────────────────────────────────────────────────┤
│                    DSP Components                       │
│  ┌──────────────────────────────────────────────────┐  │
│  │ FormantResonator (Real biquad filter)            │  │
│  │ - Coefficient calculation                        │  │
│  │ - Real-time filtering                            │  │
│  └──────────────────────────────────────────────────┘  │
│  ┌──────────────────────────────────────────────────┐  │
│  │ SubharmonicGenerator (PLL-based tracking)        │  │
│  │ - Phase error detection                          │  │
│  │ - Frequency division                             │  │
│  └──────────────────────────────────────────────────┘  │
│  ┌──────────────────────────────────────────────────┐  │
│  │ SpectralEnhancer (Overlap-add FFT)               │  │
│  │ - Windowing (Hanning)                            │  │
│  │ - Spectral envelope enhancement                  │  │
│  └──────────────────────────────────────────────────┘  │
│  ┌──────────────────────────────────────────────────┐  │
│  │ LinearSmoother (Parameter smoothing)             │  │
│  │ - Exponential smoothing                          │  │
│  │ - Click prevention                               │  │
│  └──────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────┘
```

## Core Components

### 1. PhonemeDatabase

**Purpose**: Universal phoneme representation

**Location**: `src/core/PhonemeDatabase.h` / `src/core/PhonemeDatabase.cpp`

**Responsibilities**:
- Store phoneme definitions (IPA, X-SAMPA)
- Provide phoneme lookup by symbol
- Manage phoneme metadata (voicing, place, manner)

**Key Methods**:
```cpp
class PhonemeDatabase {
public:
    bool loadFromJSON(const std::string& jsonPath);
    const Phoneme* getPhoneme(const std::string& symbol) const;
    std::vector<Phoneme> getAllPhonemes() const;
};
```

**Data Structure**:
```cpp
struct Phoneme {
    std::string symbol;        // IPA or X-SAMPA symbol
    std::string ipa;           // IPA notation
    bool voiced;               // Voiced/unvoiced
    PlaceOfArticulation place; // Bilabial, labiodental, etc.
    Manner manner;             // Plosive, fricative, etc.
    std::vector<float> formants; // F1, F2, F3, F4 frequencies
};
```

### 2. LanguageLoader

**Purpose**: Data-driven language definitions

**Location**: `src/core/LanguageLoader.h` / `src/core/LanguageLoader.cpp`

**Responsibilities**:
- Load language definitions from JSON
- Parse phoneme inventories
- Extract phonotactic rules
- Provide language metadata

**Key Methods**:
```cpp
class LanguageLoader {
public:
    bool loadFromJSON(const std::string& jsonPath);
    Language getLanguage() const;
    std::vector<Phoneme> getPhonemeInventory() const;
    std::vector<PhonotacticRule> getPhonotacticRules() const;
};
```

**JSON Schema**:
```json
{
  "name": "English",
  "code": "en",
  "phonemes": [
    {
      "symbol": "i",
      "ipa": "i",
      "voiced": true,
      "place": "front",
      "manner": "close",
      "formants": [280, 2300, 3000, 3500]
    }
  ],
  "phonotactics": [
    {
      "pattern": "CCCVCCCC",
      "description": "Up to 3 consonants initially, 4 finally"
    }
  ]
}
```

### 3. G2PEngine

**Purpose**: Grapheme-to-phoneme conversion

**Location**: `src/core/G2PEngine.h` / `src/core/G2PEngine.cpp`

**Responsibilities**:
- Convert text (graphemes) to phonemes
- Apply language-specific pronunciation rules
- Handle stress and intonation markers
- Support exception dictionaries

**Key Methods**:
```cpp
class G2PEngine {
public:
    void setLanguage(const Language& language);
    std::vector<Phoneme> textToPhonemes(const std::string& text) const;
    void addException(const std::string& word, const std::vector<Phoneme>& phonemes);
};
```

**Algorithm**: Rule-based with exception dictionary
1. Tokenize input text into words
2. Look up word in exception dictionary
3. If not found, apply pronunciation rules
4. Insert stress markers
5. Return phoneme sequence

### 4. VoiceManager

**Purpose**: Multi-voice orchestration

**Location**: `src/core/VoiceManager.h` / `src/core/VoiceManager.cpp`

**Responsibilities**:
- Voice allocation (note-on)
- Voice deallocation (note-off)
- Voice stealing (CPU limit exceeded)
- Priority management (MIDI channel, velocity)

**Key Methods**:
```cpp
class VoiceManager {
public:
    void noteOn(int midiNote, float velocity, int midiChannel);
    void noteOff(int midiNote, int midiChannel);
    void allNotesOff();
    void setMaxVoices(int maxVoices);
    float getCPULoad() const;
    int getActiveVoiceCount() const;
};
```

**Voice Stealing Algorithm**:
1. Check if CPU load exceeds threshold (30%)
2. Find oldest voice with lowest priority
3. Steal voice (graceful fade-out)
4. Allocate new voice

### 5. VoiceAllocator

**Purpose**: Intelligent voice allocation

**Location**: `src/core/VoiceAllocator.h` / `src/core/VoiceAllocator.cpp`

**Responsibilities**:
- Assign synthesis method to voice
- Allocate DSP resources
- Manage voice state

**Key Methods**:
```cpp
class VoiceAllocator {
public:
    VoiceAllocation allocate(VoicePriority priority, float frequency, float amplitude);
    void deallocate(int voiceId);
    bool shouldSteal(float cpuUsage);
};
```

## Synthesis Methods

### 1. FormantSynthesis

**Purpose**: Classic formant-based vocal synthesis

**Location**: `src/synthesis/FormantSynthesis.h` / `src/synthesis/FormantSynthesis.cpp`

**Algorithm**:
1. Generate glottal pulse train (LF model)
2. Filter through formant resonators (F1-F4)
3. Apply spectral enhancement
4. Add vibrato and tremolo

**Components**:
- GlottalSource (LF model)
- FormantResonator (4 parallel biquad filters)
- SpectralEnhancer (FFT-based spectral enhancement)

### 2. DiphoneSynthesis

**Purpose**: Unit concatenation synthesis

**Location**: `src/synthesis/DiphoneSynthesis.h` / `src/synthesis/DiphoneSynthesis.cpp`

**Algorithm**:
1. Break phoneme sequence into diphones
2. Retrieve diphone units from database
3. Concatenate units with overlap
4. Apply pitch-synchronous overlap-add (PSOLA)

**Components**:
- DiphoneDatabase (pre-recorded diphone units)
- PSOLAProcessor (pitch-synchronous overlap-add)
- UnitConcatenator (smooth transitions)

### 3. SubharmonicSynthesis

**Purpose**: Throat singing techniques

**Location**: `src/synthesis/SubharmonicSynthesis.h` / `src/synthesis/SubharmonicSynthesis.cpp`

**Algorithm**:
1. Generate fundamental frequency (F0)
2. Generate subharmonic frequency (F0 / N)
3. Mix fundamental and subharmonic
4. Apply formant filtering

**Components**:
- SubharmonicGenerator (PLL-based frequency division)
- FormantResonator (shared with FormantSynthesis)
- SubharmonicMixer (blend control)

## DSP Components

### 1. FormantResonator

**Purpose**: Real-time formant filtering

**Location**: `src/dsp/FormantResonator.h` / `src/dsp/FormantResonator.cpp`

**Implementation**: Real biquad filter (not complex poles)

**Coefficients**:
```cpp
float b0 = 1.0;
float b1 = -2.0 * r * cos(theta);
float b2 = r * r;
float a0 = 1.0;
float a1 = -2.0 * r * cos(theta) * alpha;
float a2 = r * r * alpha * alpha;
```

**Where**:
- `theta = 2 * pi * Fc / Fs`
- `r = exp(-pi * BW / Fs)`
- `alpha` controls resonance

### 2. SubharmonicGenerator

**Purpose**: Generate subharmonic frequencies

**Location**: `src/dsp/SubharmonicGenerator.h` / `src/dsp/SubharmonicGenerator.cpp`

**Implementation**: PLL-based phase tracking (not simple frequency division)

**Algorithm**:
```cpp
void updatePLL(float input) {
    double phase_error = input - phase_;
    integrator_ += phase_error * ki_;
    double control = kp_ * phase_error + integrator_;
    phase_ += control;
}
```

**Prevents**: Phase drift over time

### 3. SpectralEnhancer

**Purpose**: Enhance spectral envelope

**Location**: `src/dsp/SpectralEnhancer.h` / `src/dsp/SpectralEnhancer.cpp`

**Implementation**: Overlap-add FFT

**Algorithm**:
```cpp
applyHanningWindow(input_buffer);
fft(input_buffer);
processSpectralEnvelope();
ifft(output_buffer);
overlapAdd(output_buffer);
```

**Prevents**: Spectral leakage artifacts

### 4. LinearSmoother

**Purpose**: Smooth parameter transitions

**Location**: `src/dsp/LinearSmoother.h` / `src/dsp/LinearSmoother.cpp`

**Implementation**: Exponential smoothing

**Algorithm**:
```cpp
float process() {
    current_ += (target_ - current_) * coefficient_;
    return current_;
}
```

**Prevents**: Clicks and pops on parameter changes

## Plugin Integration

### JUCE Plugin Wrapper

**Location**: `src/plugin/ChoirV2Processor.h` / `src/plugin/ChoirV2Processor.cpp`

**Responsibilities**:
- MIDI note-on/note-off handling
- Parameter management (13 parameters)
- State management (presets, XML serialization)
- Real-time parameter smoothing
- Performance monitoring

**Audio Processing Chain**:
```
MIDI Input
    ↓
VoiceManager (note-on/note-off)
    ↓
Voice Synthesis (per voice)
    ↓
Parameter Smoothing (LinearSmoothedValue)
    ↓
Voice Processing (pan, gain, envelopes)
    ↓
Master Gain
    ↓
Reverb (juce::dsp::Reverb)
    ↓
Stereo Output
```

## Performance Optimization

### SIMD Batch Processing

**Location**: `src/core/VoiceManager.cpp`

**Implementation**: AVX2 vectorization

**Benefits**:
- Process multiple voices in parallel
- Reduce CPU usage by ~30%
- Maintain real-time performance

### Memory Management

**Pre-allocation**:
- All buffers allocated at startup
- Lock-free data structures
- Memory pools for voice objects

**Benefits**:
- Zero allocations in audio thread
- Predictable memory usage
- No garbage collection pauses

### CPU Optimization

**Strategies**:
- Voice stealing (limit to 60 voices)
- Downsampling (non-critical voices)
- SIMD vectorization
- Denormal protection

**Target**: < 30% CPU @ 60 voices

## Testing Strategy

### Unit Tests

**Location**: `tests/unit/`

**Coverage**: >90%

**Components**:
- FormantResonator tests
- SubharmonicGenerator tests
- SpectralEnhancer tests
- LinearSmoother tests
- VoiceManager tests
- G2PEngine tests

### Integration Tests

**Location**: `tests/integration/`

**Scenarios**:
- End-to-end synthesis pipeline
- Multi-voice orchestration
- Parameter changes
- Preset loading

### Performance Tests

**Location**: `tests/performance/`

**Metrics**:
- CPU usage @ 60 voices
- Latency @ 44.1kHz
- Memory usage
- Real-time safety (no xruns)

## Data Flow

### Text-to-Audio Pipeline

```
User Input (Lyrics)
    ↓
G2PEngine (text → phonemes)
    ↓
PhonemeDatabase (phoneme lookup)
    ↓
VoiceManager (allocate voices)
    ↓
Synthesis Method (formant/diphone/subharmonic)
    ↓
DSP Components (filtering, enhancement)
    ↓
Plugin Processor (mixing, reverb)
    ↓
Audio Output
```

### MIDI-to-Audio Pipeline

```
MIDI Input (note-on)
    ↓
Plugin Processor (MIDI parsing)
    ↓
VoiceManager (allocate voice)
    ↓
Voice (frequency, amplitude)
    ↓
Synthesis Method (generate audio)
    ↓
DSP Components (processing)
    ↓
Plugin Processor (mixing, reverb)
    ↓
Audio Output
```

## Future Enhancements

### Short Term
- [ ] Real-time lyrics processing
- [ ] Additional languages (Hebrew, Persian, Arabic)
- [ ] MPE (MIDI Polyphonic Expression) support
- [ ] Scala scale tuning support

### Long Term
- [ ] Machine learning-based synthesis
- [ ] Voice cloning capabilities
- [ ] Real-time pitch correction
- [ ] Advanced vocal effects (vibrato, tremolo, growl)

## References

### Research Documents
- `/Users/bretbouchard/apps/schill/white_room/docs/research/CHOIR_V2_0_RESEARCH_COMPILATION.md`
- `/Users/bretbouchard/apps/schill/white_room/docs/research/MULTI_LANGUAGE_SUPPORT.md`
- `/Users/bretbouchard/apps/schill/white_room/docs/research/SUBHARMONIC_SYNTHESIS.md`

### Academic Papers
- Klatt (1980) - Formant synthesis
- Liljencrants & Fant (1984) - LF model
- McDonnell (2018) - Subharmonic singing
- Rosenbergl (1971) - Glottal source models

---

**Document Version**: 2.0.0
**Last Updated**: 2026-01-17
**Author**: Bret Bouchard
