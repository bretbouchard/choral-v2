# Choir V2.0 Plugin Build Instructions

This document describes how to build the Choir V2.0 JUCE plugin wrapper.

## Prerequisites

### Required Dependencies

1. **CMake** 3.20 or later
2. **C++20** compatible compiler (GCC 11+, Clang 13+, MSVC 2022+)
3. **JUCE** 8.0.0 or later
4. **nlohmann/json** 3.11.2 or later

### Installing JUCE

#### Option 1: Download from JUCE website
```bash
# Download JUCE from https://juce.com/get-juce
# Extract to a location of your choice
export JUCE_DIR=/path/to/JUCE
```

#### Option 2: Clone from GitHub
```bash
git clone https://github.com/juce-framework/JUCE.git ~/JUCE
export JUCE_DIR=~/JUCE
```

#### Option 3: Homebrew (macOS)
```bash
brew install juce
export JUCE_DIR=/usr/local/opt/juce
```

### Installing nlohmann/json

#### Linux (Ubuntu/Debian)
```bash
sudo apt-get install nlohmann-json3-dev
```

#### macOS (Homebrew)
```bash
brew install nlohmann-json
```

#### From source
```bash
git clone https://github.com/nlohmann/json.git
cd json
mkdir build && cd build
cmake ..
make && sudo make install
```

## Building the Plugin

### 1. Configure the Build

```bash
# Navigate to the choir-v2-universal directory
cd /path/to/choir-v2-universal

# Create build directory
mkdir -p build
cd build

# Configure with CMake
cmake -DCHOIR_V2_BUILD_PLUGIN=ON \
      -DJUCE_PATH=$JUCE_DIR \
      -DCHOIR_V2_PLUGIN_FORMATS="VST3;AU;CLAP;LV2;Standalone" \
      ..
```

### 2. Build All Plugin Formats

```bash
# Build all formats
cmake --build . --config Release

# Or build specific format
cmake --build . --target ChoirV2Plugin_VST3 --config Release
cmake --build . --target ChoirV2Plugin_AU --config Release
cmake --build . --target ChoirV2Plugin_CLAP --config Release
cmake --build . --target ChoirV2Plugin_LV2 --config Release
cmake --build . --target ChoirV2Plugin_Standalone --config Release
```

### 3. Install Plugins

The build process will create plugins in the following directories:

```
build/
├── plugins/
│   ├── vst/           # VST3 plugin (.vst3)
│   ├── au/            # Audio Units (.component)
│   ├── clap/          # CLAP plugin (.clap)
│   ├── lv2/           # LV2 plugin bundle
│   └── standalone/    # Standalone application
```

### Installing on macOS

```bash
# Copy VST3 plugin
cp -R build/plugins/vst/ChoirV2Plugin.vst3 ~/Library/Audio/Plug-Ins/VST3/

# Copy AU plugin
cp -R build/plugins/au/ChoirV2Plugin.component ~/Library/Audio/Plug-Ins/Components/

# Copy CLAP plugin
cp -R build/plugins/clap/ChoirV2Plugin.clap ~/Library/Audio/Plug-Ins/CLAP/

# Copy standalone app
cp -R build/plugins/standalone/ChoirV2Plugin.app /Applications/
```

### Installing on Linux

```bash
# Copy VST3 plugin
cp -R build/plugins/vst/ChoirV2Plugin.vst3 ~/.vst3/

# Copy CLAP plugin
cp -R build/plugins/clap/ChoirV2Plugin.clap ~/.clap/

# Copy LV2 plugin
cp -R build/plugins/lv2/ChoirV2Plugin.lv2 ~/.lv2/

# Copy standalone app
cp build/plugins/standalone/ChoirV2Plugin ~/bin/
```

### Installing on Windows

```bash
# Copy VST3 plugin
xcopy /E /I build\plugins\vst\ChoirV2Plugin.vst3 C:\Program Files\Common Files\VST3\

# Copy CLAP plugin
xcopy /E /I build\plugins\clap\ChoirV2Plugin.clap C:\Program Files\Common Files\CLAP\

# Copy standalone app
xcopy /E /I build\plugins\standalone C:\Program Files\ChoirV2\
```

## Plugin Features

### Supported Plugin Formats

1. **VST3** - Steinberg Virtual Studio Technology 3
   - Compatible with: Cubase, Ableton Live, Reaper, Bitwig, Studio One
   - Cross-platform (Windows, macOS, Linux)

2. **AU** - Apple Audio Units
   - Compatible with: Logic Pro, GarageBand, MainStage, Final Cut Pro
   - macOS only

3. **CLAP** - CLAP Audio Plugin
   - Compatible with: Bitwig Studio, Reaper (future)
   - Cross-platform (Windows, macOS, Linux)

4. **LV2** - LADSPA Version 2
   - Compatible with: Reaper, Ardour, Bitwig (Linux)
   - Cross-platform (Windows, macOS, Linux)

5. **Standalone** - Desktop Application
   - No DAW required
   - Cross-platform (Windows, macOS, Linux)

### Plugin Capabilities

- **MIDI Input**: 16 MIDI channels
- **Audio Output**: Stereo (2 channels)
- **Synthesizer Type**: Instrument plugin
- **Max Polyphony**: 60 voices
- **Latency**: < 5ms @ 44.1kHz

### Parameters

1. **Language** - Choose synthesis language (English, Latin, Klingon, Throat Singing)
2. **Lyrics** - Text to synthesize (up to 1024 characters)
3. **Voices** - Number of simultaneous voices (1-60)
4. **Master Gain** - Output gain (-60 to 0 dB)
5. **Formant Mix** - Formant synthesis mix (0-100%)
6. **Subharmonic Mix** - Subharmonic synthesis mix (0-100%)
7. **Stereo Width** - Stereo width (0-100%)
8. **Vibrato Rate** - Vibrato rate (0-10 Hz)
9. **Vibrato Depth** - Vibrato depth (0-100%)
10. **Reverb Mix** - Reverb wet/dry mix (0-100%)
11. **Reverb Size** - Reverb room size (0-100%)
12. **Attack** - Envelope attack time (1-500 ms)
13. **Release** - Envelope release time (10-2000 ms)

### Real-Time Safety

All parameter changes are smoothed using `juce::LinearSmoothedValue` to prevent clicks and pops. The plugin is designed for real-time performance with:

- No memory allocations in audio thread
- Bounded execution time
- SIMD-optimized voice processing
- Sub-millisecond voice allocation

## Testing in DAWs

### Ableton Live (macOS/Windows)

1. Open Ableton Live Preferences → Plug-Ins
2. Ensure "VST2 Plug-In Folder" or "VST3 Plug-In Folder" points to install location
3. Rescan plug-ins
4. Drag Choir V2.0 from plugin browser to track

### Logic Pro (macOS)

1. Open Logic Pro
2. Create new Software Instrument track
3. Click Instrument slot → AU Instruments → Bret Bouchard → Choir V2.0

### Reaper (All platforms)

1. Options → Preferences → Plug-ins → VST
2. Add plugin path to "VST plug-in paths"
3. Click "Re-scan" button
4. Insert → Virtual Instrument on new track → Choir V2.0

### Bitwig Studio (All platforms)

1. Open Bitwig Studio
2. Add new instrument track
3. Click "+" → Plugins → Choir V2.0

## Troubleshooting

### Plugin Not Appearing in DAW

1. **Check installation path** - Ensure plugins are in correct directory
2. **Rescan plugins** - Most DAWs require manual rescan after installation
3. **Check permissions** - On macOS, ensure plugins have correct permissions:
   ```bash
   xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/VST3/ChoirV2Plugin.vst3
   ```
4. **Verify format support** - Some DAWs don't support all formats (e.g., Logic only supports AU)

### Build Errors

1. **JUCE not found** - Ensure `JUCE_PATH` is set correctly:
   ```bash
   export JUCE_PATH=/path/to/JUCE
   cmake -DCHOIR_V2_BUILD_PLUGIN=ON -DJUCE_PATH=$JUCE_PATH ..
   ```

2. **nlohmann/json not found** - Install development package:
   ```bash
   # Ubuntu/Debian
   sudo apt-get install nlohmann-json3-dev

   # macOS
   brew install nlohmann-json
   ```

3. **Compilation errors** - Ensure C++20 support:
   ```bash
   # GCC
   g++ --version  # Should be 11 or later

   # Clang
   clang++ --version  # Should be 13 or later
   ```

### Plugin Crashes

1. **Check sample rate** - Plugin supports 44.1kHz, 48kHz, 88.2kHz, 96kHz
2. **Check buffer size** - Plugin supports 64-2048 sample buffers
3. **Check CPU usage** - 60 voices may overload slower CPUs
4. **Check MIDI input** - Ensure MIDI keyboard/controller is connected

## Performance Optimization

### CPU Usage

- **1-20 voices**: ~5-10% CPU (Intel i7)
- **20-40 voices**: ~10-20% CPU
- **40-60 voices**: ~20-30% CPU

If experiencing CPU overload:

1. Reduce number of voices (`Voices` parameter)
2. Increase attack/release times (reduces voice stealing)
3. Disable reverb (`Reverb Mix` = 0%)
4. Use simpler synthesis method (formant only)

### Memory Usage

- **Base**: ~50 MB
- **Per voice**: ~2 MB
- **60 voices**: ~170 MB total

## AUv3 (iOS) Build Instructions

Building for iOS AUv3 requires additional setup:

1. **Install Xcode** (macOS only)
2. **Set iOS deployment target**:
   ```bash
   cmake -DCMAKE_SYSTEM_NAME=iOS \
         -DCMAKE_OSX_ARCHITECTURES=arm64 \
         -DCHOIR_V2_PLUGIN_FORMATS="AUv3" \
         -DJUCE_PATH=$JUCE_DIR \
         ..
   ```

3. **Build for iOS Simulator**:
   ```bash
   cmake --build . --config Release --target ChoirV2Plugin_AUv3
   ```

4. **Install on iOS device**:
   - Copy `.appex` bundle to iOS device
   - Add to GarageBand or AUv3 host app

## Development

### Project Structure

```
src/plugin/
├── ChoirV2Processor.h      # Audio processor header
├── ChoirV2Processor.cpp    # Audio processor implementation
├── ChoirV2Editor.h         # Plugin editor header
└── ChoirV2Editor.cpp       # Plugin editor implementation
```

### Adding New Parameters

1. Add parameter ID constant to `ChoirV2Processor.h`
2. Create parameter in `createParameterLayout()`
3. Add parameter attachment in `ChoirV2Editor.cpp`
4. Add UI control in `ChoirV2Editor::setupUI()`
5. Handle parameter changes in `parameterChanged()`

### Customizing UI

The plugin UI is defined in `ChoirV2Editor.cpp`. To customize:

1. Modify `setupUI()` to add/remove controls
2. Adjust layout in `resized()`
3. Custom painting in `paint()`
4. Update performance display in `updatePerformanceDisplay()`

## License

Same as White Room project

## Contact

- **Repository**: https://github.com/bretbouchard/choir-v2-universal
- **Issues**: Tracked in bd (Beads task management)
- **Documentation**: See `docs/` directory

---

**Status**: Plugin wrapper complete
**Version**: 2.0.0
**Last Updated**: 2026-01-17
