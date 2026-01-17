cmake_minimum_required(VERSION 3.20)
project(ChoirV2 VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build options
option(CHOIR_V2_BUILD_TESTS "Build unit tests" ON)
option(CHOIR_V2_BUILD_BENCHMARKS "Build performance benchmarks" ON)
option(CHOIR_V2_ENABLE_SIMD "Enable SIMD optimizations" ON)

# Compiler flags
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# SIMD support
if(CHOIR_V2_ENABLE_SIMD)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(-mavx2 -mfma)
    elseif(MSVC)
        add_compile_options(/arch:AVX2)
    endif()
endif()

# Source files
set(CORE_SOURCES
    src/core/PhonemeDatabase.cpp
    src/core/LanguageLoader.cpp
    src/core/G2PEngine.cpp
    src/core/ChoirV2Engine.cpp
    src/core/VoiceManager.cpp
)

set(SYNTHESIS_SOURCES
    src/synthesis/FormantSynthesis.cpp
    src/synthesis/SubharmonicSynthesis.cpp
    src/synthesis/DiphoneSynthesis.cpp
)

set(DSP_SOURCES
    src/dsp/FormantResonator.cpp
    src/dsp/GlottalSource.cpp
    src/dsp/LinearSmoother.cpp
    src/dsp/SpectralEnhancer.cpp
    src/dsp/SubharmonicGenerator.cpp
)

set(UTILS_SOURCES
    src/utils/SIMDHelpers.cpp
    src/utils/MemoryPool.cpp
    src/utils/LockFreeQueue.cpp
)

# Header files
set(HEADERS
    src/core/PhonemeDatabase.h
    src/core/LanguageLoader.h
    src/core/G2PEngine.h
    src/core/ChoirV2Engine.h
    src/core/VoiceManager.h
    src/synthesis/FormantSynthesis.h
    src/synthesis/SubharmonicSynthesis.h
    src/synthesis/DiphoneSynthesis.h
    src/synthesis/ISynthesisMethod.h
    src/dsp/FormantResonator.h
    src/dsp/GlottalSource.h
    src/dsp/LinearSmoother.h
    src/dsp/SpectralEnhancer.h
    src/dsp/SubharmonicGenerator.h
    src/utils/SIMDHelpers.h
    src/utils/MemoryPool.h
    src/utils/LockFreeQueue.h
)

# Core library
add_library(ChoirV2Core STATIC
    ${CORE_SOURCES}
    ${SYNTHESIS_SOURCES}
    ${DSP_SOURCES}
    ${UTILS_SOURCES}
    ${HEADERS}
)

target_include_directories(ChoirV2Core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# JUCE integration (will be added later)
# add_subdirectory(JUCE)

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

# Installation
install(TARGETS ChoirV2Core
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES ${HEADERS}
    DESTINATION include/ChoirV2
)

# Print configuration
message(STATUS "")
message(STATUS "Choir V2.0 Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build Tests: ${CHOIR_V2_BUILD_TESTS}")
message(STATUS "  Build Benchmarks: ${CHOIR_V2_BUILD_BENCHMARKS}")
message(STATUS "  Enable SIMD: ${CHOIR_V2_ENABLE_SIMD}")
message(STATUS "")
