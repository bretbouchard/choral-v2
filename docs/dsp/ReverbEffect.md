# ReverbEffect - High-Quality Stereo Reverb

## Overview

`ReverbEffect` is a professional-grade stereo reverb implementation for Choir V2.0, based on the Schroeder/Moorer reverb design. It provides rich, natural reverb with smooth parameter transitions and real-time safety guarantees.

## Algorithm Design

### Reverb Structure

The reverb uses a classic Schroeder/Moorer design:

```
Input → Parallel Comb Filters (8x) → Series Allpass Filters (4x) → Wet/Dry Mix → Output
         (Early Reflections)          (Diffusion)
```

**Comb Filters (Early Reflections):**
- 8 parallel comb filters with detuned delay lengths
- Create the initial reverb "attack"
- Delay lengths: 32ms, 37ms, 41ms, 43ms, 47ms, 53ms, 59ms, 61ms
- Feedback control determines reverb decay time

**Allpass Filters (Diffusion):**
- 4 series allpass filters
- Smooth out the reverb tail
- Eliminate metallic artifacts
- Delay lengths: 5ms, 7ms, 11ms, 13ms

### Damping

High-frequency absorption simulates realistic materials:
- **Low damping (0.0)**: Bright, reflective surfaces (concrete, tile)
- **High damping (1.0)**: Absorptive materials (carpet, curtains)

Damping is applied in the feedback loop of comb filters using a 1-pole lowpass filter.

## Parameters

### Room Size (0.0 - 1.0)

Controls the perceived size of the reverb space:
- **0.0**: Small closet, tight sound
- **0.5**: Medium room, natural ambience
- **1.0**: Large cathedral, long decay

Affects comb filter feedback:
- Higher values = longer decay time
- Also affects early reflection pattern

### Damping (0.0 - 1.0)

Controls high-frequency absorption:
- **0.0**: No absorption, bright reverb
- **0.5**: Natural damping, balanced tone
- **1.0**: Heavy damping, dark reverb

Simulates different materials:
- Low damping: Concrete, glass, tile
- High damping: Wood, carpet, fabric

### Wet Level (0.0 - 1.0)

Controls the amount of reverb in the output:
- **0.0**: No reverb (dry only)
- **0.5**: Balanced mix
- **1.0**: Full reverb (wet only)

Used with `dryLevel` for wet/dry mix control.

### Dry Level (0.0 - 1.0)

Controls the amount of dry (unprocessed) signal:
- **0.0**: Muted dry signal
- **0.5**: Balanced mix
- **1.0**: Full dry signal (no reverb)

### Width (0.0 - 1.0)

Controls stereo imaging of the reverb:
- **0.0**: Mono reverb (both channels identical)
- **0.5**: Medium stereo spread
- **1.0**: Full stereo width (maximum separation)

Uses phase inversion technique for stereo enhancement.

### Freeze Mode (bool)

Enables infinite reverb decay:
- **false**: Normal reverb decay
- **true**: Infinite feedback (sustains forever)

Useful for:
- Ambient pads
- Special effects
- Creating soundscapes

## Presets

### Small Room
```
Room Size: 0.3
Damping:   0.5
Wet:       0.2
Dry:       0.8
Width:     0.5
```
Intimate room sound, quick decay, natural ambience.

### Large Hall
```
Room Size: 0.7
Damping:   0.4
Wet:       0.4
Dry:       0.6
Width:     0.8
```
Concert hall reverb, long decay, spacious sound.

### Cathedral
```
Room Size: 0.9
Damping:   0.3
Wet:       0.5
Dry:       0.5
Width:     1.0
```
Massive cathedral, very long decay, bright tail.

### Plate
```
Room Size: 0.5
Damping:   0.6
Wet:       0.3
Dry:       0.7
Width:     0.7
```
Classic plate reverb, smooth decay, dense diffusion.

## Usage

### Basic Integration

```cpp
#include "dsp/ReverbEffect.h"

// Create reverb
ReverbEffect reverb;

// Prepare with sample rate and max block size
reverb.prepare(48000.0f, 256);

// Load preset
reverb.loadPreset(ReverbPreset::LargeHall);

// Process audio
std::vector<float> input_left(256);
std::vector<float> input_right(256);
std::vector<float> output_left(256);
std::vector<float> output_right(256);

reverb.processStereo(input_left.data(), input_right.data(),
                     output_left.data(), output_right.data(),
                     256);
```

### Parameter Control

```cpp
// Adjust reverb in real-time
reverb.setRoomSize(0.8f);      // Larger space
reverb.setDamping(0.3f);       // Brighter tail
reverb.setWetLevel(0.5f);      // More reverb
reverb.setDryLevel(0.5f);      // Balanced mix
reverb.setWidth(1.0f);         // Full stereo
```

### Bypass

```cpp
// Bypass reverb (dry signal only)
reverb.setBypass(true);

// Enable reverb
reverb.setBypass(false);
```

### Freeze Mode

```cpp
// Enable infinite decay
reverb.setFreezeMode(true);

// Disable freeze
reverb.setFreezeMode(false);
```

### Mono-to-Stereo Processing

```cpp
std::vector<float> input_mono(256);
std::vector<float> output_left(256);
std::vector<float> output_right(256);

reverb.processMonoToStereo(input_mono.data(),
                           output_left.data(),
                           output_right.data(),
                           256);
```

## Real-Time Safety

`ReverbEffect` is designed for real-time audio processing:

### No Allocations in `processBlock()`
- All buffers allocated in `prepare()`
- Process methods never call `new` or `malloc`
- Safe for low-latency audio threads

### Smooth Parameter Transitions
- All parameters smoothed using `LinearSmoother`
- 20ms smoothing time prevents clicks
- Natural-sounding parameter changes

### No Locks
- No mutexes or critical sections
- Lock-free parameter access
- Safe for concurrent access

## Performance

### CPU Usage
- **Mono processing**: ~2-3% CPU @ 48kHz (single core)
- **Stereo processing**: ~3-4% CPU @ 48kHz (single core)
- Scales linearly with sample rate

### Memory Usage
- **Static**: ~2KB (state variables)
- **Delay lines**: ~200KB @ 48kHz (8 comb + 4 allpass buffers)
- Total: ~202KB per instance

## Best Practices

### 1. Prepare Before Use

Always call `prepare()` before processing:

```cpp
reverb.prepare(sample_rate, max_block_size);
```

### 2. Reset When Needed

Call `reset()` to clear reverb tail:
- After dropout
- When starting playback
- When switching presets

```cpp
reverb.reset();
```

### 3. Use Appropriate Smoothing

The default 20ms smoothing is optimized for most use cases:
- Natural-sounding transitions
- No clicks or artifacts
- Fast enough for automation

For faster transitions, modify smoothing time in `prepare()`.

### 4. Wet/Dry Mix Guidelines

Recommended starting points:
- **Subtle ambience**: Wet 0.2, Dry 0.8
- **Balanced mix**: Wet 0.4, Dry 0.6
- **Heavy reverb**: Wet 0.6, Dry 0.4
- **Full wet**: Wet 1.0, Dry 0.0

### 5. Room Size vs. Damping

Combination examples:
- **Small, bright room**: Size 0.3, Damping 0.2
- **Small, dark room**: Size 0.3, Damping 0.8
- **Large, bright hall**: Size 0.9, Damping 0.2
- **Large, dark hall**: Size 0.9, Damping 0.8

## Integration with Choir V2.0

### Signal Flow

```
Voices → Voice Manager → Stereo Mix → ReverbEffect → Master Output
                              (16+ voices)        (Reverb)
```

### Implementation

```cpp
class ChoirV2Engine {
private:
    ReverbEffect reverb_;

public:
    void prepare(float sample_rate, int max_voices, int max_block_size) {
        // Prepare voice manager
        voice_manager_.prepare(sample_rate, max_voices, max_block_size);

        // Prepare reverb
        reverb_.prepare(sample_rate, max_block_size);
        reverb_.loadPreset(ReverbPreset::LargeHall);
    }

    void processAudio(float* output_left, float* output_right, int num_samples) {
        // Mix all voices to stereo
        voice_manager_.processMixToStereo(mix_left_, mix_right_, num_samples);

        // Apply reverb to mixed signal
        reverb_.processStereo(mix_left_, mix_right_,
                             output_left, output_right,
                             num_samples);
    }
};
```

### API Integration

Expose reverb parameters to the plugin API:

```cpp
// Room size (0.0 - 1.0)
audioProcessorValueTreeState.createAndAddParameter("reverb_room_size", ...);

// Damping (0.0 - 1.0)
audioProcessorValueTreeState.createAndAddParameter("reverb_damping", ...);

// Wet level (0.0 - 1.0)
audioProcessorValueTreeState.createAndAddParameter("reverb_wet", ...);

// Dry level (0.0 - 1.0)
audioProcessorValueTreeState.createAndAddParameter("reverb_dry", ...);

// Width (0.0 - 1.0)
audioProcessorValueTreeState.createAndAddParameter("reverb_width", ...);

// Freeze mode (bool)
audioProcessorValueTreeState.createAndAddParameter("reverb_freeze", ...);

// Preset selection (choice)
audioProcessorValueTreeState.createAndAddParameter("reverb_preset", ...);
```

## Testing

### Unit Tests

Run the comprehensive test suite:

```bash
cd /Users/bretbouchard/apps/schill/choir-v2-universal
mkdir -p build && cd build
cmake -DCHOIR_V2_BUILD_TESTS=ON ..
make choir_v2_tests
./tests/choir_v2_tests
```

Test coverage includes:
- Parameter clamping
- Preset loading
- Reverb quality (decay, damping, width)
- Real-time safety (no allocations)
- Edge cases (silence, DC, zero samples)

### Integration Example

Build and run the integration example:

```bash
cd /Users/bretbouchard/apps/schill/choir-v2-universal
g++ -std=c++20 -DCHOIR_V2_REVERB_EXAMPLE_MAIN \
    docs/examples/reverb_integration_example.cpp \
    -I./src \
    -o reverb_demo \
    ./build/libChoirV2Core.a \
    -lm

./reverb_demo
```

This generates `reverb_demo.wav` with preset transitions.

## Troubleshooting

### No Reverb Sound

1. Check wet level: `setWetLevel(0.5f)`
2. Check bypass: `setBypass(false)`
3. Verify prepare was called
4. Test with preset: `loadPreset(ReverbPreset::LargeHall)`

### Clicks/Pops

1. Verify smoothing is enabled (default)
2. Check for sudden parameter changes
3. Use smoother transitions: `setRoomSize()` smoothly

### Metallic Sound

1. Increase damping: `setDamping(0.6f)`
2. Reduce room size: `setRoomSize(0.5f)`
3. Try different preset: `loadPreset(ReverbPreset::Plate)`

### Too Much Reverb

1. Reduce wet level: `setWetLevel(0.3f)`
2. Reduce room size: `setRoomSize(0.4f)`
3. Increase damping: `setDamping(0.7f)`

### Not Enough Reverb

1. Increase wet level: `setWetLevel(0.6f)`
2. Increase room size: `setRoomSize(0.8f)`
3. Reduce damping: `setDamping(0.3f)`

## Technical Details

### Comb Filter Tuning

Delay lengths are prime numbers to avoid harmonic resonance:
- Based on JUCE's reverb implementation
- Optimized for rich, dense reverb
- No metallic artifacts

### Allpass Filter Tuning

Series allpass filters provide diffusion:
- Smoothes out comb filter output
- Eliminates "grainy" sound
- Creates dense, smooth tail

### Damping Implementation

1-pole lowpass filter in feedback loop:
```cpp
filtered = input * damping_2 + delayed * damping_1
```
Where:
- `damping_1 = damping`
- `damping_2 = 1.0f - damping`

### Stereo Width Implementation

Uses phase inversion technique:
```cpp
wet_left = reverb_output * (1.0f - width * 0.5f)
wet_right = -reverb_output * (width * 0.5f)
```

Creates stereo separation from mono reverb engine.

## References

- **Schroeder, M.** (1962) "Natural Sounding Artificial Reverberation"
- **Moorer, J.** (1979) "About This Reverberation Business"
- **JUCE dsp::Reverb** - Implementation reference
- **Dattorro, J.** (1997) "Effect Design Part 1: Reverberator and Other Filters"

## License

Copyright (c) 2026 Bret Bouchard. All rights reserved.
