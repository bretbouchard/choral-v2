# Choir V2.0 System Requirements

Complete hardware and software requirements for building and running Choir V2.0.

## Table of Contents

- [Development Requirements](#development-requirements)
- [Platform-Specific Requirements](#platform-specific-requirements)
- [Format-Specific Requirements](#format-specific-requirements)
- [Performance Requirements](#performance-requirements)
- [Recommended Systems](#recommended-systems)

---

## Development Requirements

### Common Requirements (All Platforms)

#### Software

- **CMake**: Version 3.20 or higher
  - Required for build system configuration
  - Download: https://cmake.org/download/

- **C++ Compiler**: Support for C++20 standard
  - Required for compilation
  - See platform-specific requirements below

- **Git**: Version 2.0 or higher
  - Required for cloning repositories
  - Download: https://git-scm.com/

- **JUCE Framework**: Version 8.0.0 or higher
  - Required for plugin development
  - Download: https://juce.com/get-juce/

#### Hardware

- **RAM**: Minimum 8 GB (16 GB recommended)
- **Storage**: 10 GB free space for build artifacts
- **CPU**: 64-bit processor required
  - x86_64 (Intel/AMD)
  - ARM64 (Apple Silicon)

---

## Platform-Specific Requirements

### macOS

#### Software Requirements

**Operating System**:
- macOS 10.15 (Catalina) or higher
- macOS 11.0 (Big Sur) or higher recommended
- macOS 12.0 (Monterey) or higher for Apple Silicon

**Development Tools**:
- **Xcode**: Version 13.0 or higher
  - Required for compilation
  - Download from Mac App Store
  - Includes: Clang, LLDB, iOS SDK

- **Command Line Tools**:
  ```bash
  xcode-select --install
  ```

- **Xcode Command Line Tools**:
  - Required for building command-line tools
  - Installed with Xcode or separately

**Optional Software**:
- **Pro Tools**: For AAX testing
- **Logic Pro X**: For AU testing
- **Reaper**: For cross-format testing

#### Hardware Requirements

**Intel Mac**:
- CPU: Intel Core i5 or higher (6th gen or later)
- RAM: 8 GB minimum, 16 GB recommended
- Storage: 10 GB free space

**Apple Silicon Mac**:
- CPU: M1 or higher
- RAM: 8 GB unified memory
- Storage: 10 GB free space

**Universal Binary Support**:
- Supports both Intel and Apple Silicon
- Requires Xcode 13.0 or higher

#### Plugin Support

| Format | Supported | Notes |
|--------|-----------|-------|
| **VST3** | ✅ Yes | Universal Binary |
| **AU** | ✅ Yes | Universal Binary |
| **AAX** | ✅ Yes | Requires AAX SDK |
| **CLAP** | ✅ Yes | Universal Binary |
| **LV2** | ✅ Yes | Universal Binary |
| **AUv3** | ✅ Yes | iOS devices only |
| **Standalone** | ✅ Yes | Universal Binary |

---

### Windows

#### Software Requirements

**Operating System**:
- Windows 10 (64-bit) or higher
- Windows 11 (64-bit) recommended

**Development Tools**:
- **Visual Studio**: Version 2019 or higher
  - Required for compilation
  - Community Edition (free)
  - Download: https://visualstudio.microsoft.com/

**Required Visual Studio Components**:
- C++ build tools
- Windows 10/11 SDK
- CMake tools for Visual Studio

**Alternative**:
- **MinGW-w64**: With GCC 10 or higher
- **CLang**: For Windows (LLVM)

**Optional Software**:
- **Pro Tools**: For AAX testing
- **Reaper**: For cross-format testing
- **FL Studio**: For VST3 testing

#### Hardware Requirements

**Minimum**:
- CPU: Intel Core i5 or AMD Ryzen 5 (6th gen or later)
- RAM: 8 GB minimum, 16 GB recommended
- Storage: 10 GB free space

**Recommended**:
- CPU: Intel Core i7 or AMD Ryzen 7 (8th gen or later)
- RAM: 16 GB or higher
- Storage: SSD with 20 GB free space
- Graphics: DirectX 11 compatible

#### Plugin Support

| Format | Supported | Notes |
|--------|-----------|-------|
| **VST3** | ✅ Yes | x86_64 only |
| **AU** | ❌ No | macOS only |
| **AAX** | ✅ Yes | Requires AAX SDK |
| **CLAP** | ✅ Yes | x86_64 only |
| **LV2** | ⚠️ Limited | Not common on Windows |
| **AUv3** | ❌ No | iOS only |
| **Standalone** | ✅ Yes | x86_64 only |

---

### Linux

#### Software Requirements

**Operating System**:
- Ubuntu 20.04 LTS or higher
- Debian 11 or higher
- Fedora 34 or higher
- Arch Linux (rolling)
- Other distributions with glibc 2.31 or higher

**Development Tools** (Ubuntu/Debian):
```bash
sudo apt update
sudo apt install build-essential
sudo apt install cmake
sudo apt install git
```

**Audio Development Libraries**:
```bash
# ALSA (Advanced Linux Sound Architecture)
sudo apt install libasound2-dev

# Freetype (for font rendering)
sudo apt install libfreetype6-dev

# X11 (for GUI)
sudo apt install libx11-dev libxext-dev libxinerama-dev libxrandr-dev libxcursor-dev

# WebKit (optional, for documentation)
sudo apt install libwebkit2gtk-4.0-dev
```

**Additional Libraries** (Fedora/RHEL):
```bash
sudo dnf groupinstall "Development Tools"
sudo dnf install cmake git alsa-lib-devel freetype-devel
```

**Additional Libraries** (Arch):
```bash
sudo pacman -S base-devel cmake git alsa-lib freetype2
```

**Optional Software**:
- **Reaper**: For cross-format testing
- **Ardour**: For LV2 testing
- **Bitwig Studio**: For LV2 testing
- **Carla**: For plugin hosting

#### Hardware Requirements

**Minimum**:
- CPU: Intel Core i5 or AMD Ryzen 5 (6th gen or later)
- RAM: 8 GB minimum, 16 GB recommended
- Storage: 10 GB free space

**Recommended**:
- CPU: Intel Core i7 or AMD Ryzen 7 (8th gen or later)
- RAM: 16 GB or higher
- Storage: SSD with 20 GB free space
- Audio: Low-latency audio interface

#### Plugin Support

| Format | Supported | Notes |
|--------|-----------|-------|
| **VST3** | ✅ Yes | x86_64 only |
| **AU** | ❌ No | macOS only |
| **AAX** | ❌ No | Pro Tools not on Linux |
| **CLAP** | ✅ Yes | x86_64 only |
| **LV2** | ✅ Yes | Primary Linux format |
| **AUv3** | ❌ No | iOS only |
| **Standalone** | ✅ Yes | x86_64 only |

---

### iOS (for AUv3)

#### Software Requirements

**Development Tools**:
- **Xcode**: Version 13.0 or higher
  - Required for iOS builds
  - Download from Mac App Store

**iOS SDK**:
- iOS SDK 12.0 or higher
- Included with Xcode

**Requirements for Distribution**:
- Apple Developer Program membership
- Valid provisioning profile
- App Store Connect access

#### Hardware Requirements

**Build Machine**:
- Mac with Xcode 13.0 or higher
- See macOS requirements above

**Test Devices**:
- iPhone with iOS 12.0 or higher
- iPad with iOS 12.0 or higher
- Both recommended for testing

**Device Recommendations**:
- iPhone 8 or newer
- iPad Pro (2017) or newer
- iPad Air (2019) or newer

#### Plugin Support

| Format | Supported | Notes |
|--------|-----------|-------|
| **AUv3** | ✅ Yes | iOS 12.0+ required |

---

## Format-Specific Requirements

### VST3

**Development**:
- JUCE 8.0.0 or higher
- VST3 SDK (included with JUCE)

**Testing**:
- Host supporting VST3 (Cubase, Reaper, etc.)
- AudioPluginVerifier (JUCE tool)

**Distribution**:
- Steinberg VST3 SDK license agreement
- No certification required

---

### AU (Audio Units)

**Development**:
- macOS 10.15 or higher
- Xcode 13.0 or higher
- AU SDK (included with Xcode)

**Testing**:
- Logic Pro X or GarageBand
- AudioPluginVerifier (JUCE tool)
- AU Lab (deprecated but useful)

**Distribution**:
- No approval required for macOS
- Must comply with Apple's AU guidelines

---

### AAX (Avid Audio Extension)

**Development**:
- AAX SDK from Avid
- Avid developer account
- Pro Tools for testing

**Testing**:
- Pro Tools 11 or higher
- Pro Tools HD (for HDX testing)

**Distribution**:
- Avid developer account required
- Avid approval/certification required
- Distributed via Avid Marketplace

**Note**: AAX SDK requires:
- Signing NDA with Avid
- Approval from Avid
- Annual renewal

---

### CLAP (CLever Audio Plug-in)

**Development**:
- CLAP specification knowledge
- CLAP validator tool
- Host supporting CLAP

**Testing**:
- Reaper 6.68+ or later
- Bitwig Studio 4.4+ or later
- clap-validator tool

**Distribution**:
- No approval required
- Free and open format
- Self-distribution

**Resources**:
- Specification: https://cleveraudio.org/spec/
- Validator: https://github.com/free-audio/clap-validator

---

### LV2 (LADSPA Version 2)

**Development**:
- LV2 specification knowledge
- LV2 headers (lv2plug.in)
- lilv library (for loading)

**Testing**:
- Ardour 6+ or later
- Carla plugin host
- lv2-validate tool

**Distribution**:
- No approval required
- Free and open format
- Self-distribution

**Resources**:
- Specification: https://lv2plug.in/
- Validator: `lv2-validate` command-line tool

---

### AUv3 (iOS Audio Units)

**Development**:
- Xcode 13.0 or higher
- iOS SDK 12.0 or higher
- Apple Developer Program membership

**Testing**:
- iOS device with iOS 12.0+
- GarageBand (iOS)
- AUM or other AUv3 host

**Distribution**:
- App Store only
- Requires App Review approval
- 70/30 revenue split with Apple

**Requirements**:
- Valid provisioning profile
- App Store Connect account
- Compliance with App Store Review Guidelines

---

### Standalone Application

**Development**:
- JUCE 8.0.0 or higher
- Platform-specific SDKs (see above)

**Testing**:
- Desktop environment matching target OS
- Audio device for testing
- MIDI keyboard for testing

**Distribution**:
- Self-distribution
- No approval required (except macOS Gatekeeper)
- Can be sold directly

**Code Signing**:
- macOS: Code signing recommended
- Windows: Code signing recommended
- Linux: No signing required

---

## Performance Requirements

### Minimum System (for Development)

**CPU**:
- Single-core performance: 2.0 GHz
- Multi-core: 4 cores recommended

**RAM**:
- 8 GB minimum
- 16 GB recommended for large projects

**Storage**:
- 10 GB free space for build artifacts
- SSD recommended for faster builds

**Audio**:
- Sample rate: 44.1 kHz - 48 kHz
- Buffer size: 256 - 512 samples
- Latency: < 20 ms

### Recommended System (for Production Use)

**CPU**:
- Single-core performance: 3.0 GHz or higher
- Multi-core: 8 cores recommended
- SIMD support: AVX2 (Intel/AMD) or NEON (ARM)

**RAM**:
- 16 GB minimum
- 32 GB recommended for large projects

**Storage**:
- 20 GB free space
- SSD required for optimal performance

**Audio**:
- Sample rate: Up to 192 kHz
- Buffer size: 64 - 256 samples
- Latency: < 10 ms

### Real-Time Performance Requirements

**DSP Load**:
- Target: < 50% CPU at 48 kHz
- Maximum: < 80% CPU at 48 kHz
- Measure with host's performance meter

**Latency**:
- Total latency: < 10 ms (recommended)
- Maximum acceptable: < 20 ms
- Includes: buffer size + processing time

**Voice Count**:
- Target: 32 voices simultaneously
- Maximum: 64 voices (depending on CPU)
- Dynamic voice stealing implemented

---

## Recommended Systems

### Budget Development System

**macOS**:
- Mac mini M1 (2020 or later)
- 16 GB unified memory
- 512 GB SSD
- Total: ~$700

**Windows**:
- Custom build or pre-built
- Intel Core i5-12400 or AMD Ryzen 5 5600X
- 16 GB DDR4 RAM
- 512 GB NVMe SSD
- Total: ~$800

**Linux**:
- Same hardware as Windows
- Ubuntu 22.04 LTS (free)
- Total: ~$800

### Professional Development System

**macOS**:
- MacBook Pro 14" (M1 Pro/Max)
- 32 GB unified memory
- 1 TB SSD
- Total: ~$2000

**Windows**:
- Custom build or laptop
- Intel Core i7-12700K or AMD Ryzen 7 5800X
- 32 GB DDR4 RAM
- 1 TB NVMe SSD
- NVIDIA RTX 3060 (for GPU-accelerated features)
- Total: ~$1500

**Linux**:
- Same hardware as Windows
- Ubuntu 22.04 LTS or Fedora 36
- Total: ~$1500

### Ultimate Development System

**macOS**:
- Mac Studio (M1 Ultra)
- 64 GB unified memory
- 2 TB SSD
- Total: ~$4000

**Windows**:
- Custom build
- Intel Core i9-13900K or AMD Ryzen 9 7950X
- 64 GB DDR5 RAM
- 2 TB NVMe SSD
- NVIDIA RTX 4080
- Total: ~$3000

**Linux**:
- Same hardware as Windows
- Ubuntu 22.04 LTS or Arch Linux
- Total: ~$3000

---

## Additional Recommendations

### Audio Hardware

**Audio Interface** (recommended for testing):
- Focusrite Scarlett 2i2 (~$160)
- Steinberg UR12 (~$150)
- Native Instruments Komplete Audio 2 (~$200)

**MIDI Controller** (recommended for testing):
- AKAI MPK Mini (~$100)
- Novation Launchkey Mini (~$150)
- Native Instruments Komplete Kontrol A-Series (~$200)

### Monitoring

**Studio Monitors** (recommended):
- PreSonus Eris E3.5 (~$100)
- KRK Rokit 5 G4 (~$300)
- Yamaha HS5 (~$400)

**Headphones** (recommended):
- Audio-Technica ATH-M40x (~$100)
- Sony MDR-7506 (~$100)
- Beyerdynamic DT 770 Pro (~$150)

---

## Troubleshooting Requirements

### Build Issues

**Problem**: CMake version too old
**Solution**: Install latest CMake from website

**Problem**: Compiler not found
**Solution**: Install Xcode (macOS) or Visual Studio (Windows)

**Problem**: JUCE not found
**Solution**: Set JUCE_PATH environment variable

### Runtime Issues

**Problem**: Plugin not loading
**Solution**: Verify architecture matches host

**Problem**: High CPU usage
**Solution**: Increase buffer size or reduce voice count

**Problem**: Audio glitches
**Solution**: Increase buffer size or check audio device

---

## Additional Resources

### Development Tools

- **CMake**: https://cmake.org/
- **JUCE**: https://juce.com/
- **Git**: https://git-scm.com/

### Learning Resources

- **JUCE Tutorial**: https://docs.juce.com/
- **Plugin Development**: https://www.youtube.com/c/TheAudioProgrammer
- **C++20**: https://en.cppreference.com/w/cpp/20

### Community Support

- **JUCE Forum**: https://forum.juce.com/
- **KVR Audio**: https://kvraudio.com/
- **REAPER Forum**: https://forum.cockos.com/

---

**Last Updated**: 2025-01-17
**Version**: 2.0.0
