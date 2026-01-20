#pragma once

// PureDSP - JUCE-free implementation
#include "PureDSP_Types.h"
#include "PureDSPFFT.h"
#include <memory>
#include <array>
#include <vector>
#include <cmath>
#include <algorithm>

// Use PureDSP types - inline wrapper for template function
template<typename T>
inline T jlimit(T minimum, T maximum, T value) {
    return PureDSP::jlimit(minimum, maximum, value);
}

constexpr auto pi = PureDSP::MathConstants<float>::pi;

namespace DSP {

//==============================================================================
// BiquadFilter - Real biquad filter implementation
// Based on: https://www.w3.org/2011/audio/audio-eq-cookbook.html
class BiquadFilter
{
public:
    BiquadFilter()
        : b0_(0), b1_(0), b2_(0), a1_(0), a2_(0),
          x1_(0), x2_(0), y1_(0), y2_(0) {}

    void reset()
    {
        x1_ = x2_ = y1_ = y2_ = 0;
    }

    // Design bandpass filter (constant skirt gain)
    void designBandpass(float frequency, float bandwidth, float sampleRate)
    {
        const float omega = 2.0f * pi * frequency / sampleRate;
        const float alpha = std::sin(omega) * std::sinh(std::log(2.0f) / 2.0f *
                                         bandwidth * omega / std::sin(omega));

        // Bandpass filter coefficients
        b0_ = alpha;
        b1_ = 0.0f;
        b2_ = -alpha;
        const float a0 = 1.0f + alpha;
        a1_ = -2.0f * std::cos(omega);
        a2_ = 1.0f - alpha;

        // Normalize by a0
        b0_ /= a0;
        b1_ /= a0;
        b2_ /= a0;
        a1_ /= a0;
        a2_ /= a0;
    }

    // Design low-shelf filter (for bass enhancement)
    void designLowShelf(float frequency, float gainDb, float sampleRate, float Q = 0.5f)
    {
        const float A = std::pow(10.0f, gainDb / 40.0f);
        const float omega = 2.0f * pi * frequency / sampleRate;
        const float alpha = std::sin(omega) / 2.0f * std::sqrt((A + 1.0f / A) *
                                                             (1.0f / Q - 1.0f) + 2.0f);

        const float b0 = A * ((A + 1.0f) - (A - 1.0f) * std::cos(omega) + 2.0f * std::sqrt(A) * alpha);
        const float b1 = 2.0f * A * ((A - 1.0f) - (A + 1.0f) * std::cos(omega));
        const float b2 = A * ((A + 1.0f) - (A - 1.0f) * std::cos(omega) - 2.0f * std::sqrt(A) * alpha);
        const float a0 = (A + 1.0f) + (A - 1.0f) * std::cos(omega) + 2.0f * std::sqrt(A) * alpha;
        const float a1 = -2.0f * ((A - 1.0f) + (A + 1.0f) * std::cos(omega));
        const float a2 = (A + 1.0f) + (A - 1.0f) * std::cos(omega) - 2.0f * std::sqrt(A) * alpha;

        b0_ = b0 / a0;
        b1_ = b1 / a0;
        b2_ = b2 / a0;
        a1_ = a1 / a0;
        a2_ = a2 / a0;
    }

    // Set raw coefficients (for custom filters)
    void setCoefficients(float b0, float b1, float b2, float a1, float a2)
    {
        b0_ = b0;
        b1_ = b1;
        b2_ = b2;
        a1_ = a1;
        a2_ = a2;
    }

    // Process single sample (transposed direct form II)
    inline float process(float x)
    {
        const float y = b0_ * x + b1_ * x1_ + b2_ * x2_ - a1_ * y1_ - a2_ * y2_;
        x2_ = x1_;
        x1_ = x;
        y2_ = y1_;
        y1_ = y;
        return y;
    }

    // Process buffer
    void processBuffer(float* output, const float* input, int numSamples)
    {
        for (int i = 0; i < numSamples; ++i)
            output[i] = process(input[i]);
    }

private:
    float b0_, b1_, b2_;  // Numerator coefficients
    float a1_, a2_;        // Denominator coefficients (a0 is normalized to 1)
    float x1_, x2_;        // Input delay line
    float y1_, y2_;        // Output delay line
};

//==============================================================================
// FormantResonator - Biquad-based formant filter
class FormantResonator
{
public:
    FormantResonator() {}

    void designResonator(float frequency, float bandwidth, float sampleRate)
    {
        filter_.designBandpass(frequency, bandwidth / frequency, sampleRate);
    }

    inline float process(float input)
    {
        return filter_.process(input);
    }

    void processBuffer(float* output, const float* input, int numSamples)
    {
        filter_.processBuffer(output, input, numSamples);
    }

    void reset()
    {
        filter_.reset();
    }

private:
    BiquadFilter filter_;
};

//==============================================================================
// Vowel definitions with 5-formant data
struct VowelDefinition
{
    const char* name;
    float f1, f2, f3, f4, f5;  // Formant frequencies (Hz)
    float b1, b2, b3, b4, b5;  // Bandwidths (Hz)
};

static const VowelDefinition vowels[] = {
    {"AA (bat)",  800, 1150, 2800, 3500, 4500,  80,  90, 120, 130, 140},
    {"AE (cat)",  700, 1600, 2600, 3500, 4500, 100, 100, 120, 130, 140},
    {"AH (cut)",  600, 1200, 2600, 3400, 4400,  80,  90, 120, 130, 140},
    {"AO (cot)",  500,  900, 2500, 3400, 4300,  60,  80, 120, 130, 140},
    {"EH (met)",  500, 1700, 2600, 3500, 4500,  80, 100, 120, 130, 140},
    {"ER (bird)", 500, 1200, 2500, 3400, 4300,  60,  80, 120, 130, 140},
    {"IH (bit)",  400, 1900, 2600, 3400, 4300,  60,  90, 120, 130, 140},
    {"IY (beat)", 300, 2200, 2900, 3500, 4500,  50,  80, 120, 130, 140},
    {"UW (boot)", 300,  850, 2200, 3400, 4200,  40,  60, 120, 130, 140},
    {"OW (boat)", 500,  900, 2300, 3400, 4200,  60,  80, 120, 130, 140}
};

//==============================================================================
// FormantSynthesis - Complete formant synthesis module
class FormantSynthesis
{
public:
    FormantSynthesis(double sampleRate)
        : sampleRate_(sampleRate)
        , currentVowel_(0)
        , vibratoLfoPhase_(0.0f)
        , vibratoRate_(5.0f)
        , vibratoDepth_(0.0f)
    {
        // Initialize all formant resonators
        for (int i = 0; i < 5; ++i)
        {
            resonators_[i] = std::make_unique<FormantResonator>();
        }

        // Set default vowel
        setCurrentVowel(0);
    }

    void setCurrentVowel(int vowelIndex)
    {
        if (vowelIndex < 0 || vowelIndex >= 10)
            return;

        currentVowel_ = vowelIndex;
        const auto& vowel = vowels[vowelIndex];

        // Design resonators for this vowel
        resonators_[0]->designResonator(vowel.f1, vowel.b1, static_cast<float>(sampleRate_));
        resonators_[1]->designResonator(vowel.f2, vowel.b2, static_cast<float>(sampleRate_));
        resonators_[2]->designResonator(vowel.f3, vowel.b3, static_cast<float>(sampleRate_));
        resonators_[3]->designResonator(vowel.f4, vowel.b4, static_cast<float>(sampleRate_));
        resonators_[4]->designResonator(vowel.f5, vowel.b5, static_cast<float>(sampleRate_));
    }

    void setVibratoRate(float rateHz)
    {
        vibratoRate_ = rateHz;
    }

    void setVibratoDepth(float depth)
    {
        vibratoDepth_ = jlimit(0.0f, 1.0f, depth);
    }

    // Process audio through formant filter bank
    void process(float* output, const float* input, int numSamples)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            // Apply vibrato if enabled
            float vibratoMod = 1.0f;
            if (vibratoDepth_ > 0.0f)
            {
                vibratoMod = 1.0f + vibratoDepth_ * std::sin(vibratoLfoPhase_);
                vibratoLfoPhase_ += 2.0f * pi * vibratoRate_ / static_cast<float>(sampleRate_);
                if (vibratoLfoPhase_ >= 2.0f * pi)
                    vibratoLfoPhase_ -= 2.0f * pi;
            }

            // Process through parallel resonators and sum
            float formantOutput = 0.0f;
            for (int j = 0; j < 5; ++j)
            {
                formantOutput += resonators_[j]->process(input[i] * vibratoMod);
            }

            // Normalize output (1/5 to compensate for summing)
            output[i] = formantOutput * 0.2f;
        }
    }

    void reset()
    {
        vibratoLfoPhase_ = 0.0f;
        for (int i = 0; i < 5; ++i)
        {
            resonators_[i]->reset();
        }
    }

private:
    double sampleRate_;
    std::array<std::unique_ptr<FormantResonator>, 5> resonators_;
    int currentVowel_;

    // Vibrato
    float vibratoLfoPhase_;
    float vibratoRate_;
    float vibratoDepth_;
};

//==============================================================================
// SubharmonicGenerator - PLL-based subharmonic generation with phase error correction
class SubharmonicGenerator
{
public:
    SubharmonicGenerator(double sampleRate)
        : sampleRate_(sampleRate)
        , phase_(0.0f)
        , frequency_(440.0f)
        , smoothedFrequency_(440.0f)
        , subharmonicMix_(0.5f)
        , bassEnhancement_(0.0f)
        , pllEnabled_(false)
        , integrator_(0.0f)
        , phaseError_(0.0f)
        , lastPhaseError_(0.0f)
    {
        // PLL parameters - further reduced gain for chirp signals
        pllProportionalGain_ = 0.001f;  // Kp (reduced from 0.01f)
        pllIntegralGain_ = 0.0001f;      // Ki (reduced from 0.001f)
        pllMinFreq_ = 20.0f;            // Hz
        pllMaxFreq_ = 1000.0f;          // Hz

        // Bass enhancement filter (low-shelf)
        bassFilter_.designLowShelf(100.0f, 4.0f, static_cast<float>(sampleRate));

        reset();
    }

    void reset()
    {
        phase_ = 0.0f;
        integrator_ = 0.0f;
        phaseError_ = 0.0f;
        lastPhaseError_ = 0.0f;
        smoothedFrequency_ = frequency_;
        bassFilter_.reset();
    }

    void setSubharmonicMix(float mix)
    {
        subharmonicMix_ = jlimit(0.0f, 1.0f, mix);
    }

    void setBassEnhancement(float amount)
    {
        bassEnhancement_ = jlimit(0.0f, 1.0f, amount);
    }

    void enablePll(bool enable)
    {
        pllEnabled_ = enable;
    }

    void setFrequency(float freqHz)
    {
        frequency_ = jlimit(pllMinFreq_, pllMaxFreq_, freqHz);
    }

    // Process audio with PLL-based pitch tracking
    void process(float* output, const float* input, int numSamples)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            float x = input[i];

            // PLL phase tracking (if enabled)
            if (pllEnabled_)
            {
                // Calculate phase error using quadrature detection
                float error = calculatePhaseError(x, phase_);

                // PI controller to track frequency (further reduced gain for chirp signals)
                float freqCorrection = pllProportionalGain_ * error +
                                       pllIntegralGain_ * integrator_;

                // Update integrator (with clamping to prevent windup)
                integrator_ += error;
                integrator_ = std::clamp(integrator_, -1.0f, 1.0f);

                // Apply frequency correction with aggressive smoothing for chirp signals
                float trackedFreq = frequency_ + freqCorrection * 10.0f;  // Reduced from 100.0f
                trackedFreq = jlimit(pllMinFreq_, pllMaxFreq_, trackedFreq);

                // Smooth frequency changes to prevent clicks during chirps
                constexpr float smoothingFactor = 0.995f;  // Very smooth
                smoothedFrequency_ = smoothedFrequency_ * smoothingFactor + trackedFreq * (1.0f - smoothingFactor);

                // Update phase using smoothed frequency
                phase_ += 2.0f * pi * smoothedFrequency_ / static_cast<float>(sampleRate_);
                if (phase_ >= 2.0f * pi)
                    phase_ -= 2.0f * pi;

                // Store phase error for debugging
                lastPhaseError_ = error;
            }
            else
            {
                // Use fixed frequency
                phase_ += 2.0f * pi * frequency_ / static_cast<float>(sampleRate_);
                if (phase_ >= 2.0f * pi)
                    phase_ -= 2.0f * pi;
            }

            // Generate subharmonic (one octave below)
            float subPhase = phase_ * 0.5f;  // Divide by 2 for suboctave
            float subharmonic = std::sin(subPhase);

            // Generate second subharmonic (two octaves below)
            float sub2Phase = phase_ * 0.25f;
            float subharmonic2 = std::sin(sub2Phase);

            // Mix subharmonics
            float wet = subharmonic * 0.7f + subharmonic2 * 0.3f;

            // Apply bass enhancement
            if (bassEnhancement_ > 0.0f)
            {
                wet = bassFilter_.process(wet);
            }

            // Mix wet and dry
            output[i] = x * (1.0f - subharmonicMix_) + wet * subharmonicMix_;
        }
    }

    // Get current PLL error (for debugging/visualization)
    float getPhaseError() const
    {
        return lastPhaseError_;
    }

private:
    // Calculate phase error using quadrature detection
    float calculatePhaseError(float input, float phase)
    {
        // Generate quadrature signals
        float iSignal = std::cos(phase);
        float qSignal = std::sin(phase);

        // Calculate phase error
        float error = std::atan2(input * qSignal, input * iSignal);

        // Normalize to [-pi, pi]
        if (error > pi)
            error -= 2.0f * pi;
        if (error < -pi)
            error += 2.0f * pi;

        return error;
    }

    double sampleRate_;

    // Phase and frequency
    float phase_;
    float frequency_;
    float smoothedFrequency_;  // Smoothed PLL output for chirp signals

    // Parameters
    float subharmonicMix_;
    float bassEnhancement_;
    bool pllEnabled_;

    // PLL state
    float pllProportionalGain_;
    float pllIntegralGain_;
    float pllMinFreq_;
    float pllMaxFreq_;
    float integrator_;
    float phaseError_;
    float lastPhaseError_;

    // Bass enhancement filter
    BiquadFilter bassFilter_;
};

//==============================================================================
// SpectralEnhancer - FFT-based spectral enhancement with harmonic excitation
class SpectralEnhancer
{
public:
    SpectralEnhancer(double sampleRate)
        : sampleRate_(sampleRate)
        , enhancementAmount_(0.0f)
        , harmonicFocus_(0.5f)
        , fftSize_(2048)
        , hopSize_(512)  // 75% overlap (2048 / 4)
        , writeIndex_(0)
        , readyToProcess_(false)
    {
        // Initialize PureDSP FFT
        fft_ = std::make_unique<PureDSP::FFT>(fftSize_);

        // Allocate buffers
        int numBins = fftSize_ / 2 + 1;
        fftBuffer_.resize(fftSize_, 0.0f);
        windowBuffer_.resize(fftSize_);
        overlapBuffer_.resize(hopSize_, 0.0f);
        inputBuffer_.resize(fftSize_, 0.0f);
        outputBuffer_.resize(fftSize_, 0.0f);
        magnitudeBuffer_.resize(numBins, 0.0f);
        phaseBuffer_.resize(numBins, 0.0f);
        complexBuffer_.resize(numBins);

        // Create Hann window for smooth overlap-add
        for (int i = 0; i < fftSize_; ++i)
        {
            windowBuffer_[i] = 0.5f * (1.0f - std::cos(2.0f * pi * i / (fftSize_ - 1)));
        }

        reset();
    }

    ~SpectralEnhancer() = default;

    void reset()
    {
        writeIndex_ = 0;
        readyToProcess_ = false;
        std::fill(inputBuffer_.begin(), inputBuffer_.end(), 0.0f);
        std::fill(outputBuffer_.begin(), outputBuffer_.end(), 0.0f);
        std::fill(overlapBuffer_.begin(), overlapBuffer_.end(), 0.0f);
        std::fill(magnitudeBuffer_.begin(), magnitudeBuffer_.end(), 0.0f);
        std::fill(phaseBuffer_.begin(), phaseBuffer_.end(), 0.0f);
    }

    void setEnhancementAmount(float amount)
    {
        enhancementAmount_ = jlimit(0.0f, 1.0f, amount);
    }

    void setHarmonicFocus(float focus)
    {
        harmonicFocus_ = jlimit(0.0f, 1.0f, focus);
    }

    // Process audio with FFT-based spectral enhancement
    void process(float* output, const float* input, int numSamples)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            // Write to input buffer with circular buffering
            inputBuffer_[writeIndex_] = input[i];

            // Check if we have a full frame
            if (writeIndex_ == hopSize_ - 1)
            {
                readyToProcess_ = true;
            }

            // Read from overlap buffer or pass through
            if (writeIndex_ < static_cast<int>(overlapBuffer_.size()))
            {
                output[i] = overlapBuffer_[writeIndex_];
            }
            else
            {
                output[i] = input[i];  // Pass-through if not ready
            }

            writeIndex_++;

            // Process frame when ready
            if (readyToProcess_ && writeIndex_ >= hopSize_)
            {
                processFrame();

                // Reset write index
                writeIndex_ = 0;
                readyToProcess_ = false;
            }
        }
    }

private:
    void processFrame()
    {
        // Apply window to input
        for (int i = 0; i < fftSize_; ++i)
        {
            fftBuffer_[i] = inputBuffer_[i] * windowBuffer_[i];
        }

        // Perform forward FFT (real-valued)
        int numBins = fftSize_ / 2 + 1;
        fft_->realForward(fftBuffer_.data(), complexBuffer_.data());

        // Convert to magnitude/phase
        for (int i = 0; i < numBins; ++i)
        {
            magnitudeBuffer_[i] = std::abs(complexBuffer_[i]);
            phaseBuffer_[i] = std::arg(complexBuffer_[i]);
        }

        // Apply spectral enhancement
        enhanceSpectrum(magnitudeBuffer_.data(), numBins);

        // Convert back to complex
        for (int i = 0; i < numBins; ++i)
        {
            complexBuffer_[i] = PureDSP::FFT::Complex(
                magnitudeBuffer_[i] * std::cos(phaseBuffer_[i]),
                magnitudeBuffer_[i] * std::sin(phaseBuffer_[i])
            );
        }

        // Perform inverse FFT (real-valued)
        fft_->realInverse(complexBuffer_.data(), outputBuffer_.data());

        // Apply window and overlap-add
        for (int i = 0; i < fftSize_; ++i)
        {
            outputBuffer_[i] *= windowBuffer_[i];
        }

        // Add to overlap buffer
        for (int i = 0; i < hopSize_; ++i)
        {
            overlapBuffer_[i] = outputBuffer_[i] + outputBuffer_[i + hopSize_];
        }

        // Shift remaining samples
        for (int i = 0; i < fftSize_ - hopSize_; ++i)
        {
            outputBuffer_[i] = outputBuffer_[i + hopSize_];
        }
    }

    void enhanceSpectrum(float* magnitudes, int numBins)
    {
        if (enhancementAmount_ <= 0.0f)
            return;  // No enhancement

        // Calculate frequency range for enhancement
        // Focus parameter selects which harmonics to enhance
        float minFreqBin = 2.0f;  // Skip DC and first bin
        float maxFreqBin = numBins * (0.3f + 0.7f * harmonicFocus_);  // Focus on high mids

        for (int i = static_cast<int>(minFreqBin); i < static_cast<int>(maxFreqBin); ++i)
        {
            // Enhance harmonics based on amount
            float enhancement = 1.0f + enhancementAmount_ * 0.5f;

            // Add subtle harmonic excitation
            if (i > 1)
            {
                float prevMag = magnitudes[i - 1];
                float nextMag = magnitudes[i + 1];
                float avgMag = (prevMag + nextMag) * 0.5f;

                // Excite peaks
                if (magnitudes[i] > avgMag)
                {
                    magnitudes[i] *= enhancement;
                }
            }
        }
    }

    double sampleRate_;

    // Parameters
    float enhancementAmount_;
    float harmonicFocus_;

    // FFT configuration
    int fftSize_;
    int hopSize_;

    // FFT object
    std::unique_ptr<PureDSP::FFT> fft_;

    // Buffers
    std::vector<float> fftBuffer_;
    std::vector<float> windowBuffer_;
    std::vector<float> overlapBuffer_;
    std::vector<float> inputBuffer_;
    std::vector<float> outputBuffer_;
    std::vector<float> magnitudeBuffer_;
    std::vector<float> phaseBuffer_;
    std::vector<PureDSP::FFT::Complex> complexBuffer_;

    // State
    int writeIndex_;
    bool readyToProcess_;
};

//==============================================================================
// LinearSmoother - Parameter smoothing to prevent clicks
class LinearSmoother
{
public:
    LinearSmoother()
        : currentValue_(0.0f)
        , targetValue_(0.0f)
        , smoothingTime_(0.01f)  // 10ms default
        , sampleRate_(44100.0)
        , countdown_(0)
    {
        reset();
    }

    void reset()
    {
        currentValue_ = targetValue_;
        countdown_ = 0;
    }

    void setup(double sampleRate, float smoothingTimeSeconds)
    {
        sampleRate_ = sampleRate;
        smoothingTime_ = smoothingTimeSeconds;
    }

    void setTargetValue(float value)
    {
        targetValue_ = value;
        countdown_ = static_cast<int>(smoothingTime_ * static_cast<float>(sampleRate_));
    }

    float getNextValue()
    {
        if (countdown_ <= 0)
            return targetValue_;

        // Linear interpolation
        float step = (targetValue_ - currentValue_) / static_cast<float>(countdown_);
        currentValue_ += step;
        countdown_--;

        return currentValue_;
    }

    // Skip to target value (instant change)
    void resetToTarget()
    {
        currentValue_ = targetValue_;
        countdown_ = 0;
    }

    bool isSmoothing() const
    {
        return countdown_ > 0;
    }

private:
    float currentValue_;
    float targetValue_;
    float smoothingTime_;
    double sampleRate_;
    int countdown_;
};

//==============================================================================
} // namespace DSP
