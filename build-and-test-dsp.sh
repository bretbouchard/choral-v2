#!/bin/bash

# Choir V2 DSP Build and Test Script
# Builds and tests the PureDSP library without requiring GUI components

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}======================================"
echo "Choir V2.0 DSP Build & Test"
echo -e "======================================${NC}"
echo ""

# Script directory (in choral_v2/)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# Project root is the same as script dir (where CMakeLists.txt is)
PROJECT_ROOT="$SCRIPT_DIR"

# Build directory
BUILD_DIR="${PROJECT_ROOT}/.build/cmake/choral_v2_dsp"

# Clean previous builds
echo -e "${YELLOW}[1/4] Cleaning previous builds...${NC}"
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

# Configure CMake (DSP-only, no plugin)
echo -e "${YELLOW}[2/4] Configuring CMake (DSP-only)...${NC}"
cd "$BUILD_DIR"

# Find JUCE (check multiple locations)
JUCE_LOCATIONS=(
    "/Users/bretbouchard/apps/schill/white_room/external/JUCE"
    "$HOME/JUCE"
    "/usr/local/JUCE"
    "/opt/JUCE"
)

JUCE_FOUND=""
for location in "${JUCE_LOCATIONS[@]}"; do
    if [ -f "$location/CMakeLists.txt" ]; then
        JUCE_FOUND="$location"
        break
    fi
done

if [ -z "$JUCE_FOUND" ]; then
    echo -e "${RED}ERROR: JUCE not found!${NC}"
    echo "Please install JUCE to one of:"
    printf '  - %s\n' "${JUCE_LOCATIONS[@]}"
    exit 1
fi

echo -e "${BLUE}Found JUCE at: $JUCE_FOUND${NC}"

cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCHOIR_V2_BUILD_TESTS=ON \
    -DCHOIR_V2_BUILD_PLUGIN=OFF \
    -DCHOIR_V2_BUILD_CORE=OFF \
    -DJUCE_PATH="$JUCE_FOUND" \
    "$PROJECT_ROOT"

# Build PureDSP library and test harness
echo -e "${YELLOW}[3/4] Building PureDSP library and test harness...${NC}"
make -j8 ChoirV2PureDSP
make -j8 dsp_test_harness

# Run tests
echo -e "${YELLOW}[4/4] Running DSP tests...${NC}"
echo ""
./tests/dsp_test_harness --verbose

echo ""
echo -e "${GREEN}======================================"
echo "Build & Test Complete!"
echo -e "======================================${NC}"
echo ""
echo "Test harness binary: ${BUILD_DIR}/tests/dsp_test_harness"
echo "PureDSP library: ${BUILD_DIR}/libChoirV2PureDSP.a"
echo ""
echo "To run tests manually:"
echo "  cd ${BUILD_DIR}/tests"
echo "  ./dsp_test_harness"
echo "  ./dsp_test_harness --module=biquad"
echo "  ./dsp_test_harness --help"
echo ""
