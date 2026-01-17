/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * VoiceManager.cpp - Real-time safe multi-voice orchestration with SIMD optimization
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#include "VoiceManager.h"
#include "../synthesis/FormantSynthesis.h"
#include "../synthesis/SubharmonicSynthesis.h"
#include "../dsp/LinearSmoother.h"
#include <cassert>
#include <cmath>
#include <algorithm>
#include <cstring>

// SIMD intrinsics for x86/x64
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
    #define CHOIR_USE_SSE
    #include <immintrin.h>
#endif

namespace ChoirV2 {

//==============================================================================
// VoiceManager Implementation
//==============================================================================

VoiceManager::VoiceManager(int maxVoices, double sampleRate)
    : maxVoices_(maxVoices)
    , sampleRate_(sampleRate)
    , maxBlockSize_(0)
    , peakCpuUsage_(0.0f)
{
    // Pre-allocate all voice instances for real-time safety
    voices_.reserve(maxVoices_);

    for (int i = 0; i < maxVoices_; ++i) {
        auto voice = std::make_unique<VoiceInstance>();
        voice->id = i;
        voices_.push_back(std::move(voice));
    }

    // Create voice allocator
    allocator_ = std::make_unique<VoiceAllocator>(maxVoices_);

    // Create parameter structures
    currentParams_ = std::make_unique<VoiceParameters>();
    targetParams_ = std::make_unique<VoiceParameters>();

    // Create synthesis methods
    formantSynthesis_ = std::make_unique<FormantSynthesis>();
    subharmonicSynthesis_ = std::make_unique<SubharmonicSynthesis>();

    // Create parameter smoothers
    masterGainSmoother_ = std::make_unique<LinearSmoother>();
    attackTimeSmoother_ = std::make_unique<LinearSmoother>();
    releaseTimeSmoother_ = std::make_unique<LinearSmoother>();
    vibratoRateSmoother_ = std::make_unique<LinearSmoother>();
    vibratoDepthSmoother_ = std::make_unique<LinearSmoother>();

    // Initialize smoothers with default time constants
    float smoothingTime = 0.05f;  // 50ms smoothing
    masterGainSmoother_->setTimeConstant(smoothingTime, static_cast<float>(sampleRate_));
    attackTimeSmoother_->setTimeConstant(smoothingTime, static_cast<float>(sampleRate_));
    releaseTimeSmoother_->setTimeConstant(smoothingTime, static_cast<float>(sampleRate_));
    vibratoRateSmoother_->setTimeConstant(smoothingTime, static_cast<float>(sampleRate_));
    vibratoDepthSmoother_->setTimeConstant(smoothingTime, static_cast<float>(sampleRate_));

    // Initialize statistics
    stats_.totalVoices = maxVoices_;
}

VoiceManager::~VoiceManager() = default;

void VoiceManager::prepare(double sampleRate, int maxBlockSize) {
    sampleRate_ = sampleRate;
    maxBlockSize_ = maxBlockSize;

    // Pre-allocate scratch buffer for real-time safety
    scratchBuffer_.resize(maxBlockSize);
    std::fill(scratchBuffer_.begin(), scratchBuffer_.end(), 0.0f);

    // Initialize synthesis methods
    SynthesisParams params;
    params.sample_rate = static_cast<float>(sampleRate);
    params.max_block_size = maxBlockSize;

    formantSynthesis_->initialize(params);
    subharmonicSynthesis_->initialize(params);

    // Update smoothers with new sample rate
    float smoothingTime = 0.05f;  // 50ms smoothing
    masterGainSmoother_->setTimeConstant(smoothingTime, static_cast<float>(sampleRate));
    attackTimeSmoother_->setTimeConstant(smoothingTime, static_cast<float>(sampleRate));
    releaseTimeSmoother_->setTimeConstant(smoothingTime, static_cast<float>(sampleRate));
    vibratoRateSmoother_->setTimeConstant(smoothingTime, static_cast<float>(sampleRate));
    vibratoDepthSmoother_->setTimeConstant(smoothingTime, static_cast<float>(sampleRate));

    // Reset all voices
    for (auto& voice : voices_) {
        voice->active = false;
        voice->frequency = 0.0f;
        voice->amplitude = 0.0f;
        voice->pan = 0.0f;
        voice->age = 0.0f;
        voice->phase = 0.0f;
        voice->attackGain = 0.0f;
        voice->releaseGain = 1.0f;
        voice->inRelease = false;
        voice->cpuUsage = 0.0f;
    }

    // Reset allocator
    allocator_->resetAll();

    // Reset statistics
    resetStats();
}

void VoiceManager::processAudio(float* outputLeft, float* outputRight, int numSamples) {
    // Real-time safety: Clear buffers first
    std::memset(outputLeft, 0, sizeof(float) * numSamples);
    std::memset(outputRight, 0, sizeof(float) * numSamples);

    // Update parameter smoothing using LinearSmoother
    updateParameterSmoothing();

    // Build SIMD batches from active voices
    SIMDBatch batches[16];  // Max 16 batches (128 voices / 8)
    int numBatches = buildSIMDBatches(batches, 16);

    // Process each batch
    for (int i = 0; i < numBatches; ++i) {
        processSIMDBatch(batches[i], outputLeft, outputRight, numSamples);
    }

    // Apply master gain (using smoothed value)
    float masterGain = masterGainSmoother_->getCurrent();

    #if defined(CHOIR_USE_SSE)
        // SIMD master gain application
        int simdSamples = (numSamples / 4) * 4;

        for (int i = 0; i < simdSamples; i += 4) {
            __m128 left = _mm_loadu_ps(&outputLeft[i]);
            __m128 right = _mm_loadu_ps(&outputRight[i]);
            __m128 gain = _mm_set1_ps(masterGain);

            left = _mm_mul_ps(left, gain);
            right = _mm_mul_ps(right, gain);

            _mm_storeu_ps(&outputLeft[i], left);
            _mm_storeu_ps(&outputRight[i], right);
        }

        // Process remaining samples
        for (int i = simdSamples; i < numSamples; ++i) {
            outputLeft[i] *= masterGain;
            outputRight[i] *= masterGain;
        }
    #else
        // Scalar fallback
        for (int i = 0; i < numSamples; ++i) {
            outputLeft[i] *= masterGain;
            outputRight[i] *= masterGain;
        }
    #endif

    // Update statistics
    stats_.activeVoices = getActiveVoiceCount();

    // Update allocator priorities (do this periodically, not every block)
    // For now, we'll skip this in processAudio for performance
}

int VoiceManager::noteOn(int midiNote, float velocity) {
    // Allocate voice using VoiceAllocator
    auto result = allocator_->allocateVoice(midiNote, velocity);

    if (!result.success) {
        return -1;  // Allocation failed
    }

    // Get voice instance
    VoiceInstance* voice = getVoice(result.voiceId);
    if (!voice) {
        return -1;  // Shouldn't happen
    }

    // Initialize voice
    VoiceRecord* voiceRecord = allocator_->getVoice(result.voiceId);
    if (voiceRecord) {
        voice->id = result.voiceId;
        voice->active = true;
        voice->frequency = voiceRecord->frequency;
        voice->amplitude = voiceRecord->amplitude;
        voice->pan = voiceRecord->pan;
        voice->age = 0.0f;
        voice->phase = 0.0f;
        voice->attackGain = 0.0f;
        voice->releaseGain = 1.0f;
        voice->inRelease = false;
        voice->cpuUsage = 0.0f;
    }

    // Update statistics
    if (result.stolen) {
        stats_.stolenVoices++;
    }

    return result.voiceId;
}

void VoiceManager::noteOff(int midiNote, float velocity) {
    (void)velocity;  // Unused for now

    // Find voice by MIDI note
    int voiceId = findVoiceByNote(midiNote);
    if (voiceId == -1) {
        return;  // Voice not found
    }

    // Get voice instance
    VoiceInstance* voice = getVoice(voiceId);
    if (!voice || !voice->active) {
        return;  // Voice not active
    }

    // Trigger release
    voice->inRelease = true;
    voice->releaseGain = 1.0f;

    // Note: We don't free the voice yet
    // It will be freed after release envelope completes
}

void VoiceManager::allNotesOff() {
    // Immediately stop all voices (no release)
    for (auto& voice : voices_) {
        if (voice->active) {
            voice->active = false;
            voice->inRelease = false;

            // Free from allocator
            allocator_->freeVoice(voice->id);
        }
    }

    stats_.activeVoices = 0;
}

void VoiceManager::setMasterGain(float gain) {
    // Clamp to valid range
    gain = std::max(0.0f, std::min(2.0f, gain));
    masterGainSmoother_->setTarget(gain);
}

void VoiceManager::setAttackTime(float attackTime) {
    // Clamp to valid range
    attackTime = std::max(0.001f, std::min(1.0f, attackTime));
    attackTimeSmoother_->setTarget(attackTime);
}

void VoiceManager::setReleaseTime(float releaseTime) {
    // Clamp to valid range
    releaseTime = std::max(0.001f, std::min(2.0f, releaseTime));
    releaseTimeSmoother_->setTarget(releaseTime);
}

void VoiceManager::setVibratoRate(float rate) {
    // Clamp to valid range
    rate = std::max(0.0f, std::min(20.0f, rate));
    vibratoRateSmoother_->setTarget(rate);
}

void VoiceManager::setVibratoDepth(float depth) {
    // Clamp to valid range
    depth = std::max(0.0f, std::min(1.0f, depth));
    vibratoDepthSmoother_->setTarget(depth);
}

VoiceInstance* VoiceManager::getVoice(int voiceId) {
    if (voiceId < 0 || voiceId >= maxVoices_) {
        return nullptr;
    }

    return voices_[voiceId].get();
}

int VoiceManager::getActiveVoiceCount() const {
    return allocator_->getActiveVoiceCount();
}

int VoiceManager::getMaxVoices() const {
    return maxVoices_;
}

const VoiceManagerStats& VoiceManager::getStats() const {
    return stats_;
}

void VoiceManager::resetStats() {
    stats_ = VoiceManagerStats();
    stats_.totalVoices = maxVoices_;
    peakCpuUsage_.store(0.0f);
}

//==============================================================================
// Private Methods
//==============================================================================

int VoiceManager::buildSIMDBatches(SIMDBatch* batches, int maxBatches) {
    int batchIndex = 0;
    int countInBatch = 0;

    for (int i = 0; i < maxVoices_; ++i) {
        VoiceInstance* voice = voices_[i].get();
        if (!voice || !voice->active) {
            continue;
        }

        // Add voice to current batch
        if (countInBatch == 0) {
            // Start new batch
            if (batchIndex >= maxBatches) {
                break;  // No more batches available
            }
            batches[batchIndex].count = 0;
        }

        batches[batchIndex].voiceIds[countInBatch] = i;
        countInBatch++;

        // Check if batch is full
        if (countInBatch == SIMDBatch::MAX_BATCH_SIZE) {
            batches[batchIndex].count = countInBatch;
            batchIndex++;
            countInBatch = 0;
        }
    }

    // Handle last partial batch
    if (countInBatch > 0 && batchIndex < maxBatches) {
        batches[batchIndex].count = countInBatch;
        batchIndex++;
    }

    return batchIndex;
}

void VoiceManager::processVoice(VoiceInstance* voice,
                                float* outputLeft,
                                float* outputRight,
                                int numSamples) {
    if (!voice || !voice->active) {
        return;
    }

    // Real-time safety: Use pre-allocated scratch buffer
    // (no dynamic allocation in audio thread)
    if (scratchBuffer_.size() < static_cast<size_t>(numSamples)) {
        return;  // Buffer too small (shouldn't happen if prepare() was called)
    }

    // Generate audio using FormantSynthesis
    // Create temporary Voice object for synthesis
    Voice tempVoice;
    tempVoice.setFrequency(voice->frequency);
    tempVoice.setAmplitude(voice->amplitude);
    tempVoice.setPan(voice->pan);

    // Create default phoneme (schwa vowel)
    Phoneme defaultPhoneme;
    defaultPhoneme.ipa = "ə";
    defaultPhoneme.category = PhonemeCategory::Vowel;
    defaultPhoneme.articulatory.is_voiced = true;

    // Synthesize using FormantSynthesis (replaces placeholder sine wave)
    auto result = formantSynthesis_->synthesizeVoice(
        &tempVoice,
        &defaultPhoneme,
        scratchBuffer_.data(),
        numSamples
    );

    if (!result.success) {
        // Fallback to sine wave if synthesis fails
        generateSineWave(voice, scratchBuffer_.data(), numSamples);
    }

    // Calculate pan gains
    float leftGain, rightGain;
    applyPan(1.0f, voice->pan, leftGain, rightGain);

    // Apply amplitude envelope
    float envelopeGain = voice->attackGain * voice->releaseGain;

    // Mix to output
    for (int i = 0; i < numSamples; ++i) {
        float sample = scratchBuffer_[i] * voice->amplitude * envelopeGain;
        outputLeft[i] += sample * leftGain;
        outputRight[i] += sample * rightGain;
    }

    // Update envelope
    updateEnvelope(voice, numSamples);

    // Update voice age
    voice->age += static_cast<float>(numSamples) / static_cast<float>(sampleRate_);
}

void VoiceManager::processSIMDBatch(const SIMDBatch& batch,
                                    float* outputLeft,
                                    float* outputRight,
                                    int numSamples) {
    // True SIMD processing across voices
    #if defined(__AVX__) && defined(__x86_64__)
        // AVX implementation: Process 8 voices in parallel
        if (batch.count == 8) {
            // Get all 8 voices
            VoiceInstance* voices[8];
            for (int i = 0; i < 8; ++i) {
                voices[i] = getVoice(batch.voiceIds[i]);
            }

            // Check if all voices are active
            bool allActive = true;
            for (int i = 0; i < 8; ++i) {
                if (!voices[i] || !voices[i]->active) {
                    allActive = false;
                    break;
                }
            }

            if (allActive) {
                // AVX SIMD processing
                for (int i = 0; i < numSamples; ++i) {
                    // Load frequencies and phases
                    __m256 freqs = _mm256_set_ps(
                        voices[7]->frequency, voices[6]->frequency,
                        voices[5]->frequency, voices[4]->frequency,
                        voices[3]->frequency, voices[2]->frequency,
                        voices[1]->frequency, voices[0]->frequency
                    );

                    __m256 phases = _mm256_set_ps(
                        voices[7]->phase, voices[6]->phase,
                        voices[5]->phase, voices[4]->phase,
                        voices[3]->phase, voices[2]->phase,
                        voices[1]->phase, voices[0]->phase
                    );

                    __m256 amplitudes = _mm256_set_ps(
                        voices[7]->amplitude, voices[6]->amplitude,
                        voices[5]->amplitude, voices[4]->amplitude,
                        voices[3]->amplitude, voices[2]->amplitude,
                        voices[1]->amplitude, voices[0]->amplitude
                    );

                    // Calculate phase increments
                    __m256 sampleRateInv = _mm256_set1_ps(1.0f / static_cast<float>(sampleRate_));
                    __m256 phaseIncs = _mm256_mul_ps(freqs, sampleRateInv);

                    // Update phases
                    phases = _mm256_add_ps(phases, phaseIncs);

                    // Wrap phases (0 to 2*pi)
                    __m256 twoPi = _mm256_set1_ps(2.0f * M_PI);
                    __m256 mask = _mm256_cmp_ps(phases, twoPi, _CMP_GE_OQ);
                    __m256 wrapped = _mm256_and_ps(mask, twoPi);
                    phases = _mm256_sub_ps(phases, wrapped);

                    // Generate sine waves using Taylor approximation (SIMD-compatible)
                    // sin(x) ≈ x - x³/6 + x⁵/120
                    __m256 samples = phases;
                    __m256 x3 = _mm256_mul_ps(phases, _mm256_mul_ps(phases, phases));
                    __m256 x5 = _mm256_mul_ps(x3, _mm256_mul_ps(phases, phases));
                    samples = _mm256_sub_ps(samples, _mm256_mul_ps(x3, _mm256_set1_ps(1.0f/6.0f)));
                    samples = _mm256_add_ps(samples, _mm256_mul_ps(x5, _mm256_set1_ps(1.0f/120.0f)));

                    // Apply amplitudes
                    samples = _mm256_mul_ps(samples, amplitudes);

                    // Store back to voices and update phases
                    float tempPhases[8];
                    float tempSamples[8];
                    _mm256_storeu_ps(tempPhases, phases);
                    _mm256_storeu_ps(tempSamples, samples);

                    // Mix to output with pan
                    for (int j = 0; j < 8; ++j) {
                        voices[j]->phase = tempPhases[j];

                        float leftGain, rightGain;
                        applyPan(1.0f, voices[j]->pan, leftGain, rightGain);

                        float envelopeGain = voices[j]->attackGain * voices[j]->releaseGain;
                        float sample = tempSamples[j] * envelopeGain;

                        outputLeft[i] += sample * leftGain;
                        outputRight[i] += sample * rightGain;
                    }
                }

                // Update envelopes
                for (int i = 0; i < 8; ++i) {
                    updateEnvelope(voices[i], numSamples);
                    voices[i]->age += static_cast<float>(numSamples) / static_cast<float>(sampleRate_);
                }

                return;  // Done with SIMD processing
            }
        }
    #endif

    // Fallback: Process each voice sequentially
    for (int i = 0; i < batch.count; ++i) {
        int voiceId = batch.voiceIds[i];
        VoiceInstance* voice = getVoice(voiceId);

        if (voice && voice->active) {
            processVoice(voice, outputLeft, outputRight, numSamples);
        }
    }
}

void VoiceManager::updateEnvelope(VoiceInstance* voice, int numSamples) {
    // Use deltaTime for accurate time-based envelope calculation
    float deltaTime = static_cast<float>(numSamples) / static_cast<float>(sampleRate_);

    // Get smoothed envelope parameters
    float attackTime = attackTimeSmoother_->getCurrent();
    float releaseTime = releaseTimeSmoother_->getCurrent();

    if (voice->inRelease) {
        // Release envelope: exponential decay
        // coefficient = exp(-deltaTime / release_time)
        float releaseCoeff = std::exp(-deltaTime / releaseTime);
        voice->releaseGain *= releaseCoeff;

        // Check if release is complete
        if (voice->releaseGain < 0.001f) {
            voice->active = false;
            voice->inRelease = false;
            allocator_->freeVoice(voice->id);
        }
    } else {
        // Attack envelope: exponential attack
        // coefficient = exp(-deltaTime / attack_time)
        float attackCoeff = std::exp(-deltaTime / attackTime);
        voice->attackGain += (1.0f - voice->attackGain) * (1.0f - attackCoeff);

        // Clamp attack gain
        voice->attackGain = std::min(1.0f, voice->attackGain);
    }
}

void VoiceManager::applyPan(float input, float pan, float& leftGain, float& rightGain) {
    // Equal-power pan law: -3dB at center
    // Left gain = cos((pan + 1) * pi / 4)
    // Right gain = cos((1 - pan) * pi / 4)

    float panNormalized = (pan + 1.0f) * 0.5f;  // -1..1 to 0..1
    float angle = panNormalized * M_PI_2;  // 0 to pi/2

    leftGain = std::cos(angle) * input;
    rightGain = std::sin(angle) * input;
}

void VoiceManager::updateParameterSmoothing() {
    // Update all parameter smoothers (one sample step)
    masterGainSmoother_->process();
    attackTimeSmoother_->process();
    releaseTimeSmoother_->process();
    vibratoRateSmoother_->process();
    vibratoDepthSmoother_->process();
}

int VoiceManager::findVoiceByNote(int midiNote) {
    // Find first active voice matching MIDI note
    for (int i = 0; i < maxVoices_; ++i) {
        VoiceRecord* voiceRecord = allocator_->getVoice(i);
        if (voiceRecord && voiceRecord->active && voiceRecord->midiNote == midiNote) {
            return i;
        }
    }

    return -1;  // Not found
}

void VoiceManager::generateSineWave(VoiceInstance* voice, float* output, int numSamples) {
    // Placeholder synthesis: Simple sine wave
    // TODO: Replace with FormantSynthesis/SubharmonicSynthesis

    float frequency = voice->frequency;
    float phaseIncrement = (2.0f * M_PI * frequency) / static_cast<float>(sampleRate_);

    for (int i = 0; i < numSamples; ++i) {
        output[i] = std::sin(voice->phase);
        voice->phase += phaseIncrement;

        // Wrap phase
        if (voice->phase >= 2.0f * M_PI) {
            voice->phase -= 2.0f * M_PI;
        }
    }
}

} // namespace ChoirV2
