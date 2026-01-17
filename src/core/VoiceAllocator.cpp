/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * VoiceAllocator.cpp - Priority-based voice allocation with stealing
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#include "VoiceAllocator.h"
#include <cassert>
#include <cmath>

namespace ChoirV2 {

VoiceAllocator::VoiceAllocator(int maxVoices)
    : maxVoices_(maxVoices)
    , rng_(std::random_device{}())
{
    // Pre-allocate all voice records for real-time safety
    voices_.reserve(maxVoices_);
    freeVoiceIds_.reserve(maxVoices_);

    // Create voice records
    for (int i = 0; i < maxVoices_; ++i) {
        auto voice = std::make_unique<VoiceRecord>();
        voice->id = i;
        voices_.push_back(std::move(voice));
        freeVoiceIds_.push_back(i);
    }
}

VoiceAllocator::~VoiceAllocator() = default;

AllocationResult VoiceAllocator::allocateVoice(int midiNote, float velocity) {
    AllocationResult result;

    // Validate inputs
    if (midiNote < 0 || midiNote > 127) {
        return result;  // Invalid MIDI note
    }

    if (velocity < 0.0f || velocity > 127.0f) {
        return result;  // Invalid velocity
    }

    // Try to find free voice
    int voiceId = findFreeVoice();

    if (voiceId == -1) {
        // No free voices, steal lowest priority voice
        voiceId = findVoiceToSteal();

        if (voiceId == -1) {
            // No voices to steal (shouldn't happen if maxVoices_ > 0)
            return result;
        }

        // Mark as stolen
        result.stolen = true;
        result.stolenFromId = voiceId;
        stats_.stolenVoices++;

        // Track priority of stolen voice
        VoiceRecord* stolenVoice = getVoice(voiceId);
        if (stolenVoice && stolenVoice->priority > 50) {
            stats_.highPriorityStolen++;
        } else {
            stats_.lowPriorityStolen++;
        }
    }

    // Initialize voice
    VoiceRecord* voice = getVoice(voiceId);
    if (!voice) {
        return result;  // Shouldn't happen
    }

    voice->midiNote = midiNote;
    voice->velocity = velocity;
    voice->active = true;
    voice->age = 0;  // Start at age 0
    voice->startTime = 0.0;  // Will be set by caller

    // Calculate MIDI note to frequency (A4 = 440Hz)
    // formula: f = 440 * 2^((midiNote - 69) / 12)
    voice->frequency = 440.0f * std::pow(2.0f, (midiNote - 69) / 12.0f);

    // Amplitude from velocity (0-1)
    voice->amplitude = velocity / 127.0f;

    // Center pan
    voice->pan = 0.0f;

    // Calculate initial priority
    voice->priority = calculatePriority(*voice);

    // Success
    result.voiceId = voiceId;
    result.success = true;
    stats_.totalAllocations++;

    return result;
}

void VoiceAllocator::freeVoice(int voiceId) {
    if (!isValidVoiceId(voiceId)) {
        return;  // Invalid voice ID
    }

    VoiceRecord* voice = getVoice(voiceId);
    if (!voice) {
        return;  // Shouldn't happen
    }

    if (!voice->active) {
        return;  // Already inactive
    }

    // Mark as inactive
    voice->active = false;
    voice->midiNote = 0;
    voice->velocity = 0.0f;
    voice->priority = 0;
    voice->age = 0;
    voice->startTime = 0.0;
    voice->frequency = 0.0f;
    voice->amplitude = 0.0f;

    // Return to free pool
    freeVoiceIds_.push_back(voiceId);
}

VoiceRecord* VoiceAllocator::getVoice(int voiceId) {
    if (!isValidVoiceId(voiceId)) {
        return nullptr;
    }

    return voices_[voiceId].get();
}

int VoiceAllocator::getActiveVoiceCount() const {
    int count = 0;
    for (const auto& voice : voices_) {
        if (voice->active) {
            count++;
        }
    }
    return count;
}

int VoiceAllocator::getMaxVoices() const {
    return maxVoices_;
}

void VoiceAllocator::updatePriorities(double deltaTime) {
    (void)deltaTime;  // Suppress unused parameter warning

    // Update age and recalculate priority for all active voices
    for (auto& voice : voices_) {
        if (voice->active) {
            // Increment age (call every ~100ms = 10 calls per second)
            voice->age++;

            // Recalculate priority with new age
            voice->priority = calculatePriority(*voice);
        }
    }
}

void VoiceAllocator::resetAll() {
    // Mark all voices as inactive
    for (auto& voice : voices_) {
        voice->active = false;
        voice->midiNote = 0;
        voice->velocity = 0.0f;
        voice->priority = 0;
        voice->age = 0;
        voice->startTime = 0.0;
        voice->frequency = 0.0f;
        voice->amplitude = 0.0f;
        voice->pan = 0.0f;
    }

    // Rebuild free list
    freeVoiceIds_.clear();
    freeVoiceIds_.reserve(maxVoices_);
    for (int i = 0; i < maxVoices_; ++i) {
        freeVoiceIds_.push_back(i);
    }

    // Reset stats
    stats_ = StealingStats();
}

const VoiceAllocator::StealingStats& VoiceAllocator::getStealingStats() const {
    return stats_;
}

int VoiceAllocator::findFreeVoice() {
    if (freeVoiceIds_.empty()) {
        return -1;  // No free voices
    }

    // Get free voice ID from back of vector
    int voiceId = freeVoiceIds_.back();
    freeVoiceIds_.pop_back();

    return voiceId;
}

int VoiceAllocator::findVoiceToSteal() {
    int lowestPriorityVoiceId = -1;
    int lowestPriority = 101;  // Above maximum priority (0-100)
    int lowestAge = 2147483647;  // Max int

    // Find voice with lowest priority
    for (int i = 0; i < maxVoices_; ++i) {
        VoiceRecord* voice = getVoice(i);
        if (!voice || !voice->active) {
            continue;
        }

        // Compare priority
        if (voice->priority < lowestPriority) {
            lowestPriority = voice->priority;
            lowestPriorityVoiceId = i;
            lowestAge = voice->age;
        } else if (voice->priority == lowestPriority) {
            // Priority tied, choose newest (lowest age)
            if (voice->age < lowestAge) {
                lowestPriorityVoiceId = i;
                lowestAge = voice->age;
            }
        }
    }

    return lowestPriorityVoiceId;
}

int VoiceAllocator::calculatePriority(const VoiceRecord& voice) {
    // Priority formula:
    // - 50% weight: MIDI velocity (0-127 mapped to 0-50)
    // - 30% weight: Note age (capped at 100, mapped to 0-30)
    // - 20% weight: Random tiebreaker (0-20)

    // Velocity contribution (0-50)
    float velocityScore = (voice.velocity / 127.0f) * 50.0f;

    // Age contribution (0-30)
    // Cap age at 100 to prevent unlimited priority growth
    int cappedAge = std::min(voice.age, 100);
    float ageScore = (cappedAge / 100.0f) * 30.0f;

    // Random tiebreaker (0-20)
    std::uniform_int_distribution<int> dist(0, 20);
    int randomScore = dist(rng_);

    // Combine scores
    int priority = static_cast<int>(velocityScore + ageScore + randomScore);

    // Clamp to 0-100 range
    priority = std::max(0, std::min(100, priority));

    return priority;
}

bool VoiceAllocator::isValidVoiceId(int voiceId) const {
    return voiceId >= 0 && voiceId < maxVoices_;
}

} // namespace ChoirV2
