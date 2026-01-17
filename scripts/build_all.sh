#!/bin/bash

# Choir V2.0 Master Build Script
# Builds all 7 plugin formats: VST3, AU, AAX, CLAP, LV2, AUv3, Standalone

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
MAGENTA='\033[0;35m'
CYAN='\033[0;36m'
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

print_format() {
    echo -e "${MAGENTA}[FORMAT]${NC} $1"
}

print_success() {
    echo -e "${CYAN}[SUCCESS]${NC} $1"
}

# Script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

cd "$PROJECT_ROOT"

# Parse command line arguments
BUILD_TYPE="Release"
CLEAN_BUILD=false
FORMATS="all"
SKIP_FAILING=false
VERBOSE=false
SUMMARY_ONLY=false

# Build results tracking
declare -A BUILD_RESULTS
TOTAL_FORMATS=0
SUCCESSFUL_FORMATS=0
FAILED_FORMATS=0

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
        --vst3)
            FORMATS="vst3"
            shift
            ;;
        --au)
            FORMATS="au"
            shift
            ;;
        --aax)
            FORMATS="aax"
            shift
            ;;
        --clap)
            FORMATS="clap"
            shift
            ;;
        --lv2)
            FORMATS="lv2"
            shift
            ;;
        --auv3)
            FORMATS="auv3"
            shift
            ;;
        --standalone)
            FORMATS="standalone"
            shift
            ;;
        --all)
            FORMATS="all"
            shift
            ;;
        --skip-failing)
            SKIP_FAILING=true
            shift
            ;;
        --verbose)
            VERBOSE=true
            shift
            ;;
        --summary-only)
            SUMMARY_ONLY=true
            shift
            ;;
        --help)
            echo "Choir V2.0 Master Build Script"
            echo ""
            echo "Builds all 7 plugin formats for Choir V2.0"
            echo ""
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Format Selection:"
            echo "  --all          Build all formats (default)"
            echo "  --vst3         Build VST3 only"
            echo "  --au           Build AU only"
            echo "  --aax          Build AAX only"
            echo "  --clap         Build CLAP only"
            echo "  --lv2          Build LV2 only"
            echo "  --auv3         Build AUv3 (iOS) only"
            echo "  --standalone   Build Standalone app only"
            echo ""
            echo "Build Options:"
            echo "  --debug        Build Debug configuration (default: Release)"
            echo "  --release      Build Release configuration"
            echo "  --clean        Clean build directories before building"
            echo "  --skip-failing Continue build even if a format fails"
            echo "  --verbose      Show verbose build output"
            echo "  --summary-only Only show summary, hide individual build output"
            echo "  --help         Show this help message"
            echo ""
            echo "Environment Variables:"
            echo "  JUCE_PATH      Path to JUCE framework (required)"
            echo "  AAX_SDK        Path to AAX SDK (required for AAX builds)"
            echo ""
            echo "Output Locations:"
            echo "  macOS:         .artifacts/macos/{format}/"
            echo "  Windows:       .artifacts/windows/{format}/"
            echo "  Linux:         .artifacts/linux/{format}/"
            echo "  iOS:           .artifacts/ios/auv3/"
            echo ""
            echo "Examples:"
            echo "  # Build all formats (Release)"
            echo "  $0"
            echo ""
            echo "  # Build all formats (Debug)"
            echo "  $0 --debug"
            echo ""
            echo "  # Build specific formats"
            echo "  $0 --vst3 --au --clap"
            echo ""
            echo "  # Clean build all formats"
            echo "  $0 --clean --all"
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
    print_error "Linux builds are not fully supported yet"
    print_error "Most formats will build, but some may fail"
    OS="linux"
else
    print_error "Unsupported OS: $OSTYPE"
    exit 1
fi

# Print header
echo ""
echo -e "${MAGENTA}╔════════════════════════════════════════════════════════════════╗${NC}"
echo -e "${MAGENTA}║${NC}        ${CYAN}Choir V2.0 - All Format Build Script${NC}                ${MAGENTA}║${NC}"
echo -e "${MAGENTA}╚════════════════════════════════════════════════════════════════╝${NC}"
echo ""
print_info "OS: $OS"
print_info "Build Type: $BUILD_TYPE"
if [ "$CLEAN_BUILD" = true ]; then
    print_info "Clean Build: Yes"
fi
if [ "$SKIP_FAILING" = true ]; then
    print_info "Skip Failing: Yes"
fi
echo ""

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

# Determine which formats to build
FORMATS_TO_BUILD=()

if [ "$FORMATS" = "all" ]; then
    FORMATS_TO_BUILD=("vst3" "au" "clap" "lv2" "standalone")

    # Add AAX if SDK is available
    if [ ! -z "$AAX_SDK" ] && [ -d "$AAX_SDK" ]; then
        FORMATS_TO_BUILD+=("aax")
    else
        print_warn "AAX SDK not found. Skipping AAX build."
    fi

    # Add AUv3 on macOS
    if [[ "$OS" == "macos" ]]; then
        FORMATS_TO_BUILD+=("auv3")
    fi
else
    # Single format selected
    FORMATS_TO_BUILD=("$FORMATS")
fi

TOTAL_FORMATS=${#FORMATS_TO_BUILD[@]}

print_step "Building $TOTAL_FORMATS format(s): ${FORMATS_TO_BUILD[*]}"
echo ""

# Function to build a format
build_format() {
    local format=$1
    local script="$SCRIPT_DIR/build_${format}.sh"

    if [ ! -f "$script" ]; then
        print_error "Build script not found: $script"
        return 1
    fi

    print_format "Building $format..."

    # Build command args
    local args=()
    if [ "$BUILD_TYPE" = "Debug" ]; then
        args+=("--debug")
    else
        args+=("--release")
    fi

    if [ "$CLEAN_BUILD" = true ]; then
        args+=("--clean")
    fi

    if [ "$VERBOSE" = true ]; then
        args+=("--verbose")
    fi

    # Run build script
    if [ "$SUMMARY_ONLY" = true ]; then
        # Suppress output, only show errors
        if "$script" "${args[@]}" > /dev/null 2>&1; then
            print_success "$format: SUCCESS"
            BUILD_RESULTS[$format]="SUCCESS"
            ((SUCCESSFUL_FORMATS++))
            return 0
        else
            print_error "$format: FAILED"
            BUILD_RESULTS[$format]="FAILED"
            ((FAILED_FORMATS++))
            return 1
        fi
    else
        # Show output
        if "$script" "${args[@]}"; then
            print_success "$format: SUCCESS"
            BUILD_RESULTS[$format]="SUCCESS"
            ((SUCCESSFUL_FORMATS++))
            return 0
        else
            print_error "$format: FAILED"
            BUILD_RESULTS[$format]="FAILED"
            ((FAILED_FORMATS++))
            return 1
        fi
    fi
}

# Build each format
for format in "${FORMATS_TO_BUILD[@]}"; do
    if build_format "$format"; then
        # Success
        :
    else
        # Failure
        if [ "$SKIP_FAILING" = false ]; then
            print_error "Build failed for $format. Use --skip-failing to continue."
            exit 1
        fi
    fi
    echo ""
done

# Print summary
echo ""
echo -e "${MAGENTA}╔════════════════════════════════════════════════════════════════╗${NC}"
echo -e "${MAGENTA}║${NC}                    ${CYAN}Build Summary${NC}                             ${MAGENTA}║${NC}"
echo -e "${MAGENTA}╚════════════════════════════════════════════════════════════════╝${NC}"
echo ""
print_info "Total Formats: $TOTAL_FORMATS"
print_success "Successful: $SUCCESSFUL_FORMATS"

if [ $FAILED_FORMATS -gt 0 ]; then
    print_error "Failed: $FAILED_FORMATS"
fi

echo ""
print_info "Results by Format:"
for format in "${FORMATS_TO_BUILD[@]}"; do
    result="${BUILD_RESULTS[$format]:-UNKNOWN}"
    if [ "$result" = "SUCCESS" ]; then
        echo -e "  ${GREEN}✓${NC} $format: $result"
    else
        echo -e "  ${RED}✗${NC} $format: $result"
    fi
done

echo ""
if [ $SUCCESSFUL_FORMATS -eq $TOTAL_FORMATS ]; then
    print_success "All builds completed successfully!"
    echo ""
    print_info "Artifacts location: $PROJECT_ROOT/.artifacts/"
    exit 0
else
    print_error "Some builds failed. Check the output above for details."
    exit 1
fi
