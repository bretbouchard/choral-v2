/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * Voice.h - Voice data structure
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#pragma once

#include <string>
#include <array>

namespace ChoirV2 {

/**
 * @brief Voice state
 *
 * Represents a single voice in the choir.
 * Each voice has its own pitch, amplitude, and synthesis state.
 */
class Voice {
public:
    Voice();
    ~Voice() = default;

    /**
     * @brief Set fundamental frequency
     * @param f0 Frequency in Hz
     */
    void setFrequency(float f0) { f0_ = f0; }

    /**
     * @brief Get fundamental frequency
     * @return Frequency in Hz
     */
    float getFrequency() const { return f0_; }

    /**
     * @brief Set amplitude
     * @param amplitude Amplitude (0-1)
     */
    void setAmplitude(float amplitude) { amplitude_ = amplitude; }

    /**
     * @brief Get amplitude
     * @return Amplitude (0-1)
     */
    float getAmplitude() const { return amplitude_; }

    /**
     * @brief Set pan position
     * @param pan Pan position (-1 to 1, -1 = left, 1 = right)
     */
    void setPan(float pan) { pan_ = pan; }

    /**
     * @brief Get pan position
     * @return Pan position (-1 to 1)
     */
    float getPan() const { return pan_; }

    /**
     * @brief Set voice active state
     * @param active true if voice is active
     */
    void setActive(bool active) { is_active_ = active; }

    /**
     * @brief Check if voice is active
     * @return true if voice is active
     */
    bool isActive() const { return is_active_; }

    /**
     * @brief Reset voice state
     */
    void reset();

private:
    float f0_ = 220.0f;           // Fundamental frequency (Hz)
    float amplitude_ = 0.5f;      // Amplitude (0-1)
    float pan_ = 0.0f;            // Pan position (-1 to 1)
    bool is_active_ = false;      // Active state
};

} // namespace ChoirV2
