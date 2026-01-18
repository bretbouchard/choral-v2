#!/bin/bash

# Choir V2.0 - Build Script with JUCE Setup
# This script helps locate JUCE and build the plugins

set -e  # Exit on error

echo "=================================="
echo "Choir V2.0 - JUCE Setup & Build"
echo "=================================="
echo ""

# Try to find JUCE in various locations
JUCE_CANDIDATES=(
    "$HOME/JUCE"
    "/usr/local/JUCE"
    "/opt/JUCE"
    "$HOME/Applications/JUCE"
    "/Applications/JUCE"
    "../JUCE"
    "../../JUCE"
)

JUCE_PATH=""

for candidate in "${JUCE_CANDIDATES[@]}"; do
    if [ -d "$candidate" ]; then
        # Check if it's the actual JUCE directory (should contain CMakeLists.txt)
        if [ -f "$candidate/CMakeLists.txt" ]; then
            JUCE_PATH="$candidate"
            echo "✓ Found JUCE at: $JUCE_PATH"
            break
        fi
    fi
done

# If JUCE not found, offer to download it
if [ -z "$JUCE_PATH" ]; then
    echo "⚠️  JUCE not found in standard locations."
    echo ""
    echo "Would you like to download JUCE 8.0.0? (y/n)"
    read -r response

    if [[ "$response" =~ ^([yY][eE][sS]|[yY])$ ]]; then
        JUCE_VERSION="8.0.0"
        JUCE_DIR="JUCE-${JUCE_VERSION}"
        JUCE_ZIP="${JUCE_DIR}.zip"

        echo "Downloading JUCE ${JUCE_VERSION}..."
        curl -L -o "$JUCE_ZIP" "https://github.com/juce-framework/JUCE/releases/download/${JUCE_VERSION}/${JUCE_ZIP}"

        echo "Extracting..."
        unzip -q "$JUCE_ZIP"
        rm "$JUCE_ZIP"

        JUCE_PATH="$PWD/${JUCE_DIR}"
        echo "✓ JUCE downloaded to: $JUCE_PATH"
    else
        echo "❌ Cannot build without JUCE. Exiting."
        exit 1
    fi
fi

# Export JUCE_PATH for CMake
export JUCE_PATH

# Build
echo ""
echo "=================================="
echo "Building Choir V2 with JUCE"
echo "=================================="
echo ""

# Clean build directory
echo "Cleaning previous build..."
rm -rf build
mkdir build
cd build

# Configure CMake
echo "Configuring CMake..."
cmake .. \
    -DJUCE_PATH="$JUCE_PATH" \
    -DCHOIR_V2_BUILD_PLUGIN=ON \
    -DCHOIR_V2_PLUGIN_FORMATS="VST3;AU;CLAP;LV2;AUv3;Standalone" \
    -DCMAKE_BUILD_TYPE=Release

# Build
echo ""
echo "Building plugins..."
cmake --build . -j8

echo ""
echo "=================================="
echo "✓ Build complete!"
echo "=================================="
echo ""
echo "Plugins built in:"
echo "  VST3:    build/plugins/vst"
echo "  AU:      build/plugins/au"
echo "  CLAP:    build/plugins/clap"
echo "  LV2:     build/plugins/lv2"
echo "  AUv3:    build/plugins/auv3"
echo "  Standalone: build/plugins/standalone"
echo ""
