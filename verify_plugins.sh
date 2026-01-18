#!/bin/bash

################################################################################
# Choir V2.0 - Plugin Verification Script
# Validates all built plugin formats for correctness and compatibility
################################################################################

set -e

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

# Verification flags
SKIP_SIGNATURE_CHECK="${SKIP_SIGNATURE_CHECK:-0}"
VERBOSE_OUTPUT="${VERBOSE_OUTPUT:-0}"

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

print_info() {
    echo "  ℹ $1"
}

check_file_exists() {
    local file="$1"
    local description="$2"

    if [ -e "${file}" ]; then
        print_success "${description} exists"
        return 0
    else
        print_error "${description} NOT found"
        print_info "Expected: ${file}"
        return 1
    fi
}

check_file_size() {
    local file="$1"
    local min_size_kb="$2"

    if [ ! -f "${file}" ]; then
        return 1
    fi

    local size_bytes=$(stat -f%z "${file}" 2>/dev/null || stat -c%s "${file}" 2>/dev/null)
    local size_kb=$((size_bytes / 1024))

    if [ ${size_kb} -gt ${min_size_kb} ]; then
        print_success "File size: ${size_kb} KB (minimum: ${min_size_kb} KB)"
        return 0
    else
        print_error "File size too small: ${size_kb} KB (minimum: ${min_size_kb} KB)"
        return 1
    fi
}

check_file_permissions() {
    local file="$1"

    if [ ! -f "${file}" ]; then
        return 1
    fi

    if [ -x "${file}" ]; then
        print_success "File is executable"
        return 0
    else
        print_warning "File is not executable (may be okay for some formats)"
        return 0
    fi
}

check_architecture() {
    local file="$1"

    if [ ! -f "${file}" ]; then
        return 1
    fi

    local arch=$(file "${file}" | grep -o "x86_64\|arm64\|universal" | head -1)

    if [ -n "${arch}" ]; then
        print_info "Architecture: ${arch}"
        return 0
    else
        print_warning "Could not determine architecture"
        return 0
    fi
}

check_dependencies() {
    local file="$1"

    if [ ! -f "${file}" ]; then
        return 1
    fi

    if [ "${VERBOSE_OUTPUT}" = "1" ]; then
        print_info "Dependencies:"
        if command -v otool >/dev/null 2>&1; then
            otool -L "${file}" 2>/dev/null | head -10 || true
        elif command -v ldd >/dev/null 2>&1; then
            ldd "${file}" 2>/dev/null | head -10 || true
        fi
    fi

    return 0
}

verify_vst3() {
    print_step "Verifying VST3 Plugin"

    local vst3_path="${PLUGIN_DIR}/vst/Choir V2.0.vst3"
    local binary_path="${vst3_path}/Contents/MacOS/Choir V2.0"
    local issues=0

    # Check bundle structure
    if ! check_file_exists "${vst3_path}" "VST3 bundle"; then
        ((issues++))
    fi

    # Check binary exists
    if ! check_file_exists "${binary_path}" "VST3 binary"; then
        ((issues++))
        return ${issues}
    fi

    # Check binary size (should be at least 100 KB for a functional plugin)
    if ! check_file_size "${binary_path}" 100; then
        ((issues++))
    fi

    # Check permissions
    if ! check_file_permissions "${binary_path}"; then
        ((issues++))
    fi

    # Check architecture
    check_architecture "${binary_path}"

    # Check dependencies
    check_dependencies "${binary_path}"

    # Check for PkgInfo
    local pkginfo="${vst3_path}/Contents/PkgInfo"
    if check_file_exists "${pkginfo}" "PkgInfo"; then
        local pkginfo_content=$(cat "${pkginfo}")
        print_info "PkgInfo: ${pkginfo_content}"

        # Validate PkgInfo format
        if [[ "${pkginfo_content}" =~ ^Bndl????$ ]]; then
            print_success "PkgInfo format valid"
        else
            print_warning "PkgInfo format unusual: ${pkginfo_content}"
            ((issues++))
        fi
    else
        print_warning "PkgInfo missing (may cause issues)"
        ((issues++))
    fi

    # Check for Info.plist
    local plist="${vst3_path}/Contents/Info.plist"
    if check_file_exists "${plist}" "Info.plist"; then
        # Try to extract bundle ID
        if command -v defaults >/dev/null 2>&1; then
            local bundle_id=$(defaults read "$(pwd)/${vst3_path}" CFBundleIdentifier 2>/dev/null || echo "unknown")
            print_info "Bundle ID: ${bundle_id}"
        fi
    else
        print_warning "Info.plist missing (may cause issues)"
        ((issues++))
    fi

    return ${issues}
}

verify_au() {
    print_step "Verifying AU Plugin"

    local component_path="${PLUGIN_DIR}/au/Choir V2.0.component"
    local binary_path="${component_path}/Contents/MacOS/Choir V2.0"
    local issues=0

    # Check bundle structure
    if ! check_file_exists "${component_path}" "AU component bundle"; then
        ((issues++))
    fi

    # Check binary exists
    if ! check_file_exists "${binary_path}" "AU binary"; then
        ((issues++))
        return ${issues}
    fi

    # Check binary size
    if ! check_file_size "${binary_path}" 100; then
        ((issues++))
    fi

    # Check permissions
    if ! check_file_permissions "${binary_path}"; then
        ((issues++))
    fi

    # Check architecture
    check_architecture "${binary_path}"

    # Check dependencies
    check_dependencies "${binary_path}"

    # Check for Info.plist (critical for AU)
    local plist="${component_path}/Contents/Info.plist"
    if check_file_exists "${plist}" "Info.plist"; then
        # Try to extract component info
        if command -v defaults >/dev/null 2>&1; then
            local component_type=$(defaults read "$(pwd)/${component_path}" CFBundlePackageType 2>/dev/null || echo "unknown")
            print_info "Component Type: ${component_type}"
        fi
    else
        print_error "Info.plist missing (CRITICAL for AU)"
        ((issues++))
    fi

    # Check for version resource
    if [ -f "${component_path}/Contents/Resources/version.plist" ]; then
        print_success "Version resource present"
    fi

    return ${issues}
}

verify_clap() {
    print_step "Verifying CLAP Plugin"

    local clap_path="${PLUGIN_DIR}/clap/Choir V2.0.clap"
    local issues=0

    # Check file exists
    if ! check_file_exists "${clap_path}" "CLAP plugin"; then
        ((issues++))
        return ${issues}
    fi

    # Check file size
    if ! check_file_size "${clap_path}" 50; then
        ((issues++))
    fi

    # Check permissions
    if ! check_file_permissions "${clap_path}"; then
        ((issues++))
    fi

    # Verify it's a valid shared library
    local file_type=$(file "${clap_path}")
    if echo "${file_type}" | grep -q "shared object\|dynamically linked"; then
        print_success "Valid shared library format"
    else
        print_error "Invalid file format: ${file_type}"
        ((issues++))
    fi

    # Check architecture
    check_architecture "${clap_path}"

    # Check dependencies
    check_dependencies "${clap_path}"

    return ${issues}
}

verify_lv2() {
    print_step "Verifying LV2 Plugin"

    local lv2_path="${PLUGIN_DIR}/lv2/Choir V2.0.lv2"
    local issues=0

    # Check bundle exists
    if ! check_file_exists "${lv2_path}" "LV2 bundle"; then
        ((issues++))
        return ${issues}
    fi

    # Check for manifest.ttl (required)
    local manifest="${lv2_path}/manifest.ttl"
    if ! check_file_exists "${manifest}" "manifest.ttl"; then
        ((issues++))
    else
        # Check manifest size
        if [ -s "${manifest}" ]; then
            print_success "Manifest not empty"
        else
            print_error "Manifest is empty"
            ((issues++))
        fi
    fi

    # Check for plugin binary
    local binary=$(find "${lv2_path}" -type f \( -name "*.so" -o -name "*.dylib" -o -name "*.dll" \) | head -1)
    if [ -n "${binary}" ]; then
        print_success "Plugin binary found: $(basename "${binary}")"

        # Check binary size
        if ! check_file_size "${binary}" 50; then
            ((issues++))
        fi

        # Check architecture
        check_architecture "${binary}"

        # Check dependencies
        check_dependencies "${binary}"
    else
        print_error "No plugin binary found"
        ((issues++))
    fi

    # Check for_ttl files
    local ttl_count=$(find "${lv2_path}" -name "*.ttl" | wc -l)
    if [ ${ttl_count} -gt 0 ]; then
        print_success "TTL files found: ${ttl_count}"
    else
        print_warning "No TTL files found (unusual for LV2)"
        ((issues++))
    fi

    return ${issues}
}

verify_auv3() {
    print_step "Verifying AUv3 Plugin"

    local appex_path="${PLUGIN_DIR}/auv/Choir V2.0.appex"
    local binary_path="${appex_path}/Contents/MacOS/Choir V2.0"
    local issues=0

    # Check app extension exists
    if ! check_file_exists "${appex_path}" "AUv3 app extension"; then
        ((issues++))
    fi

    # Check binary exists
    if ! check_file_exists "${binary_path}" "AUv3 binary"; then
        ((issues++))
        return ${issues}
    fi

    # Check binary size
    if ! check_file_size "${binary_path}" 100; then
        ((issues++))
    fi

    # Check permissions
    if ! check_file_permissions "${binary_path}"; then
        ((issues++))
    fi

    # Check architecture
    check_architecture "${binary_path}"

    # Check dependencies
    check_dependencies "${binary_path}"

    # Check for Info.plist (critical for AUv3)
    local plist="${appex_path}/Contents/Info.plist"
    if check_file_exists "${plist}" "Info.plist"; then
        # Try to extract extension info
        if command -v defaults >/dev/null 2>&1; then
            local ext_class=$(defaults read "$(pwd)/${appex_path}" NSExtensionPrincipalClass 2>/dev/null || echo "unknown")
            print_info "Extension Class: ${ext_class}"

            local ext_type=$(defaults read "$(pwd)/${appex_path}" NSExtensionPointIdentifier 2>/dev/null || echo "unknown")
            print_info "Extension Type: ${ext_type}"
        fi
    else
        print_error "Info.plist missing (CRITICAL for AUv3)"
        ((issues++))
    fi

    return ${issues}
}

verify_standalone() {
    print_step "Verifying Standalone Application"

    local app_path="${PLUGIN_DIR}/standalone/Choir V2.0.app"
    local binary_path="${app_path}/Contents/MacOS/Choir V2.0"
    local issues=0

    # Check app bundle exists
    if check_file_exists "${app_path}" "Standalone app bundle"; then
        # Check binary exists
        if check_file_exists "${binary_path}" "Standalone binary"; then
            # Check binary size
            if ! check_file_size "${binary_path}" 200; then
                ((issues++))
            fi

            # Check permissions
            if ! check_file_permissions "${binary_path}"; then
                ((issues++))
            fi

            # Check architecture
            check_architecture "${binary_path}"

            # Check dependencies
            check_dependencies "${binary_path}"

            # Check for Info.plist
            local plist="${app_path}/Contents/Info.plist"
            if check_file_exists "${plist}" "Info.plist"; then
                # Try to extract app info
                if command -v defaults >/dev/null 2>&1; then
                    local app_name=$(defaults read "$(pwd)/${app_path}" CFBundleName 2>/dev/null || echo "unknown")
                    print_info "App Name: ${app_name}"

                    local app_version=$(defaults read "$(pwd)/${app_path}" CFBundleShortVersionString 2>/dev/null || echo "unknown")
                    print_info "Version: ${app_version}"
                fi
            else
                print_warning "Info.plist missing (may cause issues)"
                ((issues++))
            fi

            # Check for icon
            if [ -d "${app_path}/Contents/Resources" ]; then
                local icon_count=$(find "${app_path}/Contents/Resources" -name "*.icns" | wc -l)
                if [ ${icon_count} -gt 0 ]; then
                    print_success "App icon found"
                fi
            fi
        else
            ((issues++))
        fi
    else
        # Check for Linux binary
        local linux_binary="${PLUGIN_DIR}/standalone/Choir V2.0"
        if check_file_exists "${linux_binary}" "Standalone binary (Linux)"; then
            if ! check_file_size "${linux_binary}" 200; then
                ((issues++))
            fi
            check_architecture "${linux_binary}"
            check_dependencies "${linux_binary}"
        else
            ((issues++))
        fi
    fi

    return ${issues}
}

verify_plugin_directory() {
    print_step "Verifying Plugin Directory Structure"

    if [ ! -d "${PLUGIN_DIR}" ]; then
        print_error "Plugin directory not found: ${PLUGIN_DIR}"
        print_info "Have you run build_all_formats.sh yet?"
        return 1
    fi

    print_success "Plugin directory exists"

    # List all subdirectories
    print_info "Subdirectories:"
    find "${PLUGIN_DIR}" -maxdepth 1 -type d | sort | while read -r dir; do
        local name=$(basename "${dir}")
        if [ "${name}" != "plugins" ]; then
            echo "    - ${name}"
        fi
    done

    # Calculate total size
    local total_size=$(du -sh "${PLUGIN_DIR}" 2>/dev/null | cut -f1)
    print_info "Total size: ${total_size}"

    return 0
}

generate_verification_report() {
    local total_issues=$1
    local report_file="${BUILD_DIR}/verification_report.txt"

    print_step "Generating Verification Report"

    {
        echo "Choir V2.0 Plugin Verification Report"
        echo "====================================="
        echo "Date: $(date)"
        echo "Build Directory: ${BUILD_DIR}"
        echo "Plugin Directory: ${PLUGIN_DIR}"
        echo ""
        echo "Total Issues Found: ${total_issues}"
        echo ""

        if [ ${total_issues} -eq 0 ]; then
            echo "Status: ALL CHECKS PASSED"
        else
            echo "Status: ISSUES FOUND"
        fi

        echo ""
        echo "Plugin Formats Verified:"
        echo "  - VST3"
        echo "  - AU (Audio Unit)"
        echo "  - CLAP"
        echo "  - LV2"
        echo "  - AUv3"
        echo "  - Standalone"
        echo ""
    } > "${report_file}"

    print_success "Report saved to: ${report_file}"
}

################################################################################
# Main Verification Process
################################################################################

main() {
    print_header "Choir V2.0 - Plugin Verification"

    # Check if plugin directory exists
    if ! verify_plugin_directory; then
        exit 1
    fi

    echo ""
    echo "Starting comprehensive verification..."
    echo ""

    local total_issues=0
    local format_issues=0

    # Verify each format
    verify_vst3
    format_issues=$?
    ((total_issues += format_issues))

    verify_au
    format_issues=$?
    ((total_issues += format_issues))

    verify_clap
    format_issues=$?
    ((total_issues += format_issues))

    verify_lv2
    format_issues=$?
    ((total_issues += format_issues))

    verify_auv3
    format_issues=$?
    ((total_issues += format_issues))

    verify_standalone
    format_issues=$?
    ((total_issues += format_issues))

    # Generate report
    generate_verification_report ${total_issues}

    # Print summary
    print_header "Verification Summary"

    if [ ${total_issues} -eq 0 ]; then
        print_success "All plugin formats verified successfully!"
        echo ""
        echo "Plugins are ready for:"
        echo "  • Testing in DAWs"
        echo "  • Distribution to users"
        echo "  • Installation in system plugin folders"
        echo ""
        exit 0
    else
        print_warning "Verification completed with ${total_issues} issue(s)"
        echo ""
        echo "Please review the output above for details."
        echo "Some issues may be warnings and not critical."
        echo ""
        exit 1
    fi
}

# Run main function
main "$@"
