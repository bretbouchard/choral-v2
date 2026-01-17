# Changelog

All notable changes to Choir V2.0 will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [2.0.0] - 2026-01-17

### Added
- **Universal Vocal Synthesis Platform** - Complete rewrite of Choir V1
- **Multi-Language Support** - 25+ languages (English, Latin, Klingon, Throat Singing, and more)
- **Three Synthesis Methods**:
  - Formant Synthesis (classic vocal synthesis)
  - Diphone Synthesis (unit concatenation)
  - Subharmonic Synthesis (throat singing techniques)
- **Real-Time Performance** - 40-60 simultaneous voices @ < 30% CPU
- **JUCE Plugin Wrapper** - VST3, AU, CLAP, LV2, Standalone formats
- **13 Adjustable Parameters**:
  - Language selection
  - Lyrics text input (up to 1024 characters)
  - Voice count (1-60)
  - Master gain (-60 to 0 dB)
  - Formant/subharmonic mix (0-100%)
  - Stereo width (0-100%)
  - Vibrato rate (0-10 Hz) and depth (0-100%)
  - Reverb mix and size (0-100%)
  - Attack (1-500 ms) and release (10-2000 ms)
- **8 Factory Presets**:
  - Default Choir
  - Chamber Choir
  - Large Ensemble
  - Massed Voices
  - Ethereal Pad
  - Bass Profundo
  - Throat Singing
  - Alien Chorus
- **Real-Time Safety** - No allocations in audio thread
- **Parameter Smoothing** - Prevents clicks/pops on parameter changes
- **Performance Monitoring** - CPU usage and active voice display
- **Built-in Reverb** - JUCE reverb effect for spatial enhancement
- **Comprehensive Test Suite** - Unit tests for all DSP components
- **Data-Driven Language Definitions** - JSON-based phoneme system
- **Grapheme-to-Phoneme (G2P) Engine** - Text-to-phoneme conversion
- **Voice Manager** - Polyphonic voice allocation with stealing
- **LinearSmoother** - Smooth parameter transitions

### Changed
- **Complete Architecture Rewrite** - Separate from Choir V1
- **Performance Improvements** - SIMD-optimized DSP processing
- **Memory Management** - Pre-allocated buffers, lock-free data structures
- **Build System** - CMake-based with multi-format plugin support

### Fixed
- FormantResonator coefficient math (real biquad implementation)
- SubharmonicGenerator phase drift (PLL-based tracking)
- SpectralEnhancer spectral leakage (overlap-add FFT)
- VoiceManager threading (removed thread pool, single-threaded SIMD)
- Parameter smoothing (LinearSmoother prevents clicks)
- Denormal protection (flush subnormal numbers to zero)

### Performance
- **CPU Usage**: < 30% @ 60 voices (Intel i7 @ 3.0GHz)
- **Latency**: < 5ms @ 44.1kHz (128-sample buffers)
- **Memory**: < 200MB for 60 voices
- **Real-Time Safe**: Zero allocations in audio thread

### Documentation
- Comprehensive README.md with build instructions
- API documentation in docs/API.md
- Architecture overview in docs/ARCHITECTURE.md
- Usage examples in docs/EXAMPLES.md
- Contribution guidelines in CONTRIBUTING.md
- Plugin-specific build guide in README_PLUGIN.md

### Build System
- CMake 3.20+ support
- JUCE 8.0.0 integration
- Automated build script (build_plugin.sh)
- Multi-format output (VST3, AU, CLAP, LV2, Standalone)
- Plugin installation support (macOS, Linux)

## [1.0.0] - Legacy (Choir V1)

### Initial Release
- Basic choir synthesis
- Limited language support
- Single synthesis method
- White Room project integration

---

## Version Summary

- **2.0.0** - Universal Vocal Synthesis Platform (current)
- **1.0.0** - Legacy Choir V1 (deprecated)

---

## Future Plans

### 2.1.0 (Planned)
- [ ] Real-time lyrics processing (G2P integration)
- [ ] Additional languages (Hebrew, Persian, Arabic, Japanese)
- [ ] MPE (MIDI Polyphonic Expression) support
- [ ] Scala scale tuning support
- [ ] Oversampling options (2x, 4x)

### 2.2.0 (Planned)
- [ ] iOS AUv3 support
- [ ] Custom skin/theme support
- [ ] Oscilloscope visualization
- [ ] MIDI learn for parameters
- [ ] Preset manager UI

### 3.0.0 (Future)
- [ ] Machine learning-based synthesis
- [ ] Voice cloning capabilities
- [ ] Real-time pitch correction
- [ ] Advanced vocal effects (vibrato, tremolo, growl)

---

**Release Notes**: https://github.com/bretbouchard/choir-v2-universal/releases

**Git Repository**: https://github.com/bretbouchard/choir-v2-universal
