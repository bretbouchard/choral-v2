#!/bin/bash

# Choir V2.0 Standalone App Build Script
# Builds standalone application for macOS and Windows

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
SIGN_APP=false
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
            SIGN_APP=true
            shift
            ;;
        --verbose)
            VERBOSE=true
            shift
            ;;
        --help)
            echo "Choir V2.0 Standalone App Build Script"
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
            echo "  --sign         Code sign the app (macOS only)"
            echo "  --verbose      Show verbose build output"
            echo "  --help         Show this help message"
            echo ""
            echo "Environment Variables:"
            echo "  JUCE_PATH      Path to JUCE framework (required)"
            echo ""
            echo "Output:"
            echo "  macOS:         .artifacts/macos/standalone/ChoirV2.app"
            echo "  Windows:       .artifacts/windows/standalone/ChoirV2.exe"
            echo "  Linux:         .artifacts/linux/standalone/ChoirV2"
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
elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "win32" ]]; then
    OS="windows"
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    OS="linux"
else
    print_error "Unsupported OS: $OSTYPE"
    exit 1
fi

print_step "Building Choir V2.0 Standalone App"
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
BUILD_DIR="$PROJECT_ROOT/.build/cmake/standalone"
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
    -DCHOIR_V2_PLUGIN_FORMATS="Standalone"
)

if [ "$VERBOSE" = true ]; then
    CMAKE_ARGS+=(-DCMAKE_VERBOSE_MAKEFILE=ON)
fi

cmake "${CMAKE_ARGS[@]}" -S "$PROJECT_ROOT" -B "$BUILD_DIR"

# Build
print_step "Building standalone app..."
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

ARTIFACT_DIR="$PROJECT_ROOT/.artifacts/$OS/standalone"
mkdir -p "$ARTIFACT_DIR"

if [[ "$OS" == "macos" ]]; then
    # Find and copy .app bundle
    APP_BUNDLE=$(find "$BUILD_DIR" -name "*.app" -type d | head -1)

    if [ -z "$APP_BUNDLE" ]; then
        print_error "App bundle not found in build directory"
        exit 1
    fi

    print_info "Found app bundle: $APP_BUNDLE"

    # Copy to artifacts
    cp -R "$APP_BUNDLE" "$ARTIFACT_DIR/"

    # Get app name
    APP_NAME=$(basename "$APP_BUNDLE")
    OUTPUT_PATH="$ARTIFACT_DIR/$APP_NAME"

    # Code sign if requested
    if [ "$SIGN_APP" = true ]; then
        print_step "Code signing app..."
        codesign --force --deep --sign - "$OUTPUT_PATH"
        print_info "App signed with ad-hoc signature"
    fi

    print_info "Standalone App: $OUTPUT_PATH"

    # Verify architecture
    print_step "Verifying architecture..."
    lipo -info "$OUTPUT_PATH/Contents/MacOS/ChoirV2"

elif [[ "$OS" == "windows" ]]; then
    # Find and copy .exe
    APP_EXE=$(find "$BUILD_DIR" -name "*.exe" -type f | head -1)

    if [ -z "$APP_EXE" ]; then
        print_error "Executable not found in build directory"
        exit 1
    fi

    print_info "Found executable: $APP_EXE"

    # Copy to artifacts
    cp "$APP_EXE" "$ARTIFACT_DIR/"

    APP_NAME=$(basename "$APP_EXE")
    OUTPUT_PATH="$ARTIFACT_DIR/$APP_NAME"

    print_info "Standalone App: $OUTPUT_PATH"

    # Verify architecture
    print_step "Verifying architecture..."
    file "$OUTPUT_PATH"

elif [[ "$OS" == "linux" ]]; then
    # Find and copy executable
    APP_BIN=$(find "$BUILD_DIR" -type f -executable | head -1)

    if [ -z "$APP_BIN" ]; then
        print_error "Executable not found in build directory"
        exit 1
    fi

    print_info "Found executable: $APP_BIN"

    # Copy to artifacts
    cp "$APP_BIN" "$ARTIFACT_DIR/"

    APP_NAME=$(basename "$APP_BIN")
    OUTPUT_PATH="$ARTIFACT_DIR/$APP_NAME"

    print_info "Standalone App: $OUTPUT_PATH"

    # Verify architecture
    print_step "Verifying architecture..."
    file "$OUTPUT_PATH"
fi

print_step "Standalone build complete!"
echo ""
print_info "App location: $OUTPUT_PATH"
echo ""
print_info "To install:"
if [[ "$OS" == "macos" ]]; then
    print_info "  cp -R \"$OUTPUT_PATH\" /Applications/"
elif [[ "$OS" == "windows" ]]; then
    print_info "  Copy to: C:\\Program Files\\ChoirV2\\"
elif [[ "$OS" == "linux" ]]; then
    print_info "  cp \"$OUTPUT_PATH\" ~/bin/  # or /usr/local/bin/"
fi
echo ""
print_info "To run:"
if [[ "$OS" == "macos" ]]; then
    print_info "  open \"$OUTPUT_PATH\""
elif [[ "$OS" == "windows" ]]; then
    print_info "  \"$OUTPUT_PATH\""
elif [[ "$OS" == "linux" ]]; then
    print_info "  \"$OUTPUT_PATH\""
fi
