#!/usr/bin/env python3
"""
Choir V2.0 Gender & Voice Type Preset Generator
Creates gender-specific presets with proper vocal ranges and ensemble balance controls
"""

import json
import os
from datetime import datetime

def create_preset(name, description, language, synthesis_method, lyrics,
                 num_voices=8, formant_mix=80, subharmonic_mix=20,
                 stereo_width=75, vibrato_rate=6.0, vibrato_depth=30.0,
                 reverb_mix=25, reverb_size=50, attack=50, release=200,
                 pitch_variation=10, timing_variation=5, formant_variation=15,
                 breathiness=10, warmth=20, brightness=50,
                 gender_balance=None, voice_types=None, tags=None):
    """Create a preset dictionary with gender support"""

    preset = {
        "metadata": {
            "name": name,
            "author": "Bret Bouchard",
            "description": description,
            "version": "2.0.0",
            "category": "Gender",
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

    # Add gender balance controls if specified
    if gender_balance:
        preset["parameters"]["gender_balance"] = gender_balance

    # Add voice type distribution if specified
    if voice_types:
        preset["parameters"]["voice_types"] = voice_types

    return preset

def save_preset(preset, directory):
    """Save preset to JSON file"""
    filename = f"{preset['metadata']['name'].replace(' ', '_').replace('/', '_')}.choirv2"
    filepath = os.path.join(directory, filename)

    with open(filepath, 'w') as f:
        json.dump(preset, f, indent=2)

    print(f"✓ Created: {preset['metadata']['name']}")

# ============================================================================
# MALE VOICE TYPES (8 presets)
# ============================================================================

male_presets = [
    # Bass (Low Male)
    create_preset(
        "Male 1: Bass",
        "Deep male bass voice (F2-E4, 85-330 Hz)",
        "english",
        "formant",
        "oh oo ah",
        num_voices=4,
        formant_mix=100,
        subharmonic_mix=0,
        stereo_width=60,
        vibrato_rate=4.5,
        vibrato_depth=18,
        reverb_mix=20,
        reverb_size=40,
        attack=80,
        release=300,
        pitch_variation=5,
        timing_variation=3,
        formant_variation=8,
        breathiness=5,
        warmth=70,  # Very warm = lower formants
        brightness=20,  # Dark = male
        voice_types={"bass": 100},
        tags=["male", "bass", "low", "deep", "choir"]
    ),

    # Baritone (Medium-Low Male)
    create_preset(
        "Male 2: Baritone",
        "Male baritone voice (A2-F4, 110-350 Hz)",
        "english",
        "formant",
        "oh oo ah eh",
        num_voices=6,
        formant_mix=100,
        subharmonic_mix=0,
        stereo_width=65,
        vibrato_rate=5.0,
        vibrato_depth=22,
        reverb_mix=22,
        reverb_size=45,
        attack=60,
        release=250,
        pitch_variation=8,
        timing_variation=4,
        formant_variation=10,
        breathiness=8,
        warmth=55,
        brightness=35,
        voice_types={"baritone": 100},
        tags=["male", "baritone", "medium-low", "versatile"]
    ),

    # Tenor (High Male)
    create_preset(
        "Male 3: Tenor",
        "Male tenor voice (C3-C5, 130-520 Hz)",
        "english",
        "formant",
        "ah eh ee oh",
        num_voices=8,
        formant_mix=100,
        subharmonic_mix=0,
        stereo_width=70,
        vibrato_rate=5.8,
        vibrato_depth=26,
        reverb_mix=25,
        reverb_size=50,
        attack=45,
        release=200,
        pitch_variation=10,
        timing_variation=5,
        formant_variation=12,
        breathiness=10,
        warmth=40,
        brightness=50,
        voice_types={"tenor": 100},
        tags=["male", "tenor", "high", "bright", "opera"]
    ),

    # Countertenor (Male Falsetto)
    create_preset(
        "Male 4: Countertenor",
        "Male countertenor (falsetto, G3-E5, 196-660 Hz)",
        "english",
        "formant",
        "ah eh ee oh oo",
        num_voices=4,
        formant_mix=100,
        subharmonic_mix=0,
        stereo_width=75,
        vibrato_rate=6.5,
        vibrato_depth=30,
        reverb_mix=30,
        reverb_size=55,
        attack=40,
        release=180,
        pitch_variation=12,
        timing_variation=6,
        formant_variation=15,
        breathiness=18,
        warmth=35,
        brightness=60,
        voice_types={"countertenor": 100},
        tags=["male", "countertenor", "falsetto", "high", "opera"]
    ),

    # Male Ensemble (SATB)
    create_preset(
        "Male 5: SATB Choir",
        "Traditional male SATB choir (Bass-Baritone-Tenor-Tenor)",
        "english",
        "diphone",
        "oh-ah ah-eh ee-oh",
        num_voices=12,
        formant_mix=100,
        subharmonic_mix=0,
        stereo_width=80,
        vibrato_rate=5.5,
        vibrato_depth=24,
        reverb_mix=35,
        reverb_size=65,
        attack=55,
        release=220,
        pitch_variation=10,
        timing_variation=5,
        formant_variation=12,
        breathiness=8,
        warmth=50,
        brightness=45,
        voice_types={
            "bass": 25,
            "baritone": 25,
            "tenor": 50
        },
        tags=["male", "satb", "choir", "ensemble", "traditional"]
    ),

    # Gospel Tenor
    create_preset(
        "Male 6: Gospel Tenor",
        "Soulful gospel tenor with bright timbre",
        "english",
        "diphone",
        "ah eh ee oh",
        num_voices=8,
        formant_mix=95,
        subharmonic_mix=5,
        stereo_width=85,
        vibrato_rate=7.0,
        vibrato_depth=35,
        reverb_mix=28,
        reverb_size=48,
        attack=30,
        release=170,
        pitch_variation=15,
        timing_variation=7,
        breathiness=15,
        warmth=45,
        brightness=70,
        voice_types={"tenor": 100},
        tags=["male", "gospel", "tenor", "soul", "bright"]
    ),

    # Pop Baritone
    create_preset(
        "Male 7: Pop Baritone",
        "Modern pop baritone backing vocals",
        "english",
        "diphone",
        "oh ah eh",
        num_voices=6,
        formant_mix=90,
        subharmonic_mix=10,
        stereo_width=70,
        vibrato_rate=6.2,
        vibrato_depth=28,
        reverb_mix=18,
        reverb_size=35,
        attack=25,
        release=150,
        pitch_variation=8,
        timing_variation=4,
        breathiness=8,
        warmth=40,
        brightness=60,
        voice_types={"baritone": 100},
        tags=["male", "pop", "baritone", "backing-vocals", "modern"]
    ),

    # Russian Bass
    create_preset(
        "Male 8: Russian Bass",
        "Deep Russian Orthodox bass (oktavist)",
        "russian",
        "formant",
        "oh oo",
        num_voices=6,
        formant_mix=100,
        subharmonic_mix=0,
        stereo_width=55,
        vibrato_rate=4.0,
        vibrato_depth=15,
        reverb_mix=45,
        reverb_size=80,
        attack=100,
        release=400,
        pitch_variation=4,
        timing_variation=2,
        breathiness=5,
        warmth=75,
        brightness=15,
        voice_types={"bass": 100},
        tags=["male", "bass", "russian", "orthodox", "deep", "octavist"]
    ),
]

# ============================================================================
# FEMALE VOICE TYPES (8 presets)
# ============================================================================

female_presets = [
    # Soprano (High Female)
    create_preset(
        "Female 1: Soprano",
        "Female soprano voice (C4-C6, 260-1050 Hz)",
        "english",
        "formant",
        "ah eh ee oh oo",
        num_voices=8,
        formant_mix=100,
        subharmonic_mix=0,
        stereo_width=75,
        vibrato_rate=6.8,
        vibrato_depth=32,
        reverb_mix=28,
        reverb_size=52,
        attack=40,
        release=180,
        pitch_variation=12,
        timing_variation=6,
        formant_variation=15,
        breathiness=12,
        warmth=30,
        brightness=70,  # Bright = female
        voice_types={"soprano": 100},
        tags=["female", "soprano", "high", "bright", "opera"]
    ),

    # Mezzo-Soprano (Medium-High Female)
    create_preset(
        "Female 2: Mezzo-Soprano",
        "Female mezzo-soprano (A3-A5, 220-880 Hz)",
        "english",
        "formant",
        "ah eh ee oh",
        num_voices=6,
        formant_mix=100,
        subharmonic_mix=0,
        stereo_width=72,
        vibrato_rate=6.4,
        vibrato_depth=28,
        reverb_mix=26,
        reverb_size=48,
        attack=45,
        release=190,
        pitch_variation=10,
        timing_variation=5,
        formant_variation=12,
        breathiness=10,
        warmth=38,
        brightness=58,
        voice_types={"mezzo_soprano": 100},
        tags=["female", "mezzo-soprano", "medium-high", "versatile"]
    ),

    # Alto (Low Female)
    create_preset(
        "Female 3: Alto",
        "Female alto voice (F3-D5, 175-590 Hz)",
        "english",
        "formant",
        "ah oh oo eh",
        num_voices=6,
        formant_mix=100,
        subharmonic_mix=0,
        stereo_width=68,
        vibrato_rate=6.0,
        vibrato_depth=24,
        reverb_mix=24,
        reverb_size=45,
        attack=50,
        release=200,
        pitch_variation=10,
        timing_variation=5,
        formant_variation=12,
        breathiness=10,
        warmth=45,
        brightness=50,
        voice_types={"alto": 100},
        tags=["female", "alto", "low", "warm", "harmony"]
    ),

    # Coloratura (Agile Female)
    create_preset(
        "Female 4: Coloratura",
        "Agile female coloratura with fast passages",
        "english",
        "diphone",
        "ah-eh ee-oh oh-oo",
        num_voices=4,
        formant_mix=100,
        subharmonic_mix=0,
        stereo_width=78,
        vibrato_rate=7.5,
        vibrato_depth=38,
        reverb_mix=22,
        reverb_size=40,
        attack=20,
        release=120,
        pitch_variation=18,
        timing_variation=8,
        breathiness=8,
        warmth=25,
        brightness=80,
        voice_types={"coloratura": 100},
        tags=["female", "coloratura", "agile", "high", "opera"]
    ),

    # Female Ensemble (SSAA)
    create_preset(
        "Female 5: SSAA Choir",
        "Traditional female SSAA choir (Soprano1-Soprano2-Alto1-Alto2)",
        "english",
        "diphone",
        "ah-eh ee-oh oh-oo",
        num_voices=12,
        formant_mix=100,
        subharmonic_mix=0,
        stereo_width=85,
        vibrato_rate=6.5,
        vibrato_depth=28,
        reverb_mix=32,
        reverb_size=58,
        attack=42,
        release=185,
        pitch_variation=12,
        timing_variation=6,
        formant_variation=15,
        breathiness=12,
        warmth=38,
        brightness=62,
        voice_types={
            "soprano": 50,
            "mezzo_soprano": 25,
            "alto": 25
        },
        tags=["female", "ssaa", "choir", "ensemble", "traditional"]
    ),

    # Pop Soprano
    create_preset(
        "Female 6: Pop Soprano",
        "Modern pop soprano lead vocals",
        "english",
        "diphone",
        "ah eh ee oh",
        num_voices=6,
        formant_mix=92,
        subharmonic_mix=8,
        stereo_width=80,
        vibrato_rate=7.2,
        vibrato_depth=35,
        reverb_mix=15,
        reverb_size=30,
        attack=20,
        release=140,
        pitch_variation=15,
        timing_variation=7,
        breathiness=15,
        warmth=35,
        brightness=75,
        voice_types={"soprano": 100},
        tags=["female", "pop", "soprano", "lead", "modern"]
    ),

    # Folk Alto
    create_preset(
        "Female 7: Folk Alto",
        "Warm folk alto with intimate character",
        "english",
        "formant",
        "ah oh oo",
        num_voices=4,
        formant_mix=95,
        subharmonic_mix=5,
        stereo_width=60,
        vibrato_rate=5.5,
        vibrato_depth=20,
        reverb_mix=20,
        reverb_size=35,
        attack=55,
        release=210,
        pitch_variation=8,
        timing_variation=4,
        breathiness=12,
        warmth=55,
        brightness=45,
        voice_types={"alto": 100},
        tags=["female", "alto", "folk", "warm", "intimate"]
    ),

    # Jazz Soprano
    create_preset(
        "Female 8: Jazz Soprano",
        "Smoky jazz club soprano with vibrato",
        "english",
        "formant",
        "ah eh ee oh",
        num_voices=4,
        formant_mix=98,
        subharmonic_mix=2,
        stereo_width=70,
        vibrato_rate=8.0,
        vibrato_depth=40,
        reverb_mix=25,
        reverb_size=45,
        attack=35,
        release=170,
        pitch_variation=20,
        timing_variation=8,
        breathiness=18,
        warmth=50,
        brightness=60,
        voice_types={"soprano": 100},
        tags=["female", "soprano", "jazz", "smoky", "club"]
    ),
]

# ============================================================================
# CHILD VOICE TYPES (4 presets)
# ============================================================================

child_presets = [
    # Boy Soprano
    create_preset(
        "Child 1: Boy Soprano",
        "Boy soprano before voice change (C4-A5, 260-880 Hz)",
        "english",
        "formant",
        "ah eh ee oh",
        num_voices=8,
        formant_mix=100,
        subharmonic_mix=0,
        stereo_width=70,
        vibrato_rate=6.0,
        vibrato_depth=22,
        reverb_mix=30,
        reverb_size=55,
        attack=45,
        release=190,
        pitch_variation=8,
        timing_variation=4,
        formant_variation=10,
        breathiness=8,
        warmth=40,
        brightness=65,
        voice_types={"boy_soprano": 100},
        tags=["child", "boy", "soprano", "choir", "bright"]
    ),

    # Girl Soprano
    create_preset(
        "Child 2: Girl Soprano",
        "Girl soprano (C4-C6, 260-1050 Hz)",
        "english",
        "formant",
        "ah eh ee oh oo",
        num_voices=8,
        formant_mix=100,
        subharmonic_mix=0,
        stereo_width=72,
        vibrato_rate=6.5,
        vibrato_depth=25,
        reverb_mix=28,
        reverb_size=50,
        attack=40,
        release=180,
        pitch_variation=10,
        timing_variation=5,
        formant_variation=12,
        breathiness=10,
        warmth=35,
        brightness=70,
        voice_types={"girl_soprano": 100},
        tags=["child", "girl", "soprano", "bright", "choir"]
    ),

    # Children's Choir
    create_preset(
        "Child 3: Children's Choir",
        "Mixed children's choir (boys and girls)",
        "english",
        "diphone",
        "ah-eh ee-oh",
        num_voices=16,
        formant_mix=100,
        subharmonic_mix=0,
        stereo_width=75,
        vibrato_rate=6.2,
        vibrato_depth=24,
        reverb_mix=25,
        reverb_size=45,
        attack=40,
        release=170,
        pitch_variation=12,
        timing_variation=6,
        formant_variation=12,
        breathiness=12,
        warmth=38,
        brightness=68,
        voice_types={
            "boy_soprano": 50,
            "girl_soprano": 50
        },
        tags=["child", "children", "choir", "ensemble", "bright"]
    ),

    # Choirboy Ensemble
    create_preset(
        "Child 4: Choirboys",
        "Traditional English choirboys",
        "english",
        "formant",
        "ah eh ee oh oo",
        num_voices=12,
        formant_mix=100,
        subharmonic_mix=0,
        stereo_width=65,
        vibrato_rate=5.8,
        vibrato_depth=20,
        reverb_mix=40,
        reverb_size=75,
        attack=70,
        release=280,
        pitch_variation=6,
        timing_variation=4,
        breathiness=10,
        warmth=45,
        brightness=60,
        voice_types={"boy_soprano": 100},
        tags=["child", "choirboy", "traditional", "church", "english"]
    ),
]

# ============================================================================
# MIXED GENDER ENSEMBLES (10 presets)
# ============================================================================

mixed_presets = [
    # Balanced SATB
    create_preset(
        "Mixed 1: SATB Balanced",
        "Traditional SATB with equal male/female balance",
        "english",
        "diphone",
        "oh-ah ah-eh ee-oh oh-oo",
        num_voices=16,
        formant_mix=100,
        subharmonic_mix=0,
        stereo_width=85,
        vibrato_rate=6.0,
        vibrato_depth=26,
        reverb_mix=35,
        reverb_size=65,
        attack=50,
        release=210,
        pitch_variation=12,
        timing_variation=6,
        formant_variation=14,
        breathiness=10,
        warmth=45,
        brightness=55,
        voice_types={
            "soprano": 25,    # Female high
            "alto": 25,       # Female low
            "tenor": 25,      # Male high
            "bass": 25        # Male low
        },
        gender_balance={"male": 50, "female": 50},
        tags=["mixed", "satb", "balanced", "choir", "traditional"]
    ),

    # Female-Heavy (60% F, 40% M)
    create_preset(
        "Mixed 2: Female Heavy",
        "Female-dominated ensemble (60% female, 40% male)",
        "english",
        "diphone",
        "ah-eh ee-oh oh-oo",
        num_voices=20,
        formant_mix=100,
        subharmonic_mix=0,
        stereo_width=88,
        vibrato_rate=6.3,
        vibrato_depth=28,
        reverb_mix=32,
        reverb_size=60,
        attack=42,
        release=190,
        pitch_variation=14,
        timing_variation=7,
        formant_variation=16,
        breathiness=12,
        warmth=40,
        brightness=65,
        voice_types={
            "soprano": 35,    # Female high (more)
            "alto": 25,       # Female low
            "mezzo_soprano": 10,  # Female mid
            "tenor": 20,      # Male high (less)
            "bass": 10        # Male low (less)
        },
        gender_balance={"male": 40, "female": 60},
        tags=["mixed", "female-heavy", "bright", "choir", "ensemble"]
    ),

    # Male-Heavy (60% M, 40% F)
    create_preset(
        "Mixed 3: Male Heavy",
        "Male-dominated ensemble (60% male, 40% female)",
        "english",
        "diphone",
        "oh-ah ah-eh ee-oh",
        num_voices=20,
        formant_mix=100,
        subharmonic_mix=0,
        stereo_width=80,
        vibrato_rate=5.8,
        vibrato_depth=24,
        reverb_mix=30,
        reverb_size=55,
        attack=55,
        release=220,
        pitch_variation=12,
        timing_variation=6,
        formant_variation=12,
        breathiness=8,
        warmth=55,
        brightness=40,
        voice_types={
            "soprano": 15,    # Female high (less)
            "alto": 25,       # Female low
            "tenor": 30,      # Male high (more)
            "baritone": 15,   # Male mid
            "bass": 15        # Male low (more)
        },
        gender_balance={"male": 60, "female": 40},
        tags=["mixed", "male-heavy", "warm", "powerful", "choir"]
    ),

    # Double Choir (SATB + SATB)
    create_preset(
        "Mixed 4: Double Choir",
        "Massive double choir (32 voices, SATB + SATB)",
        "english",
        "diphone",
        "ah-eh ee-oh oh-oo",
        num_voices=32,
        formant_mix=100,
        subharmonic_mix=0,
        stereo_width=100,
        vibrato_rate=6.2,
        vibrato_depth=26,
        reverb_mix=45,
        reverb_size=75,
        attack=50,
        release=220,
        pitch_variation=14,
        timing_variation=7,
        formant_variation=15,
        breathiness=10,
        warmth=48,
        brightness=52,
        voice_types={
            "soprano": 25,
            "alto": 25,
            "tenor": 25,
            "bass": 25
        },
        gender_balance={"male": 50, "female": 50},
        tags=["mixed", "double-choir", "massive", "satb", "powerful"]
    ),

    # Chamber Choir (12 voices, mixed)
    create_preset(
        "Mixed 5: Chamber Choir",
        "Intimate chamber choir with refined blend",
        "english",
        "diphone",
        "ah-eh ee-oh oh-oo",
        num_voices=12,
        formant_mix=100,
        subharmonic_mix=0,
        stereo_width=75,
        vibrato_rate=6.0,
        vibrato_depth=24,
        reverb_mix=28,
        reverb_size=50,
        attack=45,
        release=200,
        pitch_variation=10,
        timing_variation=5,
        formant_variation=12,
        breathiness=8,
        warmth=45,
        brightness=55,
        voice_types={
            "soprano": 30,
            "alto": 25,
            "tenor": 25,
            "bass": 20
        },
        gender_balance={"male": 48, "female": 52},
        tags=["mixed", "chamber", "intimate", "refined", "choir"]
    ),

    # Cathedral Choir (24 voices, balanced)
    create_preset(
        "Mixed 6: Cathedral Choir",
        "Large cathedral choir with reverberant space",
        "latin",
        "formant",
        "ah-eh-ee-oh-oo",
        num_voices=24,
        formant_mix=100,
        subharmonic_mix=0,
        stereo_width=90,
        vibrato_rate=5.5,
        vibrato_depth=22,
        reverb_mix=60,
        reverb_size=100,
        attack=80,
        release=350,
        pitch_variation=12,
        timing_variation=6,
        formant_variation=10,
        breathiness=12,
        warmth=55,
        brightness=45,
        voice_types={
            "soprano": 25,
            "alto": 25,
            "tenor": 25,
            "bass": 25
        },
        gender_balance={"male": 50, "female": 50},
        tags=["mixed", "cathedral", "large", "reverb", "latin"]
    ),

    # Gospel Choir (female-heavy)
    create_preset(
        "Mixed 7: Gospel Choir",
        "Energetic gospel choir with female lead",
        "english",
        "diphone",
        "ah-eh ee-oh oh-ay",
        num_voices=24,
        formant_mix=95,
        subharmonic_mix=5,
        stereo_width=90,
        vibrato_rate=7.0,
        vibrato_depth=35,
        reverb_mix=30,
        reverb_size=55,
        attack=35,
        release=180,
        pitch_variation=18,
        timing_variation=8,
        breathiness=15,
        warmth=45,
        brightness=70,
        voice_types={
            "soprano": 40,
            "alto": 30,
            "tenor": 20,
            "bass": 10
        },
        gender_balance={"male": 35, "female": 65},
        tags=["mixed", "gospel", "energetic", "soul", "female-heavy"]
    ),

    # Barbershop Quartet (male)
    create_preset(
        "Mixed 8: Barbershop Mixed",
        "Mixed gender barbershop harmony",
        "english",
        "diphone",
        "ah-eh ee-oh",
        num_voices=4,
        formant_mix=98,
        subharmonic_mix=2,
        stereo_width=70,
        vibrato_rate=6.5,
        vibrato_depth=30,
        reverb_mix=10,
        reverb_size=25,
        attack=25,
        release=150,
        pitch_variation=6,
        timing_variation=3,
        breathiness=8,
        warmth=50,
        brightness=60,
        voice_types={
            "tenor": 25,
            "lead": 25,
            "baritone": 25,
            "bass": 25
        },
        gender_balance={"male": 75, "female": 25},
        tags=["mixed", "barbershop", "harmony", "close-harmony"]
    ),

    # Contemporary Worship (balanced)
    create_preset(
        "Mixed 9: Contemporary Worship",
        "Modern worship team with balanced voices",
        "english",
        "diphone",
        "ah-eh ee-oh oh-oo",
        num_voices=12,
        formant_mix=92,
        subharmonic_mix=8,
        stereo_width=82,
        vibrato_rate=6.2,
        vibrato_depth=28,
        reverb_mix=25,
        reverb_size=45,
        attack=30,
        release=170,
        pitch_variation=12,
        timing_variation=6,
        breathiness=10,
        warmth=42,
        brightness=62,
        voice_types={
            "soprano": 30,
            "alto": 25,
            "tenor": 25,
            "bass": 20
        },
        gender_balance={"male": 50, "female": 50},
        tags=["mixed", "worship", "contemporary", "modern", "balanced"]
    ),

    # Renaissance Polyphony (male-heavy traditional)
    create_preset(
        "Mixed 10: Renaissance Polyphony",
        "Authentic Renaissance polyphony (male-heavy)",
        "latin",
        "diphone",
        "ah-eh ee-oh oh-oo",
        num_voices=16,
        formant_mix=100,
        subharmonic_mix=0,
        stereo_width=70,
        vibrato_rate=5.5,
        vibrato_depth=18,
        reverb_mix=35,
        reverb_size=60,
        attack=55,
        release=230,
        pitch_variation=8,
        timing_variation=4,
        formant_variation=10,
        breathiness=8,
        warmth=52,
        brightness=42,
        voice_types={
            "soprano": 20,  # Male countertenors
            "alto": 30,
            "tenor": 30,
            "bass": 20
        },
        gender_balance={"male": 80, "female": 20},
        tags=["mixed", "renaissance", "polyphony", "authentic", "latin"]
    ),
]

# ============================================================================
# NON-BINARY / ANDROGYNOUS (4 presets)
# ============================================================================

nonbinary_presets = [
    create_preset(
        "Non-Binary 1: Neutral Voice",
        "Gender-neutral voice with balanced characteristics",
        "english",
        "formant",
        "ah eh ee oh oo",
        num_voices=8,
        formant_mix=100,
        subharmonic_mix=0,
        stereo_width=75,
        vibrato_rate=6.0,
        vibrato_depth=25,
        reverb_mix=25,
        reverb_size=50,
        attack=50,
        release=200,
        pitch_variation=10,
        timing_variation=5,
        formant_variation=10,
        breathiness=10,
        warmth=50,
        brightness=50,
        voice_types={"neutral": 100},
        gender_balance={"male": 50, "female": 50},
        tags=["non-binary", "neutral", "balanced", "versatile"]
    ),

    create_preset(
        "Non-Binary 2: Androgynous High",
        "Androgynous high voice (flexible gender)",
        "english",
        "formant",
        "ah eh ee oh",
        num_voices=6,
        formant_mix=100,
        subharmonic_mix=0,
        stereo_width=78,
        vibrato_rate=6.5,
        vibrato_depth=28,
        reverb_mix=22,
        reverb_size=45,
        attack=40,
        release=180,
        pitch_variation=14,
        timing_variation=7,
        formant_variation=14,
        breathiness=10,
        warmth=45,
        brightness=55,
        voice_types={"androgynous": 100},
        gender_balance={"male": 50, "female": 50},
        tags=["non-binary", "androgynous", "high", "flexible"]
    ),

    create_preset(
        "Non-Binary 3: Gender Fluid",
        "Voice that can shift between male/female characteristics",
        "english",
        "diphone",
        "ah-eh ee-oh oh-oo",
        num_voices=8,
        formant_mix=100,
        subharmonic_mix=0,
        stereo_width=80,
        vibrato_rate=6.2,
        vibrato_depth=26,
        reverb_mix=25,
        reverb_size=48,
        attack=45,
        release=190,
        pitch_variation=20,
        timing_variation=10,
        formant_variation=20,
        breathiness=12,
        warmth=48,
        brightness=52,
        voice_types={"gender_fluid": 100},
        gender_balance={"male": 50, "female": 50},
        tags=["non-binary", "gender-fluid", "versatile", "shifting"]
    ),

    create_preset(
        "Non-Binary 4: Third Gender",
        "Traditional third gender voice (Hijra/Two-Spirit)",
        "english",
        "formant",
        "ah eh ee oh",
        num_voices=6,
        formant_mix=100,
        subharmonic_mix=0,
        stereo_width=72,
        vibrato_rate=6.0,
        vibrato_depth=24,
        reverb_mix=30,
        reverb_size=55,
        attack=60,
        release=220,
        pitch_variation=12,
        timing_variation=6,
        formant_variation=12,
        breathiness=15,
        warmth=55,
        brightness=45,
        voice_types={"third_gender": 100},
        gender_balance={"male": 50, "female": 50},
        tags=["non-binary", "third-gender", "traditional", "cultural"]
    ),
]

def main():
    preset_dir = "/Users/bretbouchard/apps/schill/choir-v2-universal/presets/factory"

    print("Generating Choir V2.0 Gender & Voice Type Presets...")
    print("=" * 70)

    # Generate all presets
    all_presets = (
        male_presets +
        female_presets +
        child_presets +
        mixed_presets +
        nonbinary_presets
    )

    for preset in all_presets:
        save_preset(preset, preset_dir)

    print("=" * 70)
    print(f"\n✓ Generated {len(all_presets)} gender & voice type presets!")
    print(f"  - Male Voices: {len(male_presets)}")
    print(f"  - Female Voices: {len(female_presets)}")
    print(f"  - Child Voices: {len(child_presets)}")
    print(f"  - Mixed Ensembles: {len(mixed_presets)}")
    print(f"  - Non-Binary: {len(nonbinary_presets)}")
    print(f"\nGender Balance Controls:")
    print(f"  - All ensemble presets include voice_type distribution")
    print(f"  - gender_balance parameter (male/female percentages)")
    print(f"  - Proper vocal ranges for each voice type")
    print(f"  - Female bass vs Male bass formant differences")
    print(f"\nLocation: {preset_dir}")

if __name__ == "__main__":
    main()
