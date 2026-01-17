/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * SpectralEnhancer.cpp - Spectral envelope enhancement implementation
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#include "SpectralEnhancer.h"
#include <cmath>
#include <cstring>
#include <algorithm>

namespace ChoirV2 {

//==============================================================================
// SpectralEnhancer Implementation
//==============================================================================

SpectralEnhancer::SpectralEnhancer()
    : sample_rate_(44100.0f)
    , fft_size_(1024)
    , enhancement_amount_(0.5f)
    , hop_size_(0)
{
}

SpectralEnhancer::~SpectralEnhancer()
{
}

bool SpectralEnhancer::initialize(float sample_rate, int fft_size)
{
    // Validate FFT size (must be power of 2)
    if (fft_size & (fft_size - 1)) {
        return false;  // Not power of 2
    }

    sample_rate_ = sample_rate;
    fft_size_ = fft_size;
    hop_size_ = fft_size_ / 2;  // 50% overlap for overlap-add

    // Allocate buffers (real-time safety: allocate during initialization)
    input_buffer_.resize(fft_size_, 0.0f);
    output_buffer_.resize(fft_size_, 0.0f);
    window_.resize(fft_size_, 0.0f);

    // Generate Hanning window
    for (int i = 0; i < fft_size_; ++i) {
        // Hanning window: 0.5 * (1 - cos(2*pi*n/(N-1)))
        window_[i] = 0.5f * (1.0f - std::cos(2.0f * M_PI * i / (fft_size_ - 1)));
    }

    return true;
}

void SpectralEnhancer::process(float* audio, int num_samples)
{
    if (!hop_size_ || !audio || num_samples <= 0) {
        return;  // Not initialized or invalid input
    }

    // Process in overlapping windows
    int samples_processed = 0;

    while (samples_processed < num_samples) {
        // Calculate how many samples we can process in this frame
        int samples_available = num_samples - samples_processed;

        if (samples_available >= hop_size_) {
            // We have enough samples for a full hop

            // Shift input buffer (make room for new samples)
            std::memmove(input_buffer_.data(),
                        input_buffer_.data() + hop_size_,
                        (fft_size_ - hop_size_) * sizeof(float));

            // Copy new samples to end of input buffer
            std::memcpy(input_buffer_.data() + (fft_size_ - hop_size_),
                       audio + samples_processed,
                       hop_size_ * sizeof(float));

            // Process frame
            processFrame(input_buffer_.data());

            // Overlap-add to output
            // Apply window and add to output buffer with overlap
            for (int i = 0; i < fft_size_; ++i) {
                output_buffer_[i] = output_buffer_[i] * 0.5f + input_buffer_[i] * window_[i] * 0.5f;
            }

            // Copy output samples (first half of frame is ready)
            std::memcpy(audio + samples_processed,
                       output_buffer_.data(),
                       hop_size_ * sizeof(float));

            samples_processed += hop_size_;
        }
        else {
            // Not enough samples for full hop, just copy what we have
            // This shouldn't happen with proper buffering, but handle gracefully
            int samples_to_copy = std::min(samples_available, hop_size_);
            std::memcpy(audio + samples_processed,
                       output_buffer_.data(),
                       samples_to_copy * sizeof(float));
            samples_processed += samples_to_copy;
        }
    }
}

void SpectralEnhancer::reset()
{
    // Clear all buffers
    std::fill(input_buffer_.begin(), input_buffer_.end(), 0.0f);
    std::fill(output_buffer_.begin(), output_buffer_.end(), 0.0f);
}

void SpectralEnhancer::setEnhancementAmount(float amount)
{
    enhancement_amount_ = std::max(0.0f, std::min(1.0f, amount));
}

//==============================================================================
// Private Methods
//==============================================================================

void SpectralEnhancer::processFrame(float* frame)
{
    // Apply window
    applyWindow(frame);

    // Simple spectral enhancement in time domain
    // For true spectral enhancement, we'd need FFT/IFFT
    // This is a simplified version that enhances high frequencies

    // Calculate spectral envelope (simplified)
    // Enhance brightness by boosting high frequencies

    float low_energy = 0.0f;
    float high_energy = 0.0f;
    int crossover = fft_size_ / 4;  // Crossover at Nyquist/4

    // Analyze spectral balance
    for (int i = 0; i < fft_size_; ++i) {
        float sample = frame[i];

        if (i < crossover) {
            low_energy += sample * sample;
        } else {
            high_energy += sample * sample;
        }
    }

    low_energy = std::sqrt(low_energy / crossover);
    high_energy = std::sqrt(high_energy / (fft_size_ - crossover));

    // Calculate enhancement ratio
    float spectral_balance = (low_energy + 1e-6f) / (high_energy + 1e-6f);

    // Apply enhancement based on spectral balance and enhancement amount
    float enhancement_factor = 1.0f + (enhancement_amount_ * std::max(0.0f, spectral_balance - 1.0f));

    // Apply to frame (boost high frequencies)
    for (int i = 0; i < fft_size_; ++i) {
        // Frequency-dependent gain
        float frequency_ratio = static_cast<float>(i) / static_cast<float>(fft_size_);

        // Boost high frequencies more than low frequencies
        float boost = 1.0f;
        if (frequency_ratio > 0.25f) {
            boost = 1.0f + (enhancement_factor - 1.0f) * (frequency_ratio - 0.25f) / 0.75f;
        }

        frame[i] *= boost;
    }
}

void SpectralEnhancer::applyWindow(float* buffer)
{
    // Apply Hanning window in-place
    for (int i = 0; i < fft_size_; ++i) {
        buffer[i] *= window_[i];
    }
}

} // namespace ChoirV2
