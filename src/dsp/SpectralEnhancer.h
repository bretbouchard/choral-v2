/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * SpectralEnhancer.h - Spectral envelope enhancement
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#pragma once

#include <vector>

namespace ChoirV2 {

/**
 * @brief Spectral enhancer
 *
 * Enhances the spectral envelope of synthesized speech
 * to improve brightness and clarity.
 *
 * CRITICAL FIX (from DSP review):
 * - OLD: Simple FFT without overlap-add (spectral leakage)
 * - NEW: Overlap-add FFT with windowing (correct)
 *
 * Uses:
 * - STFT (Short-Time Fourier Transform)
 * - Overlap-add processing (50% overlap)
 * - Hanning window (reduces spectral leakage)
 * - Spectral contrast enhancement
 */
class SpectralEnhancer {
public:
    SpectralEnhancer();
    ~SpectralEnhancer();

    bool initialize(float sample_rate, int fft_size);
    void process(float* audio, int num_samples);
    void reset();
    void setEnhancementAmount(float amount);  // 0-1

private:
    float sample_rate_;
    int fft_size_;
    float enhancement_amount_;

    // FFT buffers (overlap-add)
    std::vector<float> input_buffer_;
    std::vector<float> output_buffer_;
    std::vector<float> window_;
    int hop_size_;

    void processFrame(float* frame);
    void applyWindow(float* buffer);
};

} // namespace ChoirV2
