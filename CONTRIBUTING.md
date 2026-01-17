# Contributing to Choir V2.0

Thank you for your interest in contributing to Choir V2.0! This document provides guidelines and instructions for contributing to the project.

## Code of Conduct

- Be respectful and inclusive
- Provide constructive feedback
- Focus on what is best for the community
- Show empathy towards other community members

## Development Philosophy

Choir V2.0 follows **SLC** (Simple, Lovable, Complete) principles:

- **Simple**: Focused features, intuitive design, zero learning curve
- **Lovable**: Delights users, solves problems magically
- **Complete**: Full user journey, no gaps, no workarounds

### No SLC Violations

- **No stub methods** - Functions must have real implementations
- **No workarounds** - "It works but..." solutions are unacceptable
- **No TODO/FIXME** without actionable tickets
- **No "good enough"** - Temporary solutions must be tracked
- **No UnimplementedError** - Production code must work

## Getting Started

### Prerequisites

- **C++20** compiler (GCC 10+, Clang 12+, MSVC 2019+)
- **CMake** 3.20+
- **JUCE** 8.0.0+ (for plugin builds)
- **Google Test** (for unit tests)
- **Git** for version control

### Build Instructions

```bash
# Clone repository
git clone https://github.com/bretbouchard/choir-v2-universal.git
cd choir-v2-universal

# Configure build
cmake -B build -S .

# Build
cmake --build build

# Run tests
ctest --test-dir build

# Build plugin (optional)
./build_plugin.sh --all
```

## Development Workflow

### 1. Check for Existing Work

Before starting any work, check for existing bd issues:

```bash
bd ready --json
```

### 2. Create an Issue

All work must be tracked in bd (Beads task management):

```bash
bd create "Brief description of task"
```

### 3. Consult Confucius

Check Confucius for relevant patterns from past issues:

```
"What does Confucius know about [topic]?"
```

### 4. Create a Branch

```bash
git checkout -b feature/your-feature-name
# or
git checkout -b fix/your-bug-fix
```

### 5. Make Changes

- Write clean, readable code
- Follow coding standards (see below)
- Add tests for new functionality
- Update documentation

### 6. Test Your Changes

```bash
# Run unit tests
ctest --test-dir build

# Run specific test
./build/tests/unit/test_your_component

# Build and test plugin
./build_plugin.sh --all
```

### 7. Commit Changes

```bash
git add .
git commit -m "feat: Add your feature description"
```

### 8. Push and Create Pull Request

```bash
git push origin feature/your-feature-name
```

Then create a pull request on GitHub.

## Coding Standards

### C++ Style

- **Indentation**: 4 spaces (no tabs)
- **Naming**: `camelCase` for functions/variables, `PascalCase` for classes
- **Braces**: Allman style (opening brace on new line)
- **Constants**: `UPPER_SNAKE_CASE`
- **File Names**: `PascalCase.cpp` / `PascalCase.h`

Example:

```cpp
class VoiceManager
{
public:
    VoiceManager();

    void allocateVoice(float frequency, float amplitude);
    void releaseVoice(int voiceId);

private:
    static constexpr int MAX_VOICES = 60;
    float sampleRate_;
};
```

### Real-Time Safety

**Critical**: All audio processing code must be real-time safe.

- **No allocations** in `processBlock()` or audio thread
- **No locks/mutexes** in audio thread
- **No file I/O** in audio thread
- **Bounded execution time** - No unbounded loops
- **Pre-allocated buffers** - Allocate in `prepareToPlay()`

```cpp
// ❌ WRONG - Allocation in audio thread
void processBlock(float* output, int numSamples) {
    float* temp = new float[numSamples]; // DON'T DO THIS
}

// ✅ RIGHT - Pre-allocated buffer
void prepareToPlay(int numSamples) {
    tempBuffer_.resize(numSamples);
}

void processBlock(float* output, int numSamples) {
    float* temp = tempBuffer_.data(); // OK
}
```

### Parameter Smoothing

All parameter changes must be smoothed to prevent clicks/pops:

```cpp
// Use juce::LinearSmoothedValue for all parameters
juce::LinearSmoothedValue<float> masterGain_;

void processBlock(float* output, int numSamples) {
    masterGain_.applyGain(output, numSamples);
}
```

### Denormal Protection

Flush subnormal numbers to zero for performance:

```cpp
inline float flushDenormal(float x) {
    return (std::abs(x) < 1e-10f) ? 0.0f : x;
}
```

## Testing

### Unit Tests

- **Target**: >90% code coverage
- **Framework**: Google Test
- **Location**: `tests/unit/`

Example test:

```cpp
TEST(FormantResonatorTest, CalculatesCoefficientsCorrectly) {
    FormantResonator resonator;
    resonator.setFrequency(1000.0f, 44100.0f);
    resonator.setBandwidth(100.0f);

    float coeff = resonator.getCoefficient();
    EXPECT_NEAR(coeff, expectedValue, 0.001f);
}
```

### Integration Tests

- **Location**: `tests/integration/`
- **Focus**: Component interaction, data flow

### Performance Tests

- **Target**: < 30% CPU @ 60 voices
- **Target**: < 5ms latency @ 44.1kHz
- **Location**: `tests/performance/`

## Documentation

### Code Comments

- **Public APIs**: Document behavior, parameters, return values
- **Complex algorithms**: Explain the "why", not just "what"
- **TODO notes**: Must have corresponding bd issue

```cpp
/**
 * Allocates a new voice for synthesis.
 *
 * @param frequency Fundamental frequency in Hz
 * @param amplitude Linear amplitude (0.0 to 1.0)
 * @return Voice ID, or -1 if no voices available
 */
int allocateVoice(float frequency, float amplitude);
```

### README Updates

Update README.md when:
- Adding new features
- Changing build process
- Updating dependencies
- Fixing critical bugs

### API Documentation

Update `docs/API.md` when:
- Adding new public APIs
- Changing existing APIs
- Deprecating features

## Pull Request Guidelines

### PR Title

Use conventional commit format:

- `feat:` - New feature
- `fix:` - Bug fix
- `refactor:` - Code refactoring
- `docs:` - Documentation changes
- `test:` - Test additions/changes
- `perf:` - Performance improvements

### PR Description

Include:
- **Summary**: What changed and why
- **Testing**: How you tested the changes
- **Screenshots**: For UI changes (if applicable)
- **Breaking Changes**: List any breaking changes
- **Related Issues**: Link to bd issue number

### Review Process

1. **Automated Checks**: CI runs tests, linting
2. **Code Review**: Maintainer reviews code
3. **SLC Validation**: No workarounds or stubs
4. **Testing**: Manual testing in DAW (for plugin changes)
5. **Approval**: Maintainer approves and merges

## Issue Reporting

### Bug Reports

Include:
- **Description**: Clear description of the bug
- **Steps to Reproduce**: Minimal reproducible example
- **Expected Behavior**: What should happen
- **Actual Behavior**: What actually happens
- **Environment**: OS, DAW, plugin format, CPU
- **Logs**: Error messages, crash dumps

### Feature Requests

Include:
- **Problem**: What problem you're trying to solve
- **Proposed Solution**: How you think it should work
- **Alternatives**: Other approaches you considered
- **Use Case**: Why this feature is important

## Development Phases

### Phase 1: Core DSP (COMPLETE)
- FormantResonator ✅
- SubharmonicGenerator ✅
- SpectralEnhancer ✅
- LinearSmoother ✅

### Phase 2: Synthesis Engine (COMPLETE)
- FormantSynthesis ✅
- DiphoneSynthesis ✅
- SubharmonicSynthesis ✅
- VoiceManager ✅

### Phase 3: Language Support (COMPLETE)
- PhonemeDatabase ✅
- LanguageLoader ✅
- G2PEngine ✅
- 4 language definitions ✅

### Phase 4: Plugin Integration (COMPLETE)
- JUCE plugin wrapper ✅
- Parameter mapping ✅
- Preset management ✅
- UI editor ✅

### Phase 5: Testing & Validation (IN PROGRESS)
- Unit tests ✅
- Integration tests ✅
- DAW testing ⏳
- Performance validation ⏳

## Performance Guidelines

### CPU Usage

- **Target**: < 30% @ 60 voices
- **Measure**: Use built-in performance monitor
- **Optimize**: Profile before optimizing

### Memory Usage

- **Target**: < 200MB for 60 voices
- **Pre-allocate**: Allocate at startup, not runtime
- **SIMD**: Use vectorization for batch processing

### Latency

- **Target**: < 5ms @ 44.1kHz (128 samples)
- **Buffers**: Use small buffer sizes
- **Avoid**: Locks, allocations, I/O in audio thread

## Release Process

1. **Update CHANGELOG.md** - Document all changes
2. **Update version** - CMakeLists.txt, plugin info
3. **Create Git tag** - Annotated tag with release notes
4. **Build release binaries** - All plugin formats
5. **Test release** - DAW compatibility testing
6. **Create GitHub release** - Upload binaries, notes
7. **Announce** - Blog post, social media

## Community Resources

- **Repository**: https://github.com/bretbouchard/choir-v2-universal
- **Issues**: Tracked in bd (Beads task management)
- **Documentation**: See `docs/` directory
- **Research**: See `/Users/bretbouchard/apps/schill/white_room/docs/research/`

## Getting Help

- **Check Confucius** - Search for past solutions
- **Read documentation** - README.md, docs/API.md, docs/ARCHITECTURE.md
- **Search issues** - Check if problem already reported
- **Create bd issue** - Track new work
- **Ask in discussions** - GitHub Discussions (coming soon)

## License

By contributing to Choir V2.0, you agree that your contributions will be licensed under the **MIT License**.

---

**Thank you for contributing to Choir V2.0!**

Every contribution helps make Choir V2.0 better, whether it's a bug fix, new feature, documentation improvement, or performance optimization.
