#ifndef CHOIR_V2_SUBHARMONIC_GENERATOR_H
#define CHOIR_V2_SUBHARMONIC_GENERATOR_H

#include <complex>

namespace ChoirV2 {

/**
 * @brief Subharmonic generator with proper Phase-Locked Loop
 *
 * CRITICAL FIX: This implementation fixes the critical bug where a simple
 * one-pole filter was used instead of a proper PLL, causing phase drift over time.
 *
 * The PLL ensures the subharmonic phase is locked to the fundamental phase
 * divided by the ratio, maintaining harmonic relationships even as frequency
 * changes and over extended periods.
 *
 * PLL Architecture:
 * 1. Phase error detection with wrap-around handling
 * 2. PI controller (proportional + integral) for robust tracking
 * 3. Independent fundamental phase accumulation
 * 4. Subharmonic phase locked to fundamental/ratio
 */
class SubharmonicGenerator {
public:
    SubharmonicGenerator();
    ~SubharmonicGenerator();

    /**
     * @brief Generate subharmonic output with phase-locked loop
     * @param fundamentalFreq Fundamental frequency (Hz)
     * @param sampleRate Sample rate (Hz)
     * @return Subharmonic sine wave sample
     *
     * The PLL tracks the fundamental phase and generates a phase-locked
     * subharmonic at the specified ratio. Phase error is continuously
     * minimized through the PI controller.
     */
    float generate(float fundamentalFreq, double sampleRate);

    /**
     * @brief Reset PLL state
     * Clears all phase accumulators and error integrators
     */
    void reset();

    /**
     * @brief Set subharmonic ratio
     * @param r Ratio (0.5 = ÷2, 0.33 = ÷3, 0.25 = ÷4)
     */
    void setRatio(float r) { ratio = r; }

    /**
     * @brief Set mix level
     * @param m Mix amount (0.0-1.0)
     */
    void setMix(float m) { mix = m; }

    /**
     * @brief Get PLL phase error (for debugging)
     * @return Current phase error in radians
     */
    float getPhaseError() const { return pllPhaseError; }

    /**
     * @brief Get fundamental phase (for debugging)
     * @return Current fundamental phase in radians
     */
    float getFundamentalPhase() const { return phase; }

    /**
     * @brief Get subharmonic phase (for debugging)
     * @return Current subharmonic phase in radians
     */
    float getSubharmonicPhase() const { return pllPhase; }

private:
    // Subharmonic parameters
    float ratio = 0.5f;        // Division ratio (0.5 = octave down)
    float mix = 0.3f;          // Output mix level

    // Phase tracking
    float phase = 0.0f;        // Fundamental phase accumulator (radians)
    float pllPhase = 0.0f;     // Subharmonic phase (PLL output, radians)

    // PLL state
    float pllPhaseError = 0.0f; // Phase error (radians, wrapped to ±π)
    float pllIntegral = 0.0f;   // Integral accumulator (for I term)
    float pllKp = 0.1f;        // Proportional gain (response speed)
    float pllKi = 0.001f;      // Integral gain (steady-state accuracy)

    /**
     * @brief Wrap phase to [-π, π] range
     * @param p Phase to wrap
     * @return Wrapped phase
     *
     * Phase wrapping is critical for correct phase error calculation.
     * Without wrapping, the PLL would "unwind" and lose lock.
     */
    float wrapPhase(float p) const;

    // TODO: Add JUCE_LEAK_DETECTOR when JUCE is integrated
    // JUCE_LEAK_DETECTOR(SubharmonicGenerator)
};

} // namespace ChoirV2

#endif // CHOIR_V2_SUBHARMONIC_GENERATOR_H
