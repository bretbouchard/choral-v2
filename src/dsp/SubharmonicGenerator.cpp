#include "SubharmonicGenerator.h"
#include <cmath>

namespace ChoirV2 {

SubharmonicGenerator::SubharmonicGenerator() {
    // Initialize to octave-down subharmonic
    reset();
}

SubharmonicGenerator::~SubharmonicGenerator() {
    // Nothing to clean up
}

float SubharmonicGenerator::wrapPhase(float p) const {
    // Wrap phase to [-π, π] range
    // This is critical for correct phase error calculation
    while (p > M_PI) p -= 2.0f * static_cast<float>(M_PI);
    while (p < -M_PI) p += 2.0f * static_cast<float>(M_PI);
    return p;
}

float SubharmonicGenerator::generate(float fundamentalFreq, double sampleRate) {
    // Validate inputs
    if (fundamentalFreq <= 0.0f || sampleRate <= 0.0) {
        return 0.0f;
    }

    // Calculate phase increment for fundamental frequency
    // ω = 2π * f / fs
    float phaseIncrement = static_cast<float>(2.0 * M_PI * fundamentalFreq / sampleRate);

    // Update fundamental phase accumulator
    phase += phaseIncrement;
    phase = wrapPhase(phase);

    // Calculate target subharmonic phase
    // For a ratio r, the subharmonic should be at phase/r
    // This is the "reference" phase that the PLL tries to track
    float targetPhase = phase / ratio;

    // Calculate phase error
    // This is the difference between where we are and where we should be
    pllPhaseError = wrapPhase(targetPhase - pllPhase);

    // PI Controller to correct phase error
    // Proportional term: immediate response to error
    // Integral term: eliminates steady-state error
    float correction = pllKp * pllPhaseError + pllKi * pllIntegral;

    // Update integral accumulator (with limiting to prevent windup)
    pllIntegral += pllPhaseError;
    const float maxIntegral = 100.0f; // Prevent integral windup
    if (pllIntegral > maxIntegral) pllIntegral = maxIntegral;
    if (pllIntegral < -maxIntegral) pllIntegral = -maxIntegral;

    // Apply correction to subharmonic phase
    // The correction adjusts the phase increment to track the target
    pllPhase += phaseIncrement / ratio + correction;
    pllPhase = wrapPhase(pllPhase);

    // Generate subharmonic sine wave
    float subharmonic = std::sin(pllPhase);

    // Apply mix level
    return subharmonic * mix;
}

void SubharmonicGenerator::reset() {
    // Reset all phase accumulators
    phase = 0.0f;
    pllPhase = 0.0f;

    // Reset PLL state
    pllPhaseError = 0.0f;
    pllIntegral = 0.0f;
}

} // namespace ChoirV2
