# Choir V2.0 - All 6 Plugin Formats Enabled

## Summary

Successfully enabled and configured all 6 plugin formats for ChoirV2 along with re-enabling the ChoirV2Core library.

## Changes Made

### 1. CMakeLists.txt Configuration

**Location**: `/Users/bretbouchard/apps/schill/white_room/juce_backend/instruments/choral_v2/CMakeLists.txt`

#### Key Changes:

1. **Re-enabled ChoirV2Core Library** (Line 13)
   ```cmake
   option(CHOIR_V2_BUILD_CORE "Build ChoirV2Core (old ChoirV2 code)" ON)
   ```
   - Changed from `OFF` to `ON`
   - Enables the complete ChoirV2 synthesis engine

2. **Plugin Formats** (Line 14)
   ```cmake
   option(CHOIR_V2_PLUGIN_FORMATS "Plugin formats to build" "VST3;AU;CLAP;LV2;AUv3;Standalone")
   ```
   - All 6 formats configured:
     - **VST3** - macOS/Windows plugin
     - **AU** - macOS Audio Units
     - **CLAP** - CLAP plugin format
     - **LV2** - Linux plugin format
     - **AUv3** - iOS/tvOS Audio Units v3
     - **Standalone** - Desktop application

3. **Plugin Sources** (Lines 70-78)
   ```cmake
   set(PLUGIN_SOURCES
       src/plugin/ChoirV2Processor.cpp
       src/plugin/ChoirV2Editor.cpp
   )

   set(PLUGIN_HEADERS
       src/plugin/ChoirV2Processor.h
       src/plugin/ChoirV2Editor.h
   )
   ```

4. **JUCE Plugin Configuration** (Lines 229-246)
   ```cmake
   juce_add_plugin(ChoirV2Plugin
       COMPANY_NAME "Bret Bouchard"
       PLUGIN_MANUFACTURER_CODE "BBbr"
       PLUGIN_CODE "ChV2"
       FORMATS ${CHOIR_V2_PLUGIN_FORMATS}
       PRODUCT_NAME "Choir V2.0"
       VERSION ${PROJECT_VERSION}
       IS_SYNTH TRUE
       NEEDS_MIDI_INPUT TRUE
       NEEDS_MIDI_OUTPUT FALSE
       IS_MIDI_EFFECT FALSE
       VST_NUM_MIDI_INPUTS 16
       VST_NUM_MIDI_OUTPUTS 0
       AU_MAIN_TYPE "aumu"
       LV2_URI "https://bretbouchard.github.io/choir-v2-universal"
       LV2_CATEGORY "Instrument"
       COPY_PLUGIN_AFTER_BUILD TRUE
   )
   ```

5. **Output Directories** (Lines 289-323)
   - VST3: `plugins/vst/`
   - AU: `plugins/au/`
   - CLAP: `plugins/clap/`
   - LV2: `plugins/lv2/`
   - AUv3: `plugins/auv3/`
   - Standalone: `plugins/standalone/`

### 2. Build Script Updated

**Location**: `/Users/bretbouchard/apps/schill/white_room/juce_backend/instruments/choral_v2/build_all_formats.sh`

**Change** (Line 78):
```bash
cmake_args=(
    "-DCHOIR_V2_BUILD_PLUGIN=ON"
    "-DCHOIR_V2_BUILD_CORE=ON"  # Added this line
    "-DCHOIR_V2_PLUGIN_FORMATS=${PLUGIN_FORMATS}"
    "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
)
```

## Build Instructions

### Quick Start
```bash
cd /Users/bretbouchard/apps/schill/white_room/juce_backend/instruments/choral_v2
chmod +x build_all_formats.sh
./build_all_formats.sh
```

### Manual Build
```bash
mkdir -p build
cd build
cmake .. -DCHOIR_V2_BUILD_PLUGIN=ON -DCHOIR_V2_BUILD_CORE=ON
cmake --build . -j8
```

## Expected Build Output

After successful build, the following plugin artifacts will be created:

```
build/
├── plugins/
│   ├── vst/
│   │   └── Choir V2.0.vst3/          # VST3 plugin bundle
│   ├── au/
│   │   └── Choir V2.0.component/     # AU plugin bundle
│   ├── clap/
│   │   └── Choir V2.0.clap           # CLAP plugin
│   ├── lv2/
│   │   └── Choir V2.0.lv2/           # LV2 plugin bundle
│   ├── auv3/
│   │   └── Choir V2.0.appex/         # AUv3 app extension
│   └── standalone/
│       └── Choir V2.0.app/           # Standalone app (macOS)
```

## Platform Support

### macOS
- ✅ VST3 (Universal Binary)
- ✅ AU (Intel + Apple Silicon)
- ✅ CLAP
- ✅ LV2
- ✅ AUv3 (iOS/tvOS)
- ✅ Standalone

### Windows
- ✅ VST3
- ✅ CLAP
- ✅ LV2
- ✅ Standalone

### Linux
- ✅ VST3
- ✅ CLAP
- ✅ LV2
- ✅ Standalone

## Dependencies

### Required
- CMake 3.20 or higher
- C++20 compatible compiler
- JUCE 8.0.0 or higher
- nlohmann/json 3.11.2 or higher

### Environment Variables
```bash
# Set JUCE path (optional if installed in standard location)
export JUCE_PATH=/path/to/JUCE
```

## Configuration Options

### CMake Options
```bash
# Enable/disable components
-DCHOIR_V2_BUILD_PLUGIN=ON           # Build JUCE plugin (default: ON)
-DCHOIR_V2_BUILD_CORE=ON             # Build ChoirV2Core library (default: ON)
-DCHOIR_V2_BUILD_TESTS=OFF           # Build unit tests (default: OFF)
-DCHOIR_V2_BUILD_BENCHMARKS=ON       # Build benchmarks (default: ON)
-DCHOIR_V2_ENABLE_SIMD=ON            # Enable SIMD optimizations (default: ON)

# Select formats to build
-DCHOIR_V2_PLUGIN_FORMATS="VST3;AU;CLAP;LV2;AUv3;Standalone"

# Build type
-DCMAKE_BUILD_TYPE=Release           # Debug or Release
```

## Installation

### System-wide Installation (macOS)
```bash
# VST3
sudo cp -R "build/plugins/vst/Choir V2.0.vst3" /Library/Audio/Plug-Ins/VST3/

# AU
sudo cp -R "build/plugins/au/Choir V2.0.component" /Library/Audio/Plug-Ins/Components/

# Standalone
sudo cp -R "build/plugins/standalone/Choir V2.0.app" /Applications/
```

### User Installation (macOS)
```bash
# VST3
cp -R "build/plugins/vst/Choir V2.0.vst3" ~/Library/Audio/Plug-Ins/VST3/

# AU
cp -R "build/plugins/au/Choir V2.0.component" ~/Library/Audio/Plug-Ins/Components/
```

## Verification

The build script includes automatic verification of all plugin formats:

```bash
./build_all_formats.sh
```

Expected output:
```
==================================
Build Verification
==================================

➜ Verifying VST3 plugin...
✓ VST3 plugin built successfully
  Location: build/plugins/vst/Choir V2.0.vst3
  Binary: build/plugins/vst/Choir V2.0.vst3/Contents/MacOS/Choir V2.0
  Size: XXX

➜ Verifying AU plugin...
✓ AU plugin built successfully
  Location: build/plugins/au/Choir V2.0.component
  Binary: build/plugins/au/Choir V2.0.component/Contents/MacOS/Choir V2.0
  Size: XXX

➜ Verifying CLAP plugin...
✓ CLAP plugin built successfully
  Location: build/plugins/clap/Choir V2.0.clap
  Size: XXX

➜ Verifying LV2 plugin...
✓ LV2 plugin built successfully
  Location: build/plugins/lv2/Choir V2.0.lv2
  Binaries: X
  Manifest: Present

➜ Verifying AUv3 plugin...
✓ AUv3 plugin built successfully
  Location: build/plugins/auv3/Choir V2.0.appex
  Binary: build/plugins/auv3/Choir V2.0.appex/Contents/MacOS/Choir V2.0
  Size: XXX

➜ Verifying Standalone app...
✓ Standalone app built successfully
  Location: build/plugins/standalone/Choir V2.0.app
  Binary: build/plugins/standalone/Choir V2.0.app/Contents/MacOS/Choir V2.0
  Size: XXX

==================================
Verification Summary
==================================
Successfully built: 6/6 formats
✓ All plugin formats built successfully!
```

## Troubleshooting

### JUCE Not Found
```
WARNING: JUCE not found. Plugin build disabled.
Set JUCE_PATH environment variable or pass -DJUCE_PATH=/path/to/JUCE
```

**Solution**:
```bash
export JUCE_PATH=/path/to/JUCE
# Or
cmake .. -DJUCE_PATH=/path/to/JUCE
```

### Missing Dependencies
```
Could NOT find nlohmann_json
```

**Solution** (macOS):
```bash
brew install nlohmann-json
```

**Solution** (Linux):
```bash
sudo apt-get install nlohmann-json3-dev
```

### Plugin Signing (macOS)
If plugins fail to load in DAWs due to code signing:

```bash
# VST3
codesign --force --deep --sign - "build/plugins/vst/Choir V2.0.vst3"

# AU
codesign --force --deep --sign - "build/plugins/au/Choir V2.0.component"

# Standalone
codesign --force --deep --sign - "build/plugins/standalone/Choir V2.0.app"
```

## Architecture

### Library Components

1. **ChoirV2PureDSP** - PureDSP refactored synthesis engine
2. **ChoirV2Core** - Original ChoirV2 synthesis engine (now enabled)
3. **ChoirV2Plugin** - JUCE plugin wrapper

### Synthesis Methods

- **Formant Synthesis** - Traditional formant-based choir
- **Subharmonic Synthesis** - Subharmonic generator for bass voices
- **Diphone Synthesis** - Diphone-based concatenative synthesis

## Next Steps

1. **Build plugins**: Run `./build_all_formats.sh`
2. **Test in DAWs**: Load plugins in your preferred DAW
3. **Validate presets**: Test factory presets
4. **Performance testing**: Run benchmarks
5. **Package release**: Create distribution packages

## Files Modified

- `/Users/bretbouchard/apps/schill/white_room/juce_backend/instruments/choral_v2/CMakeLists.txt`
  - Line 13: Enabled `CHOIR_V2_BUILD_CORE`
  - Lines 14, 229-246: Plugin format configuration
  - Lines 289-323: Output directory configuration

- `/Users/bretbouchard/apps/schill/white_room/juce_backend/instruments/choral_v2/build_all_formats.sh`
  - Line 78: Added `-DCHOIR_V2_BUILD_CORE=ON`

## Status

✅ **Configuration Complete**
✅ **All 6 Plugin Formats Enabled**
✅ **ChoirV2Core Library Re-enabled**
✅ **Build Script Updated**
✅ **Ready to Build**

---

**Generated**: 2025-01-18
**Choir V2.0**: Universal Vocal Synthesis Platform
**Formats**: VST3, AU, CLAP, LV2, AUv3, Standalone
