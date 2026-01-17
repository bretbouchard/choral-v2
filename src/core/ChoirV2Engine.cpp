/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * ChoirV2Engine.cpp - Main synthesis engine implementation
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#include "ChoirV2Engine.h"
#include "PhonemeDatabase.h"
#include "LanguageLoader.h"
#include "G2PEngine.h"
#include "VoiceManager.h"
#include "../synthesis/FormantSynthesis.h"
#include "../synthesis/SubharmonicSynthesis.h"
#include "../synthesis/DiphoneSynthesis.h"
#include "../synthesis/ISynthesisMethod.h"
#include "../utils/LockFreeQueue.h"
#include "../utils/MemoryPool.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <sstream>
#include <unordered_map>

namespace ChoirV2 {

//==============================================================================
// Constructor
//==============================================================================
ChoirV2Engine::ChoirV2Engine(const ChoirV2Params& params)
    : params_(params)
    , initialized_(false)
{
    // Initialize performance statistics
    perf_stats_.cpu_usage = 0.0f;
    perf_stats_.active_voices = 0;
    perf_stats_.stolen_voices = 0;
    perf_stats_.average_latency = 0.0f;
    perf_stats_.buffer_underruns = 0;
}

//==============================================================================
// Destructor
//==============================================================================
ChoirV2Engine::~ChoirV2Engine()
{
    shutdown();
}

//==============================================================================
// Initialize the engine
//==============================================================================
bool ChoirV2Engine::initialize()
{
    if (initialized_) {
        return true; // Already initialized
    }

    // Create phoneme database
    phoneme_db_ = std::make_shared<PhonemeDatabase>();
    if (!phoneme_db_) {
        return false;
    }

    // Create language loader
    language_loader_ = std::make_shared<LanguageLoader>(phoneme_db_);
    if (!language_loader_) {
        return false;
    }

    // Create G2P engine
    g2p_engine_ = std::make_shared<G2PEngine>(phoneme_db_);
    if (!g2p_engine_) {
        return false;
    }

    // Create voice manager
    voice_manager_ = std::make_shared<VoiceManager>(params_.num_voices);
    if (!voice_manager_) {
        return false;
    }

    // Set CPU limit for voice stealing
    voice_manager_->setCPULimit(params_.cpu_limit);

    // Create default synthesis method (formant)
    synthesis_method_ = SynthesisMethodFactory::create(params_.synthesis_method);
    if (!synthesis_method_) {
        // Fallback to formant synthesis
        synthesis_method_ = SynthesisMethodFactory::create("formant");
        if (!synthesis_method_) {
            return false;
        }
    }

    // Initialize synthesis method
    SynthesisParams synth_params;
    synth_params.sample_rate = params_.sample_rate;
    synth_params.max_block_size = params_.max_block_size;
    synth_params.enable_simd = params_.enable_simd;
    synth_params.enable_anti_aliasing = params_.enable_anti_aliasing;
    synth_params.oversampling_factor = params_.oversampling_factor;

    if (!synthesis_method_->initialize(synth_params)) {
        return false;
    }

    initialized_ = true;
    return true;
}

//==============================================================================
// Shutdown the engine
//==============================================================================
void ChoirV2Engine::shutdown()
{
    if (!initialized_) {
        return;
    }

    // Release synthesis method
    synthesis_method_.reset();

    // Release voice manager
    voice_manager_.reset();

    // Release G2P engine
    g2p_engine_.reset();

    // Release language loader
    language_loader_.reset();

    // Release phoneme database
    phoneme_db_.reset();

    initialized_ = false;
}

//==============================================================================
// Load a language
//==============================================================================
bool ChoirV2Engine::loadLanguage(const std::string& language_file)
{
    if (!initialized_) {
        return false;
    }

    // Load language definition
    auto language = language_loader_->loadLanguage(language_file);
    if (!language) {
        return false;
    }

    // Set language in G2P engine
    g2p_engine_->setLanguage(language);

    return true;
}

//==============================================================================
// Synthesize text to audio
//==============================================================================
bool ChoirV2Engine::synthesize(
    const std::string& text,
    float* output,
    int num_samples
)
{
    if (!initialized_) {
        return false;
    }

    // Convert text to phonemes
    auto g2p_result = g2p_engine_->convert(text);
    if (!g2p_result.success) {
        return false;
    }

    // Calculate duration from phoneme sequence
    float total_duration = 0.0f;
    for (float duration : g2p_result.durations) {
        total_duration += duration;
    }

    // Calculate number of samples needed
    int samples_needed = static_cast<int>(total_duration * params_.sample_rate);

    // Clamp to requested size
    samples_needed = std::min(samples_needed, num_samples);

    // Clear output buffer
    std::fill(output, output + num_samples * 2, 0.0f); // Stereo

    // Synthesize each phoneme
    int sample_offset = 0;
    for (size_t i = 0; i < g2p_result.phonemes.size(); ++i) {
        const std::string& phoneme_symbol = g2p_result.phonemes[i];
        float phoneme_duration = g2p_result.durations[i];
        float phoneme_pitch = g2p_result.pitches.size() > i ? g2p_result.pitches[i] : 440.0f;

        // Get phoneme data
        auto phoneme = phoneme_db_->getPhoneme(phoneme_symbol);
        if (!phoneme) {
            continue; // Skip unknown phonemes
        }

        // Allocate a voice for this phoneme
        auto allocation = voice_manager_->allocateVoice(
            VoicePriority::Medium,
            phoneme_pitch,
            1.0f // Amplitude
        );

        if (!allocation.success) {
            continue; // Skip if voice allocation failed
        }

        // Get the voice
        Voice* voice = voice_manager_->getVoice(allocation.voice_id);
        if (!voice) {
            continue;
        }

        // Set phoneme
        voice->current_phoneme = phoneme_symbol;
        voice->duration = phoneme_duration;

        // Calculate samples for this phoneme
        int phoneme_samples = static_cast<int>(phoneme_duration * params_.sample_rate);
        phoneme_samples = std::min(phoneme_samples, num_samples - sample_offset);

        // Synthesize the phoneme
        auto synth_result = synthesis_method_->synthesizeVoice(
            voice,
            phoneme.get(),
            output + sample_offset * 2,
            phoneme_samples
        );

        if (!synth_result.success) {
            // Release voice on failure
            voice_manager_->releaseVoice(allocation.voice_id);
            continue;
        }

        // Update performance stats
        perf_stats_.cpu_usage = synth_result.cpu_usage;

        sample_offset += phoneme_samples;

        // Release voice after phoneme completes
        voice_manager_->releaseVoice(allocation.voice_id);

        if (sample_offset >= num_samples) {
            break;
        }
    }

    return true;
}

//==============================================================================
// Synthesize text with custom melody
//==============================================================================
bool ChoirV2Engine::synthesizeWithMelody(
    const std::string& text,
    const std::vector<float>& frequencies,
    float* output,
    int num_samples
)
{
    if (!initialized_) {
        return false;
    }

    // Convert text to phonemes
    auto g2p_result = g2p_engine_->convert(text);
    if (!g2p_result.success) {
        return false;
    }

    // Validate frequency array size
    if (frequencies.size() != g2p_result.phonemes.size()) {
        return false;
    }

    // Clear output buffer
    std::fill(output, output + num_samples * 2, 0.0f); // Stereo

    // Synthesize each phoneme with custom frequency
    int sample_offset = 0;
    for (size_t i = 0; i < g2p_result.phonemes.size(); ++i) {
        const std::string& phoneme_symbol = g2p_result.phonemes[i];
        float phoneme_duration = g2p_result.durations[i];
        float phoneme_pitch = frequencies[i];

        // Get phoneme data
        auto phoneme = phoneme_db_->getPhoneme(phoneme_symbol);
        if (!phoneme) {
            continue;
        }

        // Allocate a voice
        auto allocation = voice_manager_->allocateVoice(
            VoicePriority::Medium,
            phoneme_pitch,
            1.0f
        );

        if (!allocation.success) {
            continue;
        }

        Voice* voice = voice_manager_->getVoice(allocation.voice_id);
        if (!voice) {
            continue;
        }

        voice->current_phoneme = phoneme_symbol;
        voice->duration = phoneme_duration;

        int phoneme_samples = static_cast<int>(phoneme_duration * params_.sample_rate);
        phoneme_samples = std::min(phoneme_samples, num_samples - sample_offset);

        // Synthesize
        auto synth_result = synthesis_method_->synthesizeVoice(
            voice,
            phoneme.get(),
            output + sample_offset * 2,
            phoneme_samples
        );

        if (!synth_result.success) {
            voice_manager_->releaseVoice(allocation.voice_id);
            continue;
        }

        perf_stats_.cpu_usage = synth_result.cpu_usage;
        sample_offset += phoneme_samples;

        voice_manager_->releaseVoice(allocation.voice_id);

        if (sample_offset >= num_samples) {
            break;
        }
    }

    return true;
}

//==============================================================================
// Set synthesis method
//==============================================================================
bool ChoirV2Engine::setSynthesisMethod(const std::string& method)
{
    if (!initialized_) {
        return false;
    }

    // Create new synthesis method
    auto new_method = SynthesisMethodFactory::create(method);
    if (!new_method) {
        return false;
    }

    // Initialize new method
    SynthesisParams synth_params;
    synth_params.sample_rate = params_.sample_rate;
    synth_params.max_block_size = params_.max_block_size;
    synth_params.enable_simd = params_.enable_simd;
    synth_params.enable_anti_aliasing = params_.enable_anti_aliasing;
    synth_params.oversampling_factor = params_.oversampling_factor;

    if (!new_method->initialize(synth_params)) {
        return false;
    }

    // Replace current method
    synthesis_method_ = std::move(new_method);

    return true;
}

//==============================================================================
// Get performance statistics
//==============================================================================
ChoirV2Engine::PerfStats ChoirV2Engine::getPerformanceStats() const
{
    PerfStats stats = perf_stats_;

    // Update active voice count
    if (voice_manager_) {
        stats.active_voices = voice_manager_->getActiveVoiceCount();

        // Get voice stealing statistics
        auto stealing_stats = voice_manager_->getStealingStats();
        stats.stolen_voices = stealing_stats.total_stolen;
    }

    return stats;
}

//==============================================================================
// Allocate voices
//==============================================================================
bool ChoirV2Engine::allocateVoices()
{
    if (!voice_manager_) {
        return false;
    }

    // Voice manager is already initialized with max voices
    // This is a placeholder for future expansion
    return true;
}

//==============================================================================
// Process audio (internal)
//==============================================================================
void ChoirV2Engine::processAudio(float* output, int num_samples)
{
    // Get active voices
    auto active_voices = voice_manager_->getActiveVoices();

    if (active_voices.empty()) {
        // No active voices, output silence
        std::fill(output, output + num_samples * 2, 0.0f);
        return;
    }

    // Prepare phoneme array for SIMD processing
    std::vector<const Phoneme*> phonemes;
    for (auto* voice : active_voices) {
        auto phoneme = phoneme_db_->getPhoneme(voice->current_phoneme);
        phonemes.push_back(phoneme.get());
    }

    // Synthesize all voices using SIMD
    auto result = synthesis_method_->synthesizeVoicesSIMD(
        active_voices,
        phonemes,
        output,
        num_samples
    );

    // Update CPU usage
    perf_stats_.cpu_usage = result.cpu_usage;

    // Update voice states
    double delta_time = static_cast<double>(num_samples) / params_.sample_rate;
    voice_manager_->updateStates(delta_time);
}

//==============================================================================
// Update performance statistics
//==============================================================================
void ChoirV2Engine::updatePerformanceStats()
{
    // Update statistics from voice manager
    if (voice_manager_) {
        perf_stats_.active_voices = voice_manager_->getActiveVoiceCount();

        auto stealing_stats = voice_manager_->getStealingStats();
        perf_stats_.stolen_voices = stealing_stats.total_stolen;
    }

    // Update CPU usage from synthesis method
    if (synthesis_method_) {
        auto method_stats = synthesis_method_->getStats();
        perf_stats_.cpu_usage = method_stats.average_cpu_usage;
    }
}

} // namespace ChoirV2
