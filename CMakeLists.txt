cmake_minimum_required(VERSION 3.20)
project(ChoirV2 VERSION 2.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build options
option(CHOIR_V2_BUILD_TESTS "Build unit tests" OFF)
option(CHOIR_V2_BUILD_BENCHMARKS "Build performance benchmarks" ON)
option(CHOIR_V2_ENABLE_SIMD "Enable SIMD optimizations" ON)
option(CHOIR_V2_BUILD_PLUGIN "Build JUCE plugin" ON)
option(CHOIR_V2_BUILD_CORE "Build ChoirV2Core (old ChoirV2 code)" ON)
option(CHOIR_V2_PLUGIN_FORMATS "Plugin formats to build" "VST3;AU;CLAP;LV2;AUv3;Standalone")

# Compiler flags
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror -Wno-unused-but-set-variable)
endif()

# SIMD support
if(CHOIR_V2_ENABLE_SIMD)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
        # ARM NEON support (automatically enabled on ARM64)
        # No special flags needed for NEON on modern compilers
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        # x86_64 AVX2 support
        add_compile_options(-mavx2 -mfma)
    elseif(MSVC)
        add_compile_options(/arch:AVX2)
    endif()
endif()

# Source files (old files removed during PureDSP refactoring)
set(CORE_SOURCES
)

set(SYNTHESIS_SOURCES
)

set(DSP_SOURCES
)

set(UTILS_SOURCES
)

# Plugin source files
set(PLUGIN_SOURCES
    src/plugin/ChoirV2Processor.cpp
    src/plugin/ChoirV2Editor.cpp
)

set(PLUGIN_HEADERS
    include/plugin/ChoirV2Processor.h
    include/plugin/ChoirV2Editor.h
)

# Header files (old files removed during PureDSP refactoring)
set(HEADERS
    ${PLUGIN_HEADERS}
)

# Find nlohmann/json for JSON parsing
find_package(nlohmann_json 3.11.2 REQUIRED)

#==============================================================================
# Find JUCE (needed for PureDSP library)
#==============================================================================

# Try to find JUCE in common locations (priority order)
if(NOT JUCE_PATH)
    # 1. Project-local JUCE (for reproducible builds) - HIGHEST PRIORITY
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../../external/JUCE")
        set(JUCE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../external/JUCE")
        message(STATUS "Using project-local JUCE: ${JUCE_PATH}")
    # 2. Environment variable
    elseif(EXISTS "$ENV{JUCE_DIR}")
        set(JUCE_PATH "$ENV{JUCE_DIR}")
        message(STATUS "Using JUCE from environment variable: ${JUCE_PATH}")
    # 3. User home directory
    elseif(EXISTS "$ENV{HOME}/JUCE")
        set(JUCE_PATH "$ENV{HOME}/JUCE")
        message(STATUS "Using JUCE from user home: ${JUCE_PATH}")
    # 4. System-wide installation
    elseif(EXISTS "/usr/local/JUCE")
        set(JUCE_PATH "/usr/local/JUCE")
        message(STATUS "Using system JUCE: ${JUCE_PATH}")
    # 5. Opt installation
    elseif(EXISTS "/opt/JUCE")
        set(JUCE_PATH "/opt/JUCE")
        message(STATUS "Using opt JUCE: ${JUCE_PATH}")
    endif()
endif()

if(EXISTS "${JUCE_PATH}")
    message(STATUS "Found JUCE at: ${JUCE_PATH}")
    # Add JUCE subdirectory (but don't create plugin yet)
    add_subdirectory("${JUCE_PATH}" "${CMAKE_BINARY_DIR}/juce")
    set(JUCE_FOUND TRUE)
else()
    message(WARNING "JUCE not found. PureDSP library requires JUCE types.")
    message(WARNING "Set JUCE_PATH environment variable or pass -DJUCE_PATH=/path/to/JUCE")
    set(JUCE_FOUND FALSE)
endif()

#==============================================================================
# Choir V2 PureDSP Library
#==============================================================================

# PureDSP source files (consolidated in ChoirV2PureDSP.cpp)
set(PURE_DSP_SOURCES
    src/dsp/ChoirV2PureDSP.cpp
)

# PureDSP header files
set(PURE_DSP_HEADERS
    include/dsp/ChoirV2PureDSP.h
    include/dsp/ChoirV2DSPModules.h
    include/dsp/core/PhonemeTypes.h
    include/dsp/core/PhonemeDatabase.h
    include/dsp/core/G2PEngine.h
    include/dsp/core/LanguageLoader.h
    include/dsp/synthesis/ISynthesisMethod.h
    include/dsp/synthesis/FormantSynthesisMethod.h
    include/dsp/synthesis/DiphoneSynthesisMethod.h
    include/dsp/synthesis/SubharmonicSynthesisMethod.h
)

# Create PureDSP static library
add_library(ChoirV2PureDSP STATIC ${PURE_DSP_SOURCES})

# Include directories for PureDSP
target_include_directories(ChoirV2PureDSP PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/dsp
    ${CMAKE_SOURCE_DIR}/../../include  # For InstrumentDSP.h
)

# Set C++ standard for PureDSP
target_compile_features(ChoirV2PureDSP PUBLIC cxx_std_20)

# Link nlohmann/json for JSON parsing and JUCE
target_link_libraries(ChoirV2PureDSP PUBLIC
    nlohmann_json::nlohmann_json
    juce::juce_core
)

message(STATUS "Choir V2 PureDSP library configured")

#==============================================================================
# Choir V2 Core Library
#==============================================================================

if(CHOIR_V2_BUILD_CORE)
# Core library (old ChoirV2 code - removed during PureDSP refactoring)
message(STATUS "Choir V2 Core library: DISABLED (removed during PureDSP refactoring)")
else()
message(STATUS "Choir V2 Core library: DISABLED (PureDSP migration complete)")
endif()

# Tests
if(CHOIR_V2_BUILD_TESTS)
    enable_testing()
    find_package(GTest REQUIRED)

    add_subdirectory(tests)
endif()

# Benchmarks
if(CHOIR_V2_BUILD_BENCHMARKS)
    # Add Google Benchmark support here
    # find_package(benchmark REQUIRED)
    # add_subdirectory(tests/benchmarks)
endif()

# JUCE Plugin
if(CHOIR_V2_BUILD_PLUGIN)
    if(JUCE_FOUND)
        # Create JUCE plugin
        juce_add_plugin(ChoirV2Plugin
            COMPANY_NAME "Bret Bouchard"
            PLUGIN_MANUFACTURER_CODE "BBbr"
            PLUGIN_CODE "ChV2"
            FORMATS ${CHOIR_V2_PLUGIN_FORMATS}
            PRODUCT_NAME "Choir V2.0"
            VERSION ${PROJECT_VERSION}
            IS_SYNTH TRUE
            NEEDS_MIDI_INPUT TRUE
            NEEDS_MIDI_OUTPUT FALSE
            IS_MIDI_EFFECT FALSE
            VST_NUM_MIDI_INPUTS 16
            VST_NUM_MIDI_OUTPUTS 0
            AU_MAIN_TYPE "aumu"
            LV2_URI "https://bretbouchard.github.io/choir-v2-universal"
            LV2_CATEGORY "Instrument"
            COPY_PLUGIN_AFTER_BUILD TRUE
        )

        # Plugin sources
        target_sources(ChoirV2Plugin PRIVATE
            ${PLUGIN_SOURCES}
            ${CORE_SOURCES}
            ${SYNTHESIS_SOURCES}
            ${DSP_SOURCES}
            ${UTILS_SOURCES}
            ${HEADERS}
        )

        # Include directories
        target_include_directories(ChoirV2Plugin PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src
        )

        # Link dependencies (PureDSP only - Core library removed)
        target_link_libraries(ChoirV2Plugin PRIVATE
            ChoirV2PureDSP
            nlohmann_json::nlohmann_json
            juce::juce_audio_utils
            juce::juce_dsp
        )

        # Output directories for plugins
        set_target_properties(ChoirV2Plugin PROPERTIES
            ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/artifacts"
            LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/artifacts"
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/artifacts"
        )

        # Plugin-specific output directories
        if(TARGET ChoirV2Plugin_VST3)
            set_target_properties(ChoirV2Plugin_VST3 PROPERTIES
                LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins/vst"
            )
        endif()

        if(TARGET ChoirV2Plugin_AU)
            set_target_properties(ChoirV2Plugin_AU PROPERTIES
                LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins/au"
            )
        endif()

        if(TARGET ChoirV2Plugin_CLAP)
            set_target_properties(ChoirV2Plugin_CLAP PROPERTIES
                LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins/clap"
            )
        endif()

        if(TARGET ChoirV2Plugin_LV2)
            set_target_properties(ChoirV2Plugin_LV2 PROPERTIES
                LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins/lv2"
            )
        endif()

        if(TARGET ChoirV2Plugin_Standalone)
            set_target_properties(ChoirV2Plugin_Standalone PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins/standalone"
            )
        endif()

        if(TARGET ChoirV2Plugin_AUv3)
            set_target_properties(ChoirV2Plugin_AUv3 PROPERTIES
                LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins/auv3"
            )
        endif()

        message(STATUS "JUCE Plugin: Enabled")
        message(STATUS "  Formats: ${CHOIR_V2_PLUGIN_FORMATS}")
    else()
        message(WARNING "JUCE not found. Plugin build disabled.")
        message(WARNING "Set JUCE_PATH environment variable or pass -DJUCE_PATH=/path/to/JUCE")
    endif()
endif()

# Installation (PureDSP only - Core library removed)
install(TARGETS ChoirV2PureDSP
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES ${PURE_DSP_HEADERS}
    DESTINATION include/ChoirV2
)

# Print configuration
message(STATUS "")
message(STATUS "Choir V2.0 Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build Tests: ${CHOIR_V2_BUILD_TESTS}")
message(STATUS "  Build Benchmarks: ${CHOIR_V2_BUILD_BENCHMARKS}")
message(STATUS "  Build Plugin: ${CHOIR_V2_BUILD_PLUGIN}")
message(STATUS "  Enable SIMD: ${CHOIR_V2_ENABLE_SIMD}")
if(CHOIR_V2_BUILD_PLUGIN)
    message(STATUS "  Plugin Formats: ${CHOIR_V2_PLUGIN_FORMATS}")
endif()
message(STATUS "")
