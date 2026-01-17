/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * ReverbEffect.h - High-quality stereo reverb effect
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#pragma once

#include <vector>
#include <array>
#include "LinearSmoother.h"

namespace ChoirV2 {

/**
 * @brief Reverb preset configurations
 *
 * Pre-configured reverb spaces for common use cases.
 */
enum class ReverbPreset {
    SmallRoom,      // Intimate room sound
    LargeHall,      // Concert hall ambience
    Cathedral,      // Large cathedral reverb
    Plate,          // Classic plate reverb
    Custom          // User-defined parameters
};

/**
 * @brief High-quality stereo reverb effect
 *
 * Implements professional reverb using JUCE's dsp::Reverb algorithm.
 * Features:
 * - Stereo processing with width control
 * - Smooth parameter changes (no clicks)
 * - Multiple reverb presets
 * - Real-time safe (no allocations in process())
 * - Bypass support
 *
 * Algorithm based on Schroeder/Moorer reverb design:
 * - Parallel comb filters for early reflections
 * - Series allpass filters for diffusion
 * - Adjustable decay time and damping
 *
 * Integration:
 * - Insert before master output
 * - Apply to all voices mixed together
 * - Use 20-50ms smoothing time for natural parameter transitions
 */
class ReverbEffect {
public:
    ReverbEffect();
    ~ReverbEffect() = default;

    //==========================================================================
    // Configuration
    //==========================================================================

    /**
     * @brief Initialize reverb with sample rate and max block size
     * @param sample_rate Sample rate (Hz)
     * @param max_block_size Maximum samples per processBlock() call
     *
     * Prepares internal buffers and smoothers. Must call before processing.
     */
    void prepare(float sample_rate, int max_block_size);

    /**
     * @brief Reset reverb to initial state
     *
     * Clears delay lines and resets smoothers. Call when:
     * - Starting playback
     * - After dropout
     * - When switching presets
     */
    void reset();

    //==========================================================================
    // Parameter Control
    //==========================================================================

    /**
     * @brief Set room size (0.0-1.0)
     * @param size Room size (0.0=small, 1.0=large)
     *
     * Controls the perceived size of the reverb space.
     * Affects both decay time and early reflection pattern.
     */
    void setRoomSize(float size);

    /**
     * @brief Set damping (0.0-1.0)
     * @param damping High-frequency damping (0.0=bright, 1.0=dark)
     *
     * Controls high-frequency absorption in the reverb tail.
     * Higher values simulate more absorptive materials.
     */
    void setDamping(float damping);

    /**
     * @brief Set wet level (0.0-1.0)
     * @param wet Wet signal level (0.0=dry, 1.0=fully wet)
     *
     * Controls the amount of reverb in the output.
     * Use with dryLevel for wet/dry mix control.
     */
    void setWetLevel(float wet);

    /**
     * @brief Set dry level (0.0-1.0)
     * @param dry Dry signal level (0.0=muted, 1.0=full dry)
     *
     * Controls the amount of dry (unprocessed) signal in the output.
     * Use with wetLevel for wet/dry mix control.
     */
    void setDryLevel(float dry);

    /**
     * @brief Set stereo width (0.0-1.0)
     * @param width Stereo width (0.0=mono, 1.0=full stereo)
     *
     * Controls the stereo imaging of the reverb.
     * Lower values collapse to mono, higher values enhance stereo spread.
     */
    void setWidth(float width);

    /**
     * @brief Enable/disable freeze mode
     * @param freeze True to enable infinite decay
     *
     * Freeze mode holds the reverb tail indefinitely.
     * Useful for special effects and ambient pads.
     */
    void setFreezeMode(bool freeze);

    /**
     * @brief Load reverb preset
     * @param preset Preset to load
     *
     * Sets all parameters to preset values instantly (no smoothing).
     */
    void loadPreset(ReverbPreset preset);

    /**
     * @brief Enable/disable bypass
     * @param bypass True to bypass reverb (dry signal only)
     */
    void setBypass(bool bypass) { bypass_ = bypass; }

    //==========================================================================
    // Processing
    //==========================================================================

    /**
     * @brief Process mono input to stereo output
     * @param input Mono input buffer [num_samples]
     * @param output_left Left output buffer [num_samples]
     * @param output_right Right output buffer [num_samples]
     * @param num_samples Number of samples to process
     *
     * Processes mono input through reverb to stereo output.
     * Real-time safe: no allocations, no locks.
     */
    void processMonoToStereo(const float* input,
                             float* output_left,
                             float* output_right,
                             int num_samples);

    /**
     * @brief Process stereo input to stereo output
     * @param input_left Left input buffer [num_samples]
     * @param input_right Right input buffer [num_samples]
     * @param output_left Left output buffer [num_samples]
     * @param output_right Right output buffer [num_samples]
     * @param num_samples Number of samples to process
     *
     * Processes stereo input through reverb to stereo output.
     * Real-time safe: no allocations, no locks.
     */
    void processStereo(const float* input_left,
                       const float* input_right,
                       float* output_left,
                       float* output_right,
                       int num_samples);

    //==========================================================================
    // Query
    //==========================================================================

    /**
     * @brief Get current reverb parameters
     * @return Array of {roomSize, damping, wetLevel, dryLevel, width}
     */
    std::array<float, 5> getParameters() const;

    /**
     * @brief Check if reverb is bypassed
     * @return True if bypassed
     */
    bool isBypassed() const { return bypass_; }

private:
    //==========================================================================
    // Internal State
    //==========================================================================

    // JUCE reverb engine
    struct ReverbEngine;
    std::unique_ptr<ReverbEngine> engine_;

    // Parameter smoothers (20ms default for natural transitions)
    LinearSmoother size_smoother_;
    LinearSmoother damping_smoother_;
    LinearSmoother wet_smoother_;
    LinearSmoother dry_smoother_;
    LinearSmoother width_smoother_;

    // Current parameters (for query)
    float room_size_;
    float damping_;
    float wet_level_;
    float dry_level_;
    float width_;
    bool freeze_mode_;
    bool bypass_;

    // Sample rate
    float sample_rate_;

    //==========================================================================
    // Internal Methods
    //==========================================================================

    /**
     * @brief Update reverb engine parameters from smoothed values
     *
     * Called every processBlock to apply current smoothed parameters.
     */
    void updateEngineParameters();

    /**
     * @brief Apply wet/dry mix
     * @param dry Left/right dry samples
     * @param wet Left/right wet samples
     * @param output_left Left output (modified)
     * @param output_right Right output (modified)
     */
    inline void applyWetDryMix(float dry_left, float dry_right,
                               float wet_left, float wet_right,
                               float& output_left, float& output_right);
};

} // namespace ChoirV2
