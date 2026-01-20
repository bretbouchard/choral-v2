//==============================================================================
// PureDSPFFT.h - Minimal, dependency-free FFT implementation for PureDSP
//
// Implements Cooley-Tukey FFT algorithm with zero external dependencies
// Optimized for audio processing (powers of 2 only)
//
// Copyright (c) 2025 ChoirV2 Project
// MIT License - See LICENSE for details
//==============================================================================

#ifndef PUREDSP_FFT_H_INCLUDED
#define PUREDSP_FFT_H_INCLUDED

#include <vector>
#include <complex>
#include <cmath>
#include <algorithm>
#include <numbers>

namespace PureDSP {

//==============================================================================
// Minimal FFT implementation using Cooley-Tukey algorithm
class FFT
{
public:
    using Complex = std::complex<float>;
    using ComplexVector = std::vector<Complex>;

    //==============================================================================
    FFT(int size)
        : size_(size)
        , log2Size_(static_cast<int>(std::log2(size)))
    {
        // Verify size is power of 2
        if (size & (size - 1))
            throw std::invalid_argument("FFT size must be power of 2");

        // Pre-compute twiddle factors
        twiddleFactors_.resize(size_ / 2);
        for (int k = 0; k < size_ / 2; ++k)
        {
            float phase = -2.0f * std::numbers::pi_v<float> * k / size_;
            twiddleFactors_[k] = Complex(std::cos(phase), std::sin(phase));
        }

        // Pre-compute bit reversal indices
        bitReversalIndices_.resize(size_);
        for (int i = 0; i < size_; ++i)
        {
            bitReversalIndices_[i] = reverseBits(i, log2Size_);
        }
    }

    ~FFT() = default;

    //==============================================================================
    // Perform forward FFT (real input, complex output)
    void forward(const float* input, Complex* output)
    {
        // Convert real input to complex
        ComplexVector buffer(size_);
        for (int i = 0; i < size_; ++i)
        {
            buffer[i] = Complex(input[bitReversalIndices_[i]], 0.0f);
        }

        // Perform FFT
        perform(buffer.data());

        // Copy output
        for (int i = 0; i < size_; ++i)
        {
            output[i] = buffer[i];
        }
    }

    //==============================================================================
    // Perform inverse FFT (complex input, real output)
    void inverse(const Complex* input, float* output)
    {
        // Copy and conjugate input
        ComplexVector buffer(size_);
        for (int i = 0; i < size_; ++i)
        {
            buffer[bitReversalIndices_[i]] = std::conj(input[i]);
        }

        // Perform FFT
        perform(buffer.data());

        // Conjugate and scale to get inverse
        float scale = 1.0f / size_;
        for (int i = 0; i < size_; ++i)
        {
            output[i] = std::real(buffer[i]) * scale;
        }
    }

    //==============================================================================
    // Real-valued FFT (optimized for real input)
    // Input: size_ real samples
    // Output: size_/2 + 1 complex values (only positive frequencies)
    void realForward(const float* input, Complex* output)
    {
        // Perform full FFT
        ComplexVector fullFFT(size_);
        forward(input, fullFFT.data());

        // Copy only positive frequencies (0 to Nyquist)
        int numBins = size_ / 2 + 1;
        for (int i = 0; i < numBins; ++i)
        {
            output[i] = fullFFT[i];
        }
    }

    //==============================================================================
    // Real-valued inverse FFT
    // Input: size_/2 + 1 complex values (positive frequencies)
    // Output: size_ real samples
    void realInverse(const Complex* input, float* output)
    {
        // Reconstruct full spectrum (conjugate symmetry for real signals)
        ComplexVector fullFFT(size_);
        fullFFT[0] = input[0];  // DC component
        fullFFT[size_ / 2] = input[size_ / 2];  // Nyquist component

        // Fill positive frequencies
        for (int i = 1; i < size_ / 2; ++i)
        {
            fullFFT[i] = input[i];
            // Negative frequencies are conjugates
            fullFFT[size_ - i] = std::conj(input[i]);
        }

        // Perform inverse FFT
        inverse(fullFFT.data(), output);
    }

    //==============================================================================
    int getSize() const { return size_; }
    int getNumBins() const { return size_ / 2 + 1; }

private:
    //==============================================================================
    void perform(Complex* data)
    {
        // Cooley-Tukey FFT algorithm
        for (int stage = 1; stage <= log2Size_; ++stage)
        {
            int m = 1 << stage;  // 2^stage
            int m2 = m >> 1;     // m/2

            for (int k = 0; k < size_; k += m)
            {
                for (int j = 0; j < m2; ++j)
                {
                    int evenIndex = k + j;
                    int oddIndex = k + j + m2;

                    // Get twiddle factor
                    int twiddleIndex = (j * size_) / m;
                    Complex twiddle = twiddleFactors_[twiddleIndex];

                    // Butterfly operation
                    Complex t = twiddle * data[oddIndex];
                    Complex u = data[evenIndex];

                    data[evenIndex] = u + t;
                    data[oddIndex] = u - t;
                }
            }
        }
    }

    //==============================================================================
    int reverseBits(int n, int numBits) const
    {
        int reversed = 0;
        for (int i = 0; i < numBits; ++i)
        {
            reversed = (reversed << 1) | (n & 1);
            n >>= 1;
        }
        return reversed;
    }

    //==============================================================================
    int size_;
    int log2Size_;
    ComplexVector twiddleFactors_;
    std::vector<int> bitReversalIndices_;
};

//==============================================================================
} // namespace PureDSP

#endif // PUREDSP_FFT_H_INCLUDED
