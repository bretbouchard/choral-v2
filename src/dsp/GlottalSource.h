/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * GlottalSource.h - Glottal pulse train generator
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#pragma once

namespace ChoirV2 {

/**
 * @brief Glottal source model type
 */
enum class GlottalModel {
    Rosenberg,     // Rosenberg wave (classic)
    LF,            // Liljencrants-Fant (more accurate)
    Differentiated // Differentiated glottal flow
};

/**
 * @brief Glottal pulse train generator
 *
 * Generates the glottal source signal for vocal synthesis.
 * The glottal source is the excitation signal that is
 * subsequently filtered by the formant resonators.
 *
 * Models:
 * - Rosenberg: Simple, widely used
 * - LF: More accurate, asymmetric
 * - Differentiated: Direct glottal flow derivative
 */
class GlottalSource {
public:
    GlottalSource();
    ~GlottalSource() = default;

    /**
     * @brief Set fundamental frequency
     * @param f0 Frequency in Hz
     */
    void setFrequency(float f0);

    /**
     * @brief Set glottal model
     * @param model Model type
     */
    void setModel(GlottalModel model);

    /**
     * @brief Set pulse shape parameters
     * @param open_quotient Open quotient (0-1)
     * @param speed_quotient Speed quotient (0-1)
     * @param return_phase Return phase (0-1)
     */
    void setPulseShape(
        float open_quotient,
        float speed_quotient,
        float return_phase
    );

    /**
     * @brief Set sample rate
     * @param sample_rate Sample rate (Hz)
     */
    void setSampleRate(float sample_rate);

    /**
     * @brief Generate a single sample
     * @return Glottal source sample
     */
    float process();

    /**
     * @brief Generate a block of samples
     * @param output Output buffer
     * @param num_samples Number of samples
     */
    void processBlock(float* output, int num_samples);

    /**
     * @brief Reset phase
     */
    void reset();

private:
    // Parameters
    float f0_;                      // Fundamental frequency (Hz)
    float sample_rate_;             // Sample rate (Hz)
    GlottalModel model_;            // Glottal model

    // Pulse shape
    float open_quotient_;           // Open quotient (0-1)
    float speed_quotient_;          // Speed quotient (0-1)
    float return_phase_;            // Return phase (0-1)

    // Phase accumulator
    double phase_;                  // Current phase (0-1)
    double phase_increment_;        // Phase increment per sample

    // Helper methods
    float rosenbergPulse(double phase);
    float lfPulse(double phase);
    float differentiatedPulse(double phase);

    void updatePhaseIncrement();
};

} // namespace ChoirV2
