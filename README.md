# Choir V2.0 - Universal Vocal Synthesis Platform

## Overview

Choir V2.0 is a next-generation vocal synthesis platform capable of:
- Synthesizing any human language (natural or constructed)
- Supporting any vocal technique (including sub-harmonic frequencies)
- Processing 40-60 simultaneous voices in real-time
- Extensible through data-driven language/technique definitions

## Key Features

- **Universal Phoneme System**: Data-driven phoneme database
- **Multi-Language Support**: 25+ languages out of the box
- **Pluggable Synthesis**: Formant, Diphone, Subharmonic methods
- **Real-Time Performance**: 40-60 voices @ 44.1kHz, < 5ms latency
- **Extensible**: Create your own languages and techniques

## Status

**Phase:** Architecture & Design (Pre-Implementation)

**Next Steps:**
1. Implement core DSP components (with critical fixes)
2. Build synthesis engine
3. Add language support
4. Create plugin wrappers
5. Test and validate

## Repository Structure

This is a **separate repository** from Choir V1 (white_room).
- V1: `/Users/bretbouchard/apps/schill/white_room` (current production)
- V2: `/Users/bretbouchard/apps/schill/choir-v2-universal` (next-gen)

### Directory Layout

```
choir-v2-universal/
├── src/
│   ├── core/          # Core synthesis engine
│   ├── synthesis/     # Synthesis methods
│   ├── dsp/           # DSP components
│   ├── utils/         # Utilities
│   └── plugin/        # JUCE plugin wrapper
├── languages/         # Language definitions (JSON)
├── tests/             # Tests
├── build/             # Build output
└── CMakeLists.txt     # Main CMake config
```

## Architecture

### Core Components

- **PhonemeDatabase**: Universal phoneme representation
- **LanguageLoader**: Data-driven language definitions
- **G2PEngine**: Grapheme-to-phoneme conversion
- **VoiceManager**: Multi-voice orchestration
- **Synthesis Methods**: Pluggable synthesis backends

### DSP Components

- **FormantResonator**: Real-time formant filtering
- **GlottalSource**: Glottal pulse train generation
- **SubharmonicGenerator**: Sub-harmonic frequency synthesis
- **SpectralEnhancer**: Spectral envelope enhancement
- **LinearSmoother**: Parameter smoothing (prevents clicks)

### Performance Targets

- **Voices**: 40-60 simultaneous voices @ 30% CPU (realistic)
- **Latency**: < 5ms @ 44.1kHz (128-sample buffers)
- **Memory**: < 200MB for 60 voices
- **Quality**: Natural-sounding vocal synthesis

## Language Support

### Implemented Languages (Planned)

**Natural Languages:**
- English (US, UK, Australian, Indian)
- Latin (Ecclesiastical, Classical)
- Spanish, French, German, Italian
- Japanese, Mandarin Chinese, Korean
- Arabic, Hebrew, Persian (Farsi)

**Constructed Languages:**
- Klingon (tlhIngan Hol)
- Quenya & Sindarin (Elvish)
- Na'vi
- Dothraki

**Vocal Techniques:**
- Throat singing (Khoomei, Kargyraa)
- Sub-harmonic singing
- Overtone singing
- Inuit throat singing

## Development

### Build Requirements

- **C++20** compiler
- **CMake** 3.20+
- **JUCE** 7.0+
- **Google Test** (for unit tests)

### Build Instructions

```bash
# Clone repository
git clone https://github.com/bretbouchard/choir-v2-universal.git
cd choir-v2-universal

# Configure build
cmake -B build -S .

# Build
cmake --build build

# Run tests
ctest --test-dir build
```

## Design Principles

### SLC Development Philosophy

Choir V2.0 follows **SLC** (Simple, Lovable, Complete) principles:
- **Simple**: Focused features, intuitive design, zero learning curve
- **Lovable**: Delights users, solves problems magically
- **Complete**: Full user journey, no gaps, no workarounds

### Architecture Principles

1. **Data-Driven**: Language definitions in JSON, not hardcoded
2. **Pluggable**: Synthesis methods are interchangeable
3. **Real-Time Safe**: No allocations in audio thread
4. **SIMD-Optimized**: Use CPU vectorization for performance
5. **Testable**: Comprehensive unit and integration tests

## Research & Design

See the complete research compilation:
- **`/Users/bretbouchard/apps/schill/white_room/docs/research/CHOIR_V2_0_RESEARCH_COMPILATION.md`**

Key Documents:
- **`SPECIFICATION.md`** - Technical specification
- **`MULTI_LANGUAGE_SUPPORT.md`** - Language implementation guide
- **`SUBHARMONIC_SYNTHESIS.md`** - Sub-harmonic frequency design

## Contributing

Choir V2.0 is currently in **architecture and design phase**.

**Before implementing:**
1. Review the technical specification
2. Check for open bd issues
3. Consult Confucius for patterns
4. Follow SLC principles

**Critical Issues:**
- See bd issue **white_room-494** (Critical DSP fixes required)
- See bd issue **white_room-495** (Revised specification needed)

## License

Same as White Room project

## Contact

- **Repository**: https://github.com/bretbouchard/choir-v2-universal
- **Issues**: Tracked in bd (Beads task management)
- **Documentation**: See `docs/` directory

---

**Status**: Architecture & Design Phase
**Next Milestone**: Core DSP implementation (after critical fixes)
**Timeline**: 6-10 weeks to production-ready implementation
