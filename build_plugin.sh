#!/bin/bash

# Choir V2.0 Plugin Build Script
# Builds JUCE plugin for all formats

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
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

# Check if JUCE_PATH is set
if [ -z "$JUCE_PATH" ] && [ ! -z "$JUCE_DIR" ]; then
    export JUCE_PATH="$JUCE_DIR"
fi

if [ -z "$JUCE_PATH" ]; then
    print_warn "JUCE_PATH not set. Trying common locations..."

    # Try common JUCE locations
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

# Check if JUCE directory exists
if [ ! -d "$JUCE_PATH" ]; then
    print_error "JUCE directory not found at $JUCE_PATH"
    exit 1
fi

# Script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Parse command line arguments
BUILD_TYPE="Release"
PLUGINS_TO_BUILD="VST3;AU;CLAP;LV2;Standalone"
CLEAN_BUILD=false
INSTALL=false

while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --vst3)
            PLUGINS_TO_BUILD="VST3"
            shift
            ;;
        --au)
            PLUGINS_TO_BUILD="AU"
            shift
            ;;
        --clap)
            PLUGINS_TO_BUILD="CLAP"
            shift
            ;;
        --lv2)
            PLUGINS_TO_BUILD="LV2"
            shift
            ;;
        --standalone)
            PLUGINS_TO_BUILD="Standalone"
            shift
            ;;
        --all)
            PLUGINS_TO_BUILD="VST3;AU;CLAP;LV2;Standalone"
            shift
            ;;
        --clean)
            CLEAN_BUILD=true
            shift
            ;;
        --install)
            INSTALL=true
            shift
            ;;
        --help)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --debug        Build Debug configuration (default: Release)"
            echo "  --vst3         Build VST3 plugin only"
            echo "  --au           Build AU plugin only"
            echo "  --clap         Build CLAP plugin only"
            echo "  --lv2          Build LV2 plugin only"
            echo "  --standalone   Build Standalone app only"
            echo "  --all          Build all plugin formats (default)"
            echo "  --clean        Clean build directory before building"
            echo "  --install      Install plugins after building"
            echo "  --help         Show this help message"
            echo ""
            echo "Environment Variables:"
            echo "  JUCE_PATH      Path to JUCE framework (required)"
            echo "  JUCE_DIR       Alternative to JUCE_PATH"
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Create build directory
BUILD_DIR="$SCRIPT_DIR/build"
print_info "Build directory: $BUILD_DIR"

if [ "$CLEAN_BUILD" = true ]; then
    print_info "Cleaning build directory..."
    rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure with CMake
print_info "Configuring CMake..."
cmake -DCHOIR_V2_BUILD_PLUGIN=ON \
      -DJUCE_PATH="$JUCE_PATH" \
      -DCHOIR_V2_PLUGIN_FORMATS="$PLUGINS_TO_BUILD" \
      -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
      ..

# Build
print_info "Building plugins (Configuration: $BUILD_TYPE)..."
print_info "Formats: $PLUGINS_TO_BUILD"
cmake --build . --config "$BUILD_TYPE" -j$(sysctl -n hw.ncpu 2>/dev/null || nproc 2>/dev/null || echo 4)

# Check build result
if [ $? -eq 0 ]; then
    print_info "Build successful!"
    echo ""
    print_info "Plugin locations:"

    if [[ "$PLUGINS_TO_BUILD" == *"VST3"* ]]; then
        echo "  VST3:      $BUILD_DIR/plugins/vst/"
    fi

    if [[ "$PLUGINS_TO_BUILD" == *"AU"* ]]; then
        echo "  AU:        $BUILD_DIR/plugins/au/"
    fi

    if [[ "$PLUGINS_TO_BUILD" == *"CLAP"* ]]; then
        echo "  CLAP:      $BUILD_DIR/plugins/clap/"
    fi

    if [[ "$PLUGINS_TO_BUILD" == *"LV2"* ]]; then
        echo "  LV2:       $BUILD_DIR/plugins/lv2/"
    fi

    if [[ "$PLUGINS_TO_BUILD" == *"Standalone"* ]]; then
        echo "  Standalone: $BUILD_DIR/plugins/standalone/"
    fi

    # Install if requested
    if [ "$INSTALL" = true ]; then
        print_info "Installing plugins..."

        # Detect OS
        if [[ "$OSTYPE" == "darwin"* ]]; then
            # macOS
            INSTALL_BASE="$HOME/Library/Audio/Plug-Ins"

            if [[ "$PLUGINS_TO_BUILD" == *"VST3"* ]] && [ -d "plugins/vst" ]; then
                print_info "Installing VST3 plugin..."
                mkdir -p "$INSTALL_BASE/VST3"
                cp -R plugins/vst/*.vst3 "$INSTALL_BASE/VST3/"
            fi

            if [[ "$PLUGINS_TO_BUILD" == *"AU"* ]] && [ -d "plugins/au" ]; then
                print_info "Installing AU plugin..."
                mkdir -p "$INSTALL_BASE/Components"
                cp -R plugins/au/*.component "$INSTALL_BASE/Components/"
            fi

            if [[ "$PLUGINS_TO_BUILD" == *"CLAP"* ]] && [ -d "plugins/clap" ]; then
                print_info "Installing CLAP plugin..."
                mkdir -p "$INSTALL_BASE/CLAP"
                cp -R plugins/clap/*.clap "$INSTALL_BASE/CLAP/"
            fi

            if [[ "$PLUGINS_TO_BUILD" == *"Standalone"* ]] && [ -d "plugins/standalone" ]; then
                print_info "Installing standalone app..."
                cp -R plugins/standalone/*.app /Applications/
            fi

        elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
            # Linux
            INSTALL_BASE="$HOME/.lv2"

            if [[ "$PLUGINS_TO_BUILD" == *"VST3"* ]] && [ -d "plugins/vst" ]; then
                print_info "Installing VST3 plugin..."
                mkdir -p "$HOME/.vst3"
                cp -R plugins/vst/*.vst3 "$HOME/.vst3/"
            fi

            if [[ "$PLUGINS_TO_BUILD" == *"CLAP"* ]] && [ -d "plugins/clap" ]; then
                print_info "Installing CLAP plugin..."
                mkdir -p "$HOME/.clap"
                cp -R plugins/clap/*.clap "$HOME/.clap/"
            fi

            if [[ "$PLUGINS_TO_BUILD" == *"LV2"* ]] && [ -d "plugins/lv2" ]; then
                print_info "Installing LV2 plugin..."
                mkdir -p "$INSTALL_BASE"
                cp -R plugins/lv2/*.lv2 "$INSTALL_BASE/"
            fi

            if [[ "$PLUGINS_TO_BUILD" == *"Standalone"* ]] && [ -d "plugins/standalone" ]; then
                print_info "Installing standalone app..."
                mkdir -p "$HOME/bin"
                cp plugins/standalone/* "$HOME/bin/"
            fi

        elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "win32" ]]; then
            # Windows
            print_warn "Please install manually on Windows:"
            print_warn "  VST3:      C:\\Program Files\\Common Files\\VST3\\"
            print_warn "  CLAP:      C:\\Program Files\\Common Files\\CLAP\\"
            print_warn "  Standalone: C:\\Program Files\\ChoirV2\\"
        fi

        print_info "Installation complete!"
    fi

    echo ""
    print_info "To install manually, see README_PLUGIN.md"
else
    print_error "Build failed!"
    exit 1
fi
