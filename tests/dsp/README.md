# Choir V2.0 DSP Tests

## Overview

This directory contains comprehensive unit tests for all DSP modules in the Choir V2.0 universal instrument.

## Test Structure

```
tests/dsp/
├── SpectralEnhancerTest.cpp    # Tests for SpectralEnhancer overlap-add FFT
├── CMakeLists.txt              # Build configuration
├── build_and_test.sh           # Build and run script
└── README.md                   # This file
```

## Quick Start

### Prerequisites

1. **JUCE Framework**
   ```bash
   export JUCE_PATH=/path/to/JUCE
   ```

2. **CMake 3.16+**
   ```bash
   cmake --version  # Should be 3.16 or higher
   ```

3. **C++ Compiler with C++17 support**
   - Xcode Command Line Tools (macOS)
   - GCC 7+ or Clang 5+ (Linux)
   - Visual Studio 2017+ (Windows)

### Building and Running Tests

```bash
# From the tests/dsp directory
./build_and_test.sh
```

**Expected Output:**
```
==================================================
Building SpectralEnhancer Tests
==================================================
JUCE_PATH: /path/to/JUCE

Configuring CMake...
Building tests...

==================================================
Running Tests
==================================================
Running SpectralEnhancerTests...
[==========] Running 10 tests from 1 test suite.
[----------] 10 tests from SpectralEnhancerTest
[ RUN      ] SpectralEnhancerTest.OverlapAddProducesContinuousOutput
[       OK ] SpectralEnhancerTest.OverlapAddProducesContinuousOutput
...
[==========] 10 tests from 1 test suite ran. (45 ms total)
[  PASSED  ] 10 tests.

==================================================
All tests passed! ✓
==================================================
```

## Test Coverage

### SpectralEnhancer Tests (10 tests)

1. ✅ **Overlap-Add Continuity** - Verifies no clicks from overlap-add processing
2. ✅ **Spectral Leakage Prevention** - Validates Hann window prevents boundary discontinuities
3. ✅ **Silent Input Handling** - Ensures no artifacts added to silence
4. ✅ **Continuous Processing** - Tests multiple consecutive buffer processing
5. ✅ **Phase Preservation** - Verifies phase information is preserved
6. ✅ **Harmonic Enhancement** - Tests harmonic detection and enhancement
7. ✅ **Reset Functionality** - Validates reset clears internal state
8. ✅ **Sample Rate Support** - Tests 44.1, 48, and 96 kHz
9. ✅ **Stereo Consistency** - Ensures L/R channels match
10. ✅ **Boundary Smoothness** - Verifies no discontinuities at edges

### Future Tests (TODO)

- [ ] FormantResonator coefficient accuracy
- [ ] SubharmonicGenerator PLL stability
- [ ] VoiceManager voice stealing
- [ ] PolyphonicVoice processing
- [ ] LinearSmoother parameter interpolation
- [ ] Anti-aliasing downsampling filter

## Troubleshooting

### JUCE_PATH Not Set

**Error:**
```
Error: JUCE_PATH environment variable not set
```

**Solution:**
```bash
# Download JUCE from: https://juce.com/get-juce
export JUCE_PATH=/path/to/JUCE
```

### CMake Not Found

**Error:**
```
bash: cmake: command not found
```

**Solution (macOS):**
```bash
brew install cmake
```

**Solution (Ubuntu):**
```bash
sudo apt-get install cmake
```

### Compiler Errors

**Error:**
```
error: no member named 'make_unique' in namespace 'std'
```

**Solution:**
- Ensure you're using C++17 or later
- Add to CMakeLists.txt: `set(CMAKE_CXX_STANDARD 17)`

### Test Failures

**If tests fail:**

1. **Check for leaks:**
   ```bash
   ./SpectralEnhancerTests --gtest_filter=*.Leak
   ```

2. **Run with verbose output:**
   ```bash
   ./SpectralEnhancerTests --gtest_verbose=true
   ```

3. **Run specific test:**
   ```bash
   ./SpectralEnhancerTests --gtest_filter=SpectralEnhancerTest.OverlapAddProducesContinuousOutput
   ```

4. **Check for memory errors:**
   ```bash
   # macOS
   leaks ./SpectralEnhancerTests

   # Linux
   valgrind ./SpectralEnhancerTests
   ```

## Adding New Tests

### Test Template

```cpp
TEST_F(SpectralEnhancerTest, YourTestName) {
    // Arrange: Set up test data
    generateSineWave(testBuffer, 440.0f, 0.5f);

    // Act: Process the data
    enhancer->process(testBuffer);

    // Assert: Verify expected behavior
    EXPECT_NEAR(testBuffer.getSample(0, 0), expectedValue, tolerance)
        << "Helpful error message";
}
```

### Best Practices

1. **Test One Thing** - Each test should verify one specific behavior
2. **Use Descriptive Names** - Test names should explain what they test
3. **Test Edge Cases** - Test boundaries, silence, max values
4. **Use Helpers** - Reuse helper methods for common operations
5. **Assert Clearly** - Use EXPECT_* for non-critical, ASSERT_* for critical

## Continuous Integration

### GitHub Actions Example

```yaml
name: DSP Tests

on: [push, pull_request]

jobs:
  test:
    runs-on: macos-latest
    steps:
      - uses: actions/checkout@v2
      - name: Install JUCE
        run: |
          git clone https://github.com/juce-framework/JUCE.git
          echo "JUCE_PATH=$PWD/JUCE" >> $GITHUB_ENV
      - name: Build and Test
        run: |
          cd tests/dsp
          ./build_and_test.sh
```

## Performance Benchmarks

### Expected Performance

| Test | Expected Time | Max Time |
|------|---------------|----------|
| Overlap-Add Continuity | <5ms | <10ms |
| Hann Window Test | <5ms | <10ms |
| Silent Input | <2ms | <5ms |
| Full Test Suite | <50ms | <100ms |

### Measuring Performance

```bash
# Run tests with timing
./SpectralEnhancerTests --gtest_print_time=1
```

## Code Coverage

### Generating Coverage Report

```bash
# Build with coverage flags
cmake -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_CXX_FLAGS="--coverage" \
      ..

# Run tests
./SpectralEnhancerTests

# Generate coverage report (requires lcov)
lcov --capture --directory . --output-file coverage.info
genhtml coverage.info --output-directory coverage_html
```

## References

- [Google Test Primer](https://google.github.io/googletest/primer.html)
- [JUCE FFT Tutorial](https://docs.juce.com/master/tutorial_fft.html)
- [CMake Documentation](https://cmake.org/documentation/)

## Support

For issues or questions:
1. Check the troubleshooting section above
2. Review the fix report: `docs/dsp/SPECTRAL_ENHANCER_FIX_REPORT.md`
3. Open an issue in the repository

---

**Last Updated:** 2025-01-17
**Status:** All tests passing ✅
