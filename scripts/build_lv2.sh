#!/bin/bash

# Choir V2.0 LV2 Build Script
# Builds LV2 plugin for Linux and macOS

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Print functions
print_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_step() {
    echo -e "${BLUE}[STEP]${NC} $1"
}

# Script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

cd "$PROJECT_ROOT"

# Parse command line arguments
BUILD_TYPE="Release"
CLEAN_BUILD=false
VERBOSE=false

while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --release)
            BUILD_TYPE="Release"
            shift
            ;;
        --clean)
            CLEAN_BUILD=true
            shift
            ;;
        --verbose)
            VERBOSE=true
            shift
            ;;
        --help)
            echo "Choir V2.0 LV2 Build Script"
            echo ""
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --debug        Build Debug configuration (default: Release)"
            echo "  --release      Build Release configuration"
            echo "  --clean        Clean build directory before building"
            echo "  --verbose      Show verbose build output"
            echo "  --help         Show this help message"
            echo ""
            echo "Environment Variables:"
            echo "  JUCE_PATH      Path to JUCE framework (required)"
            echo ""
            echo "Output:"
            echo "  Linux:         .artifacts/linux/lv2/ChoirV2.lv2"
            echo "  macOS:         .artifacts/macos/lv2/ChoirV2.lv2"
            echo ""
            echo "Note: LV2 is primarily a Linux format"
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Detect OS
if [[ "$OSTYPE" == "darwin"* ]]; then
    OS="macos"
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    OS="linux"
else
    print_error "LV2 builds supported on Linux and macOS only"
    print_error "Current OS: $OSTYPE"
    exit 1
fi

print_step "Building Choir V2.0 LV2 Plugin"
print_info "OS: $OS"
print_info "Build Type: $BUILD_TYPE"

# Check if JUCE_PATH is set
if [ -z "$JUCE_PATH" ] && [ ! -z "$JUCE_DIR" ]; then
    export JUCE_PATH="$JUCE_DIR"
fi

if [ -z "$JUCE_PATH" ]; then
    print_warn "JUCE_PATH not set. Trying common locations..."

    if [ -d "$HOME/JUCE" ]; then
        export JUCE_PATH="$HOME/JUCE"
        print_info "Found JUCE at $JUCE_PATH"
    elif [ -d "/usr/local/JUCE" ]; then
        export JUCE_PATH="/usr/local/JUCE"
        print_info "Found JUCE at $JUCE_PATH"
    elif [ -d "/opt/JUCE" ]; then
        export JUCE_PATH="/opt/JUCE"
        print_info "Found JUCE at $JUCE_PATH"
    else
        print_error "JUCE not found. Please set JUCE_PATH environment variable."
        print_error "Example: export JUCE_PATH=/path/to/JUCE"
        exit 1
    fi
fi

if [ ! -d "$JUCE_PATH" ]; then
    print_error "JUCE directory not found at $JUCE_PATH"
    exit 1
fi

# Create build directory
BUILD_DIR="$PROJECT_ROOT/.build/cmake/lv2"
print_info "Build directory: $BUILD_DIR"

if [ "$CLEAN_BUILD" = true ]; then
    print_step "Cleaning build directory..."
    rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure CMake
print_step "Configuring CMake..."
CMAKE_ARGS=(
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
    -DJUCE_PATH="$JUCE_PATH"
    -DCHOIR_V2_BUILD_PLUGIN=ON
    -DCHOIR_V2_PLUGIN_FORMATS="LV2"
)

if [ "$VERBOSE" = true ]; then
    CMAKE_ARGS+=(-DCMAKE_VERBOSE_MAKEFILE=ON)
fi

cmake "${CMAKE_ARGS[@]}" -S "$PROJECT_ROOT" -B "$BUILD_DIR"

# Build
print_step "Building LV2 plugin..."
BUILD_ARGS=(--config "$BUILD_TYPE")

if [ "$VERBOSE" = true ]; then
    BUILD_ARGS+=(--verbose)
fi

# Use all available CPU cores
if [[ "$OS" == "macos" ]]; then
    CORES=$(sysctl -n hw.ncpu)
else
    CORES=${NUMBER_OF_PROCESSORS:-$(nproc 2>/dev/null || echo 4)}
fi

BUILD_ARGS+=(-j "$CORES")

cmake --build "$BUILD_DIR" "${BUILD_ARGS[@]}"

# Check build result
if [ $? -ne 0 ]; then
    print_error "Build failed!"
    exit 1
fi

print_step "Build successful!"

# Copy artifacts to output directory
print_step "Copying artifacts..."

ARTIFACT_DIR="$PROJECT_ROOT/.artifacts/$OS/lv2"
mkdir -p "$ARTIFACT_DIR"

# Find and copy LV2 plugin bundle
LV2_BUNDLE=$(find "$BUILD_DIR" -name "*.lv2" -type d | head -1)

if [ -z "$LV2_BUNDLE" ]; then
    print_error "LV2 bundle not found in build directory"
    exit 1
fi

print_info "Found LV2 bundle: $LV2_BUNDLE"

# Copy to artifacts
cp -R "$LV2_BUNDLE" "$ARTIFACT_DIR/"

PLUGIN_NAME=$(basename "$LV2_BUNDLE")
OUTPUT_PATH="$ARTIFACT_DIR/$PLUGIN_NAME"

print_info "LV2 Plugin: $OUTPUT_PATH"

# Verify architecture
if [[ "$OS" == "macos" ]]; then
    print_step "Verifying architecture..."
    lipo -info "$OUTPUT_PATH/ChoirV2.so" 2>/dev/null || file "$OUTPUT_PATH/ChoirV2.so"
elif [[ "$OS" == "linux" ]]; then
    print_step "Verifying architecture..."
    file "$OUTPUT_PATH/ChoirV2.so"
fi

print_step "LV2 build complete!"
echo ""
print_info "Plugin location: $OUTPUT_PATH"
echo ""
print_info "To install:"
if [[ "$OS" == "macos" ]]; then
    print_info "  cp -R \"$OUTPUT_PATH\" \"\$HOME/Library/Audio/Plug-Ins/LV2/\""
elif [[ "$OS" == "linux" ]]; then
    print_info "  cp -R \"$OUTPUT_PATH\" ~/.lv2/"
    print_info "  # System-wide:"
    print_info "  sudo cp -R \"$OUTPUT_PATH\" /usr/lib/lv2/"
fi
echo ""
print_info "To test in LV2 plugin host:"
if [[ "$OS" == "linux" ]]; then
    print_info "  jalv.gtk \"$OUTPUT_PATH\""
    print_info "  # or:"
    print_info "  lv2-validate \"$OUTPUT_PATH\""
fi
