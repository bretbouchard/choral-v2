/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * Voice.cpp - Voice implementation
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#include "Voice.h"

namespace ChoirV2 {

Voice::Voice() {
    // Initialize with default values
}

void Voice::reset() {
    f0_ = 220.0f;
    amplitude_ = 0.5f;
    pan_ = 0.0f;
    is_active_ = false;
}

} // namespace ChoirV2
