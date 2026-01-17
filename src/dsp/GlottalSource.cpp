/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * GlottalSource.cpp - Glottal pulse train generator implementation
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#include "GlottalSource.h"
#include <cmath>
#include <algorithm>

namespace ChoirV2 {

//==============================================================================
// GlottalSource Implementation
//==============================================================================

GlottalSource::GlottalSource()
    : f0_(110.0f)
    , sample_rate_(44100.0f)
    , model_(GlottalModel::Rosenberg)
    , open_quotient_(0.5f)
    , speed_quotient_(0.5f)
    , return_phase_(0.1f)
    , phase_(0.0)
    , phase_increment_(0.0)
{
    updatePhaseIncrement();
}

void GlottalSource::setFrequency(float f0)
{
    f0_ = std::max(20.0f, std::min(1000.0f, f0));  // Clamp to 20-1000 Hz
    updatePhaseIncrement();
}

void GlottalSource::setModel(GlottalModel model)
{
    model_ = model;
}

void GlottalSource::setPulseShape(
    float open_quotient,
    float speed_quotient,
    float return_phase
)
{
    open_quotient_ = std::max(0.1f, std::min(0.9f, open_quotient));
    speed_quotient_ = std::max(0.1f, std::min(0.9f, speed_quotient));
    return_phase_ = std::max(0.0f, std::min(0.5f, return_phase));
}

void GlottalSource::setSampleRate(float sample_rate)
{
    sample_rate_ = std::max(8000.0f, std::min(192000.0f, sample_rate));
    updatePhaseIncrement();
}

float GlottalSource::process()
{
    // Generate pulse based on current phase
    float output = 0.0f;

    switch (model_) {
        case GlottalModel::Rosenberg:
            output = rosenbergPulse(phase_);
            break;
        case GlottalModel::LF:
            output = lfPulse(phase_);
            break;
        case GlottalModel::Differentiated:
            output = differentiatedPulse(phase_);
            break;
    }

    // Advance phase
    phase_ += phase_increment_;
    if (phase_ >= 1.0) {
        phase_ -= 1.0;
    }

    return output;
}

void GlottalSource::processBlock(float* output, int num_samples)
{
    // Real-time safety: No allocation, just generate samples
    for (int i = 0; i < num_samples; ++i) {
        output[i] = process();
    }
}

void GlottalSource::reset()
{
    phase_ = 0.0;
}

//==============================================================================
// Helper Methods
//==============================================================================

float GlottalSource::rosenbergPulse(double phase)
{
    // Rosenberg wave: Classic glottal pulse model
    // Consists of opening (sinusoidal) and closing (exponential) phases

    double t = phase;  // 0 to 1

    // Calculate phase boundaries
    double t_open = open_quotient_;  // Opening phase ends here
    double t_return = t_open + (1.0 - open_quotient_) * speed_quotient_;  // Return phase ends here

    float sample = 0.0f;

    if (t < t_open) {
        // Opening phase: Sinusoidal rise
        double phase_norm = t / t_open;
        sample = static_cast<float>(0.5 * (1.0 - std::cos(M_PI * phase_norm)));
    }
    else if (t < t_return) {
        // Return phase: Exponential decay
        double phase_norm = (t - t_open) / (t_return - t_open);
        sample = static_cast<float>(std::exp(-3.0 * phase_norm));
    }
    else {
        // Closed phase: Zero
        sample = 0.0f;
    }

    return sample;
}

float GlottalSource::lfPulse(double phase)
{
    // Liljencrants-Fant model: More accurate asymmetric pulse
    // Based on Fant's LF model with shaping parameters

    double t = phase;  // 0 to 1

    // LF model parameters (derived from OQ and SQ)
    double alpha = 1.0 / (open_quotient_ * open_quotient_);
    double epsilon = 1.0 / ((1.0 - open_quotient_) * speed_quotient_);

    // Calculate time points
    double t_open = open_quotient_;
    double t_peak = open_quotient_ * 0.7;  // Peak at 70% of opening
    double t_return = t_open + (1.0 - t_open) * 0.9;  // Return phase

    float sample = 0.0f;

    if (t < t_open) {
        // Opening phase
        if (t < t_peak) {
            // Rising portion
            double phase_norm = t / t_peak;
            sample = static_cast<float>(std::pow(phase_norm, alpha));
        }
        else {
            // Falling portion (still in opening)
            double phase_norm = (t - t_peak) / (t_open - t_peak);
            sample = static_cast<float>(std::pow(1.0 - phase_norm, alpha));
        }
    }
    else if (t < t_return) {
        // Return phase: Exponential decay
        double phase_norm = (t - t_open) / (t_return - t_open);
        sample = static_cast<float>(std::exp(-epsilon * phase_norm));
    }
    else {
        // Closed phase
        sample = 0.0f;
    }

    return sample;
}

float GlottalSource::differentiatedPulse(double phase)
{
    // Differentiated glottal flow: Derivative of Rosenberg pulse
    // Used in some synthesis models as excitation

    double t = phase;
    double delta = 0.001;  // Small step for numerical differentiation

    double y1 = rosenbergPulse(t);
    double y2 = rosenbergPulse(t + delta);

    // Derivative (scaled)
    float derivative = static_cast<float>((y2 - y1) / delta);

    // Scale to reasonable amplitude
    return derivative * 0.1f;
}

void GlottalSource::updatePhaseIncrement()
{
    // Phase increment = frequency / sample_rate
    phase_increment_ = f0_ / sample_rate_;

    // Ensure phase increment is reasonable
    phase_increment_ = std::max(0.0, std::min(1.0, phase_increment_));
}

} // namespace ChoirV2
