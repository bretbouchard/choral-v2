/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * ISynthesisMethod.h - PureDSP interface for synthesis methods
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#pragma once

#include <cstdint>
#include <memory>

namespace DSP {

/**
 * @brief Synthesis method parameters (PureDSP version)
 *
 * Removed JUCE dependencies, uses standard C++ types.
 */
struct SynthesisParams {
    float sample_rate;            // Sample rate (Hz)
    int32_t max_block_size;       // Maximum block size (samples)
    bool enable_simd;             // Enable SIMD optimizations
    bool enable_anti_aliasing;    // Enable anti-aliasing filter
    float oversampling_factor;    // Oversampling factor (1, 2, or 4)
};

/**
 * @brief Synthesis result (PureDSP version)
 */
struct SynthesisResult {
    bool success;                 // Synthesis successful?
    const char* error_message;    // Error details if failed (static string)
    float cpu_usage;              // CPU usage for this block (0-1)
};

/**
 * @brief PureDSP interface for synthesis methods
 *
 * All synthesis methods must implement this interface:
 * - FormantSynthesisMethod: Traditional formant filtering
 * - DiphoneSynthesisMethod: Concatenative synthesis with diphones
 * - SubharmonicSynthesisMethod: Sub-harmonic frequency generation
 *
 * PureDSP design principles:
 * - No JUCE dependencies
 * - Real-time safe (no allocations in process methods)
 * - SIMD-friendly where possible
 * - Standard C++ only (std::vector, std::array, etc.)
 * - Deterministic execution time
 *
 * Processing interface:
 * - All audio processing uses prepare() -> process() -> reset() lifecycle
 * - No dynamic memory allocation in process() methods
 * - All buffers pre-allocated during prepare()
 */
class ISynthesisMethod {
public:
    virtual ~ISynthesisMethod() = default;

    /**
     * @brief Prepare the synthesis method for processing
     * @param params Synthesis parameters
     * @return true if prepared successfully
     *
     * This method allocates all necessary memory and initializes
     * the synthesis method. Called once before processing begins.
     *
     * MUST be real-time safe (no allocations in production).
     * All allocations should happen here, not in process().
     */
    virtual bool prepare(const SynthesisParams& params) = 0;

    /**
     * @brief Process audio (single voice)
     * @param frequency Fundamental frequency (Hz)
     * @param amplitude Amplitude (0-1)
     * @param phoneme_data Phoneme data pointer (opaque data structure)
     * @param output Output buffer (mono, pre-allocated)
     * @param num_samples Number of samples to generate
     * @return Synthesis result
     *
     * CRITICAL: This method MUST be real-time safe:
     * - No memory allocations
     * - No system calls
     * - Deterministic execution time
     * - No blocking operations
     */
    virtual SynthesisResult process(
        float frequency,
        float amplitude,
        const void* phoneme_data,
        float* output,
        int32_t num_samples
    ) = 0;

    /**
     * @brief Process audio (multiple voices, SIMD-optimized)
     * @param frequencies Fundamental frequencies (Hz)
     * @param amplitudes Amplitudes (0-1)
     * @param phoneme_data_array Phoneme data for each voice
     * @param num_voices Number of voices
     * @param output Output buffer (interleaved stereo, pre-allocated)
     * @param num_samples Number of samples to generate
     * @return Synthesis result
     *
     * CRITICAL: This method MUST be real-time safe.
     */
    virtual SynthesisResult processSIMD(
        const float* frequencies,
        const float* amplitudes,
        const void* const* phoneme_data_array,
        int32_t num_voices,
        float* output,
        int32_t num_samples
    ) = 0;

    /**
     * @brief Reset synthesis state
     *
     * Resets all internal state (filters, oscillators, etc.)
     * to initial conditions. Does not deallocate memory.
     */
    virtual void reset() = 0;

    /**
     * @brief Get method name
     * @return Method name (static string, no ownership transfer)
     */
    virtual const char* getName() const = 0;

    /**
     * @brief Performance statistics
     */
    struct MethodStats {
        float average_cpu_usage;     // Average CPU usage (0-1)
        int32_t voices_processed;    // Total voices processed
        int32_t blocks_processed;    // Total blocks processed
    };

    /**
     * @brief Get performance statistics
     * @return Current statistics
     */
    virtual MethodStats getStats() const = 0;
};

/**
 * @brief Factory for creating synthesis methods (PureDSP version)
 */
class SynthesisMethodFactory {
public:
    /**
     * @brief Create a synthesis method by name
     * @param method_name "formant", "diphone", or "subharmonic"
     * @return Synthesis method, or nullptr if invalid name
     *
     * Caller owns the returned pointer and must delete it.
     */
    static std::unique_ptr<ISynthesisMethod> create(
        const char* method_name
    );

    /**
     * @brief Get list of available methods
     * @return Array of method names (static strings)
     */
    static const char* const* getAvailableMethods(int32_t* out_count);
};

} // namespace DSP
