/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * LinearSmoother.h - Parameter smoothing (prevents clicks)
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#pragma once

#include <vector>

namespace ChoirV2 {

/**
 * @brief Linear parameter smoother
 *
 * Smooths parameter changes to prevent clicks and artifacts
 * during phoneme transitions and parameter automation.
 *
 * Uses exponential smoothing:
 * y[n] = alpha * x[n] + (1 - alpha) * y[n-1]
 *
 * Where alpha is computed from the smoothing time constant.
 */
class LinearSmoother {
public:
    LinearSmoother();
    ~LinearSmoother() = default;

    /**
     * @brief Set smoothing time constant
     * @param time_constant Smoothing time (seconds)
     * @param sample_rate Sample rate (Hz)
     */
    void setTimeConstant(float time_constant, float sample_rate);

    /**
     * @brief Set target value
     * @param target Target value
     */
    void setTarget(float target);

    /**
     * @brief Set target with immediate reset
     * @param target Target value (jumps to this value immediately)
     */
    void setTargetImmediate(float target);

    /**
     * @brief Get current smoothed value
     * @return Current smoothed value
     */
    float getCurrent() const { return current_value_; }

    /**
     * @brief Process a single sample
     * @return Smoothed sample
     */
    float process();

    /**
     * @brief Process a block of samples
     * @param output Output buffer
     * @param num_samples Number of samples
     */
    void processBlock(float* output, int num_samples);

    /**
     * @brief Reset to target value
     */
    void reset();

private:
    float current_value_;           // Current smoothed value
    float target_value_;            // Target value
    float alpha_;                   // Smoothing coefficient
    float sample_rate_;             // Sample rate (Hz)
    float time_constant_;           // Smoothing time (seconds)

    // Helper methods
    void updateAlpha();
};

/**
 * @brief SIMD batch smoother (for multiple parameters)
 *
 * Smooths multiple parameters simultaneously using SIMD.
 * Used for smoothing formant frequencies for all voices.
 */
class SIMDLinearSmoother {
public:
    SIMDLinearSmoother(int num_parameters);
    ~SIMDLinearSmoother();

    /**
     * @brief Set smoothing time constant
     * @param time_constant Smoothing time (seconds)
     * @param sample_rate Sample rate (Hz)
     */
    void setTimeConstant(float time_constant, float sample_rate);

    /**
     * @brief Set target values (SIMD-optimized)
     * @param targets Target values array
     */
    void setTargets(const float* targets);

    /**
     * @brief Process a block of samples (SIMD-optimized)
     * @param output Output buffer (num_parameters x num_samples)
     * @param num_samples Number of samples
     */
    void processBlock(float* output, int num_samples);

    /**
     * @brief Reset to target values
     */
    void reset();

private:
    int num_parameters_;
    std::vector<float> current_values_;
    std::vector<float> target_values_;
    float alpha_;
    float sample_rate_;
    float time_constant_;

    void updateAlpha();
};

} // namespace ChoirV2
