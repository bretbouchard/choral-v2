/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * VoiceManager.h - Real-time safe multi-voice orchestration with SIMD optimization
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#pragma once

#include <vector>
#include <memory>
#include <atomic>
#include <cstring>
#include "VoiceAllocator.h"
#include "Voice.h"
#include "Phoneme.h"

namespace ChoirV2 {

// Forward declarations
class LinearSmoother;
class FormantSynthesis;
class SubharmonicSynthesis;

/**
 * @brief Single voice instance state
 *
 * Real-time safe voice state with pre-allocated buffers.
 * Used by VoiceManager for voice processing.
 */
struct VoiceInstance {
    int id;                         // Voice ID (from VoiceAllocator)
    bool active;                    // Voice active state
    float frequency;                // Fundamental frequency (Hz)
    float amplitude;                // Amplitude (0-1)
    float pan;                      // Pan position (-1 to 1, left to right)
    float age;                      // Voice age (seconds)

    // Synthesis state (placeholder for future integration)
    void* synthesisState;           // Opaque pointer to synthesis state
    float phase;                    // Phase accumulator

    // Gain envelopes
    float attackGain;               // Current attack envelope gain
    float releaseGain;              // Current release envelope gain
    bool inRelease;                 // Voice in release phase

    // Performance tracking
    float cpuUsage;                 // CPU usage per voice (0-1)

    VoiceInstance()
        : id(-1)
        , active(false)
        , frequency(0.0f)
        , amplitude(0.0f)
        , pan(0.0f)
        , age(0.0f)
        , synthesisState(nullptr)
        , phase(0.0f)
        , attackGain(0.0f)
        , releaseGain(1.0f)
        , inRelease(false)
        , cpuUsage(0.0f)
    {}
};

/**
 * @brief Voice processing parameters (with smoothing)
 *
 * All parameters are smoothed to prevent clicks and pops.
 */
struct VoiceParameters {
    float masterGain;               // Master gain (0-2)
    float attackTime;               // Attack time (seconds)
    float releaseTime;              // Release time (seconds)
    float vibratoRate;              // Vibrato rate (Hz)
    float vibratoDepth;             // Vibrato depth (0-1)
    float formantShift;             // Formant shift (semitones)
    float subharmonicMix;           // Subharmonic mix (0-1)

    VoiceParameters()
        : masterGain(1.0f)
        , attackTime(0.01f)
        , releaseTime(0.1f)
        , vibratoRate(5.0f)
        , vibratoDepth(0.0f)
        , formantShift(0.0f)
        , subharmonicMix(0.0f)
    {}
};

/**
 * @brief SIMD batch processing context
 *
 * Processes voices in batches of 4 or 8 for SIMD optimization.
 */
struct SIMDBatch {
    static constexpr int MAX_BATCH_SIZE = 8;  // AVX can process 8 floats
    int voiceIds[MAX_BATCH_SIZE];             // Voice IDs in batch
    int count;                                // Actual count (1-8)

    SIMDBatch() : count(0) {
        std::memset(voiceIds, 0, sizeof(voiceIds));
    }
};

/**
 * @brief Voice manager performance statistics
 */
struct VoiceManagerStats {
    int totalVoices;                    // Total voices allocated
    int activeVoices;                   // Currently active voices
    int stolenVoices;                   // Voices stolen this session
    float averageCpuUsage;              // Average CPU usage (0-1)
    float peakCpuUsage;                 // Peak CPU usage (0-1)
    int droppedVoices;                  // Voices dropped due to CPU overload

    VoiceManagerStats()
        : totalVoices(0)
        , activeVoices(0)
        , stolenVoices(0)
        , averageCpuUsage(0.0f)
        , peakCpuUsage(0.0f)
        , droppedVoices(0)
    {}
};

/**
 * @brief Real-time safe voice orchestration manager
 *
 * Orchestrates 40-60 simultaneous voice instances with SIMD optimization.
 * Processes voices in batches for optimal CPU utilization.
 *
 * Design Principles:
 * - Real-time safe: No allocations in processAudio()
 * - SIMD-optimized: Process voices in batches of 4/8
 * - Deterministic: Bounded execution time, no mutexes
 * - Smooth: Parameter smoothing prevents clicks
 * - Efficient: Sub-millisecond voice allocation
 *
 * Processing Pipeline (per sample):
 * 1. Update parameters (with smoothing)
 * 2. Process active voices (SIMD batch)
 * 3. Apply voice gain/pan
 * 4. Mix to output buffer
 * 5. Update voice states (note-offs, releases)
 *
 * Performance Targets:
 * - 60 voices @ 44.1kHz in < 30% CPU (Intel i7)
 * - Sub-millisecond voice allocation
 * - Deterministic timing (no mutexes)
 *
 * Usage:
 * ```cpp
 * VoiceManager manager(60, 44100.0);  // 60 voices, 44.1kHz
 *
 * // Prepare for playback
 * manager.prepare(44100.0, 512);
 *
 * // Note on
 * manager.noteOn(60, 100.0f);  // Middle C, velocity 100
 *
 * // Process audio (real-time safe)
 * float* outputLeft = ...;
 * float* outputRight = ...;
 * int numSamples = 512;
 * manager.processAudio(outputLeft, outputRight, numSamples);
 *
 * // Note off
 * manager.noteOff(60, 0.0f);
 * ```
 */
class VoiceManager {
public:
    /**
     * @brief Constructor
     * @param maxVoices Maximum number of simultaneous voices (40-60)
     * @param sampleRate Sample rate in Hz
     *
     * Pre-allocates all voice instances and buffers for real-time safety.
     * No dynamic allocation during processAudio().
     */
    VoiceManager(int maxVoices = 60, double sampleRate = 44100.0);
    ~VoiceManager();

    // Prevent copying (real-time safety)
    VoiceManager(const VoiceManager&) = delete;
    VoiceManager& operator=(const VoiceManager&) = delete;

    /**
     * @brief Prepare for playback
     * @param sampleRate Sample rate in Hz
     * @param maxBlockSize Maximum samples per block
     *
     * Called once before audio processing starts.
     * Pre-allocates buffers and prepares DSP components.
     *
     * Real-time safe to call from audio thread if sample rate unchanged.
     */
    void prepare(double sampleRate, int maxBlockSize);

    /**
     * @brief Process audio (real-time safe)
     * @param outputLeft Left output buffer
     * @param outputRight Right output buffer
     * @param numSamples Number of samples to process
     *
     * Main audio processing function. Called every audio callback.
     *
     * Processing pipeline:
     * 1. Update smoothed parameters
     * 2. Process active voices in SIMD batches
     * 3. Apply voice gain and pan
     * 4. Mix down to stereo output
     * 5. Update voice states (releases, note-offs)
     *
     * Real-time safe: No allocations, bounded execution time.
     * SIMD-optimized: Processes voices in batches of 4/8.
     */
    void processAudio(float* outputLeft, float* outputRight, int numSamples);

    /**
     * @brief Note on event
     * @param midiNote MIDI note number (0-127)
     * @param velocity Note velocity (0.0-127.0)
     * @return Voice ID, or -1 if allocation failed
     *
     * Allocates a new voice for the note.
     * Uses VoiceAllocator for priority-based voice stealing.
     *
     * Real-time safe: Sub-millisecond allocation.
     */
    int noteOn(int midiNote, float velocity);

    /**
     * @brief Note off event
     * @param midiNote MIDI note number (0-127)
     * @param velocity Release velocity (0.0-127.0, typically 0)
     *
     * Triggers release envelope for matching voice(s).
     * Voice enters release phase and fades out smoothly.
     *
     * Real-time safe.
     */
    void noteOff(int midiNote, float velocity);

    /**
     * @brief All notes off (panic)
     *
     * Immediately stops all voices (no release).
     * Useful for panic button or transport stop.
     *
     * Real-time safe.
     */
    void allNotesOff();

    /**
     * @brief Set master gain
     * @param gain Master gain (0-2, linear)
     *
     * Smoothed to prevent clicks.
     */
    void setMasterGain(float gain);

    /**
     * @brief Set attack time
     * @param attackTime Attack time in seconds (0.001-1.0)
     *
     * Smoothed to prevent clicks.
     */
    void setAttackTime(float attackTime);

    /**
     * @brief Set release time
     * @param releaseTime Release time in seconds (0.001-2.0)
     *
     * Smoothed to prevent clicks.
     */
    void setReleaseTime(float releaseTime);

    /**
     * @brief Set vibrato rate
     * @param rate Vibrato rate in Hz (0-20)
     *
     * Smoothed to prevent clicks.
     */
    void setVibratoRate(float rate);

    /**
     * @brief Set vibrato depth
     * @param depth Vibrato depth (0-1)
     *
     * Smoothed to prevent clicks.
     */
    void setVibratoDepth(float depth);

    /**
     * @brief Get voice instance
     * @param voiceId Voice ID
     * @return Voice pointer, or nullptr if invalid
     *
     * Returns pointer to internal voice instance.
     * Pointer valid only during audio callback.
     * Do not store pointer across callbacks.
     */
    VoiceInstance* getVoice(int voiceId);

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
     * @brief Get performance statistics
     * @return Performance statistics
     */
    const VoiceManagerStats& getStats() const;

    /**
     * @brief Reset statistics
     *
     * Resets performance tracking counters.
     */
    void resetStats();

private:
    // Voice allocation
    std::unique_ptr<VoiceAllocator> allocator_;
    std::vector<std::unique_ptr<VoiceInstance>> voices_;
    int maxVoices_;

    // Audio processing
    double sampleRate_;
    int maxBlockSize_;

    // Pre-allocated scratch buffer for voice processing (real-time safe)
    std::vector<float> scratchBuffer_;

    // Synthesis methods
    std::unique_ptr<FormantSynthesis> formantSynthesis_;
    std::unique_ptr<SubharmonicSynthesis> subharmonicSynthesis_;

    // Parameter smoothing (using LinearSmoother)
    std::unique_ptr<LinearSmoother> masterGainSmoother_;
    std::unique_ptr<LinearSmoother> attackTimeSmoother_;
    std::unique_ptr<LinearSmoother> releaseTimeSmoother_;
    std::unique_ptr<LinearSmoother> vibratoRateSmoother_;
    std::unique_ptr<LinearSmoother> vibratoDepthSmoother_;

    // Statistics
    VoiceManagerStats stats_;
    std::atomic<float> peakCpuUsage_;

    // Target parameters (for smoothing)
    std::unique_ptr<VoiceParameters> targetParams_;
    std::unique_ptr<VoiceParameters> currentParams_;

    // SIMD batch processing
    /**
     * @brief Build SIMD batches from active voices
     * @param batches Output array of batches
     * @param maxBatches Maximum number of batches
     * @return Number of batches created
     *
     * Groups active voices into batches of up to 8 for SIMD processing.
     * Real-time safe: No allocation, uses pre-allocated batch array.
     */
    int buildSIMDBatches(SIMDBatch* batches, int maxBatches);

    /**
     * @brief Process single voice
     * @param voice Voice to process
     * @param outputLeft Left output buffer
     * @param outputRight Right output buffer
     * @param numSamples Number of samples to process
     *
     * Processes a single voice and mixes to output.
     * Placeholder for future synthesis integration.
     *
     * Real-time safe: Uses pre-allocated scratch buffer.
     */
    void processVoice(VoiceInstance* voice,
                      float* outputLeft,
                      float* outputRight,
                      int numSamples);

    /**
     * @brief Process SIMD batch of voices
     * @param batch SIMD batch to process
     * @param outputLeft Left output buffer
     * @param outputRight Right output buffer
     * @param numSamples Number of samples to process
     *
     * Processes multiple voices using SIMD operations.
     * Currently processes voices sequentially (placeholder for SIMD).
     *
     * Real-time safe.
     */
    void processSIMDBatch(const SIMDBatch& batch,
                          float* outputLeft,
                          float* outputRight,
                          int numSamples);

    /**
     * @brief Update voice envelopes
     * @param voice Voice to update
     * @param numSamples Number of samples processed
     *
     * Updates attack/release envelopes for voice.
     * Handles voice state transitions (active -> release -> inactive).
     *
     * Real-time safe.
     */
    void updateEnvelope(VoiceInstance* voice, int numSamples);

    /**
     * @brief Apply pan to stereo outputs
     * @param input Input sample
     * @param pan Pan position (-1 to 1)
     * @param leftGain Output left gain
     * @param rightGain Output right gain
     *
     * Calculates equal-power pan gains.
     * Pan law: -3dB at center.
     *
     * Real-time safe.
     */
    void applyPan(float input, float pan, float& leftGain, float& rightGain);

    /**
     * @brief Update parameter smoothing
     *
     * Interpolates current parameters toward target parameters.
     * Called every audio callback.
     *
     * Real-time safe.
     */
    void updateParameterSmoothing();

    /**
     * @brief Find voice by MIDI note
     * @param midiNote MIDI note number
     * @return Voice ID, or -1 if not found
     *
     * Finds active voice matching MIDI note.
     * Returns first matching voice if multiple exist.
     *
     * Real-time safe.
     */
    int findVoiceByNote(int midiNote);

    /**
     * @brief Generate sine wave for voice
     * @param voice Voice to generate for
     * @param output Output buffer (pre-allocated)
     * @param numSamples Number of samples to generate
     *
     * Placeholder synthesis: Simple sine wave.
     * Will be replaced by FormantSynthesis/SubharmonicSynthesis.
     *
     * Real-time safe: No allocation.
     */
    void generateSineWave(VoiceInstance* voice, float* output, int numSamples);
};

} // namespace ChoirV2
