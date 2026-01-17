/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * ISynthesisMethod.h - Interface for synthesis methods
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#pragma once

#include <memory>
#include <vector>
#include <string>

namespace ChoirV2 {

// Forward declarations
struct Phoneme;
class Voice;

/**
 * @brief Synthesis method parameters
 */
struct SynthesisParams {
    float sample_rate;            // Sample rate (Hz)
    int max_block_size;           // Maximum block size (samples)
    bool enable_simd;             // Enable SIMD optimizations
    bool enable_anti_aliasing;    // Enable anti-aliasing filter
    float oversampling_factor;    // Oversampling factor (1, 2, or 4)
};

/**
 * @brief Synthesis result
 */
struct SynthesisResult {
    bool success;                 // Synthesis successful?
    std::string error_message;    // Error details if failed
    float cpu_usage;              // CPU usage for this block (0-1)
};

/**
 * @brief Interface for synthesis methods
 *
 * All synthesis methods must implement this interface:
 * - FormantSynthesis: Traditional formant filtering
 * - DiphoneSynthesis: Concatenative synthesis with diphones
 * - SubharmonicSynthesis: Sub-harmonic frequency generation
 *
 * This allows plugging in different synthesis algorithms
 * without changing the rest of the engine.
 */
class ISynthesisMethod {
public:
    virtual ~ISynthesisMethod() = default;

    /**
     * @brief Initialize the synthesis method
     * @param params Synthesis parameters
     * @return true if initialized successfully
     */
    virtual bool initialize(const SynthesisParams& params) = 0;

    /**
     * @brief Process a single voice
     * @param voice Voice to synthesize
     * @param phoneme Current phoneme to synthesize
     * @param output Output buffer (mono)
     * @param num_samples Number of samples to generate
     * @return Synthesis result
     */
    virtual SynthesisResult synthesizeVoice(
        Voice* voice,
        const Phoneme* phoneme,
        float* output,
        int num_samples
    ) = 0;

    /**
     * @brief Process multiple voices (SIMD-optimized)
     * @param voices Voices to synthesize
     * @param phonemes Phonemes for each voice
     * @param output Output buffer (interleaved stereo)
     * @param num_samples Number of samples to generate
     * @return Synthesis result
     */
    virtual SynthesisResult synthesizeVoicesSIMD(
        const std::vector<Voice*>& voices,
        const std::vector<const Phoneme*>& phonemes,
        float* output,
        int num_samples
    ) = 0;

    /**
     * @brief Reset synthesis state
     */
    virtual void reset() = 0;

    /**
     * @brief Get method name
     */
    virtual std::string getName() const = 0;

    /**
     * @brief Get performance statistics
     */
    struct MethodStats {
        float average_cpu_usage;     // Average CPU usage (0-1)
        int voices_processed;        // Total voices processed
        int blocks_processed;        // Total blocks processed
    };
    virtual MethodStats getStats() const = 0;
};

/**
 * @brief Factory for creating synthesis methods
 */
class SynthesisMethodFactory {
public:
    /**
     * @brief Create a synthesis method by name
     * @param method_name "formant", "diphone", or "subharmonic"
     * @return Synthesis method, or nullptr if invalid name
     */
    static std::unique_ptr<ISynthesisMethod> create(
        const std::string& method_name
    );

    /**
     * @brief Get list of available methods
     */
    static std::vector<std::string> getAvailableMethods();
};

} // namespace ChoirV2
