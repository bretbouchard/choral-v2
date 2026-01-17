#include <iostream>
#include <cassert>
#include <cmath>
#include "../../src/dsp/LinearSmoother.h"

using namespace ChoirV2;

void testLinearSmootherExponentialDecay() {
    std::cout << "Testing LinearSmoother exponential decay characteristic..." << std::endl;

    LinearSmoother smoother;
    smoother.setTimeConstant(0.01f, 44100.0f);  // 10ms smoothing
    smoother.reset();
    smoother.setTarget(1.0f);

    // Process samples and verify exponential decay toward target
    float prev_value = smoother.getCurrent();
    float step_size = 0.0f;

    for (int i = 0; i < 100; ++i) {
        float value = smoother.process();

        // Value should always increase toward target (1.0)
        assert(value >= prev_value && "Value increases toward target");
        assert(value <= 1.0f && "Value never exceeds target");

        // Step size should decrease exponentially
        if (i > 0) {
            float current_step = value - prev_value;
            if (step_size > 0.0001f) {
                // Later steps should be smaller (exponential decay)
                assert(current_step <= step_size * 1.01f && "Steps decrease exponentially");
            }
            step_size = current_step;
        }

        prev_value = value;
    }

    std::cout << "  - Final value after 100 samples: " << prev_value << std::endl;
    std::cout << "  - Target value: 1.0" << std::endl;
    std::cout << "✓ LinearSmoother exponential decay test passed" << std::endl;
}

void testLinearSmootherSettlingTime() {
    std::cout << "\nTesting LinearSmoother settling time accuracy..." << std::endl;

    LinearSmoother smoother;
    float smoothing_time_ms = 20.0f;  // 20ms
    float sample_rate = 44100.0f;
    smoother.setTimeConstant(smoothing_time_ms / 1000.0f, sample_rate);
    smoother.reset();
    smoother.setTarget(1.0f);

    // Calculate expected settling time (5 * time constant for < 1% error)
    int expected_samples = static_cast<int>(5.0f * smoothing_time_ms / 1000.0f * sample_rate);
    std::cout << "  - Expected settling samples (5*tau): " << expected_samples << std::endl;

    // Process until settled (within 1% of target)
    int settled_samples = 0;
    float value;
    do {
        value = smoother.process();
        settled_samples++;
    } while (std::abs(value - 1.0f) > 0.01f && settled_samples < expected_samples * 2);

    float error_percent = std::abs(value - 1.0f) * 100.0f;
    std::cout << "  - Settled after " << settled_samples << " samples" << std::endl;
    std::cout << "  - Final value: " << value << std::endl;
    std::cout << "  - Error from target: " << error_percent << "%" << std::endl;

    // Should settle within approximately 5 time constants
    assert(settled_samples <= expected_samples * 1.5f && "Settles within expected time");
    assert(error_percent < 1.0f && "Settles within 1% of target");

    std::cout << "✓ LinearSmoother settling time test passed" << std::endl;
}

void testLinearSmootherReset() {
    std::cout << "\nTesting LinearSmoother reset functionality..." << std::endl;

    LinearSmoother smoother;
    smoother.setTimeConstant(0.01f, 44100.0f);
    smoother.setTarget(1.0f);

    // Process some samples to get partial smoothing
    for (int i = 0; i < 50; ++i) {
        smoother.process();
    }

    float partial_value = smoother.getCurrent();
    std::cout << "  - Partial value after 50 samples: " << partial_value << std::endl;

    // Reset should jump to target immediately
    smoother.reset();
    float reset_value = smoother.getCurrent();
    std::cout << "  - Value after reset: " << reset_value << std::endl;

    assert(reset_value == 1.0f && "Reset jumps to target");

    // Test setTargetImmediate (same as reset)
    smoother.setTarget(0.0f);
    for (int i = 0; i < 50; ++i) {
        smoother.process();
    }
    partial_value = smoother.getCurrent();

    smoother.setTargetImmediate(0.0f);
    reset_value = smoother.getCurrent();
    assert(reset_value == 0.0f && "setTargetImmediate jumps to target");

    std::cout << "✓ LinearSmoother reset test passed" << std::endl;
}

void testLinearSmootherMultipleSmoothingTimes() {
    std::cout << "\nTesting LinearSmoother with multiple smoothing times..." << std::endl;

    float smoothing_times_ms[] = {5.0f, 10.0f, 20.0f, 50.0f};
    float sample_rate = 44100.0f;

    for (float smoothing_time_ms : smoothing_times_ms) {
        LinearSmoother smoother;
        smoother.setTimeConstant(smoothing_time_ms / 1000.0f, sample_rate);
        smoother.reset();
        smoother.setTarget(1.0f);

        // Process for 5 time constants
        int num_samples = static_cast<int>(5.0f * smoothing_time_ms / 1000.0f * sample_rate);
        for (int i = 0; i < num_samples; ++i) {
            smoother.process();
        }

        float final_value = smoother.getCurrent();
        float error = std::abs(final_value - 1.0f);

        std::cout << "  - Smoothing time " << smoothing_time_ms << "ms: ";
        std::cout << "final value = " << final_value << ", error = " << (error * 100.0f) << "%" << std::endl;

        // Should be within 1% of target
        assert(error < 0.01f && "Settles within 1% for all smoothing times");
    }

    std::cout << "✓ LinearSmoother multiple smoothing times test passed" << std::endl;
}

void testLinearSmootherClickPrevention() {
    std::cout << "\nTesting LinearSmoother click prevention (step input)..." << std::endl;

    LinearSmoother smoother;
    smoother.setTimeConstant(0.01f, 44100.0f);  // 10ms
    smoother.reset();
    smoother.setTarget(0.0f);

    // Process with zero input
    for (int i = 0; i < 100; ++i) {
        smoother.process();
    }

    // Apply sudden step change (would cause click without smoothing)
    smoother.setTarget(1.0f);

    // Check that first sample after step is NOT 1.0 (would be instant click)
    float first_sample = smoother.process();
    std::cout << "  - First sample after step: " << first_sample << std::endl;
    std::cout << "  - Target value: 1.0" << std::endl;

    assert(first_sample < 1.0f && "First sample is smoothed (not instant)");
    assert(first_sample > 0.0f && "First sample starts moving toward target");

    // Verify smooth transition (no sudden jumps)
    float prev_value = first_sample;
    for (int i = 0; i < 100; ++i) {
        float value = smoother.process();
        float delta = std::abs(value - prev_value);

        // Maximum step size should be small (prevents clicks)
        assert(delta < 0.01f && "No sudden jumps (click prevention)");

        prev_value = value;
    }

    std::cout << "  - Maximum delta between samples: < 0.01" << std::endl;
    std::cout << "✓ LinearSmoother click prevention test passed" << std::endl;
}

void testLinearSmootherProcessBlock() {
    std::cout << "\nTesting LinearSmoother processBlock..." << std::endl;

    LinearSmoother smoother;
    smoother.setTimeConstant(0.01f, 44100.0f);
    smoother.reset();
    smoother.setTarget(1.0f);

    // Process a block of samples
    const int block_size = 64;
    float output[block_size];

    smoother.processBlock(output, block_size);

    // Verify block processing matches individual processing
    LinearSmoother smoother2;
    smoother2.setTimeConstant(0.01f, 44100.0f);
    smoother2.reset();
    smoother2.setTarget(1.0f);

    for (int i = 0; i < block_size; ++i) {
        float expected = smoother2.process();
        float actual = output[i];

        assert(std::abs(expected - actual) < 0.0001f && "Block matches individual processing");
    }

    std::cout << "  - Block size: " << block_size << " samples" << std::endl;
    std::cout << "  - Block processing matches individual samples" << std::endl;
    std::cout << "✓ LinearSmoother processBlock test passed" << std::endl;
}

void testLinearSmootherDifferentSampleRates() {
    std::cout << "\nTesting LinearSmoother at different sample rates..." << std::endl;

    float sample_rates[] = {44100.0f, 48000.0f, 96000.0f};

    for (float sample_rate : sample_rates) {
        LinearSmoother smoother;
        smoother.setTimeConstant(0.01f, sample_rate);
        smoother.reset();
        smoother.setTarget(1.0f);

        // Process for 5 time constants
        int num_samples = static_cast<int>(5.0f * 0.01f * sample_rate);
        for (int i = 0; i < num_samples; ++i) {
            smoother.process();
        }

        float final_value = smoother.getCurrent();
        float error = std::abs(final_value - 1.0f);

        std::cout << "  - Sample rate " << sample_rate << " Hz: ";
        std::cout << "final value = " << final_value << ", error = " << (error * 100.0f) << "%" << std::endl;

        // Should settle to within 1% regardless of sample rate
        assert(error < 0.01f && "Settles within 1% at all sample rates");
    }

    std::cout << "✓ LinearSmoother sample rate independence test passed" << std::endl;
}

void testSIMDLinearSmoother() {
    std::cout << "\nTesting SIMDLinearSmoother (batch processing)..." << std::endl;

    const int num_parameters = 8;
    SIMDLinearSmoother smoother(num_parameters);

    smoother.setTimeConstant(0.01f, 44100.0f);

    // Set different targets for each parameter
    float targets[num_parameters];
    for (int i = 0; i < num_parameters; ++i) {
        targets[i] = static_cast<float>(i + 1) * 0.1f;  // 0.1, 0.2, 0.3, ...
    }
    smoother.setTargets(targets);
    smoother.reset();

    // Process a block
    const int block_size = 64;
    float output[num_parameters * block_size];

    smoother.processBlock(output, block_size);

    // Verify each parameter smoothed correctly
    for (int param = 0; param < num_parameters; ++param) {
        float final_value = output[param * block_size + block_size - 1];
        float target = targets[param];
        float error = std::abs(final_value - target);

        std::cout << "  - Parameter " << param << " (target " << target << "): ";
        std::cout << "final = " << final_value << ", error = " << (error * 100.0f) << "%" << std::endl;

        assert(error < 0.1f && "SIMD smoother reaches targets");
    }

    std::cout << "✓ SIMDLinearSmoother test passed" << std::endl;
}

void testLinearSmootherNegativeValues() {
    std::cout << "\nTesting LinearSmoother with negative values..." << std::endl;

    LinearSmoother smoother;
    smoother.setTimeConstant(0.01f, 44100.0f);
    smoother.reset();
    smoother.setTarget(0.0f);

    // Smooth to negative target
    smoother.setTarget(-1.0f);

    float prev_value = 0.0f;
    for (int i = 0; i < 100; ++i) {
        float value = smoother.process();

        // Value should always decrease toward -1.0
        assert(value <= prev_value && "Value decreases toward negative target");
        assert(value >= -1.0f && "Value never exceeds negative target");

        prev_value = value;
    }

    std::cout << "  - Final value: " << prev_value << std::endl;
    std::cout << "  - Target: -1.0" << std::endl;
    std::cout << "✓ LinearSmoother negative values test passed" << std::endl;
}

void testLinearSmootherZeroSmoothing() {
    std::cout << "\nTesting LinearSmoother with zero smoothing time..." << std::endl;

    LinearSmoother smoother;
    smoother.setTimeConstant(0.0f, 44100.0f);  // Zero time constant = instant
    smoother.reset();
    smoother.setTarget(1.0f);

    // With zero smoothing, should reach target immediately
    float value = smoother.process();
    std::cout << "  - First sample with zero smoothing: " << value << std::endl;

    assert(std::abs(value - 1.0f) < 0.0001f && "Zero smoothing = instant change");

    std::cout << "✓ LinearSmoother zero smoothing test passed" << std::endl;
}

int main() {
    std::cout << "=== LinearSmoother Unit Tests ===" << std::endl;
    std::cout << "Testing parameter smoothing for click-free phoneme transitions" << std::endl;
    std::cout << "Implementation: Exponential smoothing with configurable time constant" << std::endl;

    try {
        testLinearSmootherExponentialDecay();
        testLinearSmootherSettlingTime();
        testLinearSmootherReset();
        testLinearSmootherMultipleSmoothingTimes();
        testLinearSmootherClickPrevention();
        testLinearSmootherProcessBlock();
        testLinearSmootherDifferentSampleRates();
        testSIMDLinearSmoother();
        testLinearSmootherNegativeValues();
        testLinearSmootherZeroSmoothing();

        std::cout << "\n✅ All LinearSmoother tests passed!" << std::endl;
        std::cout << "\nKey features verified:" << std::endl;
        std::cout << "  ✓ Exponential smoothing characteristic" << std::endl;
        std::cout << "  ✓ Accurate settling time (within 1% of target)" << std::endl;
        std::cout << "  ✓ Reset and immediate set functionality" << std::endl;
        std::cout << "  ✓ Works across multiple smoothing times (5-50ms)" << std::endl;
        std::cout << "  ✓ Click prevention (smooth step transitions)" << std::endl;
        std::cout << "  ✓ Block processing for efficiency" << std::endl;
        std::cout << "  ✓ Sample rate independence" << std::endl;
        std::cout << "  ✓ SIMD batch processing (for formant frequencies)" << std::endl;
        std::cout << "  ✓ Handles negative values correctly" << std::endl;
        std::cout << "  ✓ Zero smoothing mode (instant changes)" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n❌ Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
