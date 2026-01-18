# ChoirV2PureDSP Parameter Quick Reference

**Quick reference for all 33 parameters in Choir V2 PureDSP**

---

## Parameter Categories

### 🎛️ Master (3 parameters)
| Parameter ID | Range | Default | Description |
|-------------|-------|---------|-------------|
| `masterVolume` | 0.0 - 1.0 | 0.7 | Master output volume |
| `stereoWidth` | 0.0 - 1.0 | 0.5 | Stereo imaging width |
| `polyphony` | 1 - 128 | 64 | Max simultaneous voices |

### 🎤 Vowel Control (3 parameters)
| Parameter ID | Range | Default | Description |
|-------------|-------|---------|-------------|
| `vowelX` | 0.0 - 1.0 | 0.5 | Front/Back vowel position |
| `vowelY` | 0.0 - 1.0 | 0.5 | Open/Closed vowel position |
| `vowelZ` | 0.0 - 1.0 | 0.5 | Rounded/Spread vowel position |

### 🎚️ Formants (2 parameters)
| Parameter ID | Range | Default | Description |
|-------------|-------|---------|-------------|
| `formantScale` | 0.5 - 2.0 | 1.0 | Formant frequency scaling |
| `formantShift` | -12.0 - +12.0 | 0.0 | Formant shift (semitones) |

### 💨 Breath (2 parameters)
| Parameter ID | Range | Default | Description |
|-------------|-------|---------|-------------|
| `breathMix` | 0.0 - 1.0 | 0.3 | Breath noise mix |
| `breathColor` | 0.0 - 1.0 | 0.5 | Breath noise color |

### 🎵 Vibrato (3 parameters)
| Parameter ID | Range | Default | Description |
|-------------|-------|---------|-------------|
| `vibratoRate` | 1.0 - 15.0 Hz | 5.0 | Vibrato speed (Hz) |
| `vibratoDepth` | 0.0 - 1.0 | 0.5 | Vibrato depth |
| `vibratoDelay` | 0.0 - 1.0 | 0.1 | Vibrato onset delay (sec) |

### 👥 Ensemble (3 parameters)
| Parameter ID | Range | Default | Description |
|-------------|-------|---------|-------------|
| `tightness` | 0.0 - 1.0 | 0.5 | Ensemble timing tightness |
| `ensembleSize` | 0.0 - 1.0 | 0.5 | Number of voices per note |
| `voiceSpread` | 0.0 - 1.0 | 0.5 | Stereo spread of voices |

### 📈 Envelope - ADSR (4 parameters)
| Parameter ID | Range | Default | Description |
|-------------|-------|---------|-------------|
| `attack` | 0.001 - 5.0 sec | 0.1 | Attack time |
| `decay` | 0.001 - 5.0 sec | 0.2 | Decay time |
| `sustain` | 0.0 - 1.0 | 0.7 | Sustain level |
| `release` | 0.001 - 10.0 sec | 0.3 | Release time |

### 🎼 SATB Blend (4 parameters)
| Parameter ID | Range | Default | Description |
|-------------|-------|---------|-------------|
| `sopranoLevel` | 0.0 - 1.0 | 1.0 | Soprano section level |
| `altoLevel` | 0.0 - 1.0 | 0.8 | Alto section level |
| `tenorLevel` | 0.0 - 1.0 | 0.6 | Tenor section level |
| `bassLevel` | 0.0 - 1.0 | 0.4 | Bass section level |

### 🌊 Effects - Reverb (3 parameters)
| Parameter ID | Range | Default | Description |
|-------------|-------|---------|-------------|
| `reverbMix` | 0.0 - 1.0 | 0.3 | Reverb wet/dry mix |
| `reverbDecay` | 0.1 - 10.0 sec | 2.0 | Reverb decay time |
| `reverbPredelay` | 0.0 - 0.1 sec | 0.02 | Reverb predelay |

### ✨ Spectral Enhancement (2 parameters)
| Parameter ID | Range | Default | Description |
|-------------|-------|---------|-------------|
| `spectralEnhancement` | 0.0 - 1.0 | 0.5 | Spectral enhancement amount |
| `harmonicsBoost` | 0.0 - 1.0 | 0.5 | High harmonics boost |

### 🔊 Subharmonic Generation (2 parameters)
| Parameter ID | Range | Default | Description |
|-------------|-------|---------|-------------|
| `subharmonicMix` | 0.0 - 1.0 | 0.0 | Subharmonic mix |
| `subharmonicDepth` | 0.0 - 1.0 | 0.5 | Subharmonic depth |

### 🔀 Diphone/Coarticulation (2 parameters)
| Parameter ID | Range | Default | Description |
|-------------|-------|---------|-------------|
| `coarticulationAmount` | 0.0 - 1.0 | 0.5 | Coarticulation smoothing |
| `transitionSpeed` | 0.0 - 1.0 | 0.5 | Diphone transition speed |

---

## Usage Examples

### Reading Parameters
```cpp
ChoirV2PureDSP dsp;

// Get master volume
float volume = dsp.getParameter(ChoirV2Parameters::MASTER_VOLUME);
// Returns: 0.7f

// Get vibrato rate
float vibRate = dsp.getParameter(ChoirV2Parameters::VIBRATO_RATE);
// Returns: 5.0f

// Get formant shift
float shift = dsp.getParameter(ChoirV2Parameters::FORMANT_SHIFT);
// Returns: 0.0f (center = no shift)
```

### Setting Parameters
```cpp
ChoirV2PureDSP dsp;

// Set master volume to 80%
dsp.setParameter(ChoirV2Parameters::MASTER_VOLUME, 0.8f);

// Set vibrato rate to 7 Hz
dsp.setParameter(ChoirV2Parameters::VIBRATO_RATE, 7.0f);

// Set formant shift up 3 semitones
dsp.setParameter(ChoirV2Parameters::FORMANT_SHIFT, 3.0f);

// Set polyphony to 32 voices
dsp.setParameter(ChoirV2Parameters::POLPHONY, 32.0f);
```

### Automatic Value Clamping
```cpp
// All parameters are automatically clamped to valid ranges
dsp.setParameter(ChoirV2Parameters::MASTER_VOLUME, 1.5f);  // Over range
// Clamped to 1.0f

dsp.setParameter(ChoirV2Parameters::MASTER_VOLUME, -0.2f); // Under range
// Clamped to 0.0f

dsp.setParameter(ChoirV2Parameters::VIBRATO_RATE, 20.0f); // Over max (15.0 Hz)
// Clamped to 15.0f
```

---

## Preset Management

### Saving Presets
```cpp
ChoirV2PureDSP dsp;

// Configure parameters
dsp.setParameter(ChoirV2Parameters::VIBRATO_RATE, 6.5f);
dsp.setParameter(ChoirV2Parameters::REVERB_MIX, 0.5f);
// ... configure more parameters ...

// Save to JSON buffer
char jsonBuffer[4096];
bool success = dsp.savePreset(jsonBuffer, 4096);

if (success) {
    // jsonBuffer contains complete preset
    // Save to file, send over network, etc.
}
```

### Loading Presets
```cpp
ChoirV2PureDSP dsp;

// Load from JSON string
const char* jsonPreset = "{\"masterVolume\":0.8,\"vibratoRate\":6.5,...}";
bool success = dsp.loadPreset(jsonPreset);

if (success) {
    // All parameters restored from preset
    // Unknown/missing parameters keep current values
}
```

---

## Parameter ID Constants

All parameter IDs are defined in `ChoirV2PureDSP.h`:

```cpp
namespace ChoirV2Parameters {
    constexpr const char* MASTER_VOLUME = "masterVolume";
    constexpr const char* STEREO_WIDTH = "stereoWidth";
    constexpr const char* POLPHONY = "polyphony";
    // ... etc for all 33 parameters
}
```

Use these constants instead of string literals for type safety:

```cpp
// ✅ Good - uses constant
dsp.setParameter(ChoirV2Parameters::VIBRATO_RATE, 5.0f);

// ❌ Bad - hard-coded string (typos not caught at compile time)
dsp.setParameter("vibratoRate", 5.0f);
```

---

## Common Preset Configurations

### 🎵 Ethereal Choir
```cpp
dsp.setParameter(ChoirV2Parameters::REVERB_MIX, 0.6f);
dsp.setParameter(ChoirV2Parameters::REVERB_DECAY, 4.0f);
dsp.setParameter(ChoirV2Parameters::SUSTAIN, 0.8f);
dsp.setParameter(ChoirV2Parameters::RELEASE, 2.0f);
dsp.setParameter(ChoirV2Parameters::ENSEMBLE_SIZE, 0.8f);
dsp.setParameter(ChoirV2Parameters::BREATH_MIX, 0.4f);
```

### 🎤 Close Microphone
```cpp
dsp.setParameter(ChoirV2Parameters::REVERB_MIX, 0.1f);
dsp.setParameter(ChoirV2Parameters::BREATH_MIX, 0.6f);
dsp.setParameter(ChoirV2Parameters::STEREO_WIDTH, 0.3f);
dsp.setParameter(ChoirV2Parameters::SPECTRAL_ENHANCEMENT, 0.7f);
```

### 👶 Children's Choir
```cpp
dsp.setParameter(ChoirV2Parameters::FORMANT_SHIFT, 4.0f);  // Shift up
dsp.setParameter(ChoirV2Parameters::ENSEMBLE_SIZE, 0.4f);  // Smaller ensemble
dsp.setParameter(ChoirV2Parameters::VIBRATO_RATE, 6.0f);   // Faster vibrato
dsp.setParameter(ChoirV2Parameters::BREATH_MIX, 0.2f);     // Less breath
```

### 🎻 Gospel Choir
```cpp
dsp.setParameter(ChoirV2Parameters::VIBRATO_DEPTH, 0.7f);
dsp.setParameter(ChoirV2Parameters::VIBRATO_RATE, 5.5f);
dsp.setParameter(ChoirV2Parameters::SUBHARMONIC_MIX, 0.4f);
dsp.setParameter(ChoirV2Parameters::REVERB_MIX, 0.5f);
dsp.setParameter(ChoirV2Parameters::ENSEMBLE_SIZE, 0.9f);  // Large ensemble
```

---

## Thread Safety

### ✅ Thread-Safe Operations
- `getParameter()` - Lock-free read
- `setParameter()` - Atomic write

### ⚠️ Audio Thread Safety
```cpp
// ✅ Safe - called from audio thread
float vol = dsp.getParameter(ChoirV2Parameters::MASTER_VOLUME);

// ✅ Safe - called from any thread
dsp.setParameter(ChoirV2Parameters::MASTER_VOLUME, 0.8f);

// ❌ Avoid - Dynamic allocation in audio thread
char* buffer = new char[4096];  // DON'T DO THIS in audio thread
dsp.savePreset(buffer, 4096);
```

**Best Practice**: Call `savePreset()` from GUI thread, not audio thread.

---

## Performance Notes

| Operation | Complexity | Time (approx) |
|-----------|-----------|---------------|
| `getParameter()` | O(1) | < 1 μs |
| `setParameter()` | O(1) | < 1 μs |
| `savePreset()` | O(n) | < 100 μs (n=33) |
| `loadPreset()` | O(n*m) | < 500 μs (n=33, m=len) |

**All operations are real-time safe** (no dynamic allocation, no blocking).

---

## Error Handling

### Invalid Parameter ID
```cpp
float value = dsp.getParameter("unknownParameter");
// Returns: 0.0f (graceful fallback)

dsp.setParameter("unknownParameter", 0.5f);
// Silently ignored (no error thrown)
```

### Buffer Too Small
```cpp
char buffer[10];
bool success = dsp.savePreset(buffer, 10);
// Returns: false (buffer too small for full preset)
```

### Invalid JSON
```cpp
const char* badJson = "{not valid json}";
bool success = dsp.loadPreset(badJson);
// Returns: true (graceful - parameters unchanged if not found)
```

---

## Summary

- **33 parameters** total
- **7 categories** (Master, Vowel, Formants, Breath, Vibrato, Ensemble, ADSR, SATB, Effects, Spectral, Subharmonic, Diphone)
- **Thread-safe** operations
- **Real-time safe** (no allocation in audio thread)
- **Automatic clamping** to valid ranges
- **JSON preset format** for save/load
- **Backward compatible** with Choral v1

**Quick Test**:
```cpp
ChoirV2PureDSP dsp;
assert(dsp.getParameter(ChoirV2Parameters::MASTER_VOLUME) == 0.7f);  // Default
dsp.setParameter(ChoirV2Parameters::MASTER_VOLUME, 0.9f);
assert(dsp.getParameter(ChoirV2Parameters::MASTER_VOLUME) == 0.9f);  // Updated
```
