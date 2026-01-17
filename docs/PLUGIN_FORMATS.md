# Choir V2.0 Plugin Formats Guide

Comprehensive information about all 7 supported plugin formats.

## Table of Contents

- [Format Overview](#format-overview)
- [VST3](#vst3)
- [AU (Audio Units)](#au-audio-units)
- [AAX (Avid Audio Extension)](#aax-avid-audio-extension)
- [CLAP (CLever Audio Plug-in)](#clap-clever-audio-plug-in)
- [LV2 (LADSPA Version 2)](#lv2-ladspa-version-2)
- [AUv3 (Audio Units Version 3)](#auv3-audio-units-version-3)
- [Standalone Application](#standalone-application)
- [Format Comparison](#format-comparison)
- [Best Practices](#best-practices)

---

## Format Overview

| Format | Platforms | Status | Primary DAWs | Distribution |
|--------|-----------|--------|--------------|--------------|
| **VST3** | macOS, Windows, Linux | ✅ Production | Cubase, Reaper, Live | Broad |
| **AU** | macOS only | ✅ Production | Logic, GarageBand, Live | macOS App Store |
| **AAX** | macOS, Windows | ✅ Production | Pro Tools | Avid Marketplace |
| **CLAP** | macOS, Windows, Linux | ✅ Production | Reaper, Bitwig | Free/open |
| **LV2** | Linux, macOS | ✅ Production | Ardour, Bitwig | Free/open |
| **AUv3** | iOS only | ✅ Production | GarageBand, AUM | App Store |
| **Standalone** | macOS, Windows, Linux | ✅ Production | Desktop apps | Direct download |

---

## VST3

### Overview

**VST3** (Virtual Studio Technology 3) is Steinberg's flagship plugin format, widely supported across most modern DAWs.

### Technical Details

- **Bundle Structure**: `.vst3` bundle (macOS), `.vst3` folder (Windows/Linux)
- **Architecture**: Universal Binary (macOS), x86_64 (Windows/Linux)
- **MIDI Support**: 16 MIDI input channels
- **Audio I/O**: Stereo output, configurable input
- **Parameters**: Automatable parameters with sample accuracy
- **UI**: Resizable editor window

### Bundle Structure (macOS)

```
ChoirV2.vst3/
├── Contents/
│   ├── MacOS/
│   │   └── ChoirV2           # Binary (Universal)
│   ├── Resources/
│   │   └── info.plist        # Plugin metadata
│   └── version.plist
```

### Bundle Structure (Windows)

```
ChoirV2.vst3/
└── Contents/
    ├── x86_64-win/
    │   └── ChoirV2.vst3/
    │       └── ChoirV2.dll    # Binary
    └── Resources/
        └── plugin.ico
```

### Installation Locations

#### macOS

```bash
# System-wide
sudo cp -R ChoirV2.vst3 /Library/Audio/Plug-Ins/VST3/

# User-specific
cp -R ChoirV2.vst3 ~/Library/Audio/Plug-Ins/VST3/
```

#### Windows

```cmd
REM Copy to VST3 directory
xcopy /E /I ChoirV2.vst3 "C:\Program Files\Common Files\VST3\"
```

#### Linux

```bash
# User-specific
mkdir -p ~/.vst3
cp -R ChoirV2.vst3 ~/.vst3/

# System-wide
sudo cp -R ChoirV2.vst3 /usr/lib/vst3/
```

### Host Compatibility

**Full Support**:
- Steinberg Cubase 5+
- PreSonus Studio One 2+
- Reaper 4+
- Ableton Live 9+
- Bitwig Studio 1+
- FL Studio 20+
- Renoise 3+

**Partial Support**:
- Apple Logic Pro X (via wrapper)
- Ardour (via Carla)

### Common Issues

**Issue**: Plugin not appearing in DAW
**Solution**: Rescan plugin pool in DAW preferences

**Issue**: "Damaged plugin" error (macOS)
**Solution**: Re-sign plugin: `codesign --force --deep --sign - ChoirV2.vst3`

**Issue**: Crashes on load
**Solution**: Verify architecture matches DAW (Intel vs ARM)

---

## AU (Audio Units)

### Overview

**AU** (Audio Units) is Apple's native plugin format, deeply integrated into macOS and iOS.

### Technical Details

- **Bundle Structure**: `.component` bundle
- **Architecture**: Universal Binary (Intel + Apple Silicon)
- **MIDI Support**: 16 MIDI input channels
- **Audio I/O**: Stereo output, configurable input
- **Parameters**: Automatable with sample accuracy
- **UI**: Native Cocoa view, resizable
- **Type**: `aumu` (Music Device)

### Bundle Structure

```
ChoirV2.component/
├── Contents/
│   ├── MacOS/
│   │   └── ChoirV2           # Binary (Universal)
│   ├── Resources/
│   │   └── Info.plist        # Component metadata
│   └── version.plist
```

### Installation

```bash
# System-wide
sudo cp -R ChoirV2.component /Library/Audio/Plug-Ins/Components/

# User-specific
cp -R ChoirV2.component ~/Library/Audio/Plug-Ins/Components/

# Restart AU component registry
sudo killall -9 AUComponentHelper
```

### Host Compatibility

**Full Support**:
- Apple Logic Pro X
- Apple GarageBand
- MainStage 3
- Ableton Live 9+
- Reaper 4+
- Digital Performer 8+
- Ardour 6+

**No Support**:
- Pro Tools (use AAX instead)

### AU Specific Features

- **Offline Bounce**: Optimized for offline rendering
- **Sample Rate Conversion**: Automatic SRC support
- **MIDI Learn**: Built-in MIDI learn support
- **Presets**: AU preset system (.aupreset)

### Common Issues

**Issue**: Plugin not visible in Logic Pro
**Solution**:
1. Verify plugin is in correct location
2. Rescan plugins in Logic preferences
3. Restart Logic Pro

**Issue**: Validation fails
**Solution**: Use AudioPluginVerifier to identify issues

**Issue**: Plugin crashes Logic Pro
**Solution**: Check crash logs in Console.app

---

## AAX (Avid Audio Extension)

### Overview

**AAX** is Avid's proprietary plugin format for Pro Tools, requiring special SDK and certification.

### Technical Details

- **Bundle Structure**: `.aaxplugin` bundle
- **Architecture**: Universal Binary (macOS), x86_64 (Windows)
- **MIDI Support**: 16 MIDI input channels
- **Audio I/O**: Configurable I/O (mono, stereo, surround)
- **Parameters**: Pro Tools automation enabled
- **UI**: Resizable editor window
- **SDK**: Requires Avid AAX SDK

### Bundle Structure (macOS)

```
ChoirV2.aaxplugin/
├── Contents/
│   ├── MacOS/
│   │   └── ChoirV2           # Binary
│   ├── Resources/
│   │   └── Info.plist
│   └── version.plist
```

### Installation

#### macOS

```bash
# User-specific
cp -R ChoirV2.aaxplugin ~/Library/Application\ Support/Avid/Audio/Plug-Ins/

# System-wide
sudo cp -R ChoirV2.aaxplugin "/Library/Application Support/Avid/Audio/Plug-Ins/"
```

#### Windows

```cmd
REM Copy to AAX plugins directory
xcopy /E /I ChoirV2.aaxplugin "C:\Program Files\Common Files\Avid\Audio\Plug-Ins\"
```

### Host Compatibility

**Full Support**:
- Pro Tools 11+ (Native)
- Pro Tools HD 11+ (HDX systems)
- Pro Tools Ultimate

**Requirements**:
- Avid account for distribution
- AAX SDK license
- Plugin certification from Avid

### AAX Specific Features

- **Pro Tools Automation**: Full automation support
- **Clip Effects**: Support for clip-based effects
- **Hardware Integration**: HDX acceleration support
- ** Surround Sound**: Up to 7.1 surround support

### Common Issues

**Issue**: Plugin not visible in Pro Tools
**Solution**:
1. Verify plugin is in correct location
2. Restart Pro Tools
3. Check plugin compatibility with Pro Tools version

**Issue**: "Not authorized" error
**Solution**: Requires Avid developer account and proper signing

**Issue**: Performance issues in HDX
**Solution**: Optimize DSP code for real-time constraints

---

## CLAP (CLever Audio Plug-in)

### Overview

**CLAP** is a modern, open-source plugin format focused on performance and developer experience.

### Technical Details

- **File Extension**: `.clap`
- **Architecture**: Universal Binary (macOS), x86_64 (Windows/Linux)
- **MIDI Support**: 16 MIDI input channels
- **Audio I/O**: Configurable I/O
- **Parameters**: Advanced parameter modulation
- **UI**: Resizable, scaleable UI
- **Spec**: https://cleveraudio.org/spec/

### File Structure

```
ChoirV2.clap         # Single binary file (shared library)
```

### Installation

#### macOS

```bash
# User-specific
mkdir -p ~/Library/Audio/Plug-Ins/CLAP
cp ChoirV2.clap ~/Library/Audio/Plug-Ins/CLAP/

# System-wide
sudo cp ChoirV2.clap /Library/Audio/Plug-Ins/CLAP/
```

#### Windows

```cmd
REM Copy to CLAP directory
copy ChoirV2.clap "C:\Program Files\Common Files\CLAP\"
```

#### Linux

```bash
# User-specific
mkdir -p ~/.clap
cp ChoirV2.clap ~/.clap/

# System-wide
sudo cp ChoirV2.clap /usr/lib/clap/
```

### Host Compatibility

**Full Support**:
- Reaper 6.68+
- Bitwig Studio 4.4+
- Waveform 12+
- REAPER (via plugin)

**Growing Support**:
- Tracktion Waveform
- LMMS (planned)
- Ardour (planned)

### CLAP Specific Features

- **Polyphonic Modulation**: Advanced voice-level modulation
- **Event Expression**: Precise parameter automation
- **Plugin Pool**: Efficient plugin management
- **State Extension**: Flexible state management
- **Note Ports**: Multiple MIDI input/output ports
- **Tail Management**: Automatic tail detection
- **Thread Safety**: Multi-threaded processing

### Common Issues

**Issue**: Plugin not recognized by host
**Solution**: Verify host supports CLAP format

**Issue**: Validation errors
**Solution**: Use `clap-validator` to test plugin

**Issue**: Performance issues
**Solution**: Optimize for single-threaded processing

---

## LV2 (LADSPA Version 2)

### Overview

**LV2** is a plugin standard for Linux, also supported on other platforms, focused on open standards.

### Technical Details

- **Bundle Structure**: `.lv2` bundle
- **Architecture**: x86_64 (Linux), Universal (macOS)
- **MIDI Support**: 16 MIDI input channels
- **Audio I/O**: Configurable I/O
- **Parameters**: Advanced parameter control
- **UI**: Optional UI (native or external)
- **Spec**: https://lv2plug.in/

### Bundle Structure

```
ChoirV2.lv2/
├── manifest.ttl              # Plugin metadata
├── ChoirV2.ttl               # Plugin description
├── ChoirV2.so                # Binary (Linux)
├── ui/
│   └── ChoirV2_ui.so         # UI binary (optional)
└── presets/
    └── *.preset.ttl          # Preset files
```

### Installation

#### Linux

```bash
# User-specific
mkdir -p ~/.lv2
cp -R ChoirV2.lv2 ~/.lv2/

# System-wide
sudo cp -R ChoirV2.lv2 /usr/lib/lv2/
```

#### macOS

```bash
# User-specific
mkdir -p ~/Library/Audio/Plug-Ins/LV2
cp -R ChoirV2.lv2 ~/Library/Audio/Plug-Ins/LV2/
```

### Host Compatibility

**Full Support**:
- Ardour 6+
- Bitwig Studio (Linux)
- Reaper (Linux)
- Qtractor
- Muse-Seq
- Carla

**Growing Support**:
- LMMS
- Rosegarden
- Non-Mixer

### LV2 Specific Features

- **RDF Description**: Rich metadata via Turtle files
- **Presets**: Standardized preset format
- **UI Extension**: Native or external UIs
- **Parameters**: Advanced parameter control
- **State**: Flexible state management
- **Port Groups**: Logical port organization
- **Scale Points**: Discrete parameter values

### Common Issues

**Issue**: Plugin not found by host
**Solution**: Verify bundle is in LV2_PATH

**Issue**: Missing UI
**Solution**: UI is optional in LV2, host provides generic UI

**Issue**: Validation errors
**Solution**: Use `lv2-validate` to check manifest.ttl

---

## AUv3 (Audio Units Version 3)

### Overview

**AUv3** is Apple's modern plugin format for iOS devices, requiring App Store distribution.

### Technical Details

- **Bundle Structure**: `.auv3` app extension
- **Architecture**: ARM64 (iOS devices)
- **MIDI Support**: 16 MIDI input channels
- **Audio I/O**: Configurable I/O
- **Parameters**: iOS automation support
- **UI**: UIKit-based UI
- **Distribution**: App Store only

### Bundle Structure

```
ChoirV2.app/
├── Plugins/
│   └── ChoirV2.auv3/
│       └── Contents/
│           ├── MacOS/
│           │   └── ChoirV2       # Binary (ARM64)
│           └── Resources/
│               └── Info.plist
└── ChoirV2                      # Host app
```

### Installation

AUv3 plugins **cannot be installed manually**. They must be:

1. Embedded in a host app (container app)
2. Distributed via App Store
3. Or installed via TestFlight for testing

### Host Compatibility

**Full Support**:
- GarageBand (iOS)
- Logic Pro (iOS/Mac)
- AUM (Audio Mixer)
- Cubasis (iOS)
- NanoStudio 2
- Brambos apps

**Requirements**:
- iOS 12.0 or higher
- App Store developer account
- Proper provisioning profiles

### AUv3 Specific Features

- **iOS Integration**: Deep iOS system integration
- **Touch UI**: Optimized for touch interface
- **Inter-App Audio**: IAA support
- **AU Parameters**: iOS parameter automation
- **MIDI Controller**: External MIDI controller support
- **Background Audio**: Background audio support
- **Document Picker**: File import/export

### Common Issues

**Issue**: Plugin not visible in host
**Solution**:
1. Rescan plugins in host app
2. Restart host app
3. Check iOS version compatibility

**Issue**: "Plugin not compatible" error
**Solution**: Verify iOS version and device architecture

**Issue**: Distribution issues
**Solution**: Requires proper App Store submission process

---

## Standalone Application

### Overview

The standalone application allows Choir V2.0 to be used without a DAW, as a desktop instrument.

### Technical Details

- **Bundle Structure**: `.app` bundle (macOS), `.exe` (Windows), binary (Linux)
- **Architecture**: Universal Binary (macOS), x86_64 (Windows/Linux)
- **MIDI Support**: All available MIDI inputs
- **Audio I/O**: Configurable audio device
- **UI**: Full-featured UI with preferences
- **Settings**: Persistent settings and configurations

### App Structure (macOS)

```
ChoirV2.app/
├── Contents/
│   ├── MacOS/
│   │   └── ChoirV2           # Binary (Universal)
│   ├── Resources/
│   │   ├── Icon.icns
│   │   └── Info.plist
│   └── PkgInfo
```

### Installation

#### macOS

```bash
# Copy to Applications
cp -R ChoirV2.app /Applications/

# Or drag-and-drop in Finder
```

#### Windows

```cmd
REM Copy to Program Files
xcopy /E /I ChoirV2 "C:\Program Files\ChoirV2\"

REM Create desktop shortcut
mklink "C:\Users\Public\Desktop\ChoirV2.lnk" "C:\Program Files\ChoirV2\ChoirV2.exe"
```

#### Linux

```bash
# Copy to bin
mkdir -p ~/bin
cp ChoirV2 ~/bin/

# Or /usr/local/bin
sudo cp ChoirV2 /usr/local/bin/

# Create desktop entry
cat > ~/.local/share/applications/choirv2.desktop <<EOF
[Desktop Entry]
Name=Choir V2.0
Exec=/usr/local/bin/ChoirV2
Type=Application
Categories=Audio;AudioVideo;
Icon=choirv2
EOF
```

### Standalone Features

- **Audio Device Selection**: Choose any audio interface
- **MIDI Device Selection**: Choose any MIDI input
- **Buffer Size Control**: Adjustable buffer size
- **Sample Rate Selection**: Support for various sample rates
- **Preset Management**: Load/save presets
- **MIDI Learn**: MIDI controller mapping
- **Recording**: Built-in audio recording (optional)

### Common Issues

**Issue**: No audio output
**Solution**: Check audio device selection in preferences

**Issue**: No MIDI input
**Solution**: Verify MIDI device is connected and selected

**Issue**: High latency
**Solution**: Reduce buffer size in audio preferences

---

## Format Comparison

### Feature Matrix

| Feature | VST3 | AU | AAX | CLAP | LV2 | AUv3 | Standalone |
|---------|------|----|----|----|-----|-----|------------|
| **Platform** | Cross | macOS | Win/Mac | Cross | Linux | iOS | Cross |
| **MIDI** | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| **Automation** | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ❌ |
| **Presets** | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| **Resizable UI** | ✅ | ✅ | ✅ | ✅ | ⚠️ | ✅ | ✅ |
| **Scaleable UI** | ⚠️ | ⚠️ | ⚠️ | ✅ | ⚠️ | ✅ | ✅ |
| **Poly Mod** | ❌ | ❌ | ❌ | ✅ | ❌ | ❌ | ❌ |
| **Offline Bounce** | ✅ | ✅ | ✅ | ✅ | ✅ | ❌ | ❌ |
| **Sidechain** | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ❌ |
| **Multi-Output** | ✅ | ✅ | ✅ | ✅ | ✅ | ⚠️ | ❌ |

### Distribution Comparison

| Format | Distribution | Revenue Share | Approval Process |
|--------|--------------|---------------|------------------|
| **VST3** | Direct/Partners | 100% | None |
| **AU** | App Store/Direct | 70-100% | None (macOS) |
| **AAX** | Avid Marketplace | 70% | Required |
| **CLAP** | Direct | 100% | None |
| **LV2** | Direct | 100% | None |
| **AUv3** | App Store | 70% | Required |
| **Standalone** | Direct | 100% | None |

---

## Best Practices

### Development

1. **Test All Formats**: Test each format independently
2. **Format-Specific Code**: Minimize format-specific code paths
3. **Parameter Smoothing**: Smooth all parameter changes
4. **Thread Safety**: Ensure thread-safe parameter access
5. **State Restoration**: Test save/load functionality

### Distribution

1. **Code Signing**: Always sign plugins (macOS/Windows)
2. **Notarization**: Notarize macOS plugins
3. **Format Priority**: Prioritize VST3 + AU for macOS
4. **Testing**: Test in multiple DAWs before release
5. **Documentation**: Provide format-specific installation instructions

### Performance

1. **Optimize DSP**: Keep DSP code efficient
2. **Avoid Blocking**: Never block in audio thread
3. **Memory Pooling**: Use memory pools for voice allocation
4. **SIMD**: Use SIMD optimizations where possible
5. **SSE/NEON**: Target CPU-specific optimizations

### User Experience

1. **Resizable UI**: Implement resizable UI where supported
2. **Presets**: Include quality factory presets
3. **Documentation**: Provide comprehensive documentation
4. **Demo Version**: Consider time-limited demo version
5. **Support**: Provide responsive user support

---

## Additional Resources

### Format Specifications

- [VST3 SDK](https://steinberg.net/vst3-sdk/)
- [AU Documentation](https://developer.apple.com/documentation/audiounits)
- [AAX SDK](https://developer.avid.com/aax)
- [CLAP Specification](https://cleveraudio.org/spec/)
- [LV2 Specification](https://lv2plug.in/)

### Testing Tools

- **AudioPluginVerifier** (JUCE)
- **clap-validator** (CLAP)
- **lv2-validate** (LV2)
- **AU Lab** (macOS AU)

### Host Applications

- **Reaper** - All formats (cross-platform)
- **Logic Pro X** - AU, VST3 (macOS)
- **Cubase** - VST3 (cross-platform)
- **Pro Tools** - AAX (cross-platform)
- **Ardour** - LV2, VST3 (Linux)

---

**Last Updated**: 2025-01-17
**Version**: 2.0.0
