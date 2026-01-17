/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * LinearSmoother.cpp - Parameter smoothing implementation
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#include "LinearSmoother.h"
#include <cmath>
#include <vector>
#include <algorithm>

namespace ChoirV2 {

//==============================================================================
// LinearSmoother Implementation
//==============================================================================

LinearSmoother::LinearSmoother()
    : current_value_(0.0f)
    , target_value_(0.0f)
    , alpha_(1.0f)
    , sample_rate_(44100.0f)
    , time_constant_(0.01f)  // 10ms default
{
}

void LinearSmoother::setTimeConstant(float time_constant, float sample_rate)
{
    time_constant_ = time_constant;
    sample_rate_ = sample_rate;
    updateAlpha();
}

void LinearSmoother::setTarget(float target)
{
    target_value_ = target;
}

void LinearSmoother::setTargetImmediate(float target)
{
    target_value_ = target;
    current_value_ = target;
}

float LinearSmoother::process()
{
    // Exponential smoothing: y[n] = alpha * x[n] + (1 - alpha) * y[n-1]
    current_value_ = alpha_ * target_value_ + (1.0f - alpha_) * current_value_;
    return current_value_;
}

void LinearSmoother::processBlock(float* output, int num_samples)
{
    for (int i = 0; i < num_samples; ++i) {
        output[i] = process();
    }
}

void LinearSmoother::reset()
{
    current_value_ = target_value_;
}

void LinearSmoother::updateAlpha()
{
    // Calculate alpha from time constant
    // For exponential smoothing: alpha = 1 - exp(-1 / (tau * fs))
    // where tau is time constant in seconds, fs is sample rate
    if (time_constant_ > 0.0f && sample_rate_ > 0.0f) {
        float num_samples = time_constant_ * sample_rate_;
        alpha_ = 1.0f - std::exp(-1.0f / num_samples);
    } else {
        alpha_ = 1.0f;  // No smoothing
    }
}

//==============================================================================
// SIMDLinearSmoother Implementation
//==============================================================================

SIMDLinearSmoother::SIMDLinearSmoother(int num_parameters)
    : num_parameters_(num_parameters)
    , alpha_(1.0f)
    , sample_rate_(44100.0f)
    , time_constant_(0.01f)
{
    current_values_.resize(num_parameters, 0.0f);
    target_values_.resize(num_parameters, 0.0f);
}

SIMDLinearSmoother::~SIMDLinearSmoother() = default;

void SIMDLinearSmoother::setTimeConstant(float time_constant, float sample_rate)
{
    time_constant_ = time_constant;
    sample_rate_ = sample_rate;
    updateAlpha();
}

void SIMDLinearSmoother::setTargets(const float* targets)
{
    std::copy(targets, targets + num_parameters_, target_values_.begin());
}

void SIMDLinearSmoother::processBlock(float* output, int num_samples)
{
    for (int i = 0; i < num_samples; ++i) {
        for (int j = 0; j < num_parameters_; ++j) {
            // Exponential smoothing for each parameter
            current_values_[j] = alpha_ * target_values_[j] +
                                (1.0f - alpha_) * current_values_[j];
            output[j * num_samples + i] = current_values_[j];
        }
    }
}

void SIMDLinearSmoother::reset()
{
    std::copy(target_values_.begin(), target_values_.end(), current_values_.begin());
}

void SIMDLinearSmoother::updateAlpha()
{
    if (time_constant_ > 0.0f && sample_rate_ > 0.0f) {
        float num_samples = time_constant_ * sample_rate_;
        alpha_ = 1.0f - std::exp(-1.0f / num_samples);
    } else {
        alpha_ = 1.0f;
    }
}

} // namespace ChoirV2
