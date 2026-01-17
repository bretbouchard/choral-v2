# Choir V2.0 Technical Specification

**Status**: DRAFT - Pending Critical Fixes (see bd issues white_room-494, white_room-495)

**Version**: 2.0.0-draft

**Last Updated**: 2026-01-17

---

## Executive Summary

Choir V2.0 is a **universal vocal synthesis platform** capable of synthesizing any human language (natural or constructed) and supporting any vocal technique, including sub-harmonic frequencies.

**Key Capabilities:**
- 40-60 simultaneous voices @ 30% CPU (realistic target)
- < 5ms latency @ 44.1kHz
- Data-driven language definitions (JSON)
- Pluggable synthesis methods (formant, diphone, subharmonic)
- Real-time safe (no allocations in audio thread)

**Current Status:**
- Phase: Architecture & Design (Pre-Implementation)
- Blocker: Critical DSP fixes required (6-10 weeks)
- Next: Revised specification incorporating senior DSP engineer feedback

---

## Architecture Overview

### System Components

```
┌─────────────────────────────────────────────────────────┐
│                    Choir V2.0 Engine                    │
├─────────────────────────────────────────────────────────┤
│  ┌───────────────┐  ┌──────────────┐  ┌──────────────┐ │
│  │ Phoneme       │  │ Language     │  │ G2P          │ │
│  │ Database      │  │ Loader       │  │ Engine       │ │
│  └───────────────┘  └──────────────┘  └──────────────┘ │
├─────────────────────────────────────────────────────────┤
│  ┌──────────────────────────────────────────────────┐  │
│  │              Voice Manager                        │  │
│  │  - Voice allocation                               │  │
│  │  - Voice stealing (CPU limit)                     │  │
│  │  - Priority management                            │  │
│  └──────────────────────────────────────────────────┘  │
├─────────────────────────────────────────────────────────┤
│  ┌───────────────┐  ┌──────────────┐  ┌──────────────┐ │
│  │ Formant       │  │ Diphone      │  │ Subharmonic  │ │
│  │ Synthesis     │  │ Synthesis    │  │ Synthesis    │ │
│  └───────────────┘  └──────────────┘  └──────────────┘ │
├─────────────────────────────────────────────────────────┤
│  ┌──────────────────────────────────────────────────┐  │
│  │              DSP Components                       │  │
│  │  - FormantResonator (FIXED: real biquad)         │  │
│  │  - GlottalSource                                 │  │
│  │  - LinearSmoother (prevents clicks)              │  │
│  │  - SpectralEnhancer (FIXED: overlap-add FFT)     │  │
│  │  - SubharmonicGenerator (FIXED: PLL)             │  │
│  └──────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────┘
```

---

## Critical Fixes Required (from DSP Review)

### 1. FormantResonator - Coefficient Math

**Issue**: Complex pole math is incorrect

**Fix**: Use real biquad coefficients

```cpp
// OLD (incorrect):
std::complex<float> pole = std::polar<float>(r, theta);

// NEW (correct):
float b0 = 1.0;
float b1 = -2.0 * r * cos(theta);
float b2 = r * r;
float a0 = 1.0;
float a1 = -2.0 * r * cos(theta) * alpha;
float a2 = r * r * alpha * alpha;
```

### 2. SubharmonicGenerator - PLL Phase Drift

**Issue**: Simple frequency division causes phase drift

**Fix**: Use PLL with proper phase error detection

```cpp
// OLD (incorrect):
float subharmonic_phase = phase_ / ratio_;

// NEW (correct):
void updatePLL(float input) {
    double phase_error = input - phase_;
    integrator_ += phase_error * ki_;
    double control = kp_ * phase_error + integrator_;
    phase_ += control;
}
```

### 3. SpectralEnhancer - Spectral Leakage

**Issue**: Missing overlap-add processing

**Fix**: Add overlap-add with windowing

```cpp
// OLD (incorrect):
fft(input_buffer);

// NEW (correct):
applyHanningWindow(input_buffer);
fft(input_buffer);
processSpectralEnvelope();
ifft(output_buffer);
overlapAdd(output_buffer);
```

### 4. VoiceManager - Remove Threading

**Issue**: Thread pool is not real-time safe

**Fix**: Use single-threaded SIMD batch processing

```cpp
// OLD (incorrect):
std::vector<std::thread> workers_;

// NEW (correct):
void processVoicesSIMD(
    const std::vector<Voice*>& voices,
    float* output,
    int num_samples
);
```

### 5. Parameter Smoothing - Prevent Clicks

**Issue**: No smoothing during phoneme transitions

**Fix**: Add LinearSmoother to all DSP modules

```cpp
// NEW (required):
LinearSmoother f1_smoother_;
LinearSmoother f2_smoother_;
LinearSmoother f3_smoother_;
LinearSmoother f4_smoother_;
```

---

## Performance Targets (Revised)

### Original (Over-Optimistic)

- 100 voices @ 30% CPU ❌ NOT FEASIBLE
- < 5ms latency ✅ Achievable
- < 200MB memory ✅ Achievable

### Revised (Realistic)

- **40-60 voices @ 30% CPU** ✅ ACHIEVABLE
  - With SIMD batch processing
  - With voice stealing
  - With downsampling (non-critical voices)

- **< 5ms latency** ✅ ACHIEVABLE
  - 128-sample buffers @ 44.1kHz
  - No allocations in audio thread
  - Single-threaded SIMD processing

- **< 200MB memory** ✅ ACHIEVABLE
  - ~3MB per voice (60KB × 50 voices)
  - Pre-allocated memory pools
  - Lock-free data structures

---

## Missing Components (Must Add)

### 1. LinearSmoother

```cpp
class LinearSmoother {
    void setTimeConstant(float time_constant, float sample_rate);
    void setTarget(float target);
    float process();
    void processBlock(float* output, int num_samples);
};
```

### 2. Anti-Aliasing Filter

```cpp
class AntiAliasingFilter {
    void setOversamplingFactor(int factor);  // 1x, 2x, 4x
    float process(float input);
};
```

### 3. Voice Stealing Algorithm

```cpp
class VoiceStealer {
    int findStealCandidate(VoicePriority priority);
    bool shouldSteal(float cpu_usage);
};
```

### 4. Voice Allocator

```cpp
class VoiceAllocator {
    VoiceAllocation allocate(
        VoicePriority priority,
        float frequency,
        float amplitude
    );
};
```

### 5. Denormal Protection

```cpp
inline float flushDenormal(float x) {
    return (std::abs(x) < 1e-10f) ? 0.0f : x;
}
```

---

## Implementation Roadmap

### Phase 1: Core DSP Fixes (2-3 weeks)

**Priority**: CRITICAL (blocks all implementation)

- [ ] Fix FormantResonator coefficients (real biquad)
- [ ] Fix SubharmonicGenerator PLL (phase error detection)
- [ ] Fix SpectralEnhancer FFT (overlap-add)
- [ ] Add LinearSmoother to all DSP modules
- [ ] Remove threading from VoiceManager
- [ ] Add denormal protection
- [ ] Unit tests for all DSP components

**Dependencies**: None (foundational work)

**Deliverables**:
- Fixed DSP implementations
- Comprehensive unit tests
- Performance benchmarks

### Phase 2: Synthesis Engine (2-3 weeks)

**Priority**: HIGH (core functionality)

- [ ] Implement FormantSynthesis method
- [ ] Implement DiphoneSynthesis method
- [ ] Implement SubharmonicSynthesis method
- [ ] Integrate synthesis methods with VoiceManager
- [ ] SIMD batch processing (AVX2)
- [ ] Integration tests

**Dependencies**: Phase 1 complete

**Deliverables**:
- Working synthesis engine
- SIMD-optimized voice processing
- Performance validation

### Phase 3: Language Support (2-3 weeks)

**Priority**: MEDIUM (extensibility)

- [ ] Implement PhonemeDatabase (JSON loader)
- [ ] Implement LanguageLoader (JSON parser)
- [ ] Implement G2PEngine (rule-based)
- [ ] Create English language definition
- [ ] Create Latin language definition
- [ ] Create Klingon language definition
- [ ] Language tests

**Dependencies**: Phase 2 complete

**Deliverables**:
- 3 language definitions (English, Latin, Klingon)
- G2P conversion tests
- Language validation tools

### Phase 4: Plugin Integration (1-2 weeks)

**Priority**: MEDIUM (DAW integration)

- [ ] JUCE plugin wrapper
- [ ] Parameter mapping
- [ ] Preset management
- [ ] UI editor
- [ ] DAW testing

**Dependencies**: Phase 3 complete

**Deliverables**:
- VST3/AU/AUv3 plugins
- Plugin presets
- DAW validation

### Phase 5: Testing & Validation (1-2 weeks)

**Priority**: HIGH (quality assurance)

- [ ] Unit tests (>90% coverage)
- [ ] Integration tests
- [ ] Performance benchmarks
- [ ] DAW testing (Logic, Ableton, Reaper)
- [ ] Real-world usage tests

**Dependencies**: Phase 4 complete

**Deliverables**:
- Test report
- Performance analysis
- DAW compatibility matrix

---

## Timeline

**Total**: 6-10 weeks to production-ready implementation

- **Phase 1**: 2-3 weeks (CRITICAL - must complete first)
- **Phase 2**: 2-3 weeks (can start after Phase 1)
- **Phase 3**: 2-3 weeks (can overlap with Phase 2)
- **Phase 4**: 1-2 weeks (after Phase 2 & 3)
- **Phase 5**: 1-2 weeks (final validation)

**Parallel Work**:
- Phase 2 and Phase 3 can overlap (synthesis + languages)
- Phase 4 can start once Phase 2 is complete
- Phase 5 runs in parallel with Phase 4

---

## Success Criteria

### Functional Requirements

- [ ] Synthesize 40-60 simultaneous voices
- [ ] Support 3+ languages (English, Latin, Klingon)
- [ ] Support 3+ synthesis methods
- [ ] Real-time safe (no xruns)
- [ ] No SLC violations (no stubs, workarounds)

### Performance Requirements

- [ ] 40-60 voices @ 30% CPU (realistic)
- [ ] < 5ms latency @ 44.1kHz
- [ ] < 200MB memory
- [ ] Zero allocations in audio thread

### Quality Requirements

- [ ] >90% unit test coverage
- [ ] All integration tests passing
- [ ] DAW compatibility (Logic, Ableton, Reaper)
- [ ] Zero critical bugs
- [ ] Zero SLC violations

---

## Open Issues

### Critical (Blockers)

- **white_room-494**: Critical DSP fixes required
  - FormantResonator coefficients
  - SubharmonicGenerator PLL
  - SpectralEnhancer FFT
  - VoiceManager threading
  - Parameter smoothing

- **white_room-495**: Revised specification needed
  - Incorporate all DSP fixes
  - Update performance targets
  - Add missing components
  - Create implementation guide

### High Priority

- Define exact voice stealing algorithm
- Specify SIMD optimization strategy
- Design language JSON schema
- Plan G2P rule system

### Medium Priority

- Choose FFT library (KISS FFT vs. PocketFFT)
- Design preset file format
- Plan testing strategy
- Define DAW compatibility test suite

---

## References

### Research Documents

- `/Users/bretbouchard/apps/schill/white_room/docs/research/CHOIR_V2_0_RESEARCH_COMPILATION.md`
- `/Users/bretbouchard/apps/schill/white_room/docs/research/MULTI_LANGUAGE_SUPPORT.md`
- `/Users/bretbouchard/apps/schill/white_room/docs/research/SUBHARMONIC_SYNTHESIS.md`

### Technical Standards

- IPA (International Phonetic Alphabet)
- X-SAMPA (IPA ASCII encoding)
- JSON Schema (language definitions)

### Academic Papers

- Rosenbergl (1971) - Glottal source models
- Liljencrants & Fant (1984) - LF model
- Klatt (1980) - Formant synthesis
- McDonnell (2018) - Subharmonic singing

---

**Document Status**: DRAFT - PENDING CRITICAL FIXES

**Next Review**: After Phase 1 completion (DSP fixes)

**Owner**: Bret Bouchard

**Contributors**: Senior DSP Engineer (review), Claude Code (implementation)
