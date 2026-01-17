#!/bin/bash

# Choir V2.0 AUv3 (iOS) Build Script
# Builds AUv3 plugin for iOS devices

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
IOS_DEPLOYMENT_TARGET="12.0"

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
        --ios-version)
            IOS_DEPLOYMENT_TARGET="$2"
            shift 2
            ;;
        --help)
            echo "Choir V2.0 AUv3 (iOS) Build Script"
            echo ""
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --debug        Build Debug configuration (default: Release)"
            echo "  --release      Build Release configuration"
            echo "  --clean        Clean build directory before building"
            echo "  --verbose      Show verbose build output"
            echo "  --ios-version  Set iOS deployment target (default: 12.0)"
            echo "  --help         Show this help message"
            echo ""
            echo "Environment Variables:"
            echo "  JUCE_PATH      Path to JUCE framework (required)"
            echo ""
            echo "Output:"
            echo "  iOS:           .artifacts/ios/auv3/ChoirV2.auv3/"
            echo ""
            echo "Requirements:"
            echo "  - macOS with Xcode"
            echo "  - iOS SDK (installed with Xcode)"
            echo "  - Code signing certificate (for release builds)"
            echo ""
            echo "Note: AUv3 builds are macOS-only (but run on iOS devices)"
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Verify we're on macOS
if [[ "$OSTYPE" != "darwin"* ]]; then
    print_error "AUv3 builds require macOS"
    print_error "Current OS: $OSTYPE"
    exit 1
fi

print_step "Building Choir V2.0 AUv3 Plugin for iOS"
print_info "Build Type: $BUILD_TYPE"
print_info "iOS Deployment Target: $IOS_DEPLOYMENT_TARGET"

# Check for Xcode
if ! command -v xcodebuild &> /dev/null; then
    print_error "Xcode not found. AUv3 builds require Xcode."
    exit 1
fi

print_info "Xcode: $(xcodebuild -version)"

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
BUILD_DIR="$PROJECT_ROOT/.build/xcode/auv3"
print_info "Build directory: $BUILD_DIR"

if [ "$CLEAN_BUILD" = true ]; then
    print_step "Cleaning build directory..."
    rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure CMake for iOS
print_step "Configuring CMake for iOS..."
CMAKE_ARGS=(
    -DCMAKE_SYSTEM_NAME=iOS
    -DCMAKE_OSX_DEPLOYMENT_TARGET="$IOS_DEPLOYMENT_TARGET"
    -DCMAKE_OSX_ARCHITECTURES=arm64
    -DCMAKE_OSX_SYSROOT=$(xcrun --sdk iphoneos --show-sdk-path)
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
    -DJUCE_PATH="$JUCE_PATH"
    -DCHOIR_V2_BUILD_PLUGIN=ON
    -DCHOIR_V2_PLUGIN_FORMATS="AUv3"
    -DJUCE_BUILD_AUv3_APP_EXTENSION=ON
)

if [ "$VERBOSE" = true ]; then
    CMAKE_ARGS+=(-DCMAKE_VERBOSE_MAKEFILE=ON)
fi

cmake "${CMAKE_ARGS[@]}" -S "$PROJECT_ROOT" -B "$BUILD_DIR"

# Build
print_step "Building AUv3 plugin for iOS..."
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

ARTIFACT_DIR="$PROJECT_ROOT/.artifacts/ios/auv3"
mkdir -p "$ARTIFACT_DIR"

# Find and copy AUv3 extension
AUv3_EXT=$(find "$BUILD_DIR" -name "*.auv3" -type d | head -1)

if [ -z "$AUv3_EXT" ]; then
    print_error "AUv3 extension not found in build directory"
    exit 1
fi

print_info "Found AUv3 extension: $AUv3_EXT"

# Copy to artifacts
cp -R "$AUv3_EXT" "$ARTIFACT_DIR/"

PLUGIN_NAME=$(basename "$AUv3_EXT")
OUTPUT_PATH="$ARTIFACT_DIR/$PLUGIN_NAME"

print_info "AUv3 Plugin: $OUTPUT_PATH"

# Verify architecture
print_step "Verifying architecture..."
lipo -info "$OUTPUT_PATH/ChoirV2" 2>/dev/null || file "$OUTPUT_PATH/ChoirV2"

print_step "AUv3 build complete!"
echo ""
print_info "Plugin location: $OUTPUT_PATH"
echo ""
print_warn "Note: AUv3 plugins must be installed via App Store or TestFlight"
print_warn "      Direct installation requires developer provisioning profile"
echo ""
print_info "To test on device:"
print_info "  1. Create an Xcode project with the AUv3 extension"
print_info "  2. Add your provisioning profile"
print_info "  3. Deploy to device via Xcode"
echo ""
print_info "For testing in GarageBand or AUM:"
print_info "  - Install the host app containing your AUv3 extension"
print_info "  - Launch the host app and scan for new plugins"
echo ""
print_info "For more information on AUv3 distribution:"
print_info "  https://developer.apple.com/documentation/audiounits"
