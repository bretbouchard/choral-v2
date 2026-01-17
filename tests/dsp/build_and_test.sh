#!/bin/bash

# Build script for SpectralEnhancer tests

set -e  # Exit on error

echo "=================================================="
echo "Building SpectralEnhancer Tests"
echo "=================================================="

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check JUCE_PATH
if [ -z "$JUCE_PATH" ]; then
    echo -e "${RED}Error: JUCE_PATH environment variable not set${NC}"
    echo "Please set JUCE_PATH to point to your JUCE installation"
    echo "Example: export JUCE_PATH=/path/to/JUCE"
    exit 1
fi

if [ ! -d "$JUCE_PATH" ]; then
    echo -e "${RED}Error: JUCE_PATH directory does not exist: $JUCE_PATH${NC}"
    exit 1
fi

echo -e "${GREEN}JUCE_PATH: $JUCE_PATH${NC}"

# Create build directory
BUILD_DIR=".build/cmake/spectral_enhancer_tests"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo ""
echo "Configuring CMake..."
cmake -DCMAKE_BUILD_TYPE=Release \
      -DJUCE_PATH="$JUCE_PATH" \
      ../..

echo ""
echo "Building tests..."
cmake --build . --config Release -- -j$(sysctl -n hw.ncpu 2>/dev/null || echo 4)

echo ""
echo "=================================================="
echo "Running Tests"
echo "=================================================="

if [ -f "./SpectralEnhancerTests" ]; then
    echo -e "${GREEN}Running SpectralEnhancerTests...${NC}"
    ./SpectralEnhancerTests --gtest_color=yes

    TEST_EXIT_CODE=$?

    if [ $TEST_EXIT_CODE -eq 0 ]; then
        echo ""
        echo -e "${GREEN}=================================================="
        echo "All tests passed! ✓"
        echo "==================================================${NC}"
    else
        echo ""
        echo -e "${RED}=================================================="
        echo "Some tests failed! ✗"
        echo "==================================================${NC}"
        exit $TEST_EXIT_CODE
    fi
else
    echo -e "${RED}Error: Test executable not found${NC}"
    exit 1
fi

cd - > /dev/null
