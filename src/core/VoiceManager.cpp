/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * VoiceManager.cpp - Real-time safe multi-voice orchestration with SIMD optimization
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#include "VoiceManager.h"
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

    // Update sample rate for allocator
    // (VoiceAllocator doesn't need sample rate, but synthesis will)

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

    // Update parameter smoothing
    updateParameterSmoothing();

    // Build SIMD batches from active voices
    SIMDBatch batches[16];  // Max 16 batches (128 voices / 8)
    int numBatches = buildSIMDBatches(batches, 16);

    // Process each batch
    for (int i = 0; i < numBatches; ++i) {
        processSIMDBatch(batches[i], outputLeft, outputRight, numSamples);
    }

    // Apply master gain
    float masterGain = currentParams_->masterGain;

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
    targetParams_->masterGain = gain;
}

void VoiceManager::setAttackTime(float attackTime) {
    // Clamp to valid range
    attackTime = std::max(0.001f, std::min(1.0f, attackTime));
    targetParams_->attackTime = attackTime;
}

void VoiceManager::setReleaseTime(float releaseTime) {
    // Clamp to valid range
    releaseTime = std::max(0.001f, std::min(2.0f, releaseTime));
    targetParams_->releaseTime = releaseTime;
}

void VoiceManager::setVibratoRate(float rate) {
    // Clamp to valid range
    rate = std::max(0.0f, std::min(20.0f, rate));
    targetParams_->vibratoRate = rate;
}

void VoiceManager::setVibratoDepth(float depth) {
    // Clamp to valid range
    depth = std::max(0.0f, std::min(1.0f, depth));
    targetParams_->vibratoDepth = depth;
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

    // Generate audio (placeholder: simple sine wave)
    // TODO: Replace with FormantSynthesis/SubharmonicSynthesis
    generateSineWave(voice, scratchBuffer_.data(), numSamples);

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
    // Process each voice in batch
    // TODO: Implement true SIMD processing across voices
    // For now, process sequentially (placeholder for SIMD implementation)

    for (int i = 0; i < batch.count; ++i) {
        int voiceId = batch.voiceIds[i];
        VoiceInstance* voice = getVoice(voiceId);

        if (voice && voice->active) {
            processVoice(voice, outputLeft, outputRight, numSamples);
        }
    }

    // Future SIMD implementation:
    // - Load 8 voice frequencies into __m256
    // - Load 8 voice phases into __m256
    // - Generate 8 samples in parallel
    // - Store to 8 temporary buffers
    // - Mix to output with weighted gains
}

void VoiceManager::updateEnvelope(VoiceInstance* voice, int numSamples) {
    float deltaTime = static_cast<float>(numSamples) / static_cast<float>(sampleRate_);

    if (voice->inRelease) {
        // Release envelope
        float releaseTime = currentParams_->releaseTime;
        float releaseCoeff = std::exp(-1.0f / (releaseTime * static_cast<float>(sampleRate_)));

        voice->releaseGain *= releaseCoeff;

        // Check if release is complete
        if (voice->releaseGain < 0.001f) {
            voice->active = false;
            voice->inRelease = false;
            allocator_->freeVoice(voice->id);
        }
    } else {
        // Attack envelope
        float attackTime = currentParams_->attackTime;
        float attackCoeff = std::exp(-1.0f / (attackTime * static_cast<float>(sampleRate_)));

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
    // Simple linear interpolation toward target
    // TODO: Replace with LinearSmoother class

    const float smoothingCoeff = 0.01f;  // Smoothing factor

    currentParams_->masterGain += (targetParams_->masterGain - currentParams_->masterGain) * smoothingCoeff;
    currentParams_->attackTime += (targetParams_->attackTime - currentParams_->attackTime) * smoothingCoeff;
    currentParams_->releaseTime += (targetParams_->releaseTime - currentParams_->releaseTime) * smoothingCoeff;
    currentParams_->vibratoRate += (targetParams_->vibratoRate - currentParams_->vibratoRate) * smoothingCoeff;
    currentParams_->vibratoDepth += (targetParams_->vibratoDepth - currentParams_->vibratoDepth) * smoothingCoeff;
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
