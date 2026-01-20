# Choir V2.0 Factory Presets

## Overview

This directory contains factory presets for Choir V2.0, a next-generation choir synthesizer featuring PureDSP technology with FFT-based spectral enhancement.

## Factory Presets

### 1. Cathedral Choir
**Large cathedral ensemble with rich reverb and deep subharmonics**
- **Character**: Grand, spacious, atmospheric
- **Best For**: Epic soundtracks, ambient music, cinematic scoring
- **Key Features**: 40 voices, 75% reverb mix, strong subharmonics (65%), rich spectral enhancement

### 2. Chamber Choir
**Intimate chamber ensemble with dry, focused sound**
- **Character**: Intimate, clear, classical
- **Best For**: Classical music, chamber arrangements, focused vocal lines
- **Key Features**: 24 voices, minimal reverb (15%), tight ensemble spread (35%), clear formants

### 3. Gospel Choir
**Energetic gospel ensemble with strong vibrato and rich harmonics**
- **Character**: Energetic, soulful, expressive
- **Best For**: Gospel music, soul arrangements, energetic compositions
- **Key Features**: Strong vibrato (25% depth), 32 voices, boosted spectral enhancement (60%)

### 4. Ethereal Choir
**Soft, breathy, and atmospheric - perfect for ambient music**
- **Character**: Soft, atmospheric, ambient
- **Best For**: Ambient music, soundscapes, meditation, relaxation
- **Key Features**: Slow attack (0.6s), long release (2.0s), heavy reverb (85%), breathy texture

### 5. Synth Choir
**Synthetic choir with boosted harmonics and modern electronic sound**
- **Character**: Electronic, bright, modern
- **Best For**: Electronic music, pop production, modern arrangements
- **Key Features**: High spectral enhancement (80%), bright harmonics (80% focus), fast attack (0.15s)

## Preset Format

All presets are in JSON format with the following structure:

```json
{
  "presetName": "Preset Name",
  "author": "Author Name",
  "description": "Preset description",
  "tags": ["tag1", "tag2", "tag3"],
  "parameters": {
    "masterVolume": 0.0-1.0,
    "polyphony": 1-40,
    "ensembleSpread": 0.0-1.0,
    // ... 40+ parameters
  }
}
```

## Parameter Categories

### Master
- `masterVolume`: Overall output level (0.0-1.0)
- `polyphony`: Maximum voices (1-40)
- `ensembleSpread`: Stereo spread (0.0-1.0)

### Voice Sections
- `voiceSectionSoprano`: Soprano level (0.0-1.0)
- `voiceSectionAlto`: Alto level (0.0-1.0)
- `voiceSectionTenor`: Tenor level (0.0-1.0)
- `voiceSectionBass`: Bass level (0.0-1.0)

### Vowels
- `vowelA`, `vowelE`, `vowelI`, `vowelO`, `vowelU`: Vowel mixing (0.0-1.0)

### Formants & Vibrato
- `formantShift`: Formant frequency shift (0.0-1.0)
- `vibratoRate`: Vibrato speed (Hz)
- `vibratoDepth`: Vibrato intensity (0.0-1.0)
- `vibratoDelay`: Vibrato onset (seconds)

### Envelope (ADSR)
- `attack`: Attack time (seconds)
- `decay`: Decay time (seconds)
- `sustain`: Sustain level (0.0-1.0)
- `release`: Release time (seconds)

### Subharmonic Generation
- `subharmonicMix`: Subharmonic level (0.0-1.0)
- `subharmonicOctave`: Octave shift (0.0-1.0)
- `pllEnabled`: PLL tracking (boolean)
- `pllSensitivity`: PLL sensitivity (0.0-1.0)

### Spectral Enhancement
- `spectralEnhancement`: Enhancement amount (0.0-1.0)
- `harmonicFocus`: Frequency focus (0.0-1.0)
- `bassEnhancement`: Bass boost (0.0-1.0)

### Reverb
- `reverbMix`: Reverb wet/dry (0.0-1.0)
- `reverbSize`: Room size (0.0-1.0)
- `reverbDecay`: Decay time (0.0-1.0)
- `reverbPredelay`: Pre-delay (0.0-1.0)
- `reverbLowCut`: Low frequency damping (0.0-1.0)
- `reverbHighCut`: High frequency damping (0.0-1.0)

### Effects
- `breathNoise`: Breath noise level (0.0-1.0)
- `noiseFilter`: Noise filter frequency (0.0-1.0)
- `stereoWidth`: Stereo width (0.0-1.0)
- `detune`: Detune amount (0.0-1.0)

## Loading Presets

Presets can be loaded via:

1. **JUCE Preset Browser**: Built-in preset menu in the plugin
2. **State File Loading**: Load `.preset.json` files directly
3. **XML Import**: JUCE state XML format supported

## Creating Custom Presets

To create custom presets:

1. Copy an existing preset as a template
2. Adjust parameters to taste
3. Save as `[Name].preset.json` in this directory
4. Tag with relevant categories for easy browsing

## Tips for Preset Design

1. **Start with the vowel** - Set vowel mix first, then adjust other parameters
2. **Use ensemble spread** - Controls stereo width and perceived size
3. **Layer voice sections** - Mix soprano/alto/tenor/bass for different textures
4. **Subharmonics add depth** - Use 30-50% for natural warmth, 60%+ for electronic sounds
5. **Spectral enhancement adds clarity** - Use 40-60% for presence, 70%+ for brightness
6. **Reverb creates space** - Match reverb to intended acoustic environment
7. **Vibrato adds expression** - Faster rate = more energy, deeper = more intensity

## Technical Notes

- All presets are compatible with PureDSP FFT implementation
- Spectral enhancement uses real FFT processing (2048-point)
- Subharmonic generation uses PLL-based pitch tracking
- Formants use biquad filter banks
- Reverb uses algorithmic reverb (no convolution)

## Version

- **Choir V2.0**: PureDSP implementation with FFT spectral enhancement
- **PureDSP**: JUCE-free DSP library
- **Format**: JSON-based preset system
- **Compatibility**: All plugin formats (VST3, AU, CLAP, LV2, AUv3, Standalone)

---

**Author**: Bret Bouchard
**Version**: 2.0.0
**Last Updated**: January 2025
