/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * FormantResonator.h - Real-time formant filtering resonator
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#pragma once

#include <array>

namespace ChoirV2 {

/**
 * @brief Formant resonator (biquad filter)
 *
 * Implements a second-order IIR bandpass filter for formant synthesis.
 *
 * CRITICAL FIX (from DSP review):
 * - OLD: Used complex pole math (incorrect)
 * - NEW: Uses real biquad coefficients (correct)
 *
 * Formant equation:
 * H(z) = (b0 + b1*z^-1 + b2*z^-2) / (a0 + a1*z^-1 + a2*z^-2)
 */
class FormantResonator {
public:
    FormantResonator();
    ~FormantResonator() = default;

    void setParameters(float center_frequency, float bandwidth, float sample_rate);
    float process(float input);
    void processBlock(const float* input, float* output, int num_samples);
    void reset();
    std::array<float, 5> getCoefficients() const;

private:
    float b0_, b1_, b2_;
    float a0_, a1_, a2_;
    float x1_, x2_;
    float y1_, y2_;
    float center_frequency_;
    float bandwidth_;
    float sample_rate_;

    void computeCoefficients();
};

} // namespace ChoirV2
