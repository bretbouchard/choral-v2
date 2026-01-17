/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * ChoirV2Engine.h - Main synthesis engine
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
class PhonemeDatabase;
class LanguageLoader;
class G2PEngine;
class VoiceManager;
class ISynthesisMethod;

/**
 * @brief Choir V2.0 main engine parameters
 */
struct ChoirV2Params {
    // Voice configuration
    int num_voices;               // Number of simultaneous voices (40-60)
    float sample_rate;            // Sample rate (Hz)
    int max_block_size;           // Maximum block size (samples)

    // Synthesis parameters
    std::string synthesis_method; // "formant", "diphone", "subharmonic"
    bool enable_simd;             // Enable SIMD optimizations

    // Performance parameters
    float cpu_limit;              // CPU usage limit (0.0-1.0)
    int voice_stealing_threshold; // Voices before stealing starts

    // Quality parameters
    bool enable_anti_aliasing;    // Enable anti-aliasing filter
    bool enable_spectral_enhancement;  // Enhance spectral envelope
    float oversampling_factor;    // Oversampling for anti-aliasing (1x, 2x, 4x)
};

/**
 * @brief Choir V2.0 main synthesis engine
 *
 * Orchestrates all components:
 * - Loads languages and phonemes
 * - Converts text to phonemes
 * - Manages multiple voices
 * - Applies synthesis methods
 * - Outputs audio
 */
class ChoirV2Engine {
public:
    explicit ChoirV2Engine(const ChoirV2Params& params);
    ~ChoirV2Engine();

    /**
     * @brief Initialize the engine
     * @return true if initialized successfully
     */
    bool initialize();

    /**
     * @brief Load a language
     * @param language_file Path to JSON file
     * @return true if loaded successfully
     */
    bool loadLanguage(const std::string& language_file);

    /**
     * @brief Synthesize text to audio
     * @param text Input text
     * @param output Buffer to receive audio (interleaved stereo)
     * @param num_samples Number of samples to generate
     * @return true if synthesis succeeded
     */
    bool synthesize(
        const std::string& text,
        float* output,
        int num_samples
    );

    /**
     * @brief Synthesize with custom melody
     * @param text Input text
     * @param frequencies Pitch frequencies for each phoneme (Hz)
     * @param output Buffer to receive audio
     * @param num_samples Number of samples to generate
     * @return true if synthesis succeeded
     */
    bool synthesizeWithMelody(
        const std::string& text,
        const std::vector<float>& frequencies,
        float* output,
        int num_samples
    );

    /**
     * @brief Set synthesis method
     * @param method "formant", "diphone", or "subharmonic"
     * @return true if method set successfully
     */
    bool setSynthesisMethod(const std::string& method);

    /**
     * @brief Get current engine parameters
     */
    const ChoirV2Params& getParams() const { return params_; }

    /**
     * @brief Get performance statistics
     */
    struct PerfStats {
        float cpu_usage;              // Current CPU usage (0-1)
        int active_voices;            // Currently active voices
        int stolen_voices;            // Voices stolen due to CPU limit
        float average_latency;        // Average latency (ms)
        int buffer_underruns;         // Number of buffer underruns
    };
    PerfStats getPerformanceStats() const;

    /**
     * @brief Shutdown the engine and release resources
     */
    void shutdown();

private:
    ChoirV2Params params_;

    // Components
    std::shared_ptr<PhonemeDatabase> phoneme_db_;
    std::shared_ptr<LanguageLoader> language_loader_;
    std::shared_ptr<G2PEngine> g2p_engine_;
    std::shared_ptr<VoiceManager> voice_manager_;
    std::shared_ptr<ISynthesisMethod> synthesis_method_;

    // Internal state
    bool initialized_;
    PerfStats perf_stats_;

    // Helper methods
    bool allocateVoices();
    void processAudio(float* output, int num_samples);
    void updatePerformanceStats();
};

} // namespace ChoirV2
