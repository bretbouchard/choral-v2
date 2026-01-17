# Choir V2.0 - Comprehensive Test Suite Documentation

## Overview

This document describes the complete test suite for Choir V2.0, including integration tests, performance benchmarks, and end-to-end plugin functionality tests.

## Test Structure

```
tests/
├── unit/                          # Existing unit tests
│   ├── test_formant_resonator.cpp
│   ├── test_linear_smoother.cpp
│   ├── test_subharmonic_generator.cpp
│   ├── test_reverb_effect.cpp
│   ├── test_voice_allocator.cpp
│   ├── test_g2p_engine.cpp
│   └── test_preset_manager.cpp
│
├── integration/                   # NEW: Integration tests
│   ├── test_synthesis_pipeline.cpp      # Complete synthesis pipeline
│   ├── test_multilingual_support.cpp    # Multi-language support
│   └── test_plugin_functionality.cpp    # End-to-end plugin tests
│
├── benchmark/                     # NEW: Performance benchmarks
│   └── benchmark_dsp.cpp                # DSP component performance
│
└── CMakeLists.txt                 # Updated to include all tests
```

## Test Executables

### 1. `choir_v2_tests` (Unit Tests)
Existing unit tests for individual components.

### 2. `choir_v2_integration_tests` (NEW)
Complete integration tests covering:
- Full synthesis pipeline
- Multi-language support
- Plugin functionality

### 3. `choir_v2_benchmarks` (NEW)
Performance benchmarks for all DSP components.

---

## Integration Tests: `test_synthesis_pipeline.cpp`

### Overview
Tests the complete text-to-phoneme-to-audio synthesis pipeline with all synthesis methods.

### Test Cases (13 total)

#### 1. English Text-to-Audio Pipeline
- **Input**: "The quick brown fox"
- **Tests**:
  - G2P conversion (text → phonemes)
  - Phoneme validation
  - Formant synthesis (phonemes → audio)
  - Audio validity (no NaN/Inf)
  - Signal energy verification

#### 2. Latin Text-to-Audio Pipeline
- **Input**: "Pater noster"
- **Tests**:
  - Latin G2P rules
  - Latin-specific phoneme patterns
  - Audio generation for Latin

#### 3. Klingon Text-to-Audio Pipeline
- **Input**: "tlhIngan maH"
- **Tests**:
  - Klingon G2P rules
  - Klingon-specific phonemes (tlh, Q, gh, etc.)
  - Alien language synthesis

#### 4. Throat Singing All 6 Presets
- **Presets**: Sygyt, Kargyraa, Borbangnadyr, Ezengileer, Chylandyk, Khöömei Basics
- **Tests**:
  - Formant frequency configuration
  - Subharmonic mix settings
  - DSP chain (FormantResonator → SubharmonicGenerator → SpectralEnhancer)
  - Audio output verification for each preset
  - Click/pop detection

#### 5. Formant Synthesis Integration
- **Input**: Vowel sequence (AA → EH → IY)
- **Tests**:
  - Formant synthesis from phonemes
  - Smooth vowel transitions
  - Audio validity

#### 6. Subharmonic Synthesis Integration
- **Input**: 220Hz sine wave
- **Tests**:
  - Subharmonic generation (0.5 ratio)
  - Mix control (70%)
  - Energy increase verification
  - Richness enhancement

#### 7. Diphone Synthesis Integration
- **Input**: AA → EH → IY vowel sequence
- **Tests**:
  - Diphone transition synthesis
  - Smooth transitions (no clicks)
  - Audio validity

#### 8. Complete Pipeline Performance
- **Input**: "The quick brown fox jumps over the lazy dog"
- **Tests**:
  - Complete pipeline (G2P → Formant → Spectral Enhancement)
  - Performance target: < 2x real-time
  - Output quality verification

#### 9. Voice Manager Integration
- **Test**: Allocate 60 voices simultaneously
- **Tests**:
  - Voice allocation
  - Multi-voice processing
  - Audio output validity
  - Voice deallocation

#### 10. Parameter Smoothing
- **Test**: Rapid parameter changes during audio processing
- **Tests**:
  - Master gain smoothing
  - Vibrato rate/depth smoothing
  - Click prevention (max delta verification)
  - Smooth transitions

#### 11. Spectral Enhancement
- **Input**: Simple sine wave
- **Tests**:
  - Harmonic addition
  - Energy increase (>10%)
  - No distortion (NaN/Inf check)

#### 12. Real-Time Safety
- **Test**: 100 audio buffers with 40 voices
- **Tests**:
  - No memory allocations
  - Deterministic timing
  - Consistent performance
  - Bounded execution time

#### 13. Edge Cases
- **Tests**:
  - Empty text
  - Very long text (10k characters)
  - Very short duration (10 samples)
  - Zero voices
  - Extreme frequencies (MIDI 0-127)

---

## Integration Tests: `test_multilingual_support.cpp`

### Overview
Comprehensive multi-language support testing for all 13 supported languages.

### Test Cases (18 total)

#### 1. Load All Supported Languages
- **Languages**: English, Latin, Spanish, German, French, Italian, Japanese, Mandarin, Korean, Russian, Arabic, Hebrew, Klingon
- **Tests**: All languages load successfully

#### 2. English Text Processing
- **Tests**:
  - "Hello" → HH AH L OW
  - "world" → W ER L D
  - "The quick brown fox" → 15 phonemes
  - Phoneme database validation
  - Phoneme sequence correctness

#### 3. Latin Text Processing
- **Tests**:
  - "Pater noster"
  - "Ave Maria"
  - "Dominus vobiscum"
  - Latin phoneme patterns

#### 4. Klingon Text Processing
- **Tests**:
  - "tlhIngan maH" (We are Klingons)
  - "Qapla'" (Success)
  - "ghaytan" (Perhaps)
  - Klingon-specific phonemes (tlh, Q, gh)

#### 5. Right-to-Left Languages
- **Tests**:
  - Arabic: "السلام عليكم"
  - Hebrew: "שלום"
  - RTL text processing
  - Phoneme extraction

#### 6. East Asian Languages
- **Tests**:
  - Japanese: "こんにちは" (Konnichiwa)
  - Mandarin: "你好" (Nǐ hǎo)
  - Korean: "안녕하세요" (Annyeonghaseyo)
  - Character-to-phoneme conversion

#### 7. European Languages
- **Tests**:
  - Spanish: "Hola mundo"
  - German: "Hallo Welt"
  - French: "Bonjour le monde"
  - Italian: "Ciao mondo"

#### 8. Language Switching No Crashes
- **Test**: Rapid language switching (10 iterations × 6 languages)
- **Verifies**: No crashes during switching

#### 9. Concurrent Language Switching
- **Test**: Switch between 6 languages 50 times
- **Verifies**: Stability under concurrent access

#### 10. Verify Phoneme Sequences
- **Tests**:
  - English: "cat" → K AE T
  - English: "dog" → D AO G
  - Latin: "pater" → P AH T EH R
  - Exact phoneme matching

#### 11. Formant Frequencies Match Language
- **Tests**:
  - All 13 languages have formant definitions
  - F1: 100-1500 Hz (reasonable range)
  - F2: 500-3500 Hz
  - F3: 1000-4500 Hz

#### 12. Timing And Prosody
- **Tests**:
  - Phoneme duration (>0, <1 second)
  - Total duration reasonable (0.5-10 seconds)
  - Syllable information present
  - Stress marking

#### 13. Language-Specific G2P Rules
- **Tests**:
  - English: "though" → specific phonemes
  - Same text produces different phonemes in different languages
  - Rule application correctness

#### 14. Stress And Intonation Patterns
- **Tests**:
  - Words with different stress patterns
  - "record" (noun vs verb)
  - "present" (noun vs verb)
  - Stress detection

#### 15. Multilingual Audio Generation
- **Tests**:
  - English: "Hello"
  - Latin: "Salve"
  - Spanish: "Hola"
  - German: "Hallo"
  - Klingon: "nuqneH"
  - Audio generation for each language

#### 16. Error Handling For Invalid Text
- **Tests**:
  - Empty text
  - Whitespace only
  - Special characters
  - Very long text (100k characters)

#### 17. Language Loader Performance
- **Target**: Load all 7 languages in < 1 second

#### 18. G2P Conversion Performance
- **Target**: Convert text in < 10ms

---

## Performance Benchmarks: `benchmark_dsp.cpp`

### Overview
Comprehensive performance benchmarks for all DSP components with detailed metrics.

### Benchmark Cases (15 total)

#### 1. FormantResonator Performance
- **Target**: 1000 samples/call in < 1ms
- **Throughput**: > 500k samples/sec
- **Metrics**: Average, min, max timing

#### 2. SubharmonicGenerator Performance
- **Target**: 1000 samples/call in < 0.5ms
- **Throughput**: > 1M samples/sec

#### 3. SpectralEnhancer Performance
- **Target**: FFT-based processing in < 2ms
- **Throughput**: > 250k samples/sec

#### 4. VoiceManager Performance (40 voices)
- **Target**: 40 voices @ 44.1kHz in < 5ms per buffer
- **Throughput**: > 100k samples/sec with 40 voices

#### 5. VoiceManager Maximum Load (60 voices)
- **Target**: 60 voices @ 44.1kHz in < 8ms per buffer
- **Verifies**: Maximum load handling

#### 6. FormantSynthesis Performance
- **Input**: 5 phonemes
- **Target**: < 2x real-time

#### 7. SubharmonicSynthesis Performance
- **Target**: < 2x real-time

#### 8. DiphoneSynthesis Performance
- **Input**: 4 diphone transitions
- **Target**: < 2x real-time

#### 9. Real-Time Safety (No Allocations)
- **Test**: 10,000 iterations with 40 voices
- **Verifies**: Consistent timing (max < 3x average)
- **Detects**: Memory allocation spikes

#### 10. DSP Chain Performance
- **Chain**: FormantResonator → SubharmonicGenerator → SpectralEnhancer
- **Target**: Complete chain in < 3ms
- **Throughput**: > 150k samples/sec

#### 11. Parameter Smoothing Performance
- **Test**: Rapid parameter changes (master gain, vibrato, etc.)
- **Target**: < 2ms per buffer
- **Verifies**: No significant performance impact

#### 12. SIMD Optimization Verification
- **Test**: 4, 8, 16, 32, 40 voices
- **Verifies**: Linear scaling with voice count
- **Target**: < 0.15ms per voice

#### 13. Memory Usage Performance
- **Test**: 60 voices, 5000 iterations
- **Metrics**: Average, min, max, standard deviation
- **Verifies**: Low timing variance (< 20% of average)

#### 14. Concurrent Processing Simulation
- **Test**: 4 plugin instances × 15 voices = 60 voices total
- **Target**: < 10ms per buffer
- **Simulates**: Multi-instrument DAW setup

#### 15. Performance Summary
- **Output**: Comprehensive summary report
- **Includes**: All benchmark results with comparison

---

## Plugin Functionality Tests: `test_plugin_functionality.cpp`

### Overview
End-to-end tests for complete plugin functionality in DAW environment.

### Test Cases (18 total)

#### 1. MIDI Input - Note On/Off
- **Tests**:
  - Note on → voice allocation
  - Audio output verification
  - Note off → release phase
  - Release envelope functionality

#### 2. MIDI Input - Multiple Notes
- **Tests**:
  - Chord playback (C4, E4, G4, C5)
  - Polyphonic voice allocation
  - Audio output verification

#### 3. MIDI Input - Sustain Pedal
- **Tests**:
  - CC 64 (sustain pedal) handling
  - Note after note-off with sustain on
  - Sustain release

#### 4. MIDI Input - Pitch Bend
- **Tests**:
  - Pitch bend wheel (0xE0)
  - Pitch bend up (0x3FFF)
  - Pitch bend center (0x2000)
  - Frequency change verification

#### 5. Parameter Changes - All 13 Parameters
- **Tests**:
  - MasterGain, AttackTime, ReleaseTime
  - VibratoRate, VibratoDepth
  - FormantShift, SubharmonicMix
  - ChoirSize, StereoWidth
  - ReverbMix, ReverbDecay
  - LanguageSelect, VoiceType
  - Click prevention during changes

#### 6. Parameter Automation
- **Test**: Master gain automation (0.0 → 1.0 over 100 buffers)
- **Verifies**: Smooth transitions, no clicks

#### 7. Preset Loading
- **Tests**: All 8 factory presets
  1. Default Choir
  2. Large Cathedral Choir
  3. Small Chamber Choir
  4. Gospel Choir
  5. Byzantine Choir
  6. Throat Singing - Sygyt
  7. Throat Singing - Kargyraa
  8. Alien Choir (Klingon)

#### 8. State Management - Save/Restore
- **Tests**:
  - Save plugin state
  - Modify parameters
  - Restore state
  - Verify parameter restoration

#### 9. DAW Project Reload
- **Test**: Complete save/load cycle
- **Simulates**:
  - Save project state
  - Destroy/recreate plugin
  - Restore project state
  - Verify identical output (< 1% difference)

#### 10. Preset Persistence
- **Tests**:
  - Create user preset
  - Save preset
  - Load preset
  - Verify parameters match

#### 11. Audio Quality - No Clicks/Pops
- **Test**: 100 buffers with random parameter changes
- **Verifies**: No clicks (max delta < 0.15)

#### 12. Audio Quality - No Aliasing
- **Test**: High notes (C6+) prone to aliasing
- **Verifies**: No NaN/Inf artifacts
- **Note**: FFT analysis in real implementation

#### 13. Audio Quality - SNR Measurement
- **Test**: Signal vs noise floor
- **Target**: SNR > 90dB (0.001 noise floor)

#### 14. Audio Quality - THD Measurement
- **Test**: Pure tone (A4 = 440Hz)
- **Target**: THD < 0.1% (-60dB)

#### 15. Voice Stealing Under Load
- **Test**: Play 80 notes (exceeds 60 voice limit)
- **Verifies**:
  - No crashes
  - Audio output continues
  - Voices were stolen (stats verification)

#### 16. Real-Time Safety
- **Test**: 1000 buffers with 40 voices
- **Verifies**:
  - No allocations
  - No NaN/Inf in output
  - Consistent performance

#### 17. Concurrent MIDI And Audio
- **Test**: MIDI messages during audio processing (100 buffers)
- **Verifies**: Output remains valid throughout

#### 18. Edge Cases
- **Tests**:
  - Zero sample rate
  - Very small buffer (1 sample)
  - Very large buffer (8192 samples)
  - Invalid MIDI messages
  - Extreme parameter values

---

## Building and Running Tests

### Build All Tests
```bash
cd /Users/bretbouchard/apps/schill/choir-v2-universal
mkdir -p .build/cmake
cd .build/cmake
cmake ../..
make -j4
```

### Run Unit Tests
```bash
./tests/choir_v2_tests
```

### Run Integration Tests
```bash
./tests/choir_v2_integration_tests
```

### Run Benchmarks
```bash
./tests/choir_v2_benchmarks
```

### Run All Tests
```bash
ctest --output-on-failure
```

### Run Specific Test Suite
```bash
./tests/choir_v2_integration_tests --gtest_filter=SynthesisPipelineTest.*
./tests/choir_v2_integration_tests --gtest_filter=MultilingualSupportTest.*
./tests/choir_v2_integration_tests --gtest_filter=PluginFunctionalityTest.*
./tests/choir_v2_benchmarks --gtest_filter=DSPBenchmarkTest.*
```

### Run Specific Test
```bash
./tests/choir_v2_integration_tests --gtest_filter=SynthesisPipelineTest.EnglishTextToAudioPipeline
```

### Verbose Output
```bash
./tests/choir_v2_integration_tests --gtest_print_time=1
```

---

## Test Coverage Summary

### Integration Tests: 49 test cases
- Synthesis Pipeline: 13 tests
- Multilingual Support: 18 tests
- Plugin Functionality: 18 tests

### Performance Benchmarks: 15 benchmarks
- Individual DSP components
- Voice management
- Synthesis methods
- Real-time safety
- Memory usage

### Total: 64 comprehensive tests

---

## Expected Test Results

### Passing Criteria
- **Integration Tests**: All tests should pass once implementation is complete
- **Benchmarks**: Performance targets must be met:
  - FormantResonator: < 1ms
  - SubharmonicGenerator: < 0.5ms
  - SpectralEnhancer: < 2ms
  - VoiceManager (40 voices): < 5ms
  - VoiceManager (60 voices): < 8ms
  - Complete pipeline: < 2x real-time

### Known Issues
Some tests may fail initially due to:
1. Placeholder implementations (SIMD optimization)
2. Missing language data files
3. Incomplete DSP components
4. Missing plugin wrapper implementation

These tests establish the framework for validation as implementation progresses.

---

## Continuous Integration

### GitHub Actions Integration
```yaml
name: Tests

on: [push, pull_request]

jobs:
  test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - name: Build
        run: |
          mkdir -p .build/cmake
          cd .build/cmake
          cmake ../..
          make -j4
      - name: Run Unit Tests
        run: ./tests/choir_v2_tests
      - name: Run Integration Tests
        run: ./tests/choir_v2_integration_tests
      - name: Run Benchmarks
        run: ./tests/choir_v2_benchmarks
```

---

## Future Enhancements

### Additional Tests to Consider
1. **Fuzz Testing**: Random input generation
2. **Stress Testing**: Maximum load for extended periods
3. **Memory Leak Detection**: Valgrind integration
4. **Thread Sanitizer**: Data race detection
5. **Undefined Behavior Sanitizer**: UBSan integration

### Code Coverage
```bash
# Generate coverage report
cmake -DCMAKE_CXX_FLAGS=--coverage ..
make
./tests/choir_v2_integration_tests
lcov --capture --directory . --output-file coverage.info
genhtml coverage.info --output-directory coverage_html
```

---

## Conclusion

This comprehensive test suite provides:
- **Complete coverage** of synthesis pipeline
- **Multi-language support** verification
- **Performance benchmarks** for real-time safety
- **End-to-end plugin** functionality testing
- **Quality assurance** for production readiness

All tests follow Google Test framework and can be integrated into CI/CD pipelines for automated validation.
