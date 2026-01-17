# ReverbEffect API Quick Reference

## Constructor

```cpp
ReverbEffect();
```
Creates a new reverb effect instance. Call `prepare()` before processing.

## Initialization

### prepare()
```cpp
void prepare(float sample_rate, int max_block_size);
```
Initialize the reverb engine.
- **sample_rate**: Sample rate in Hz (e.g., 48000.0f)
- **max_block_size**: Maximum samples per process call (e.g., 256)

### reset()
```cpp
void reset();
```
Clear all delay lines and reset smoothers. Call after dropout or when switching presets.

## Parameter Control

### setRoomSize()
```cpp
void setRoomSize(float size);
```
- **size**: 0.0 (small) to 1.0 (large)

### setDamping()
```cpp
void setDamping(float damping);
```
- **damping**: 0.0 (bright) to 1.0 (dark)

### setWetLevel()
```cpp
void setWetLevel(float wet);
```
- **wet**: 0.0 (dry) to 1.0 (fully wet)

### setDryLevel()
```cpp
void setDryLevel(float dry);
```
- **dry**: 0.0 (muted) to 1.0 (full dry)

### setWidth()
```cpp
void setWidth(float width);
```
- **width**: 0.0 (mono) to 1.0 (full stereo)

### setFreezeMode()
```cpp
void setFreezeMode(bool freeze);
```
- **freeze**: true for infinite decay, false for normal

### setBypass()
```cpp
void setBypass(bool bypass);
```
- **bypass**: true to bypass (dry only), false to process

## Presets

### loadPreset()
```cpp
void loadPreset(ReverbPreset preset);
```
Load a preset configuration instantly.

**Available presets:**
- `ReverbPreset::SmallRoom` - Intimate space (0.3 size, 0.5 damping)
- `ReverbPreset::LargeHall` - Concert hall (0.7 size, 0.4 damping)
- `ReverbPreset::Cathedral` - Massive space (0.9 size, 0.3 damping)
- `ReverbPreset::Plate` - Classic plate (0.5 size, 0.6 damping)
- `ReverbPreset::Custom` - User-defined (no change)

## Processing

### processMonoToStereo()
```cpp
void processMonoToStereo(const float* input,
                         float* output_left,
                         float* output_right,
                         int num_samples);
```
Process mono input to stereo output.
- **input**: Mono input buffer [num_samples]
- **output_left**: Left output buffer [num_samples]
- **output_right**: Right output buffer [num_samples]
- **num_samples**: Number of samples to process

### processStereo()
```cpp
void processStereo(const float* input_left,
                   const float* input_right,
                   float* output_left,
                   float* output_right,
                   int num_samples);
```
Process stereo input to stereo output.
- **input_left**: Left input buffer [num_samples]
- **input_right**: Right input buffer [num_samples]
- **output_left**: Left output buffer [num_samples]
- **output_right**: Right output buffer [num_samples]
- **num_samples**: Number of samples to process

## Query

### getParameters()
```cpp
std::array<float, 5> getParameters() const;
```
Returns current parameters as:
- `[0]`: roomSize
- `[1]`: damping
- `[2]`: wetLevel
- `[3]`: dryLevel
- `[4]`: width

### isBypassed()
```cpp
bool isBypassed() const;
```
Returns true if reverb is bypassed.

## Usage Examples

### Basic Setup
```cpp
ReverbEffect reverb;
reverb.prepare(48000.0f, 256);
reverb.loadPreset(ReverbPreset::LargeHall);
```

### Process Audio
```cpp
float input[256];
float output_left[256];
float output_right[256];

// Fill input...
reverb.processMonoToStereo(input, output_left, output_right, 256);
```

### Real-Time Control
```cpp
// Smooth parameter changes
reverb.setRoomSize(0.8f);
reverb.setDamping(0.3f);
reverb.setWetLevel(0.5f);
reverb.setDryLevel(0.5f);
```

### Bypass
```cpp
// Enable bypass (dry only)
reverb.setBypass(true);

// Disable bypass (process)
reverb.setBypass(false);
```

### Freeze Mode
```cpp
// Enable infinite decay
reverb.setFreezeMode(true);

// Disable freeze
reverb.setFreezeMode(false);
```

## Preset Quick Reference

| Preset | Size | Damping | Wet | Dry | Width | Best For |
|--------|------|---------|-----|-----|-------|----------|
| SmallRoom | 0.3 | 0.5 | 0.2 | 0.8 | 0.5 | Intimate ambience |
| LargeHall | 0.7 | 0.4 | 0.4 | 0.6 | 0.8 | Concert sound |
| Cathedral | 0.9 | 0.3 | 0.5 | 0.5 | 1.0 | Epic spaces |
| Plate | 0.5 | 0.6 | 0.3 | 0.7 | 0.7 | Smooth decay |

## Performance Notes

- **CPU**: 2-4% @ 48kHz (single core)
- **Memory**: ~202KB per instance
- **Latency**: Zero (no lookahead)
- **Smoothing**: 20ms (prevents clicks)

## Best Practices

1. **Always prepare before processing**
   ```cpp
   reverb.prepare(sample_rate, max_block_size);
   ```

2. **Reset when needed**
   ```cpp
   reverb.reset();  // Clear reverb tail
   ```

3. **Use balanced wet/dry mix**
   ```cpp
   reverb.setWetLevel(0.4f);
   reverb.setDryLevel(0.6f);
   ```

4. **Adjust room size and damping together**
   ```cpp
   // Small, bright room
   reverb.setRoomSize(0.3f);
   reverb.setDamping(0.2f);

   // Large, dark hall
   reverb.setRoomSize(0.9f);
   reverb.setDamping(0.7f);
   ```

5. **Use bypass for A/B comparison**
   ```cpp
   reverb.setBypass(true);   // Dry
   reverb.setBypass(false);  // Wet
   ```

## Thread Safety

- **Parameter updates**: Thread-safe (atomic)
- **Processing**: Not thread-safe (use single audio thread)
- **prepare/reset**: Not thread-safe (call before processing)

## Error Handling

The reverb does not throw exceptions. Invalid parameters are clamped:
- Values < 0.0 → clamped to 0.0
- Values > 1.0 → clamped to 1.0
- Null pointers → undefined behavior (ensure valid buffers)

## See Also

- [Full Documentation](./ReverbEffect.md)
- [Integration Example](../examples/reverb_integration_example.cpp)
- [Unit Tests](../../tests/unit/test_reverb_effect.cpp)
