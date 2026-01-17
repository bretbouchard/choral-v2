# Choir V2.0 Build Instructions

Complete guide for building Choir V2.0 plugin in all 7 formats.

## Table of Contents

- [Quick Start](#quick-start)
- [Prerequisites](#prerequisites)
- [Environment Setup](#environment-setup)
- [Building Individual Formats](#building-individual-formats)
- [Building All Formats](#building-all-formats)
- [Troubleshooting](#troubleshooting)
- [Advanced Usage](#advanced-usage)

---

## Quick Start

### macOS (All Formats)

```bash
# Set JUCE path
export JUCE_PATH=/path/to/JUCE

# Build all formats (Release)
./scripts/build_all.sh

# Or build individual format
./scripts/build_vst3.sh
./scripts/build_au.sh
./scripts/build_clap.sh
```

### Windows (VST3, CLAP, Standalone, AAX)

```bash
# Set JUCE path
set JUCE_PATH=C:\path\to\JUCE

# Build all formats
scripts\build_all.sh

# Or build individual format
scripts\build_vst3.sh
scripts\build_clap.sh
```

### Linux (VST3, CLAP, LV2, Standalone)

```bash
# Set JUCE path
export JUCE_PATH=/path/to/JUCE

# Build all formats
./scripts/build_all.sh

# Or build individual format
./scripts/build_vst3.sh
./scripts/build_clap.sh
./scripts/build_lv2.sh
```

---

## Prerequisites

### Common Requirements

- **CMake**: Version 3.20 or higher
- **C++ Compiler**: Support for C++20
  - macOS: Xcode 13.0+ (Clang)
  - Windows: Visual Studio 2019+ (MSVC)
  - Linux: GCC 10+ or Clang 12+
- **Git**: For cloning dependencies

### macOS Requirements

- **Xcode**: 13.0 or higher
- **Xcode Command Line Tools**: `xcode-select --install`
- **JUCE Framework**: 8.0.0 or higher

### Windows Requirements

- **Visual Studio**: 2019 or higher
- **CMake Tools**: For Visual Studio
- **JUCE Framework**: 8.0.0 or higher

### Linux Requirements

- **Build Essentials**: `sudo apt install build-essential`
- **ALSA Development**: `sudo apt install libasound2-dev`
- **Freetype Development**: `sudo apt install libfreetype6-dev`
- **JUCE Framework**: 8.0.0 or higher

### AAX Build Requirements (Optional)

- **AAX SDK**: From Avid (requires developer account)
- **Pro Tools**: For testing

---

## Environment Setup

### 1. Install JUCE Framework

#### Option A: Download from GitHub

```bash
# Clone JUCE repository
git clone https://github.com/juce-framework/JUCE.git ~/JUCE

# Set environment variable
export JUCE_PATH=~/JUCE

# Add to ~/.bash_profile or ~/.zshrc for persistence
echo 'export JUCE_PATH=~/JUCE' >> ~/.bash_profile
```

#### Option B: Download from Website

1. Visit https://juce.com/get-juce/
2. Download latest release
3. Extract to ~/JUCE
4. Set environment variable as above

### 2. Install AAX SDK (Optional)

```bash
# Extract AAX SDK to ~/AAX_SDK
export AAX_SDK=~/AAX_SDK

# Add to ~/.bash_profile or ~/.zshrc
echo 'export AAX_SDK=~/AAX_SDK' >> ~/.bash_profile
```

### 3. Verify Installation

```bash
# Check JUCE path
ls $JUCE_PATH

# Check CMake version
cmake --version

# Check compiler
clang --version  # macOS
gcc --version    # Linux
cl.exe           # Windows
```

---

## Building Individual Formats

### VST3 (Virtual Studio Technology 3)

**Platforms**: macOS, Windows, Linux

```bash
# macOS + Windows + Linux
./scripts/build_vst3.sh

# Debug build
./scripts/build_vst3.sh --debug

# Intel only (macOS)
./scripts/build_vst3.sh --intel

# Apple Silicon only (macOS)
./scripts/build_vst3.sh --arm

# Clean build
./scripts/build_vst3.sh --clean
```

**Output**: `.artifacts/{os}/vst3/ChoirV2.vst3`

---

### AU (Audio Units)

**Platforms**: macOS only

```bash
# Build AU
./scripts/build_au.sh

# Debug build
./scripts/build_au.sh --debug

# Clean build
./scripts/build_au.sh --clean
```

**Output**: `.artifacts/macos/au/ChoirV2.component`

---

### AAX (Avid Audio Extension)

**Platforms**: macOS, Windows

**Requires**: AAX SDK

```bash
# Build AAX
./scripts/build_aax.sh

# Debug build
./scripts/build_aax.sh --debug

# Clean build
./scripts/build_aax.sh --clean
```

**Output**: `.artifacts/{os}/aax/ChoirV2.aaxplugin`

---

### CLAP (CLever Audio Plug-in)

**Platforms**: macOS, Windows, Linux

```bash
# Build CLAP
./scripts/build_clap.sh

# Debug build
./scripts/build_clap.sh --debug

# Clean build
./scripts/build_clap.sh --clean
```

**Output**: `.artifacts/{os}/clap/ChoirV2.clap`

---

### LV2 (LADSPA Version 2)

**Platforms**: Linux, macOS

```bash
# Build LV2
./scripts/build_lv2.sh

# Debug build
./scripts/build_lv2.sh --debug

# Clean build
./scripts/build_lv2.sh --clean
```

**Output**: `.artifacts/{os}/lv2/ChoirV2.lv2`

---

### AUv3 (Audio Units Version 3)

**Platforms**: iOS (built from macOS)

**Requires**: Xcode, iOS SDK

```bash
# Build AUv3 for iOS
./scripts/build_auv3.sh

# Debug build
./scripts/build_auv3.sh --debug

# Set iOS deployment target
./scripts/build_auv3.sh --ios-version 12.0

# Clean build
./scripts/build_auv3.sh --clean
```

**Output**: `.artifacts/ios/auv3/ChoirV2.auv3`

---

### Standalone Application

**Platforms**: macOS, Windows, Linux

```bash
# Build standalone app
./scripts/build_standalone.sh

# Debug build
./scripts/build_standalone.sh --debug

# Intel only (macOS)
./scripts/build_standalone.sh --intel

# Apple Silicon only (macOS)
./scripts/build_standalone.sh --arm

# Clean build
./scripts/build_standalone.sh --clean
```

**Output**:
- macOS: `.artifacts/macos/standalone/ChoirV2.app`
- Windows: `.artifacts/windows/standalone/ChoirV2.exe`
- Linux: `.artifacts/linux/standalone/ChoirV2`

---

## Building All Formats

### Master Build Script

The `build_all.sh` script builds all supported formats for your platform.

```bash
# Build all formats (Release)
./scripts/build_all.sh

# Build all formats (Debug)
./scripts/build_all.sh --debug

# Clean build all formats
./scripts/build_all.sh --clean

# Continue build even if a format fails
./scripts/build_all.sh --skip-failing

# Show summary only (less verbose)
./scripts/build_all.sh --summary-only

# Build specific formats only
./scripts/build_all.sh --vst3 --au --clap
```

### What Gets Built?

#### macOS
- VST3 (Universal Binary: Intel + Apple Silicon)
- AU (Universal Binary)
- CLAP (Universal Binary)
- LV2 (Universal Binary)
- Standalone (Universal Binary)
- AUv3 (iOS - ARM64 only)
- AAX (if SDK is available)

#### Windows
- VST3 (x86_64)
- CLAP (x86_64)
- Standalone (x86_64)
- AAX (if SDK is available)

#### Linux
- VST3 (x86_64)
- CLAP (x86_64)
- LV2 (x86_64)
- Standalone (x86_64)

---

## Troubleshooting

### Common Issues

#### JUCE Not Found

**Error**: `JUCE not found. Please set JUCE_PATH environment variable.`

**Solution**:
```bash
export JUCE_PATH=/path/to/JUCE
./scripts/build_all.sh
```

#### AAX SDK Not Found

**Error**: `AAX SDK not found. AAX builds require the Avid AAX SDK.`

**Solution**:
```bash
# Skip AAX builds if SDK not available
./scripts/build_all.sh --skip-failing

# Or install AAX SDK and set path
export AAX_SDK=/path/to/AAX_SDK
./scripts/build_all.sh
```

#### Build Failures

**Error**: Compilation errors or linker errors

**Solutions**:
1. Clean build: `./scripts/build_all.sh --clean`
2. Check compiler version: `cmake --version`
3. Verify JUCE version: `ls $JUCE_PATH`
4. Check system requirements: See [Prerequisites](#prerequisites)

#### Code Signing Errors (macOS)

**Error**: `code signing failed`

**Solution**:
```bash
# Build with ad-hoc signing (automatic)
./scripts/build_all.sh

# Or sign with your certificate
codesign --force --deep --sign "Developer ID Application: Your Name" path/to/plugin
```

#### Plugin Not Loading in DAW

**Possible Causes**:
1. Wrong architecture (Intel vs ARM)
2. Missing dependencies
3. Code signing issues

**Solutions**:
```bash
# Check architecture
lipo -info path/to/plugin

# Verify code signature
codesign -dv path/to/plugin

# Reinstall plugin
cp -R path/to/plugin ~/Library/Audio/Plug-Ins/VST3/
```

---

## Advanced Usage

### Custom Build Types

```bash
# Debug build with symbols
./scripts/build_all.sh --debug

# Release build with optimization
./scripts/build_all.sh --release

# Minimum size build
cmake -DCMAKE_BUILD_TYPE=MinSizeRel ...
```

### Parallel Builds

The build scripts automatically use all available CPU cores:

```bash
# Uses all cores by default
./scripts/build_all.sh

# Manual control
cmake --build . --parallel 4
```

### Verbose Output

```bash
# Show detailed build output
./scripts/build_all.sh --verbose

# Or set environment variable
CMAKE_VERBOSE_MAKEFILE=ON ./scripts/build_all.sh
```

### Custom Install Locations

Edit the build scripts to change artifact locations:

```bash
# Default: .artifacts/{os}/{format}/
# Custom: modify ARTIFACT_DIR in build scripts
```

### Continuous Integration

See `.github/workflows/` for CI/CD examples:

```yaml
# .github/workflows/build.yml
name: Build Choir V2.0
on: [push, pull_request]
jobs:
  build:
    runs-on: macos-latest
    steps:
      - uses: actions/checkout@v3
      - name: Build all formats
        run: ./scripts/build_all.sh --release
```

---

## Verification

### Testing Plugin Loading

#### macOS

```bash
# Test with AudioPluginVerifier (JUCE tool)
/Applications/JUCE/examples/Plugins/AudioPluginVerifier.app

# Test in AU Lab
/Applications/AU\ Lab.app

# Test in logic Pro (if installed)
open -a Logic\ Pro
```

#### Windows

```bash
# Test with AudioPluginVerifier
C:\JUCE\examples\Plugins\AudioPluginVerifier.exe

# Test in REAPER (if installed)
"C:\Program Files\REAPER\reaper.exe"
```

#### Linux

```bash
# Test with jalv
jalv.gtk https://bretbouchard.github.io/choir-v2-universal

# Test with carla
carla-plugin
```

### Verify Plugin Structure

```bash
# macOS: Check bundle structure
ls -R ChoirV2.vst3/

# Windows: Check DLL structure
dir ChoirV2.vst3\

# Linux: Check shared library
file ChoirV2.so
```

---

## Next Steps

After successful build:

1. **Test in DAW**: Load plugin in your preferred DAW
2. **Verify MIDI Input**: Test note triggering
3. **Check Audio Output**: Verify sound generation
4. **Test Parameters**: Verify all controls work
5. **Performance Test**: Check CPU usage
6. **Distribute**: Package for distribution

See [PLUGIN_FORMATS.md](./PLUGIN_FORMATS.md) for format-specific notes.

---

## Additional Resources

- [JUCE Documentation](https://docs.juce.com/)
- [Plugin Format Specifications](./PLUGIN_FORMATS.md)
- [System Requirements](./REQUIREMENTS.md)
- [Troubleshooting Guide](./TROUBLESHOOTING.md)

---

**Last Updated**: 2025-01-17
**Build Version**: 2.0.0
