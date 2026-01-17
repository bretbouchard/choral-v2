/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * SIMDHelpers.h - SIMD optimization utilities
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#pragma once

#include <vector>

namespace ChoirV2 {

/**
 * @brief SIMD helper functions
 *
 * Provides SIMD-optimized operations for vector processing.
 * Uses AVX2 (256-bit) for maximum performance.
 *
 * Operations:
 * - Vector addition/multiplication
 * - Vector dot product
 * - Vector sqrt (for normalization)
 * - Vector interpolation
 */
class SIMDHelpers {
public:
    /**
     * @brief Vector addition (SIMD-optimized)
     * @param a Input vector A
     * @param b Input vector B
     * @param output Output vector (a + b)
     * @param size Vector size
     */
    static void add(const float* a, const float* b, float* output, int size);

    /**
     * @brief Vector multiplication (SIMD-optimized)
     * @param a Input vector A
     * @param b Input vector B
     * @param output Output vector (a * b)
     * @param size Vector size
     */
    static void multiply(const float* a, const float* b, float* output, int size);

    /**
     * @brief Vector scale (SIMD-optimized)
     * @param a Input vector
     * @param scale Scale factor
     * @param output Output vector (a * scale)
     * @param size Vector size
     */
    static void scale(const float* a, float scale, float* output, int size);

    /**
     * @brief Vector interpolation (SIMD-optimized)
     * @param a Start vector
     * @param b End vector
     * @param t Interpolation factor (0-1)
     * @param output Output vector (lerp(a, b, t))
     * @param size Vector size
     */
    static void lerp(const float* a, const float* b, float t, float* output, int size);

    /**
     * @brief Check if SIMD is available
     */
    static bool isSIMDAvailable();

private:
    static bool simd_available_;
    static bool detected_;
};

} // namespace ChoirV2
