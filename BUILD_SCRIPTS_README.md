# Choir V2.0 Build Scripts

Quick reference guide for building Choir V2.0 plugins.

---

## Available Scripts

### 1. `build_all_formats.sh` (Main Build Script)

Builds all 6 plugin formats with comprehensive verification.

**Usage:**
```bash
./build_all_formats.sh
```

**Environment Variables:**
- `CMAKE_BUILD_TYPE` - Release, Debug, RelWithDebInfo (default: Release)
- `BUILD_JOBS` - Number of parallel jobs (default: 8)
- `VERBOSE_BUILD` - Enable verbose output (default: 0)

**Examples:**
```bash
# Debug build with verbose output
CMAKE_BUILD_TYPE=Debug VERBOSE_BUILD=1 ./build_all_formats.sh

# Build with 4 parallel jobs
BUILD_JOBS=4 ./build_all_formats.sh
```

**What it does:**
1. Cleans previous build
2. Configures CMake for all formats
3. Builds all plugins in parallel
4. Verifies each format
5. Displays summary report

---

### 2. `verify_plugins.sh` (Verification Script)

Validates all built plugin formats for correctness.

**Usage:**
```bash
./verify_plugins.sh
```

**Environment Variables:**
- `SKIP_SIGNATURE_CHECK` - Skip code signature verification (default: 0)
- `VERBOSE_OUTPUT` - Show detailed information (default: 0)

**What it checks:**
- File existence and structure
- File sizes (minimum thresholds)
- File permissions
- Binary architecture (x86_64, arm64, universal)
- Dependencies and linked libraries
- Bundle integrity (Info.plist, manifests)
- Generates verification report

**Output:**
- Console report with color-coded status
- Detailed report at `build/verification_report.txt`

---

### 3. `QUICK_START.sh` (Interactive Menu)

Interactive menu for common build scenarios.

**Usage:**
```bash
./QUICK_START.sh
```

**Menu Options:**
1. Build all 6 formats
2. Build VST3 only
3. Build AU only
4. Build CLAP only
5. Build Standalone only
6. Verify existing builds
7. Exit

---

## Common Workflows

### First Time Build

```bash
./build_all_formats.sh
./verify_plugins.sh
```

### Quick Iteration Development

```bash
# Use interactive menu
./QUICK_START.sh
```

### Debug Build

```bash
CMAKE_BUILD_TYPE=Debug ./build_all_formats.sh
```

### Verify Only

```bash
./verify_plugins.sh
```

### Clean Build

```bash
rm -rf build
./build_all_formats.sh
```

---

## Build Outputs

Built plugins are located in:
```
build/plugins/
├── vst/Choir V2.0.vst3
├── au/Choir V2.0.component
├── clap/Choir V2.0.clap
├── lv2/Choir V2.0.lv2
├── auv/Choir V2.0.appex
└── standalone/Choir V2.0.app
```

---

## Platform-Specific Notes

### macOS
- Builds VST3, AU, CLAP, LV2, AUv3, Standalone
- Output: macOS bundles (.app, .component, .vst3)
- Requires Xcode Command Line Tools

### Linux
- Builds VST3, CLAP, LV2, Standalone
- Output: Shared libraries (.so) and binaries
- Requires build-essential and X11 development packages

### Windows
- Builds VST3, CLAP, LV2, Standalone
- Output: DLLs and executables
- Requires Visual Studio 2019 or later

---

## Troubleshooting

### Script Not Executable

```bash
chmod +x build_all_formats.sh verify_plugins.sh QUICK_START.sh
```

### CMake Not Found

```bash
# macOS
brew install cmake

# Linux
sudo apt-get install cmake
```

### Build Failures

```bash
# Clean build directory
rm -rf build

# Rebuild
./build_all_formats.sh
```

### Permission Denied (macOS)

```bash
# Remove quarantine attribute
xattr -cr build/plugins/vst/Choir\ V2.0.vst3
```

---

## Advanced Usage

### Build Individual Formats

```bash
cd build
cmake .. -DCHOIR_V2_BUILD_PLUGIN=ON -DCHOIR_V2_PLUGIN_FORMATS="VST3;AU"
cmake --build . -j8
```

### Custom Build Directory

```bash
mkdir custom_build
cd custom_build
cmake .. -DCHOIR_V2_BUILD_PLUGIN=ON
cmake --build .
```

### Specific Architecture (macOS)

```bash
# Apple Silicon only
cmake .. -DCMAKE_OSX_ARCHITECTURES=arm64

# Intel only
cmake .. -DCMAKE_OSX_ARCHITECTURES=x86_64

# Universal binary
cmake .. -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"
```

---

## Documentation

For comprehensive documentation, see:
- **PLUGIN_BUILD_GUIDE.md** - Complete build and installation guide
- **CMakeLists.txt** - CMake configuration details

---

## Support

For build issues:
1. Check this README
2. Review PLUGIN_BUILD_GUIDE.md
3. Run verify_plugins.sh for diagnostics
4. Check build logs in build/ directory

---

**Version**: Choir V2.0
**Last Updated**: 2025-01-18
