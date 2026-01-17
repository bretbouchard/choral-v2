/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * VoiceManager.h - Multi-voice orchestration manager
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#pragma once

#include <memory>
#include <vector>
#include <queue>

namespace ChoirV2 {

// Forward declarations
class ISynthesisMethod;

/**
 * @brief Voice state
 */
enum class VoiceState {
    Idle,           // Voice not in use
    Attack,         // Initial attack phase
    Sustain,        // Sustaining a note
    Decay,          // Decay phase
    Release         // Release phase
};

/**
 * @brief Voice priority for voice stealing
 */
enum class VoicePriority {
    Critical = 0,   // Cannot be stolen (e.g., lead melody)
    High = 1,       // Important (e.g., bass)
    Medium = 2,     // Normal (e.g., harmony)
    Low = 3         // Can be stolen (e.g., background)
};

/**
 * @brief Single voice instance
 *
 * Represents one synthesized voice in the choir.
 * Can be a note, a phoneme, or a continuous sound.
 */
struct Voice {
    int id;                         // Unique voice ID
    VoiceState state;               // Current state
    VoicePriority priority;         // Priority for voice stealing

    // Audio parameters
    float frequency;                // Fundamental frequency (Hz)
    float amplitude;                // Amplitude (0-1)
    float pan;                      // Pan position (-1 to 1)

    // Phoneme/syllable parameters
    std::string current_phoneme;    // Current phoneme being synthesized
    float phoneme_progress;         // Progress through phoneme (0-1)

    // Timing
    double start_time;              // When voice started (seconds)
    double duration;                // Total duration (seconds)

    // Envelope
    float attack_time;              // Attack time (seconds)
    float decay_time;               // Decay time (seconds)
    float sustain_level;            // Sustain level (0-1)
    float release_time;             // Release time (seconds)
};

/**
 * @brief Voice allocation result
 */
struct VoiceAllocation {
    bool success;                   // Voice allocated successfully?
    int voice_id;                   // Allocated voice ID (if successful)
    std::string error_message;      // Error details (if failed)
};

/**
 * @brief Voice manager
 *
 * Manages multiple simultaneous voices with:
 * - Voice allocation (find free voice or steal low-priority voice)
 * - Voice stealing (when CPU limit is reached)
 * - Priority management (important voices preserved)
 * - State tracking (attack, sustain, decay, release)
 *
 * CRITICAL: Single-threaded SIMD implementation (no threading)
 * for real-time safety.
 */
class VoiceManager {
public:
    VoiceManager(int max_voices);
    ~VoiceManager();

    /**
     * @brief Allocate a voice
     * @param priority Voice priority
     * @param frequency Fundamental frequency (Hz)
     * @param amplitude Amplitude (0-1)
     * @return Voice allocation result
     */
    VoiceAllocation allocateVoice(
        VoicePriority priority,
        float frequency,
        float amplitude
    );

    /**
     * @brief Release a voice
     * @param voice_id Voice ID to release
     */
    void releaseVoice(int voice_id);

    /**
     * @brief Get a voice by ID
     * @param voice_id Voice ID
     * @return Voice pointer, or nullptr if not found
     */
    Voice* getVoice(int voice_id);

    /**
     * @brief Get all active voices
     * @return Vector of active voice pointers
     */
    std::vector<Voice*> getActiveVoices();

    /**
     * @brief Get number of active voices
     */
    int getActiveVoiceCount() const;

    /**
     * @brief Get number of idle voices
     */
    int getIdleVoiceCount() const;

    /**
     * @brief Update voice states (call each audio block)
     * @param delta_time Time since last update (seconds)
     */
    void updateStates(double delta_time);

    /**
     * @brief Set CPU limit for voice stealing
     * @param cpu_limit CPU usage limit (0-1)
     */
    void setCPULimit(float cpu_limit);

    /**
     * @brief Get voice stealing statistics
     */
    struct StealingStats {
        int total_stolen;           // Total voices stolen
        int high_priority_stolen;   // High-priority voices stolen (bad)
        int low_priority_stolen;    // Low-priority voices stolen (OK)
    };
    StealingStats getStealingStats() const;

private:
    std::vector<std::unique_ptr<Voice>> voices_;
    std::queue<int> free_voice_queue_;

    float cpu_limit_;
    StealingStats stealing_stats_;

    // Voice allocation
    VoiceAllocation allocateFreeVoice(
        VoicePriority priority,
        float frequency,
        float amplitude
    );

    VoiceAllocation stealVoice(
        VoicePriority priority,
        float frequency,
        float amplitude
    );

    int findStealCandidate(VoicePriority priority) const;
    bool shouldSteal() const;
};

} // namespace ChoirV2
