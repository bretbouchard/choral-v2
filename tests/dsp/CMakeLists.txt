# CMakeLists.txt for SpectralEnhancer tests
cmake_minimum_required(VERSION 3.16)
project(ChoirV2SpectralEnhancerTests)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add compile options
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find JUCE
# Note: You'll need to set JUCE_PATH environment variable or pass it via CMake
if(NOT DEFINED JUCE_PATH)
    set(JUCE_PATH "$ENV{JUCE_PATH}")
    if(NOT EXISTS "${JUCE_PATH}")
        message(FATAL_ERROR "JUCE_PATH not set. Please set JUCE_PATH environment variable.")
    endif()
endif()

# Add JUCE modules
add_subdirectory(${JUCE_PATH} ${CMAKE_BINARY_DIR}/juce)

# Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)

# SpectralEnhancer library
add_library(SpectralEnhancer STATIC
    ../../src/dsp/SpectralEnhancer.cpp
    ../../src/dsp/SpectralEnhancer.h
)

target_include_directories(SpectralEnhancer PUBLIC
    ${CMAKE_SOURCE_DIR}/../../src
    ${JUCE_PATH}/modules
)

target_link_libraries(SpectralEnhancer PUBLIC
    juce::juce_audio_basics
)

# Test executable
add_executable(SpectralEnhancerTests
    SpectralEnhancerTest.cpp
)

target_link_libraries(SpectralEnhancerTests
    SpectralEnhancer
    gtest_main
    gmock_main
)

# Enable testing
enable_testing()
add_test(NAME SpectralEnhancerTest COMMAND SpectralEnhancerTests)
