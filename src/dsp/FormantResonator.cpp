/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * FormantResonator.cpp - Real-time formant filtering resonator implementation
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#include "FormantResonator.h"
#include <cmath>

namespace ChoirV2 {

//==============================================================================
// FormantResonator Implementation
//==============================================================================

FormantResonator::FormantResonator()
    : b0_(0.0f), b1_(0.0f), b2_(0.0f)
    , a0_(1.0f), a1_(0.0f), a2_(0.0f)
    , x1_(0.0f), x2_(0.0f)
    , y1_(0.0f), y2_(0.0f)
    , center_frequency_(1000.0f)
    , bandwidth_(100.0f)
    , sample_rate_(44100.0f)
{
}

void FormantResonator::setParameters(float center_frequency, float bandwidth, float sample_rate)
{
    center_frequency_ = center_frequency;
    bandwidth_ = bandwidth;
    sample_rate_ = sample_rate;
    computeCoefficients();
}

float FormantResonator::process(float input)
{
    // Biquad difference equation:
    // y[n] = (b0*x[n] + b1*x[n-1] + b2*x[n-2] - a1*y[n-1] - a2*y[n-2]) / a0

    float output = (b0_ * input + b1_ * x1_ + b2_ * x2_ - a1_ * y1_ - a2_ * y2_) / a0_;

    // Update state
    x2_ = x1_;
    x1_ = input;
    y2_ = y1_;
    y1_ = output;

    return output;
}

void FormantResonator::processBlock(const float* input, float* output, int num_samples)
{
    for (int i = 0; i < num_samples; ++i) {
        output[i] = process(input[i]);
    }
}

void FormantResonator::reset()
{
    x1_ = 0.0f;
    x2_ = 0.0f;
    y1_ = 0.0f;
    y2_ = 0.0f;
}

std::array<float, 5> FormantResonator::getCoefficients() const
{
    return {b0_, b1_, b2_, a1_, a2_};
}

void FormantResonator::computeCoefficients()
{
    // Compute biquad bandpass filter coefficients for formant resonance
    //
    // Using the "RBJ Audio EQ Cookbook" bandpass filter design:
    // H(s) = s / (s^2 + s/Q + 1)  (normalized)
    //
    // Where:
    // - omega = 2 * pi * f0 / fs
    // - alpha = sin(omega) / (2 * Q)
    // - Q = f0 / bandwidth
    //
    // Biquad coefficients:
    // b0 = alpha
    // b1 = 0
    // b2 = -alpha
    // a0 = 1 + alpha
    // a1 = -2 * cos(omega)
    // a2 = 1 - alpha

    // Prevent division by zero
    if (sample_rate_ <= 0.0f || center_frequency_ <= 0.0f || bandwidth_ <= 0.0f) {
        b0_ = 1.0f;
        b1_ = 0.0f;
        b2_ = 0.0f;
        a0_ = 1.0f;
        a1_ = 0.0f;
        a2_ = 0.0f;
        return;
    }

    // Compute Q factor from bandwidth
    // Q = f0 / BW
    float q_factor = center_frequency_ / bandwidth_;

    // Normalize frequency
    float omega = 2.0f * M_PI * center_frequency_ / sample_rate_;

    // Compute sin and cos
    float sin_omega = std::sin(omega);
    float cos_omega = std::cos(omega);

    // Compute alpha
    float alpha = sin_omega / (2.0f * q_factor);

    // Compute coefficients
    b0_ = alpha;
    b1_ = 0.0f;
    b2_ = -alpha;
    a0_ = 1.0f + alpha;
    a1_ = -2.0f * cos_omega;
    a2_ = 1.0f - alpha;

    // Normalize by a0
    b0_ /= a0_;
    b1_ /= a0_;
    b2_ /= a0_;
    a1_ /= a0_;
    a2_ /= a0_;
    a0_ = 1.0f;
}

} // namespace ChoirV2
