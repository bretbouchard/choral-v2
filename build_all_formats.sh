#!/bin/bash

################################################################################
# Choir V2.0 - Comprehensive Plugin Build Script
# Builds all 6 plugin formats: VST3, AU, CLAP, LV2, AUv3, Standalone
################################################################################

set -e  # Exit on error
set -o pipefail  # Exit on pipe failure

################################################################################
# Configuration
################################################################################

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Directory configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="${SCRIPT_DIR}"
BUILD_DIR="${PROJECT_DIR}/build"
PLUGIN_DIR="${BUILD_DIR}/plugins"

# Plugin formats to build
PLUGIN_FORMATS="VST3;AU;CLAP;LV2;AUv3;Standalone"

# Build configuration
CMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE:-Release}"
BUILD_JOBS="${BUILD_JOBS:-8}"
VERBOSE_BUILD="${VERBOSE_BUILD:-0}"

################################################################################
# Functions
################################################################################

print_header() {
    echo ""
    echo "=================================="
    echo "$1"
    echo "=================================="
    echo ""
}

print_step() {
    echo -e "${BLUE}➜${NC} $1"
}

print_success() {
    echo -e "${GREEN}✓${NC} $1"
}

print_error() {
    echo -e "${RED}✗${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}⚠${NC} $1"
}

cleanup_build() {
    print_step "Cleaning previous build..."
    rm -rf "${BUILD_DIR}"
    mkdir -p "${BUILD_DIR}"
    print_success "Build directory cleaned"
}

configure_cmake() {
    print_step "Configuring CMake for ${CMAKE_BUILD_TYPE} build..."

    cd "${BUILD_DIR}"

    cmake_args=(
        "-DCHOIR_V2_BUILD_PLUGIN=ON"
        "-DCHOIR_V2_BUILD_CORE=ON"
        "-DCHOIR_V2_PLUGIN_FORMATS=${PLUGIN_FORMATS}"
        "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
    )

    # Add verbose flag if requested
    if [ "${VERBOSE_BUILD}" = "1" ]; then
        cmake_args+=("-DCMAKE_VERBOSE_MAKEFILE=ON")
    fi

    cmake .. "${cmake_args[@]}"

    print_success "CMake configuration complete"
}

build_plugins() {
    print_step "Building all plugin formats (${BUILD_JOBS} parallel jobs)..."

    cd "${BUILD_DIR}"

    if [ "${VERBOSE_BUILD}" = "1" ]; then
        cmake --build . -j"${BUILD_JOBS}" --verbose
    else
        cmake --build . -j"${BUILD_JOBS}"
    fi

    print_success "Build complete"
}

verify_vst3() {
    print_step "Verifying VST3 plugin..."

    local vst3_found=false
    local vst3_path=""

    # Check for macOS VST3 bundle
    if [ -d "${PLUGIN_DIR}/vst/Choir V2.0.vst3" ]; then
        vst3_path="${PLUGIN_DIR}/vst/Choir V2.0.vst3"

        # Check for binary inside bundle
        if [ -f "${PLUGIN_DIR}/vst/Choir V2.0.vst3/Contents/MacOS/Choir V2.0" ]; then
            vst3_found=true

            # Get file size
            local size=$(du -h "${PLUGIN_DIR}/vst/Choir V2.0.vst3/Contents/MacOS/Choir V2.0" | cut -f1)

            print_success "VST3 plugin built successfully"
            echo "  Location: ${vst3_path}"
            echo "  Binary: ${vst3_path}/Contents/MacOS/Choir V2.0"
            echo "  Size: ${size}"

            # Check for PkgInfo
            if [ -f "${PLUGIN_DIR}/vst/Choir V2.0.vst3/Contents/PkgInfo" ]; then
                echo "  PkgInfo: $(cat "${PLUGIN_DIR}/vst/Choir V2.0.vst3/Contents/PkgInfo")"
            fi

            return 0
        fi
    fi

    # Check for Linux VST3
    if [ -f "${PLUGIN_DIR}/vst/Choir V2.0.vst3/Choir V2.0.so" ]; then
        vst3_found=true
        print_success "VST3 plugin built successfully (Linux)"
        echo "  Location: ${PLUGIN_DIR}/vst/Choir V2.0.vst3/Choir V2.0.so"
        return 0
    fi

    # Check for Windows VST3
    if [ -f "${PLUGIN_DIR}/vst/Choir V2.0.vst3/Choir V2.0.vst3" ]; then
        vst3_found=true
        print_success "VST3 plugin built successfully (Windows)"
        echo "  Location: ${PLUGIN_DIR}/vst/Choir V2.0.vst3/Choir V2.0.vst3"
        return 0
    fi

    if [ "${vst3_found}" = false ]; then
        print_error "VST3 plugin NOT found"
        echo "  Expected location: ${PLUGIN_DIR}/vst/Choir V2.0.vst3"
        return 1
    fi
}

verify_au() {
    print_step "Verifying AU plugin..."

    if [ -d "${PLUGIN_DIR}/au/Choir V2.0.component" ]; then
        local component_path="${PLUGIN_DIR}/au/Choir V2.0.component"

        # Check for binary inside bundle
        if [ -f "${component_path}/Contents/MacOS/Choir V2.0" ]; then
            local size=$(du -h "${component_path}/Contents/MacOS/Choir V2.0" | cut -f1)

            print_success "AU plugin built successfully"
            echo "  Location: ${component_path}"
            echo "  Binary: ${component_path}/Contents/MacOS/Choir V2.0"
            echo "  Size: ${size}"

            # Check for Info.plist
            if [ -f "${component_path}/Contents/Info.plist" ]; then
                echo "  Info.plist: Present"
            fi

            return 0
        fi
    fi

    print_error "AU plugin NOT found"
    echo "  Expected location: ${PLUGIN_DIR}/au/Choir V2.0.component"
    return 1
}

verify_clap() {
    print_step "Verifying CLAP plugin..."

    if [ -f "${PLUGIN_DIR}/clap/Choir V2.0.clap" ]; then
        local size=$(du -h "${PLUGIN_DIR}/clap/Choir V2.0.clap" | cut -f1)

        print_success "CLAP plugin built successfully"
        echo "  Location: ${PLUGIN_DIR}/clap/Choir V2.0.clap"
        echo "  Size: ${size}"

        # Verify it's a valid shared library
        if file "${PLUGIN_DIR}/clap/Choir V2.0.clap" | grep -q "shared object"; then
            echo "  Type: Valid shared library"
        fi

        return 0
    fi

    print_error "CLAP plugin NOT found"
    echo "  Expected location: ${PLUGIN_DIR}/clap/Choir V2.0.clap"
    return 1
}

verify_lv2() {
    print_step "Verifying LV2 plugin..."

    if [ -d "${PLUGIN_DIR}/lv2/Choir V2.0.lv2" ]; then
        local lv2_path="${PLUGIN_DIR}/lv2/Choir V2.0.lv2"

        # Check for manifest.ttl
        if [ -f "${lv2_path}/manifest.ttl" ]; then
            print_success "LV2 plugin built successfully"
            echo "  Location: ${lv2_path}"

            # Count binaries
            local binary_count=$(find "${lv2_path}" -name "*.so" -o -name "*.dylib" -o -name "*.dll" | wc -l)
            echo "  Binaries: ${binary_count}"

            # Check for manifest
            echo "  Manifest: Present"

            # List plugin files
            local file_count=$(find "${lv2_path}" -type f | wc -l)
            echo "  Total files: ${file_count}"

            return 0
        fi
    fi

    print_error "LV2 plugin NOT found"
    echo "  Expected location: ${PLUGIN_DIR}/lv2/Choir V2.0.lv2"
    return 1
}

verify_auv3() {
    print_step "Verifying AUv3 plugin..."

    if [ -f "${PLUGIN_DIR}/auv/Choir V2.0.appex/Contents/MacOS/Choir V2.0" ]; then
        local appex_path="${PLUGIN_DIR}/auv/Choir V2.0.appex"
        local size=$(du -h "${appex_path}/Contents/MacOS/Choir V2.0" | cut -f1)

        print_success "AUv3 plugin built successfully"
        echo "  Location: ${appex_path}"
        echo "  Binary: ${appex_path}/Contents/MacOS/Choir V2.0"
        echo "  Size: ${size}"

        # Check for Info.plist
        if [ -f "${appex_path}/Contents/Info.plist" ]; then
            echo "  Info.plist: Present"
        fi

        return 0
    fi

    print_error "AUv3 plugin NOT found"
    echo "  Expected location: ${PLUGIN_DIR}/auv/Choir V2.0.appex"
    return 1
}

verify_standalone() {
    print_step "Verifying Standalone app..."

    local standalone_found=false

    # Check for macOS app bundle
    if [ -f "${PLUGIN_DIR}/standalone/Choir V2.0.app/Contents/MacOS/Choir V2.0" ]; then
        local app_path="${PLUGIN_DIR}/standalone/Choir V2.0.app"
        local size=$(du -h "${app_path}/Contents/MacOS/Choir V2.0" | cut -f1)

        standalone_found=true
        print_success "Standalone app built successfully"
        echo "  Location: ${app_path}"
        echo "  Binary: ${app_path}/Contents/MacOS/Choir V2.0"
        echo "  Size: ${size}"

        # Check for Info.plist
        if [ -f "${app_path}/Contents/Info.plist" ]; then
            echo "  Info.plist: Present"
        fi

        return 0
    fi

    # Check for Linux binary
    if [ -f "${PLUGIN_DIR}/standalone/Choir V2.0" ]; then
        standalone_found=true
        local size=$(du -h "${PLUGIN_DIR}/standalone/Choir V2.0" | cut -f1)

        print_success "Standalone app built successfully (Linux)"
        echo "  Location: ${PLUGIN_DIR}/standalone/Choir V2.0"
        echo "  Size: ${size}"

        return 0
    fi

    # Check for Windows executable
    if [ -f "${PLUGIN_DIR}/standalone/Choir V2.0.exe" ]; then
        standalone_found=true
        print_success "Standalone app built successfully (Windows)"
        echo "  Location: ${PLUGIN_DIR}/standalone/Choir V2.0.exe"
        return 0
    fi

    if [ "${standalone_found}" = false ]; then
        print_error "Standalone app NOT found"
        echo "  Expected location: ${PLUGIN_DIR}/standalone/"
        return 1
    fi
}

verify_build() {
    print_header "Build Verification"

    local success_count=0
    local total_count=6

    # Verify each format
    verify_vst3 && ((success_count++)) || true
    verify_au && ((success_count++)) || true
    verify_clap && ((success_count++)) || true
    verify_lv2 && ((success_count++)) || true
    verify_auv3 && ((success_count++)) || true
    verify_standalone && ((success_count++)) || true

    echo ""
    print_header "Verification Summary"
    echo "Successfully built: ${success_count}/${total_count} formats"

    if [ ${success_count} -eq ${total_count} ]; then
        print_success "All plugin formats built successfully!"
        return 0
    else
        print_warning "Some formats failed to build"
        return 1
    fi
}

print_summary() {
    print_header "Build Complete!"

    echo "Plugin files are located in: ${PLUGIN_DIR}"
    echo ""
    echo "Next steps:"
    echo "  1. Test plugins in your DAW"
    echo "  2. Run verification script: ./verify_plugins.sh"
    echo "  3. Install system-wide (optional):"
    echo "     - macOS VST3: cp -R \"${PLUGIN_DIR}/vst/Choir V2.0.vst3\" /Library/Audio/Plug-Ins/VST3/"
    echo "     - macOS AU: cp -R \"${PLUGIN_DIR}/au/Choir V2.0.component\" /Library/Audio/Plug-Ins/Components/"
    echo ""
}

################################################################################
# Main Build Process
################################################################################

main() {
    print_header "Choir V2.0 - Build All Plugin Formats"

    echo "Build Configuration:"
    echo "  Type: ${CMAKE_BUILD_TYPE}"
    echo "  Jobs: ${BUILD_JOBS}"
    echo "  Formats: ${PLUGIN_FORMATS}"
    echo ""

    # Check if CMakeLists.txt exists
    if [ ! -f "${PROJECT_DIR}/CMakeLists.txt" ]; then
        print_error "CMakeLists.txt not found in ${PROJECT_DIR}"
        print_error "Please ensure you're in the correct directory"
        exit 1
    fi

    # Execute build steps
    cleanup_build
    configure_cmake
    build_plugins

    # Verify outputs
    verify_build

    # Print summary
    print_summary

    # Exit with appropriate code
    if verify_build >/dev/null 2>&1; then
        exit 0
    else
        exit 1
    fi
}

# Run main function
main "$@"
