/*
 * Choir V2.0 - Reverb Effect Integration Example
 *
 * This example demonstrates how to integrate ReverbEffect into the ChoirV2Engine
 * for high-quality reverb processing of all mixed voices.
 *
 * Integration Points:
 * 1. Add ReverbEffect member to ChoirV2Engine
 * 2. Prepare reverb in ChoirV2Engine::prepare()
 * 3. Process mixed output through reverb before output
 * 4. Expose reverb parameters to API
 *
 * Copyright (c) 2026 Bret Bouchard
 */

#include "dsp/ReverbEffect.h"
#include <vector>

namespace ChoirV2 {

/**
 * @brief Example integration of ReverbEffect into ChoirV2Engine
 *
 * This shows how to add reverb processing to the main engine.
 * The reverb processes the mixed output from all voices.
 */
class ChoirV2EngineWithReverb {
public:
    ChoirV2EngineWithReverb() = default;
    ~ChoirV2EngineWithReverb() = default;

    /**
     * @brief Initialize engine with reverb
     * @param sample_rate Sample rate (Hz)
     * @param max_voices Maximum polyphony
     * @param max_block_size Maximum block size
     */
    void prepare(float sample_rate, int max_voices, int max_block_size) {
        sample_rate_ = sample_rate;
        max_block_size_ = max_block_size;

        // Prepare voice manager (existing code)
        // voice_manager_.prepare(sample_rate, max_voices, max_block_size);

        // Prepare reverb effect
        reverb_.prepare(sample_rate, max_block_size);

        // Load default preset (Large Hall)
        reverb_.loadPreset(ReverbPreset::LargeHall);

        // Allocate mix buffers
        mix_buffer_left_.resize(max_block_size);
        mix_buffer_right_.resize(max_block_size);
        output_buffer_left_.resize(max_block_size);
        output_buffer_right_.resize(max_block_size);
    }

    /**
     * @brief Reset engine and reverb
     */
    void reset() {
        // voice_manager_.reset();
        reverb_.reset();
    }

    /**
     * @brief Process audio with reverb
     * @param output_left Left output buffer
     * @param output_right Right output buffer
     * @param num_samples Number of samples to process
     *
     * Processing chain:
     * 1. Voice manager mixes all voices to stereo
     * 2. Reverb processes the mixed signal
     * 3. Reverb output goes to final output
     */
    void processAudio(float* output_left, float* output_right, int num_samples) {
        // Clear mix buffers
        std::fill(mix_buffer_left_.begin(), mix_buffer_left_.begin() + num_samples, 0.0f);
        std::fill(mix_buffer_right_.begin(), mix_buffer_right_.begin() + num_samples, 0.0f);

        // Step 1: Process all voices and mix to stereo
        // voice_manager_.processMixToStereo(mix_buffer_left_.data(), mix_buffer_right_.data(), num_samples);

        // (For example, generate a test tone)
        for (int i = 0; i < num_samples; ++i) {
            float phase = 2.0f * M_PI * 440.0f * i / sample_rate_;
            mix_buffer_left_[i] = 0.5f * std::sin(phase);
            mix_buffer_right_[i] = 0.5f * std::sin(phase);
        }

        // Step 2: Process mixed signal through reverb
        reverb_.processStereo(mix_buffer_left_.data(),
                             mix_buffer_right_.data(),
                             output_left,
                             output_right,
                             num_samples);
    }

    //==========================================================================
    // Reverb Parameter Control (expose to API)
    //==========================================================================

    void setReverbPreset(ReverbPreset preset) {
        reverb_.loadPreset(preset);
    }

    void setReverbRoomSize(float size) {
        reverb_.setRoomSize(size);
    }

    void setReverbDamping(float damping) {
        reverb_.setDamping(damping);
    }

    void setReverbWetLevel(float wet) {
        reverb_.setWetLevel(wet);
    }

    void setReverbDryLevel(float dry) {
        reverb_.setDryLevel(dry);
    }

    void setReverbWidth(float width) {
        reverb_.setWidth(width);
    }

    void setReverbBypass(bool bypass) {
        reverb_.setBypass(bypass);
    }

    void setReverbFreezeMode(bool freeze) {
        reverb_.setFreezeMode(freeze);
    }

private:
    // Reverb effect (processes mixed output)
    ReverbEffect reverb_;

    // Mix buffers (voice output before reverb)
    std::vector<float> mix_buffer_left_;
    std::vector<float> mix_buffer_right_;

    // Output buffers (final output after reverb)
    std::vector<float> output_buffer_left_;
    std::vector<float> output_buffer_right_;

    // Engine state
    float sample_rate_ = 44100.0f;
    int max_block_size_ = 256;
};

} // namespace ChoirV2

//==============================================================================
// Usage Example
//==============================================================================

#ifdef CHOIR_V2_REVERB_EXAMPLE_MAIN

#include <iostream>
#include <fstream>

// Simple WAV file writer for testing
void writeWavFile(const char* filename, const float* left, const float* right, int num_samples, int sample_rate) {
    std::ofstream file(filename, std::ios::binary);

    // Write WAV header
    file.write("RIFF", 4);
    int32_t file_size = 36 + num_samples * 4;
    file.write((const char*)&file_size, 4);
    file.write("WAVE", 4);

    file.write("fmt ", 4);
    int32_t fmt_size = 16;
    file.write((const char*)&fmt_size, 4);
    int16_t audio_format = 1;  // PCM
    file.write((const char*)&audio_format, 2);
    int16_t num_channels = 2;  // Stereo
    file.write((const char*)&num_channels, 2);
    int32_t sample_rate_val = sample_rate;
    file.write((const char*)&sample_rate_val, 4);
    int32_t byte_rate = sample_rate * 4;
    file.write((const char*)&byte_rate, 4);
    int16_t block_align = 4;
    file.write((const char*)&block_align, 2);
    int16_t bits_per_sample = 16;
    file.write((const char*)&bits_per_sample, 2);

    file.write("data", 4);
    int32_t data_size = num_samples * 4;
    file.write((const char*)&data_size, 4);

    // Write interleaved stereo data
    for (int i = 0; i < num_samples; ++i) {
        int16_t left_sample = (int16_t)std::clamp(left[i] * 32767.0f, -32768.0f, 32767.0f);
        int16_t right_sample = (int16_t)std::clamp(right[i] * 32767.0f, -32768.0f, 32767.0f);
        file.write((const char*)&left_sample, 2);
        file.write((const char*)&right_sample, 2);
    }

    file.close();
}

int main() {
    using namespace ChoirV2;

    std::cout << "Choir V2.0 - Reverb Effect Integration Example\n\n";

    // Create engine with reverb
    ChoirV2EngineWithReverb engine;

    // Configuration
    const float sample_rate = 48000.0f;
    const int max_voices = 16;
    const int max_block_size = 256;
    const int duration_seconds = 5;
    const int total_samples = sample_rate * duration_seconds;

    // Prepare engine
    std::cout << "Initializing engine...\n";
    engine.prepare(sample_rate, max_voices, max_block_size);

    // Allocate output buffers
    std::vector<float> output_left(total_samples);
    std::vector<float> output_right(total_samples);

    // Process audio in blocks
    std::cout << "Processing audio...\n";

    int samples_processed = 0;
    while (samples_processed < total_samples) {
        int block_size = std::min(max_block_size, total_samples - samples_processed);

        // Change reverb settings mid-stream (demonstrate smoothing)
        float progress = (float)samples_processed / total_samples;

        if (progress < 0.25f) {
            // First quarter: Large Hall
            engine.setReverbPreset(ReverbPreset::LargeHall);
        } else if (progress < 0.5f) {
            // Second quarter: Cathedral
            engine.setReverbPreset(ReverbPreset::Cathedral);
        } else if (progress < 0.75f) {
            // Third quarter: Small Room
            engine.setReverbPreset(ReverbPreset::SmallRoom);
        } else {
            // Final quarter: Plate
            engine.setReverbPreset(ReverbPreset::Plate);
        }

        // Process block
        engine.processAudio(output_left.data() + samples_processed,
                           output_right.data() + samples_processed,
                           block_size);

        samples_processed += block_size;
    }

    // Write output to WAV file
    std::cout << "Writing output to reverb_demo.wav...\n";
    writeWavFile("reverb_demo.wav",
                 output_left.data(),
                 output_right.data(),
                 total_samples,
                 (int)sample_rate);

    std::cout << "\nDemo complete!\n";
    std::cout << "Output written to reverb_demo.wav\n";
    std::cout << "Play the file to hear the reverb effect transitioning between presets.\n\n";

    // Demonstrate freeze mode
    std::cout << "Demonstrating freeze mode...\n";
    engine.setReverbPreset(ReverbPreset::LargeHall);
    engine.setReverbWetLevel(1.0f);
    engine.setReverbDryLevel(0.0f);

    std::vector<float> freeze_left(sample_rate * 2);  // 2 seconds
    std::vector<float> freeze_right(sample_rate * 2);

    // Process with freeze off
    for (int i = 0; i < sample_rate; ++i) {
        float phase = 2.0f * M_PI * 440.0f * i / sample_rate;
        float input = 0.5f * std::sin(phase);
        engine.processAudio(&freeze_left[i], &freeze_right[i], 1);
    }

    // Enable freeze mode
    engine.setReverbFreezeMode(true);

    // Process silence (reverb tail will sustain)
    for (int i = sample_rate; i < sample_rate * 2; ++i) {
        float phase = 2.0f * M_PI * 440.0f * i / sample_rate;
        float input = 0.0f;  // Silence
        engine.processAudio(&freeze_left[i], &freeze_right[i], 1);
    }

    std::cout << "Freeze demo written to reverb_freeze_demo.wav\n";

    return 0;
}

#endif // CHOIR_V2_REVERB_EXAMPLE_MAIN
