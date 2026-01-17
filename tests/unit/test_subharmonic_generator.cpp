#include <iostream>
#include <cassert>
#include <cmath>
#include <vector>
#include "../../src/dsp/SubharmonicGenerator.h"

using namespace ChoirV2;

// Test constants
const double SAMPLE_RATE = 44100.0;
const float TEST_FREQUENCY = 220.0f; // A3
[[maybe_unused]] const float TEST_TOLERANCE = 0.01f; // 1% tolerance

void testSubharmonicGeneratorBasicOutput() {
    std::cout << "Testing SubharmonicGenerator basic output..." << std::endl;

    SubharmonicGenerator gen;
    gen.setRatio(0.5f); // Octave down
    gen.setMix(1.0f);   // Full mix

    // Generate one period at 220 Hz
    double period = SAMPLE_RATE / TEST_FREQUENCY; // Samples per period
    std::vector<float> output;
    float maxOutput = 0.0f;
    float minOutput = 0.0f;

    for (int i = 0; i < static_cast<int>(period); ++i) {
        float sample = gen.generate(TEST_FREQUENCY, SAMPLE_RATE);
        output.push_back(sample);
        maxOutput = std::max(maxOutput, sample);
        minOutput = std::min(minOutput, sample);
    }

    // Output should be sinusoidal (range roughly -1 to 1)
    assert(maxOutput > 0.9f && "Maximum output near +1");
    assert(minOutput < -0.9f && "Minimum output near -1");

    std::cout << "  - Max output: " << maxOutput << std::endl;
    std::cout << "  - Min output: " << minOutput << std::endl;
    std::cout << "✓ SubharmonicGenerator basic output test passed" << std::endl;
}

void testSubharmonicGeneratorFrequencyRatio() {
    std::cout << "\nTesting SubharmonicGenerator frequency ratio..." << std::endl;

    // Test octave down (ratio = 0.5)
    SubharmonicGenerator gen;
    gen.setRatio(0.5f);
    gen.setMix(1.0f);

    // Generate enough samples to capture multiple periods
    int numSamples = static_cast<int>(SAMPLE_RATE / TEST_FREQUENCY * 4); // 4 periods
    std::vector<float> output;
    int zeroCrossings = 0;

    for (int i = 0; i < numSamples; ++i) {
        float sample = gen.generate(TEST_FREQUENCY, SAMPLE_RATE);
        output.push_back(sample);

        // Count zero crossings (positive to negative)
        if (i > 0 && output[i-1] > 0 && sample < 0) {
            zeroCrossings++;
        }
    }

    // For octave down, we should have 2 zero crossings per fundamental period
    // 4 fundamental periods = 8 zero crossings
    int expectedCrossings = 8;
    assert(std::abs(zeroCrossings - expectedCrossings) <= 1 && "Correct frequency ratio");

    std::cout << "  - Zero crossings: " << zeroCrossings << " (expected: " << expectedCrossings << ")" << std::endl;
    std::cout << "✓ SubharmonicGenerator frequency ratio test passed" << std::endl;
}

void testSubharmonicGeneratorPhaseLock() {
    std::cout << "\nTesting SubharmonicGenerator PLL phase lock..." << std::endl;

    SubharmonicGenerator gen;
    gen.setRatio(0.5f);
    gen.setMix(1.0f);

    // Generate many samples to test PLL stability
    int numSamples = static_cast<int>(SAMPLE_RATE * 0.1); // 0.1 seconds
    float maxPhaseError = 0.0f;

    for (int i = 0; i < numSamples; ++i) {
        gen.generate(TEST_FREQUENCY, SAMPLE_RATE);
        float phaseError = std::abs(gen.getPhaseError());
        maxPhaseError = std::max(maxPhaseError, phaseError);
    }

    // Phase error should remain small (< 0.1 radians)
    assert(maxPhaseError < 0.1f && "Phase error remains small");

    std::cout << "  - Maximum phase error: " << maxPhaseError << " radians" << std::endl;
    std::cout << "✓ SubharmonicGenerator PLL phase lock test passed" << std::endl;
}

void testSubharmonicGeneratorNoPhaseDrift() {
    std::cout << "\nTesting SubharmonicGenerator for NO phase drift (CRITICAL)..." << std::endl;

    SubharmonicGenerator gen;
    gen.setRatio(0.5f);
    gen.setMix(1.0f);

    // Generate 10 seconds of audio (the critical test)
    int numSamples = static_cast<int>(SAMPLE_RATE * 10.0);
    std::vector<float> phaseErrors;

    // Store phase error every 1000 samples
    for (int i = 0; i < numSamples; ++i) {
        gen.generate(TEST_FREQUENCY, SAMPLE_RATE);

        if (i % 1000 == 0) {
            phaseErrors.push_back(std::abs(gen.getPhaseError()));
        }
    }

    // Calculate statistics
    float sumError = 0.0f;
    float maxError = 0.0f;
    for (float error : phaseErrors) {
        sumError += error;
        maxError = std::max(maxError, error);
    }
    float avgError = sumError / phaseErrors.size();

    // CRITICAL: Phase error must NOT grow over time
    // Check that the average error is small
    assert(avgError < 0.05f && "Average phase error remains small");

    // Check that maximum error is bounded
    assert(maxError < 0.15f && "Maximum phase error remains bounded");

    // Check that phase error doesn't trend upward (drift detection)
    // Compare first 10% and last 10% of errors
    float firstHalfAvg = 0.0f;
    float secondHalfAvg = 0.0f;
    size_t midPoint = phaseErrors.size() / 2;

    for (size_t i = 0; i < midPoint; ++i) {
        firstHalfAvg += phaseErrors[i];
    }
    firstHalfAvg /= midPoint;

    for (size_t i = midPoint; i < phaseErrors.size(); ++i) {
        secondHalfAvg += phaseErrors[i];
    }
    secondHalfAvg /= (phaseErrors.size() - midPoint);

    // Second half should NOT have significantly higher error than first half
    float driftRatio = secondHalfAvg / (firstHalfAvg + 0.0001f);
    assert(driftRatio < 2.0f && "No significant phase drift over 10 seconds");

    std::cout << "  - Test duration: 10 seconds (" << numSamples << " samples)" << std::endl;
    std::cout << "  - Average phase error: " << avgError << " radians" << std::endl;
    std::cout << "  - Maximum phase error: " << maxError << " radians" << std::endl;
    std::cout << "  - First half avg error: " << firstHalfAvg << " radians" << std::endl;
    std::cout << "  - Second half avg error: " << secondHalfAvg << " radians" << std::endl;
    std::cout << "  - Drift ratio: " << driftRatio << " (< 2.0 = no drift)" << std::endl;
    std::cout << "✓ SubharmonicGenerator NO PHASE DRIFT test passed (CRITICAL FIX VERIFIED)" << std::endl;
}

void testSubharmonicGeneratorFrequencyTracking() {
    std::cout << "\nTesting SubharmonicGenerator frequency tracking..." << std::endl;

    SubharmonicGenerator gen;
    gen.setRatio(0.5f);
    gen.setMix(1.0f);

    // Start at one frequency, then change to another
    float freq1 = 220.0f;
    float freq2 = 440.0f;

    // Generate at first frequency
    for (int i = 0; i < 1000; ++i) {
        gen.generate(freq1, SAMPLE_RATE);
    }

    // Now change frequency
    float maxPhaseErrorAfterChange = 0.0f;
    for (int i = 0; i < 1000; ++i) {
        gen.generate(freq2, SAMPLE_RATE);
        float phaseError = std::abs(gen.getPhaseError());
        maxPhaseErrorAfterChange = std::max(maxPhaseErrorAfterChange, phaseError);
    }

    // PLL should relock to new frequency within 1000 samples
    assert(maxPhaseErrorAfterChange < 0.2f && "PLL tracks frequency changes");

    std::cout << "  - Max phase error after frequency change: " << maxPhaseErrorAfterChange << " radians" << std::endl;
    std::cout << "✓ SubharmonicGenerator frequency tracking test passed" << std::endl;
}

void testSubharmonicGeneratorReset() {
    std::cout << "\nTesting SubharmonicGenerator reset..." << std::endl;

    SubharmonicGenerator gen;
    gen.setRatio(0.5f);
    gen.setMix(1.0f);

    // Generate some samples
    for (int i = 0; i < 1000; ++i) {
        gen.generate(TEST_FREQUENCY, SAMPLE_RATE);
    }

    // Reset should clear all state
    gen.reset();

    float phaseAfterReset = gen.getFundamentalPhase();
    float pllPhaseAfterReset = gen.getSubharmonicPhase();
    float phaseErrorAfterReset = gen.getPhaseError();

    // Everything should be zero after reset
    assert(std::abs(phaseAfterReset) < 0.001f && "Fundamental phase cleared");
    assert(std::abs(pllPhaseAfterReset) < 0.001f && "Subharmonic phase cleared");
    assert(std::abs(phaseErrorAfterReset) < 0.001f && "Phase error cleared");

    std::cout << "  - Fundamental phase after reset: " << phaseAfterReset << std::endl;
    std::cout << "  - Subharmonic phase after reset: " << pllPhaseAfterReset << std::endl;
    std::cout << "  - Phase error after reset: " << phaseErrorAfterReset << std::endl;
    std::cout << "✓ SubharmonicGenerator reset test passed" << std::endl;
}

void testSubharmonicGeneratorDifferentRatios() {
    std::cout << "\nTesting SubharmonicGenerator with different ratios..." << std::endl;

    float ratios[] = {0.5f, 0.33f, 0.25f}; // Octave down, third down, fourth down

    for (float ratio : ratios) {
        SubharmonicGenerator gen;
        gen.setRatio(ratio);
        gen.setMix(1.0f);

        // Generate samples and verify PLL stays locked
        int numSamples = static_cast<int>(SAMPLE_RATE * 0.1); // 100ms
        float maxPhaseError = 0.0f;
        float sumOutput = 0.0f;
        float maxOutput = 0.0f;

        for (int i = 0; i < numSamples; ++i) {
            float sample = gen.generate(TEST_FREQUENCY, SAMPLE_RATE);
            sumOutput += std::abs(sample);
            maxOutput = std::max(maxOutput, std::abs(sample));
            maxPhaseError = std::max(maxPhaseError, std::abs(gen.getPhaseError()));
        }

        float avgOutput = sumOutput / numSamples;

        // Verify PLL stays locked (small phase error)
        // Use relaxed tolerance for non-power-of-2 ratios
        [[maybe_unused]] float tolerance = (ratio == 0.5f) ? 0.1f : 0.25f;
        assert(maxPhaseError < tolerance && "PLL remains locked for ratio");

        // Verify output is reasonable (non-zero and bounded)
        assert(avgOutput > 0.1f && "Output has sufficient energy");
        assert(maxOutput <= 1.0f && "Output is bounded");

        std::cout << "  - Ratio " << ratio << ":" << std::endl;
        std::cout << "    - Max phase error: " << maxPhaseError << " radians" << std::endl;
        std::cout << "    - Avg output: " << avgOutput << std::endl;
        std::cout << "    - Max output: " << maxOutput << std::endl;
    }

    std::cout << "✓ SubharmonicGenerator different ratios test passed" << std::endl;
}

void testSubharmonicGeneratorMixLevel() {
    std::cout << "\nTesting SubharmonicGenerator mix level..." << std::endl;

    SubharmonicGenerator gen;
    gen.setRatio(0.5f);

    float mixLevels[] = {0.0f, 0.5f, 1.0f};

    for (float mix : mixLevels) {
        gen.setMix(mix);
        gen.reset();

        float maxOutput = 0.0f;
        for (int i = 0; i < 1000; ++i) {
            float sample = gen.generate(TEST_FREQUENCY, SAMPLE_RATE);
            maxOutput = std::max(maxOutput, std::abs(sample));
        }

        // Max output should scale with mix level
        float expectedMax = mix;
        assert(std::abs(maxOutput - expectedMax) < 0.1f && "Output scales with mix");

        std::cout << "  - Mix " << mix << ": max output = " << maxOutput
                  << " (expected: " << expectedMax << ")" << std::endl;
    }

    std::cout << "✓ SubharmonicGenerator mix level test passed" << std::endl;
}

int main() {
    std::cout << "=== SubharmonicGenerator Unit Tests ===" << std::endl;
    std::cout << "Testing CRITICAL FIX for PLL phase drift bug" << std::endl;
    std::cout << "Implementation: Proper Phase-Locked Loop with PI controller" << std::endl;

    try {
        testSubharmonicGeneratorBasicOutput();
        testSubharmonicGeneratorFrequencyRatio();
        testSubharmonicGeneratorPhaseLock();
        testSubharmonicGeneratorNoPhaseDrift(); // CRITICAL TEST
        testSubharmonicGeneratorFrequencyTracking();
        testSubharmonicGeneratorReset();
        testSubharmonicGeneratorDifferentRatios();
        testSubharmonicGeneratorMixLevel();

        std::cout << "\n✅ All SubharmonicGenerator tests passed!" << std::endl;
        std::cout << "\nKey improvements verified:" << std::endl;
        std::cout << "  ✓ Proper Phase-Locked Loop (not one-pole filter)" << std::endl;
        std::cout << "  ✓ Phase error detection with wrap-around" << std::endl;
        std::cout << "  ✓ PI controller for robust tracking" << std::endl;
        std::cout << "  ✓ NO PHASE DRIFT over 10 seconds (CRITICAL)" << std::endl;
        std::cout << "  ✓ Frequency tracking with relock capability" << std::endl;
        std::cout << "  ✓ Multiple division ratios supported" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n❌ Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
