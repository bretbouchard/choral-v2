/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * VoiceAllocator.h - Priority-based voice allocation with stealing
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#pragma once

#include <vector>
#include <memory>
#include <algorithm>
#include <random>

namespace ChoirV2 {

/**
 * @brief Voice allocation record
 *
 * Real-time safe voice tracking with priority-based stealing.
 * No dynamic allocation during audio processing.
 */
struct VoiceRecord {
    int id;                         // Unique voice ID
    int midiNote;                   // MIDI note number (0-127)
    float velocity;                 // MIDI velocity (0-127)
    bool active;                    // Voice active state
    int priority;                   // Priority score (0-100, higher = more important)
    int age;                        // Age counter for LRU tracking
    double startTime;               // When voice started (seconds)

    // For real-time safety, use fixed-size arrays
    float frequency;                // Fundamental frequency (Hz)
    float amplitude;                // Amplitude (0-1)
    float pan;                      // Pan position (-1 to 1)

    VoiceRecord()
        : id(-1)
        , midiNote(0)
        , velocity(0.0f)
        , active(false)
        , priority(0)
        , age(0)
        , startTime(0.0)
        , frequency(0.0f)
        , amplitude(0.0f)
        , pan(0.0f)
    {}
};

/**
 * @brief Voice allocation result
 */
struct AllocationResult {
    int voiceId;                    // Allocated voice ID (-1 if failed)
    bool stolen;                    // Was this voice stolen from another?
    int stolenFromId;               // Original voice ID (if stolen)
    bool success;                   // Allocation successful?

    AllocationResult()
        : voiceId(-1)
        , stolen(false)
        , stolenFromId(-1)
        , success(false)
    {}
};

/**
 * @brief Voice allocator with priority-based stealing
 *
 * Manages allocation of limited voice resources to MIDI notes.
 * Implements priority-based voice stealing when all voices are active.
 *
 * Design Principles:
 * - Real-time safe: No dynamic allocation during audio processing
 * - Priority-based: Higher velocity + longer duration = higher priority
 * - LRU-aware: Newest voices are first to be stolen
 * - Predictable: O(n) worst-case, O(1) best-case
 *
 * Priority Calculation:
 * - MIDI velocity: 50% weight (higher velocity = higher priority)
 * - Note age: 30% weight (older notes = higher priority)
 * - Random tiebreaker: 20% weight (prevent rhythmic stealing patterns)
 *
 * Usage:
 * ```cpp
 * VoiceAllocator allocator(60);  // 60 voice polyphony
 *
 * // Allocate voice for MIDI note
 * auto result = allocator.allocateVoice(60, 100.0f);  // Middle C, velocity 100
 * if (result.success) {
 *     // Voice allocated (result.voiceId)
 *     if (result.stolen) {
 *         // Voice was stolen from result.stolenFromId
 *     }
 * }
 *
 * // Free voice when note released
 * allocator.freeVoice(result.voiceId);
 *
 * // Update priorities periodically (~100ms)
 * allocator.updatePriorities(deltaTime);
 * ```
 */
class VoiceAllocator {
public:
    /**
     * @brief Constructor
     * @param maxVoices Maximum number of simultaneous voices
     *
     * Pre-allocates all voice records for real-time safety.
     * No dynamic allocation during allocateVoice()/freeVoice().
     */
    explicit VoiceAllocator(int maxVoices = 60);
    ~VoiceAllocator();

    // Prevent copying (real-time safety)
    VoiceAllocator(const VoiceAllocator&) = delete;
    VoiceAllocator& operator=(const VoiceAllocator&) = delete;

    /**
     * @brief Allocate voice for note
     * @param midiNote MIDI note number (0-127)
     * @param velocity Note velocity (0.0-127.0)
     * @return Allocation result with voice ID
     *
     * Allocation strategy:
     * 1. If free voice available, allocate new voice
     * 2. If all voices active, steal lowest priority voice
     * 3. Priority = weighted combination of velocity, age, and randomness
     *
     * Real-time safe: No dynamic allocation, O(n) worst-case.
     */
    AllocationResult allocateVoice(int midiNote, float velocity);

    /**
     * @brief Free voice
     * @param voiceId Voice ID to free
     *
     * Marks voice as inactive and returns to free pool.
     * Safe to call with invalid voice ID (no-op).
     */
    void freeVoice(int voiceId);

    /**
     * @brief Get voice by ID
     * @param voiceId Voice ID
     * @return Voice pointer, or nullptr if invalid
     *
     * Returns pointer to internal voice record.
     * Pointer valid until next allocateVoice()/freeVoice().
     * Do not store pointer across audio callbacks.
     */
    VoiceRecord* getVoice(int voiceId);

    /**
     * @brief Get active voice count
     * @return Number of active voices
     */
    int getActiveVoiceCount() const;

    /**
     * @brief Get maximum voice count
     * @return Maximum number of voices
     */
    int getMaxVoices() const;

    /**
     * @brief Update voice priorities
     * @param deltaTime Time since last update (seconds)
     *
     * Call every ~100ms to update priority scores.
     * Increases age for active voices (prevents stealing of long notes).
     * Recalculates priority scores based on age.
     *
     * Real-time safe: No dynamic allocation, O(n).
     */
    void updatePriorities(double deltaTime);

    /**
     * @brief Reset all voices
     *
     * Marks all voices as inactive and resets state.
     * Useful for panic button or all-notes-off.
     */
    void resetAll();

    /**
     * @brief Get voice stealing statistics
     */
    struct StealingStats {
        int totalAllocations;        // Total allocations
        int stolenVoices;            // Total voices stolen
        int highPriorityStolen;      // High-priority voices stolen (bad)
        int lowPriorityStolen;       // Low-priority voices stolen (OK)

        StealingStats()
            : totalAllocations(0)
            , stolenVoices(0)
            , highPriorityStolen(0)
            , lowPriorityStolen(0)
        {}
    };

    const StealingStats& getStealingStats() const;

private:
    std::vector<std::unique_ptr<VoiceRecord>> voices_;
    std::vector<int> freeVoiceIds_;
    int maxVoices_;
    StealingStats stats_;
    std::mt19937 rng_;  // For random tiebreaker in priority

    /**
     * @brief Find free voice ID
     * @return Voice ID, or -1 if none available
     *
     * O(1) amortized (uses free list).
     */
    int findFreeVoice();

    /**
     * @brief Find voice to steal (lowest priority)
     * @return Voice ID to steal, or -1 if none active
     *
     * Stealing strategy:
     * 1. Lowest priority score (calculatePriority())
     * 2. Newest voice (lowest age) if priority tied
     * 3. Random tiebreaker
     *
     * O(n) worst-case.
     */
    int findVoiceToSteal();

    /**
     * @brief Calculate voice priority
     * @param voice Voice to calculate priority for
     * @return Priority score (0-100, higher = more important)
     *
     * Priority formula:
     * - 50% weight: MIDI velocity (higher = more important)
     * - 30% weight: Note age (older = more important)
     * - 20% weight: Random tiebreaker (prevent rhythmic patterns)
     *
     * Example:
     * - Velocity 100, age 10: priority = 50 + 30 = 80
     * - Velocity 50, age 0: priority = 25 + 0 = 25
     *
     * Real-time safe: No allocation, O(1).
     */
    int calculatePriority(const VoiceRecord& voice);

    /**
     * @brief Validate voice ID
     * @param voiceId Voice ID to validate
     * @return True if valid
     */
    bool isValidVoiceId(int voiceId) const;
};

} // namespace ChoirV2
