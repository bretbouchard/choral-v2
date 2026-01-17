# LinearSmoother Quick Reference

## When to Use LinearSmoother

Use **LinearSmoother** whenever a parameter can change abruptly and cause clicks:

- ✅ Formant frequency transitions (phoneme changes)
- ✅ Filter cutoff/bandwidth changes
- ✅ Amplitude envelope transitions
- ✅ Pitch modulation changes
- ✅ Any automation or user-controlled parameter

## Basic Usage

### 1. Initialize

```cpp
#include "LinearSmoother.h"

using namespace ChoirV2;

// Create smoother
LinearSmoother smoother;

// Configure smoothing time (10-20ms typical for formants)
smoother.setTimeConstant(0.01f, 44100.0f);  // 10ms at 44.1kHz

// Set initial target
smoother.setTarget(0.0f);
smoother.reset();  // Start at target
```

### 2. On Parameter Change

```cpp
// When parameter changes (e.g., new phoneme)
smoother.setTarget(newValue);
// DO NOT set smoother.current_value_ directly!
// The smoother will glide from old value to new value
```

### 3. In Process Loop

```cpp
void process(float* output, int num_samples) {
    for (int i = 0; i < num_samples; ++i) {
        // Get smoothed value for this sample
        float smooth_value = smoother.process();

        // Use smoothed value in DSP
        filter.setFrequency(smooth_value);
        output[i] = filter.process(input[i]);
    }
}
```

## Smoothing Time Guidelines

| Parameter | Recommended Smoothing Time | Reason |
|-----------|---------------------------|---------|
| Formant frequencies | 10-20ms | Smooth transitions, preserve naturalness |
| Formant bandwidths | 10-20ms | Match frequency smoothing |
| Amplitude | 5-10ms | Faster, prevent pumping |
| Pitch | 10-15ms | Smooth but responsive |
| Filter cutoff | 5-10ms | Fast, prevent zipper noise |

## Advanced Usage

### Block Processing (More Efficient)

```cpp
void processBlock(float* output, int num_samples) {
    // Process entire block at once
    float smooth_values[num_samples];
    smoother.processBlock(smooth_values, num_samples);

    // Use smoothed values
    for (int i = 0; i < num_samples; ++i) {
        filter.setFrequency(smooth_values[i]);
        output[i] = filter.process(input[i]);
    }
}
```

### Immediate Changes (Bypass)

```cpp
// When you need instant change (e.g., bypass, reset)
smoother.setTargetImmediate(newValue);
// Current value jumps to target immediately
```

### Multiple Parameters (SIMD)

```cpp
// Smooth multiple formant frequencies at once
const int num_formants = 5;
SIMDLinearSmoother smoother(num_formants);
smoother.setTimeConstant(0.01f, 44100.0f);

// Set all formant targets
float targets[5] = {800.0f, 1150.0f, 2900.0f, 3900.0f, 4950.0f};
smoother.setTargets(targets);
smoother.reset();

// Process block
const int block_size = 64;
float output[5 * block_size];  // Interleaved: [F0[s0], F0[s1], ..., F1[s0], ...]
smoother.processBlock(output, block_size);

// Extract formant values for each sample
for (int i = 0; i < block_size; ++i) {
    for (int f = 0; f < num_formants; ++f) {
        float freq = output[f * block_size + i];
        formants[f].setFrequency(freq);
    }
    // ... process sample
}
```

## Common Patterns

### Phoneme Transition

```cpp
class FormantFilter {
    LinearSmoother freq_smoother[5];  // 5 formants
    LinearSmoother bw_smoother[5];    // 5 bandwidths

public:
    void setPhoneme(const Phoneme& phoneme) {
        // Update all formant targets
        for (int i = 0; i < 5; ++i) {
            freq_smoother[i].setTarget(phoneme.freq[i]);
            bw_smoother[i].setTarget(phoneme.bw[i]);
        }
    }

    void process(float* output, int num_samples) {
        for (int i = 0; i < num_samples; ++i) {
            // Get smoothed values for this sample
            for (int f = 0; f < 5; ++f) {
                float freq = freq_smoother[f].process();
                float bw = bw_smoother[f].process();
                resonators[f].setCoefficients(freq, bw, sampleRate);
            }
            // ... process sample
        }
    }
};
```

### Parameter Modulation

```cpp
class ModulatedFilter {
    LinearSmoother base_freq_smoother;
    LinearSmoother mod_depth_smoother;

public:
    void setBaseFrequency(float freq) {
        base_freq_smoother.setTarget(freq);
    }

    void setModulationDepth(float depth) {
        mod_depth_smoother.setTarget(depth);
    }

    void process(float* output, const float* modulator, int num_samples) {
        for (int i = 0; i < num_samples; ++i) {
            float base = base_freq_smoother.process();
            float depth = mod_depth_smoother.process();

            float modulated_freq = base + depth * modulator[i];
            filter.setFrequency(modulated_freq);
            output[i] = filter.process(input[i]);
        }
    }
};
```

## Testing Your Smoother Integration

### Verification Checklist

- [ ] No clicks when parameter changes abruptly
- [ ] Smooth transitions (audibly smooth)
- [ ] Settling time appropriate for use case
- [ ] Works at all sample rates (44.1, 48, 96 kHz)
- [ ] Reset functionality works correctly
- [ ] No zipper noise on automation

### Simple Click Test

```cpp
// Test: Step change should be smooth
smoother.setTarget(0.0f);
smoother.reset();

// Process silence
for (int i = 0; i < 1000; ++i) {
    smoother.process();
}

// Sudden step (would click without smoothing)
smoother.setTarget(1.0f);

// Check: First sample should NOT be 1.0
float first = smoother.process();
assert(first < 1.0f && first > 0.0f);

// Check: Smooth transition (no jumps)
for (int i = 0; i < 100; ++i) {
    float prev = smoother.getCurrent();
    float curr = smoother.process();
    float delta = std::abs(curr - prev);
    assert(delta < 0.01f);  // Max step size
}
```

## Performance Tips

1. **Use block processing** when possible (more cache-friendly)
2. **SIMD smoother** for multiple related parameters (formants)
3. **Zero smoothing** for parameters that don't need it (setTimeConstant(0))
4. **Pre-compute coefficients** if smoothing time never changes

## Troubleshooting

### Problem: Still hearing clicks

**Cause**: Smoothing time too short

**Solution**:
```cpp
// Increase smoothing time
smoother.setTimeConstant(0.02f, sampleRate);  // Try 20ms instead of 10ms
```

### Problem: Too slow/lethargic

**Cause**: Smoothing time too long

**Solution**:
```cpp
// Decrease smoothing time
smoother.setTimeConstant(0.005f, sampleRate);  // Try 5ms
```

### Problem: Parameter doesn't change

**Cause**: Forgot to call `process()` or using `getCurrent()` without processing

**Solution**:
```cpp
// WRONG:
float value = smoother.getCurrent();  // Doesn't advance!

// CORRECT:
float value = smoother.process();     // Advances one sample
```

### Problem: First sample is wrong after reset

**Cause**: Using `reset()` incorrectly

**Solution**:
```cpp
// After reset, call process() to get first smoothed value
smoother.reset();
float value = smoother.process();  // Now at target
```

## Integration Checklist

- [ ] Add `#include "LinearSmoother.h"`
- [ ] Create smoother member variable(s)
- [ ] Initialize in constructor (setTimeConstant)
- [ ] Call setTarget() when parameter changes
- [ ] Call process() in audio loop
- [ ] Test for click-free transitions
- [ ] Verify at different sample rates
- [ ] Document smoothing time in header

## Example: Complete Integration

```cpp
#pragma once
#include "LinearSmoother.h"
#include "FormantResonator.h"

namespace ChoirV2 {

class FormantFilter {
    LinearSmoother freq_smoother_[5];
    LinearSmoother bw_smoother_[5];
    FormantResonator resonators_[5];
    double sample_rate_;

public:
    FormantFilter(double sample_rate)
        : sample_rate_(sample_rate)
    {
        // Initialize smoothers with 15ms smoothing
        for (int i = 0; i < 5; ++i) {
            freq_smoother_[i].setTimeConstant(0.015f, sample_rate);
            bw_smoother_[i].setTimeConstant(0.015f, sample_rate);
        }
    }

    void setPhoneme(const Phoneme& phoneme) {
        for (int i = 0; i < 5; ++i) {
            freq_smoother_[i].setTarget(phoneme.freq[i]);
            bw_smoother_[i].setTarget(phoneme.bw[i]);
        }
    }

    float process(float input) {
        float output = input;

        for (int i = 0; i < 5; ++i) {
            // Get smoothed formant values
            float freq = freq_smoother_[i].process();
            float bw = bw_smoother_[i].process();

            // Update resonator
            resonators_[i].setCoefficients(freq, bw, sample_rate_);

            // Process through resonator
            output = resonators_[i].process(output);
        }

        return output;
    }
};

} // namespace ChoirV2
```

---

**Quick Reference Guide** - For complete implementation details, see `LINEAR_SMOOTHER_IMPLEMENTATION.md`
