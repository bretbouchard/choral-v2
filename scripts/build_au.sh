#!/bin/bash

# Choir V2.0 AU (Audio Unit) Build Script
# Builds AU plugin for macOS (Intel + Apple Silicon)

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
ARCHS="x86_64;arm64"
SIGN_PLUGIN=false
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
        --intel)
            ARCHS="x86_64"
            shift
            ;;
        --arm)
            ARCHS="arm64"
            shift
            ;;
        --universal)
            ARCHS="x86_64;arm64"
            shift
            ;;
        --clean)
            CLEAN_BUILD=true
            shift
            ;;
        --sign)
            SIGN_PLUGIN=true
            shift
            ;;
        --verbose)
            VERBOSE=true
            shift
            ;;
        --help)
            echo "Choir V2.0 AU (Audio Unit) Build Script"
            echo ""
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --debug        Build Debug configuration (default: Release)"
            echo "  --release      Build Release configuration"
            echo "  --intel        Build Intel (x86_64) only"
            echo "  --arm          Build Apple Silicon (arm64) only"
            echo "  --universal    Build Universal Binary (Intel + Apple Silicon, default)"
            echo "  --clean        Clean build directory before building"
            echo "  --sign         Code sign the plugin"
            echo "  --verbose      Show verbose build output"
            echo "  --help         Show this help message"
            echo ""
            echo "Environment Variables:"
            echo "  JUCE_PATH      Path to JUCE framework (required)"
            echo "  DEVELOPER_DIR  Path to Xcode (for code signing)"
            echo ""
            echo "Output:"
            echo "  macOS:         .artifacts/macos/au/ChoirV2.component"
            echo ""
            echo "Note: AU plugins are macOS only"
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
else
    print_error "AU plugins are macOS only"
    print_error "Current OS: $OSTYPE"
    exit 1
fi

print_step "Building Choir V2.0 AU Plugin"
print_info "OS: $OS"
print_info "Build Type: $BUILD_TYPE"
print_info "Architecture: $ARCHS"

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
BUILD_DIR="$PROJECT_ROOT/.build/cmake/au"
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
    -DCHOIR_V2_PLUGIN_FORMATS="AU"
)

if [ "$VERBOSE" = true ]; then
    CMAKE_ARGS+=(-DCMAKE_VERBOSE_MAKEFILE=ON)
fi

cmake "${CMAKE_ARGS[@]}" -S "$PROJECT_ROOT" -B "$BUILD_DIR"

# Build
print_step "Building AU plugin..."
BUILD_ARGS=(--config "$BUILD_TYPE")

if [ "$VERBOSE" = true ]; then
    BUILD_ARGS+=(--verbose)
fi

# Use all available CPU cores
CORES=$(sysctl -n hw.ncpu)
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

ARTIFACT_DIR="$PROJECT_ROOT/.artifacts/macos/au"
mkdir -p "$ARTIFACT_DIR"

# Find and copy AU component
AU_COMPONENT=$(find "$BUILD_DIR" -name "*.component" -type d | head -1)

if [ -z "$AU_COMPONENT" ]; then
    print_error "AU component not found in build directory"
    exit 1
fi

print_info "Found AU component: $AU_COMPONENT"

# Copy to artifacts
cp -R "$AU_COMPONENT" "$ARTIFACT_DIR/"

# Get plugin name
PLUGIN_NAME=$(basename "$AU_COMPONENT")
OUTPUT_PATH="$ARTIFACT_DIR/$PLUGIN_NAME"

# Code sign if requested
if [ "$SIGN_PLUGIN" = true ]; then
    print_step "Code signing plugin..."
    codesign --force --deep --sign - "$OUTPUT_PATH"
    print_info "Plugin signed with ad-hoc signature"
fi

print_info "AU Plugin: $OUTPUT_PATH"

# Verify architecture
print_step "Verifying architecture..."
lipo -info "$OUTPUT_PATH/Contents/MacOS/ChoirV2"

print_step "AU build complete!"
echo ""
print_info "Plugin location: $OUTPUT_PATH"
echo ""
print_info "To install:"
print_info "  cp -R \"$OUTPUT_PATH\" \"/Library/Audio/Plug-Ins/Components/\""
print_info "  cp -R \"$OUTPUT_PATH\" \"$HOME/Library/Audio/Plug-Ins/Components/\""
echo ""
print_info "To test in AudioPluginVerifier:"
print_info "  /Applications/JUCE/examples/Plugins/AudioPluginVerifier.app"
echo ""
print_info "To test in AU Lab:"
print_info "  /Applications/AU Lab.app"
echo ""
print_warn "Note: AU plugins require component registration"
print_warn "      After installation, run 'sudo killall -9 AUComponentHelper'"
