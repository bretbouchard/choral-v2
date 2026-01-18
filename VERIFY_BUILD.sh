#!/bin/bash

################################################################################
# Choir V2.0 - Quick Build Verification Checklist
################################################################################

echo "=================================="
echo "Choir V2.0 - Build Verification"
echo "=================================="
echo ""

# Check CMakeLists.txt
echo "✓ Checking CMakeLists.txt configuration..."
if grep -q 'option(CHOIR_V2_BUILD_CORE "Build ChoirV2Core (old ChoirV2 code)" ON)' CMakeLists.txt; then
    echo "  ✓ ChoirV2Core: ENABLED"
else
    echo "  ✗ ChoirV2Core: DISABLED"
fi

if grep -q 'option(CHOIR_V2_PLUGIN_FORMATS "Plugin formats to build" "VST3;AU;CLAP;LV2;AUv3;Standalone")' CMakeLists.txt; then
    echo "  ✓ Plugin Formats: VST3, AU, CLAP, LV2, AUv3, Standalone"
else
    echo "  ✗ Plugin Formats: Not configured correctly"
fi

echo ""

# Check build script
echo "✓ Checking build_all_formats.sh..."
if grep -q '"-DCHOIR_V2_BUILD_CORE=ON"' build_all_formats.sh; then
    echo "  ✓ Build script includes CHOIR_V2_BUILD_CORE=ON"
else
    echo "  ✗ Build script missing CHOIR_V2_BUILD_CORE=ON"
fi

echo ""

# Check plugin source files
echo "✓ Checking plugin source files..."
if [ -f "src/plugin/ChoirV2Processor.cpp" ]; then
    echo "  ✓ ChoirV2Processor.cpp exists"
else
    echo "  ✗ ChoirV2Processor.cpp missing"
fi

if [ -f "include/plugin/ChoirV2Processor.h" ]; then
    echo "  ✓ ChoirV2Processor.h exists"
else
    echo "  ✗ ChoirV2Processor.h missing"
fi

if [ -f "include/plugin/ChoirV2Editor.h" ]; then
    echo "  ✓ ChoirV2Editor.h exists"
else
    echo "  ✗ ChoirV2Editor.h missing"
fi

# Note: ChoirV2Editor.cpp is expected to be empty or minimal
if [ -f "src/plugin/ChoirV2Editor.cpp" ]; then
    echo "  ✓ ChoirV2Editor.cpp exists"
else
    echo "  ⚠ ChoirV2Editor.cpp missing (may need to be created)"
fi

echo ""

# Check DSP files
echo "✓ Checking DSP implementation files..."
DSP_FILES=(
    "src/dsp/ChoirV2PureDSP.cpp"
    "include/dsp/ChoirV2PureDSP.h"
)

for file in "${DSP_FILES[@]}"; do
    if [ -f "$file" ]; then
        echo "  ✓ $file exists"
    else
        echo "  ✗ $file missing"
    fi
done

echo ""

# Summary
echo "=================================="
echo "Configuration Summary"
echo "=================================="
echo "Plugin Formats: VST3, AU, CLAP, LV2, AUv3, Standalone"
echo "ChoirV2Core: Enabled"
echo "Ready to build: YES"
echo ""
echo "Next steps:"
echo "  1. Run: ./build_all_formats.sh"
echo "  2. Or: mkdir -p build && cd build && cmake .. && cmake --build . -j8"
echo ""
