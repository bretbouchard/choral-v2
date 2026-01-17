#!/usr/bin/env python3
"""
Additional language presets to reach 50 total
"""

import json
import os
from datetime import datetime

def create_preset(name, description, language, synthesis_method, lyrics,
                 num_voices=8, formant_mix=80, subharmonic_mix=20,
                 stereo_width=75, vibrato_rate=6.0, vibrato_depth=30.0,
                 reverb_mix=25, reverb_size=50, attack=50, release=200,
                 pitch_variation=10, timing_variation=5, formant_variation=15,
                 breathiness=10, warmth=20, brightness=50, tags=None):
    """Create a preset dictionary"""

    preset = {
        "metadata": {
            "name": name,
            "author": "Bret Bouchard",
            "description": description,
            "version": "2.0.0",
            "category": "Factory",
            "tags": tags or [],
            "created_date": datetime.now().isoformat(),
            "modified_date": datetime.now().isoformat(),
            "plugin_version": "2.0.0"
        },
        "parameters": {
            "num_voices": num_voices,
            "master_gain": -3.0,
            "language": language,
            "lyrics": lyrics,
            "synthesis_method": synthesis_method,
            "formant_mix": formant_mix,
            "subharmonic_mix": subharmonic_mix,
            "stereo_width": stereo_width,
            "vibrato_rate": vibrato_rate,
            "vibrato_depth": vibrato_depth,
            "reverb_mix": reverb_mix,
            "reverb_size": reverb_size,
            "attack_time": attack,
            "release_time": release,
            "enable_anti_aliasing": True,
            "enable_spectral_enhancement": True,
            "oversampling_factor": 1.0,
            "pitch_variation": pitch_variation,
            "timing_variation": timing_variation,
            "formant_variation": formant_variation,
            "breathiness": breathiness,
            "warmth": warmth,
            "brightness": brightness
        },
        "is_factory": True,
        "is_read_only": True
    }
    return preset

def save_preset(preset, directory):
    """Save preset to JSON file"""
    filename = f"{preset['metadata']['name'].replace(' ', '_').replace('/', '_')}.choirv2"
    filepath = os.path.join(directory, filename)

    with open(filepath, 'w') as f:
        json.dump(preset, f, indent=2)

    print(f"✓ Created: {preset['metadata']['name']}")

# ============================================================================
# ADDITIONAL LANGUAGE PRESETS (7 more to reach 50)
# ============================================================================

additional_presets = [
    # ARABIC (1 preset) - Real language
    create_preset(
        "Arabic 1: Quran Recitation",
        "Traditional Quranic recitation style",
        "arabic",
        "formant",
        "ah eh ee oh oo",
        num_voices=4,
        formant_mix=100,
        subharmonic_mix=0,
        stereo_width=50,
        vibrato_rate=5.0,
        vibrato_depth=18,
        reverb_mix=35,
        reverb_size=60,
        attack=70,
        release=300,
        pitch_variation=5,
        timing_variation=3,
        breathiness=12,
        warmth=55,
        brightness=45,
        tags=["arabic", "quran", "recitation", "traditional", "islamic"]
    ),

    # HEBREW (1 preset) - Real language
    create_preset(
        "Hebrew 1: Synagogue Chant",
        "Traditional Jewish synagogue chanting",
        "hebrew",
        "formant",
        "ah eh ee oh",
        num_voices=8,
        formant_mix=98,
        subharmonic_mix=2,
        stereo_width=60,
        vibrato_rate=4.8,
        vibrato_depth=16,
        reverb_mix=32,
        reverb_size=55,
        attack=85,
        release=320,
        pitch_variation=6,
        timing_variation=4,
        breathiness=14,
        warmth=52,
        brightness=40,
        tags=["hebrew", "synagogue", "jewish", "chant", "traditional"]
    ),

    # SANSKRIT/HINDI (1 preset) - Real language
    create_preset(
        "Sanskrit 1: Vedic Chant",
        "Ancient Vedic mantra chanting",
        "hindi",
        "formant",
        "ah eh ih oh oo",
        num_voices=6,
        formant_mix=100,
        subharmonic_mix=0,
        stereo_width=55,
        vibrato_rate=4.5,
        vibrato_depth=12,
        reverb_mix=38,
        reverb_size=65,
        attack=95,
        release=380,
        pitch_variation=4,
        timing_variation=3,
        breathiness=16,
        warmth=58,
        brightness=38,
        tags=["hindi", "sanskrit", "vedic", "chant", "mantra", "meditation"]
    ),

    # SPANISH (1 preset) - Real language
    create_preset(
        "Spanish 1: Flamenco Choir",
        "Traditional flamenco cantaor voices",
        "spanish",
        "diphone",
        "ah-eh ee-oh oh-ay",
        num_voices=6,
        formant_mix=94,
        subharmonic_mix=6,
        stereo_width=68,
        vibrato_rate=7.2,
        vibrato_depth=32,
        reverb_mix=15,
        reverb_size=30,
        attack=20,
        release=140,
        pitch_variation=10,
        timing_variation=5,
        breathiness=8,
        warmth=48,
        brightness=65,
        tags=["spanish", "flamenco", "cantaor", "traditional", "passionate"]
    ),

    # FRENCH (1 preset) - Real language
    create_preset(
        "French 1: Chorale Francaise",
        "French romantic chorale style",
        "french",
        "formant",
        "ah eh ee oh oo",
        num_voices=24,
        formant_mix=96,
        subharmonic_mix=4,
        stereo_width=78,
        vibrato_rate=5.8,
        vibrato_depth=24,
        reverb_mix=28,
        reverb_size=48,
        attack=48,
        release=210,
        pitch_variation=11,
        timing_variation=5,
        breathiness=11,
        warmth=46,
        brightness=58,
        tags=["french", "chorale", "romantic", "ensemble"]
    ),

    # GERMAN (1 preset) - Real language
    create_preset(
        "German 1: Lieder Choir",
        "German lied art song style",
        "german",
        "diphone",
        "ah-eh ee-oh oh-oo",
        num_voices=12,
        formant_mix=93,
        subharmonic_mix=7,
        stereo_width=72,
        vibrato_rate=6.3,
        vibrato_depth=26,
        reverb_mix=22,
        reverb_size=42,
        attack=42,
        release=190,
        pitch_variation=9,
        timing_variation=4,
        breathiness=9,
        warmth=44,
        brightness=62,
        tags=["german", "lieder", "art-song", "romantic"]
    ),

    # RUSSIAN (1 preset) - Real language
    create_preset(
        "Russian 1: Orthodox Choir",
        "Russian Orthodox church choir",
        "russian",
        "formant",
        "ah eh ee oh oo",
        num_voices=16,
        formant_mix=100,
        subharmonic_mix=0,
        stereo_width=65,
        vibrato_rate=4.2,
        vibrato_depth=14,
        reverb_mix=42,
        reverb_size=75,
        attack=90,
        release=360,
        pitch_variation=5,
        timing_variation=3,
        breathiness=10,
        warmth=62,
        brightness=38,
        tags=["russian", "orthodox", "church", "choir", "traditional"]
    ),
]

def main():
    preset_dir = "/Users/bretbouchard/apps/schill/choir-v2-universal/presets/factory"

    print("\nAdding Additional Language Presets...")
    print("=" * 60)

    for preset in additional_presets:
        save_preset(preset, preset_dir)

    print("=" * 60)
    print(f"\n✓ Added {len(additional_presets)} additional presets!")

if __name__ == "__main__":
    main()
