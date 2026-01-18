# Choir V2.0 Plugin Build Guide

Complete guide for building, installing, and testing all 6 plugin formats of Choir V2.0.

---

## Table of Contents

1. [Quick Start](#quick-start)
2. [Prerequisites](#prerequisites)
3. [Building All Formats](#building-all-formats)
4. [Building Individual Formats](#building-individual-formats)
5. [Verification](#verification)
6. [Installation](#installation)
7. [Testing in DAWs](#testing-in-daws)
8. [Troubleshooting](#troubleshooting)
9. [Advanced Configuration](#advanced-configuration)

---

## Quick Start

**Build all 6 plugin formats:**

```bash
cd juce_backend/instruments/choral_v2
./build_all_formats.sh
```

**Verify builds:**

```bash
./verify_plugins.sh
```

---

## Prerequisites

### Required Tools

- **CMake** (3.15+)
- **C++ Compiler** (Xcode on macOS, GCC/Clang on Linux, Visual Studio on Windows)
- **JUCE** (configured in CMake)
- **Python** (for build scripts)

### macOS Requirements

```bash
# Install Xcode Command Line Tools
xcode-select --install

# Install CMake
brew install cmake

# Verify JUCE is available
# (Assumes JUCE is already configured in your project)
```

### Linux Requirements

```bash
# Install build dependencies
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    libx11-dev \
    libxrandr-dev \
    libxcursor-dev \
    libxinerama-dev \
    libfreetype6-dev \
    libasound2-dev \
    libwebkit2gtk-4.0-dev \
    libglu1-mesa-dev
```

---

## Building All Formats

### Standard Build

```bash
cd juce_backend/instruments/choral_v2
./build_all_formats.sh
```

This will:
1. Clean previous build
2. Configure CMake for all 6 formats
3. Build all formats in parallel (8 jobs)
4. Verify outputs
5. Display summary

### Build Configuration Options

**Set build type:**

```bash
# Release build (default, optimized)
CMAKE_BUILD_TYPE=Release ./build_all_formats.sh

# Debug build (with symbols)
CMAKE_BUILD_TYPE=Debug ./build_all_formats.sh

# RelWithDebInfo (optimized with debug symbols)
CMAKE_BUILD_TYPE=RelWithDebInfo ./build_all_formats.sh
```

**Control parallel jobs:**

```bash
# Use 4 parallel jobs
BUILD_JOBS=4 ./build_all_formats.sh

# Use all available cores
BUILD_JOBS=$(nproc) ./build_all_formats.sh  # Linux
BUILD_JOBS=$(sysctl -n hw.ncpu) ./build_all_formats.sh  # macOS
```

**Verbose output:**

```bash
# Show detailed compilation output
VERBOSE_BUILD=1 ./build_all_formats.sh
```

---

## Building Individual Formats

### Build VST3 Only

```bash
cd build
cmake .. -DCHOIR_V2_BUILD_PLUGIN=ON -DCHOIR_V2_PLUGIN_FORMATS="VST3"
cmake --build . -j8
```

### Build AU Only

```bash
cd build
cmake .. -DCHOIR_V2_BUILD_PLUGIN=ON -DCHOIR_V2_PLUGIN_FORMATS="AU"
cmake --build . -j8
```

### Build CLAP Only

```bash
cd build
cmake .. -DCHOIR_V2_BUILD_PLUGIN=ON -DCHOIR_V2_PLUGIN_FORMATS="CLAP"
cmake --build . -j8
```

### Build LV2 Only

```bash
cd build
cmake .. -DCHOIR_V2_BUILD_PLUGIN=ON -DCHOIR_V2_PLUGIN_FORMATS="LV2"
cmake --build . -j8
```

### Build AUv3 Only

```bash
cd build
cmake .. -DCHOIR_V2_BUILD_PLUGIN=ON -DCHOIR_V2_PLUGIN_FORMATS="AUv3"
cmake --build . -j8
```

### Build Standalone Only

```bash
cd build
cmake .. -DCHOIR_V2_BUILD_PLUGIN=ON -DCHOIR_V2_PLUGIN_FORMATS="Standalone"
cmake --build . -j8
```

### Build Multiple Specific Formats

```bash
cd build
cmake .. -DCHOIR_V2_BUILD_PLUGIN=ON -DCHOIR_V2_PLUGIN_FORMATS="VST3;AU;CLAP"
cmake --build . -j8
```

---

## Verification

### Run Full Verification

```bash
./verify_plugins.sh
```

This checks:
- File existence
- File sizes (minimum thresholds)
- File permissions
- Binary architecture (x86_64, arm64, universal)
- Dependencies and linked libraries
- Bundle structure (for macOS bundles)
- Info.plist and manifest files
- Bundle identifiers and versions

### Verification Options

**Skip signature checks:**

```bash
SKIP_SIGNATURE_CHECK=1 ./verify_plugins.sh
```

**Verbose output:**

```bash
VERBOSE_OUTPUT=1 ./verify_plugins.sh
```

### Verification Report

After verification, a report is generated at:

```
build/verification_report.txt
```

---

## Installation

### macOS System-Wide Installation

**VST3:**

```bash
sudo cp -R build/plugins/vst/Choir\ V2.0.vst3 /Library/Audio/Plug-Ins/VST3/
```

**AU:**

```bash
sudo cp -R build/plugins/au/Choir\ V2.0.component /Library/Audio/Plug-Ins/Components/
```

**CLAP:**

```bash
sudo mkdir -p /Library/Audio/Plug-Ins/CLAP
sudo cp build/plugins/clap/Choir\ V2.0.clap /Library/Audio/Plug-Ins/CLAP/
```

**LV2:**

```bash
sudo cp -R build/plugins/lv2/Choir\ V2.0.lv2 /Library/Audio/Plug-Ins/LV2/
```

**AUv3:**

```bash
# AUv3 typically requires packaging in an app container
# See advanced section for details
```

**Standalone:**

```bash
sudo cp -R build/plugins/standalone/Choir\ V2.0.app /Applications/
```

### User-Space Installation (macOS)

**VST3:**

```bash
mkdir -p ~/Library/Audio/Plug-Ins/VST3
cp -R build/plugins/vst/Choir\ V2.0.vst3 ~/Library/Audio/Plug-Ins/VST3/
```

**AU:**

```bash
mkdir -p ~/Library/Audio/Plug-Ins/Components
cp -R build/plugins/au/Choir\ V2.0.component ~/Library/Audio/Plug-Ins/Components/
```

**CLAP:**

```bash
mkdir -p ~/Library/Audio/Plug-Ins/CLAP
cp build/plugins/clap/Choir\ V2.0.clap ~/Library/Audio/Plug-Ins/CLAP/
```

**LV2:**

```bash
mkdir -p ~/Library/Audio/Plug-Ins/LV2
cp -R build/plugins/lv2/Choir\ V2.0.lv2 ~/Library/Audio/Plug-Ins/LV2/
```

**Standalone:**

```bash
cp -R build/plugins/standalone/Choir\ V2.0.app ~/Applications/
```

### Linux Installation

**VST3:**

```bash
mkdir -p ~/.vst3
cp -R build/plugins/vst/Choir\ V2.0.vst3 ~/.vst3/
```

**CLAP:**

```bash
mkdir -p ~/.clap
cp build/plugins/clap/Choir\ V2.0.clap ~/.clap/
```

**LV2:**

```bash
mkdir -p ~/.lv2
cp -R build/plugins/lv2/Choir\ V2.0.lv2 ~/.lv2/
```

**Standalone:**

```bash
cp build/plugins/standalone/Choir\ V2.0 ~/bin/
```

---

## Testing in DAWs

### Recommended DAWs by Format

**VST3:**
- Ableton Live
- Cubase
- Studio One
- Reaper
- FL Studio

**AU (macOS only):**
- Logic Pro
- GarageBand
- Ableton Live
- Digital Performer
- MainStage

**CLAP:**
- Reaper
- Bitwig Studio
- Waveform Free

**LV2:**
- Ardour
- Qtractor
- Carla
- Reaper (via plugin)

**AUv3 (macOS only):**
- Logic Pro
- GarageBand
- MainStage
- AUM (iOS)

**Standalone:**
- Direct testing
- Routing via BlackHole (macOS)
- Routing via JACK (Linux)

### Basic Testing Checklist

- [ ] Plugin loads without crash
- [ ] Plugin UI displays correctly
- [ ] Audio passes through without distortion
- [ ] Parameters can be adjusted
- [ ] Presets can be loaded/saved
- [ ] Plugin state persists across DAW restarts
- [ ] MIDI input is received
- [ ] Automation works
- [ ] Plugin scans correctly in DAW plugin manager
- [ ] CPU usage is reasonable

### DAW-Specific Notes

**Ableton Live:**
- Rescan plugins: Options → Preferences → File Folder → Plug-In Sources → Rescan
- Supports VST3 and AU on macOS

**Logic Pro:**
- Rescan plugins: Logic Pro → Preferences → Plug-in Manager → Reset & Rescan
- AU and AUv3 supported

**Reaper:**
- Supports VST3, AU, CLAP, and LV2
- Rescan: Options → Preferences → Plug-ins → Re-scan

**Studio One:**
- Supports VST3 and AU
- Rescan: Studio One → Options → Locations → VST Plugins

---

## Troubleshooting

### Build Issues

**CMake configuration fails:**

```bash
# Clean CMake cache
rm -rf build/CMakeCache.txt build/CMakeFiles

# Reconfigure
cd build
cmake ..
```

**Compilation errors:**

```bash
# Clean build completely
rm -rf build
./build_all_formats.sh
```

**Missing dependencies (macOS):**

```bash
# Install Xcode command line tools
xcode-select --install

# Accept Xcode license
sudo xcodebuild -license accept
```

**Missing dependencies (Linux):**

```bash
# Install all required development packages
sudo apt-get install -y build-essential libx11-dev libxrandr-dev libxcursor-dev libxinerama-dev libfreetype6-dev libasound2-dev libwebkit2gtk-4.0-dev libglu1-mesa-dev
```

### Plugin Loading Issues

**Plugin not appearing in DAW:**

1. Verify installation path is correct for your DAW
2. Rescan plugins in DAW plugin manager
3. Check plugin bundle structure is intact
4. Verify plugin architecture matches your DAW (x86_64 vs arm64)

**Plugin crashes on load:**

1. Test in standalone mode first
2. Check Console.app for crash logs (macOS)
3. Verify all dependencies are linked correctly
4. Try rebuilding in Debug mode for better error messages

**"File cannot be opened" error (macOS):**

```bash
# Remove quarantine attribute
sudo xattr -cr /Library/Audio/Plug-Ins/VST3/Choir\ V2.0.vst3

# Or for user installation
xattr -cr ~/Library/Audio/Plug-Ins/VST3/Choir\ V2.0.vst3
```

**Plugin not validated by DAW:**

- Run verification script to check bundle structure
- Ensure Info.plist (macOS) or manifest.ttl (LV2) is present and valid
- Check file permissions on plugin binaries

### Architecture-Specific Issues

**Apple Silicon (M1/M2/M3) issues:**

```bash
# Check plugin architecture
file build/plugins/vst/Choir\ V2.0.vst3/Contents/MacOS/Choir\ V2.0

# Build for arm64 specifically
cmake .. -DCMAKE_OSX_ARCHITECTURES=arm64

# Build universal binary (x86_64 + arm64)
cmake .. -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"
```

**Rosetta 2 translation:**

```bash
# If plugin is x86_64 only, ensure DAW is running under Rosetta
# Right-click DAW app → Get Info → Open using Rosetta
```

---

## Advanced Configuration

### Custom CMake Options

**Set custom install prefix:**

```bash
cmake .. -DCMAKE_INSTALL_PREFIX=/custom/path
```

**Enable specific JUCE modules:**

```bash
cmake .. -DJUCE_AVAILABLE_MODULES=juce_audio_processors;juce_audio_utils
```

**Disable formats at CMake level:**

```bash
# Edit CMakeLists.txt to set available formats
set(JUCE_PLUGIN_VST3_ENABLED ON)
set(JUCE_PLUGIN_AU_ENABLED ON)
set(JUCE_PLUGIN_CLAP_ENABLED ON)
set(JUCE_PLUGIN_LV2_ENABLED ON)
set(JUCE_PLUGIN_AUV3_ENABLED ON)
```

### Code Signing (macOS)

**Self-signed for development:**

```bash
# Create self-signed certificate
# (Requires Keychain Access setup)

# Sign plugin
codesign --force --deep --sign "Developer ID Application: Your Name" \
    build/plugins/vst/Choir\ V2.0.vst3
```

**Hardened runtime:**

```bash
codesign --force --deep --options runtime --sign "Developer ID Application: Your Name" \
    build/plugins/vst/Choir\ V2.0.vst3
```

### Notarization (macOS Distribution)

**Required for distribution outside Mac App Store:**

```bash
# Notarize plugin
xcrun notarytool submit build/plugins/vst/Choir\ V2.0.vst3.zip \
    --apple-id "your@email.com" \
    --password "app-specific-password" \
    --team-id "TEAMID" \
    --wait

# Staple ticket to plugin
xcrun stapler staple build/plugins/vst/Choir\ V2.0.vst3
```

### Profile-Guided Optimization (PGO)

**Build with profiling:**

```bash
# Step 1: Build with profiling
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build . -j8

# Step 2: Run plugin with representative workload
# (Use standalone or DAW)

# Step 3: Rebuild with profile data
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j8
```

---

## Performance Optimization

### Reduce Binary Size

**Strip debug symbols:**

```bash
# After build
strip -S build/plugins/vst/Choir\ V2.0.vst3/Contents/MacOS/Choir\ V2.0
```

**Enable link-time optimization:**

```bash
cmake .. -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON
```

### Reduce CPU Usage

- Disable unused UI features in release builds
- Optimize DSP code with SIMD instructions
- Reduce timer resolution for UI updates
- Use vector operations for audio processing

---

## Continuous Integration

### GitHub Actions Example

```yaml
name: Build Choir V2.0

on: [push, pull_request]

jobs:
  build:
    runs-on: macos-latest
    steps:
      - uses: actions/checkout@v3
      - name: Install dependencies
        run: brew install cmake
      - name: Build all formats
        run: |
          cd juce_backend/instruments/choral_v2
          ./build_all_formats.sh
      - name: Verify builds
        run: |
          cd juce_backend/instruments/choral_v2
          ./verify_plugins.sh
```

---

## Additional Resources

- **JUCE Documentation**: https://docs.juce.com/
- **VST3 SDK**: https://steinberg.net/github/vst3_dev/
- **CLAP Specification**: https://cleveraudio.org/
- **LV2 Specification**: https://lv2plug.in/
- **AU Documentation**: https://developer.apple.com/documentation/audiounit

---

## Support

For issues specific to Choir V2.0:

1. Check this guide's troubleshooting section
2. Run verification script for detailed diagnostics
3. Consult build logs in `build/` directory
4. Check JUCE forums for general JUCE issues
5. Report bugs to project issue tracker

---

**Last Updated**: 2025-01-18
**Plugin Version**: Choir V2.0
**Supported Formats**: VST3, AU, CLAP, LV2, AUv3, Standalone
