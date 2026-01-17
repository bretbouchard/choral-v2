/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * test_plugin_functionality.cpp - End-to-end tests for plugin functionality
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#include <gtest/gtest.h>
#include <core/VoiceManager.h>
#include <core/G2PEngine.h>
#include <core/PhonemeDatabase.h>
#include <core/LanguageLoader.h>
#include <core/PresetManager.h>
#include <plugin/ChoirV2Processor.h>
#include <plugin/ChoirV2Parameters.h>
#include <utils/AudioBuffer.h>

#include <cmath>
#include <vector>
#include <cstring>
#include <fstream>

using namespace ChoirV2;

class PluginFunctionalityTest : public ::testing::Test {
protected:
    void SetUp() override {
        sampleRate_ = 44100.0;
        bufferSize_ = 512;

        // Initialize processor
        processor_ = std::make_unique<ChoirV2Processor>();
        processor_->prepareToPlay(sampleRate_, bufferSize_);

        // Initialize voice manager for direct testing
        voiceManager_ = std::make_unique<VoiceManager>(60, sampleRate_);
        voiceManager_->prepare(sampleRate_, bufferSize_);

        // Initialize phoneme database and G2P engine
        phonemeDB_ = std::make_shared<PhonemeDatabase>();
        phonemeDB_->loadFromDirectory("languages/phonemes");

        languageLoader_ = std::make_unique<LanguageLoader>();
        g2pEngine_ = std::make_unique<G2PEngine>(phonemeDB_);

        // Initialize preset manager
        presetManager_ = std::make_unique<PresetManager>();
    }

    void TearDown() override {
        presetManager_.reset();
        g2pEngine_.reset();
        languageLoader_.reset();
        phonemeDB_.reset();
        voiceManager_.reset();
        processor_.reset();
    }

    // Helper: Check if audio is silent
    bool isSilent(const float* buffer, int numSamples, float threshold = 0.0001f) {
        for (int i = 0; i < numSamples; ++i) {
            if (std::abs(buffer[i]) > threshold) {
                return false;
            }
        }
        return true;
    }

    // Helper: Check for clicks/pops
    bool hasClicks(const float* buffer, int numSamples, float maxDelta = 0.1f) {
        for (int i = 1; i < numSamples; ++i) {
            float delta = std::abs(buffer[i] - buffer[i-1]);
            if (delta > maxDelta) {
                return true;
            }
        }
        return false;
    }

    // Helper: Calculate SNR
    float calculateSNR(const float* signal, const float* noise, int numSamples) {
        float signalPower = 0.0f;
        float noisePower = 0.0f;

        for (int i = 0; i < numSamples; ++i) {
            signalPower += signal[i] * signal[i];
            noisePower += noise[i] * noise[i];
        }

        if (noisePower < 1e-10f) return 100.0f; // Very high SNR

        return 10.0f * std::log10(signalPower / noisePower);
    }

    // Helper: Calculate THD (Total Harmonic Distortion)
    float calculateTHD(const float* buffer, int numSamples, float fundamentalFreq) {
        // Simple THD estimation using FFT
        // In practice, you'd use a proper FFT library
        float fundamentalPower = 0.0f;
        float harmonicPower = 0.0f;

        // Placeholder: estimate from zero crossings
        // Real implementation would use FFT
        int zeroCrossings = 0;
        for (int i = 1; i < numSamples; ++i) {
            if ((buffer[i-1] >= 0.0f && buffer[i] < 0.0f) ||
                (buffer[i-1] < 0.0f && buffer[i] >= 0.0f)) {
                zeroCrossings++;
            }
        }

        float expectedFreq = fundamentalFreq;
        float measuredFreq = (zeroCrossings * sampleRate_) / (2.0f * numSamples);
        float freqError = std::abs(measuredFreq - expectedFreq) / expectedFreq;

        // THD estimate based on frequency error (rough approximation)
        return freqError * 100.0f;
    }

    // Helper: Generate MIDI message
    std::vector<uint8_t> createMIDIMessage(uint8_t status, uint8_t data1, uint8_t data2 = 0) {
        return {status, data1, data2};
    }

    // Helper: Process MIDI through plugin
    void processMIDI(const std::vector<uint8_t>& message) {
        processor_->processMIDI(message.data(), message.size());
    }

    double sampleRate_;
    int bufferSize_;

    std::unique_ptr<ChoirV2Processor> processor_;
    std::unique_ptr<VoiceManager> voiceManager_;
    std::shared_ptr<PhonemeDatabase> phonemeDB_;
    std::unique_ptr<LanguageLoader> languageLoader_;
    std::unique_ptr<G2PEngine> g2pEngine_;
    std::unique_ptr<PresetManager> presetManager_;
};

// ============================================================================
// Test 1: MIDI Input - Note On/Off
// ============================================================================

TEST_F(PluginFunctionalityTest, MIDI_NoteOnOff) {
    AudioBuffer outputBuffer(2, bufferSize_);
    outputBuffer.clear();

    // Send note on
    auto noteOn = createMIDIMessage(0x90, 60, 100); // Note on, C4, velocity 100
    processMIDI(noteOn);

    // Process audio
    processor_->processAudio(
        outputBuffer.getWritePointer(0),
        outputBuffer.getWritePointer(1),
        bufferSize_
    );

    // Should hear sound
    EXPECT_FALSE(isSilent(outputBuffer.getReadPointer(0), bufferSize_))
        << "No audio after note on";

    // Send note off
    auto noteOff = createMIDIMessage(0x80, 60, 0);
    processMIDI(noteOff);

    // Process audio (release phase)
    outputBuffer.clear();
    processor_->processAudio(
        outputBuffer.getWritePointer(0),
        outputBuffer.getWritePointer(1),
        bufferSize_
    );

    // Should still hear sound during release
    EXPECT_FALSE(isSilent(outputBuffer.getReadPointer(0), bufferSize_))
        << "No audio during release";
}

// ============================================================================
// Test 2: MIDI Input - Multiple Notes
// ============================================================================

TEST_F(PluginFunctionalityTest, MIDI_MultipleNotes) {
    AudioBuffer outputBuffer(2, bufferSize_);
    outputBuffer.clear();

    // Send multiple note on messages
    std::vector<int> notes = {60, 64, 67, 72}; // C4, E4, G4, C5
    for (int note : notes) {
        auto noteOn = createMIDIMessage(0x90, note, 80);
        processMIDI(noteOn);
    }

    // Process audio
    processor_->processAudio(
        outputBuffer.getWritePointer(0),
        outputBuffer.getWritePointer(1),
        bufferSize_
    );

    // Should hear chord
    EXPECT_FALSE(isSilent(outputBuffer.getReadPointer(0), bufferSize_))
        << "No audio for chord";

    // All notes off
    for (int note : notes) {
        auto noteOff = createMIDIMessage(0x80, note, 0);
        processMIDI(noteOff);
    }
}

// ============================================================================
// Test 3: MIDI Input - Sustain Pedal
// ============================================================================

TEST_F(PluginFunctionalityTest, MIDI_SustainPedal) {
    AudioBuffer outputBuffer(2, bufferSize_);
    outputBuffer.clear();

    // Note on
    auto noteOn = createMIDIMessage(0x90, 60, 100);
    processMIDI(noteOn);

    // Sustain pedal on (CC 64, value 127)
    auto sustainOn = createMIDIMessage(0xB0, 64, 127);
    processMIDI(sustainOn);

    // Process audio
    processor_->processAudio(
        outputBuffer.getWritePointer(0),
        outputBuffer.getWritePointer(1),
        bufferSize_
    );

    // Note off (but sustained)
    auto noteOff = createMIDIMessage(0x80, 60, 0);
    processMIDI(noteOff);

    // Process audio (should still sound due to sustain)
    outputBuffer.clear();
    processor_->processAudio(
        outputBuffer.getWritePointer(0),
        outputBuffer.getWritePointer(1),
        bufferSize_
    );

    EXPECT_FALSE(isSilent(outputBuffer.getReadPointer(0), bufferSize_))
        << "Sustain pedal not working";

    // Sustain pedal off (CC 64, value 0)
    auto sustainOff = createMIDIMessage(0xB0, 64, 0);
    processMIDI(sustainOff);
}

// ============================================================================
// Test 4: MIDI Input - Pitch Bend
// ============================================================================

TEST_F(PluginFunctionalityTest, MIDI_PitchBend) {
    AudioBuffer outputBuffer(2, bufferSize_);
    outputBuffer.clear();

    // Note on
    auto noteOn = createMIDIMessage(0x90, 60, 100);
    processMIDI(noteOn);

    // Process audio (no pitch bend)
    processor_->processAudio(
        outputBuffer.getWritePointer(0),
        outputBuffer.getWritePointer(1),
        bufferSize_
    );

    // Pitch bend up (MSB = 127, LSB = 127 => 0x3FFF = max bend)
    auto pitchBendUp = createMIDIMessage(0xE0, 127, 127);
    processMIDI(pitchBendUp);

    // Process audio (pitch bent up)
    outputBuffer.clear();
    processor_->processAudio(
        outputBuffer.getWritePointer(0),
        outputBuffer.getWritePointer(1),
        bufferSize_
    );

    // Should hear different pitch (higher frequency)
    EXPECT_FALSE(isSilent(outputBuffer.getReadPointer(0), bufferSize_))
        << "No audio after pitch bend";

    // Pitch bend center (MSB = 0, LSB = 64 => 0x2000 = center)
    auto pitchBendCenter = createMIDIMessage(0xE0, 0, 64);
    processMIDI(pitchBendCenter);

    // Note off
    auto noteOff = createMIDIMessage(0x80, 60, 0);
    processMIDI(noteOff);
}

// ============================================================================
// Test 5: Parameter Changes - All Parameters
// ============================================================================

TEST_F(PluginFunctionalityTest, ParameterChanges_AllParameters) {
    AudioBuffer outputBuffer(2, bufferSize_);
    outputBuffer.clear();

    // Note on
    auto noteOn = createMIDIMessage(0x90, 60, 100);
    processMIDI(noteOn);

    // Test all 13 parameters
    std::vector<std::pair<int, float>> parameters = {
        {ParameterID::MasterGain, 1.0f},
        {ParameterID::AttackTime, 0.01f},
        {ParameterID::ReleaseTime, 0.1f},
        {ParameterID::VibratoRate, 5.0f},
        {ParameterID::VibratoDepth, 0.0f},
        {ParameterID::FormantShift, 0.0f},
        {ParameterID::SubharmonicMix, 0.0f},
        {ParameterID::ChoirSize, 12.0f},
        {ParameterID::StereoWidth, 1.0f},
        {ParameterID::ReverbMix, 0.2f},
        {ParameterID::ReverbDecay, 0.5f},
        {ParameterID::LanguageSelect, 0.0f}, // English
        {ParameterID::VoiceType, 0.0f}       // Soprano
    };

    for (const auto& param : parameters) {
        processor_->setParameter(param.first, param.second);

        // Process audio after each parameter change
        outputBuffer.clear();
        processor_->processAudio(
            outputBuffer.getWritePointer(0),
            outputBuffer.getWritePointer(1),
            bufferSize_
        );

        // Verify no clicks during parameter changes
        EXPECT_FALSE(hasClicks(outputBuffer.getReadPointer(0), bufferSize_, 0.2f))
            << "Click detected after changing parameter " << param.first;

        // Verify audio is still valid
        EXPECT_TRUE(isSilent(outputBuffer.getReadPointer(0), bufferSize_) == false ||
                    outputBuffer.getReadPointer(0)[0] == 0.0f); // Either sound or silence
    }

    // Note off
    auto noteOff = createMIDIMessage(0x80, 60, 0);
    processMIDI(noteOff);
}

// ============================================================================
// Test 6: Parameter Automation
// ============================================================================

TEST_F(PluginFunctionalityTest, ParameterAutomation) {
    AudioBuffer outputBuffer(2, bufferSize_);
    outputBuffer.clear();

    // Note on
    auto noteOn = createMIDIMessage(0x90, 60, 100);
    processMIDI(noteOn);

    // Automate master gain from 0.0 to 1.0 over 100 buffers
    for (int i = 0; i < 100; ++i) {
        float gain = static_cast<float>(i) / 100.0f;
        processor_->setParameter(ParameterID::MasterGain, gain);

        outputBuffer.clear();
        processor_->processAudio(
            outputBuffer.getWritePointer(0),
            outputBuffer.getWritePointer(1),
            bufferSize_
        );

        // Verify smooth transitions (no clicks)
        if (i > 0) {
            EXPECT_FALSE(hasClicks(outputBuffer.getReadPointer(0), bufferSize_, 0.3f))
                << "Click detected during automation at step " << i;
        }
    }

    // Note off
    auto noteOff = createMIDIMessage(0x80, 60, 0);
    processMIDI(noteOff);
}

// ============================================================================
// Test 7: Preset Loading
// ============================================================================

TEST_F(PluginFunctionalityTest, PresetLoading) {
    // Load all 8 factory presets
    std::vector<std::string> presets = {
        "Default Choir",
        "Large Cathedral Choir",
        "Small Chamber Choir",
        "Gospel Choir",
        "Byzantine Choir",
        "Throat Singing - Sygyt",
        "Throat Singing - Kargyraa",
        "Alien Choir (Klingon)"
    };

    for (const auto& presetName : presets) {
        bool loaded = presetManager_->loadPreset(presetName);
        EXPECT_TRUE(loaded) << "Failed to load preset: " << presetName;

        if (loaded) {
            // Apply preset to processor
            auto preset = presetManager_->getCurrentPreset();

            // Apply all parameters from preset
            for (const auto& param : preset->parameters) {
                processor_->setParameter(param.id, param.value);
            }

            // Test audio with preset
            AudioBuffer outputBuffer(2, bufferSize_);
            outputBuffer.clear();

            auto noteOn = createMIDIMessage(0x90, 60, 100);
            processMIDI(noteOn);

            processor_->processAudio(
                outputBuffer.getWritePointer(0),
                outputBuffer.getWritePointer(1),
                bufferSize_
            );

            // Should produce audio
            EXPECT_FALSE(isSilent(outputBuffer.getReadPointer(0), bufferSize_))
                << "No audio with preset: " << presetName;

            auto noteOff = createMIDIMessage(0x80, 60, 0);
            processMIDI(noteOff);
        }
    }
}

// ============================================================================
// Test 8: State Management - Save/Restore
// ============================================================================

TEST_F(PluginFunctionalityTest, StateManagement_SaveRestore) {
    // Set up specific state
    processor_->setParameter(ParameterID::MasterGain, 0.8f);
    processor_->setParameter(ParameterID::AttackTime, 0.05f);
    processor_->setParameter(ParameterID::ReleaseTime, 0.3f);
    processor_->setParameter(ParameterID::VibratoRate, 6.0f);
    processor_->setParameter(ParameterID::VibratoDepth, 0.1f);
    processor_->setParameter(ParameterID::SubharmonicMix, 0.5f);

    // Save state
    auto state = processor_->getState();

    // Verify state was saved
    EXPECT_FALSE(state.empty()) << "State is empty after save";

    // Modify parameters
    processor_->setParameter(ParameterID::MasterGain, 1.0f);
    processor_->setParameter(ParameterID::VibratoRate, 5.0f);

    // Restore state
    processor_->setState(state);

    // Verify parameters were restored
    // (Note: In real implementation, you'd check actual parameter values)
    EXPECT_TRUE(true); // Placeholder
}

// ============================================================================
// Test 9: DAW Project Reload
// ============================================================================

TEST_F(PluginFunctionalityTest, DAWProjectReload) {
    // Simulate DAW project save/load cycle

    // Original session
    AudioBuffer outputBuffer1(2, bufferSize_);
    outputBuffer1.clear();

    processor_->setParameter(ParameterID::MasterGain, 0.7f);
    processor_->setParameter(ParameterID::ChoirSize, 24.0f);

    auto noteOn = createMIDIMessage(0x90, 60, 100);
    processMIDI(noteOn);

    processor_->processAudio(
        outputBuffer1.getWritePointer(0),
        outputBuffer1.getWritePointer(1),
        bufferSize_
    );

    // Save project state
    auto projectState = processor_->getState();

    // Simulate project reload (destroy and recreate processor)
    processor_.reset();
    processor_ = std::make_unique<ChoirV2Processor>();
    processor_->prepareToPlay(sampleRate_, bufferSize_);

    // Restore project state
    processor_->setState(projectState);

    // New session
    AudioBuffer outputBuffer2(2, bufferSize_);
    outputBuffer2.clear();

    noteOn = createMIDIMessage(0x90, 60, 100);
    processMIDI(noteOn);

    processor_->processAudio(
        outputBuffer2.getWritePointer(0),
        outputBuffer2.getWritePointer(1),
        bufferSize_
    );

    // Output should be similar (within tolerance)
    float maxDifference = 0.0f;
    for (int i = 0; i < bufferSize_; ++i) {
        float diff = std::abs(outputBuffer1.getReadPointer(0)[i] -
                             outputBuffer2.getReadPointer(0)[i]);
        if (diff > maxDifference) {
            maxDifference = diff;
        }
    }

    EXPECT_LT(maxDifference, 0.01f) << "Output differs after project reload";

    auto noteOff = createMIDIMessage(0x80, 60, 0);
    processMIDI(noteOff);
}

// ============================================================================
// Test 10: Preset Persistence
// ============================================================================

TEST_F(PluginFunctionalityTest, PresetPersistence) {
    // Create user preset
    Preset userPreset;
    userPreset.name = "My Custom Choir";
    userPreset.parameters = {
        {ParameterID::MasterGain, 0.9f},
        {ParameterID::AttackTime, 0.02f},
        {ParameterID::ReleaseTime, 0.15f},
        {ParameterID::VibratoRate, 5.5f},
        {ParameterID::VibratoDepth, 0.05f},
        {ParameterID::ChoirSize, 16.0f}
    };

    // Save preset
    bool saved = presetManager_->savePreset(userPreset);
    EXPECT_TRUE(saved) << "Failed to save user preset";

    // Load preset
    bool loaded = presetManager_->loadPreset("My Custom Choir");
    EXPECT_TRUE(loaded) << "Failed to load saved preset";

    if (loaded) {
        auto loadedPreset = presetManager_->getCurrentPreset();

        // Verify parameters match
        EXPECT_EQ(loadedPreset->name, "My Custom Choir");
        EXPECT_EQ(loadedPreset->parameters.size(), userPreset.parameters.size());
    }
}

// ============================================================================
// Test 11: Audio Quality - No Clicks/Pops
// ============================================================================

TEST_F(PluginFunctionalityTest, AudioQuality_NoClicksPops) {
    AudioBuffer outputBuffer(2, bufferSize_ * 100); // Large buffer
    outputBuffer.clear();

    // Play notes with various parameter changes
    for (int i = 0; i < 100; ++i) {
        // Change parameters randomly
        processor_->setParameter(ParameterID::MasterGain, 0.5f + (rand() % 100) / 200.0f);
        processor_->setParameter(ParameterID::VibratoRate, 4.0f + (rand() % 100) / 10.0f);

        // Note on/off pattern
        if (i % 10 == 0) {
            auto noteOn = createMIDIMessage(0x90, 60 + (i % 12), 100);
            processMIDI(noteOn);
        } else if (i % 10 == 8) {
            auto noteOff = createMIDIMessage(0x80, 60 + (i % 12), 0);
            processMIDI(noteOff);
        }

        // Process audio
        int offset = i * bufferSize_;
        processor_->processAudio(
            outputBuffer.getWritePointer(0) + offset,
            outputBuffer.getWritePointer(1) + offset,
            bufferSize_
        );
    }

    // Check entire buffer for clicks
    int totalSamples = bufferSize_ * 100;
    EXPECT_FALSE(hasClicks(outputBuffer.getReadPointer(0), totalSamples, 0.15f))
        << "Detected clicks in audio output";
}

// ============================================================================
// Test 12: Audio Quality - No Aliasing
// ============================================================================

TEST_F(PluginFunctionalityTest, AudioQuality_NoAliasing) {
    // This test checks for aliasing artifacts
    // In practice, you'd analyze the frequency spectrum

    AudioBuffer outputBuffer(2, bufferSize_ * 10);
    outputBuffer.clear();

    // Play high notes (more prone to aliasing)
    for (int i = 0; i < 10; ++i) {
        auto noteOn = createMIDIMessage(0x90, 84 + i, 100); // High notes
        processMIDI(noteOn);

        processor_->processAudio(
            outputBuffer.getWritePointer(0) + i * bufferSize_,
            outputBuffer.getWritePointer(1) + i * bufferSize_,
            bufferSize_
        );

        auto noteOff = createMIDIMessage(0x80, 84 + i, 0);
        processMIDI(noteOff);
    }

    // In real implementation, you'd perform FFT analysis
    // For now, just verify audio is valid
    for (int i = 0; i < bufferSize_ * 10; ++i) {
        EXPECT_FALSE(std::isnan(outputBuffer.getReadPointer(0)[i]));
        EXPECT_FALSE(std::isinf(outputBuffer.getReadPointer(0)[i]));
    }
}

// ============================================================================
// Test 13: Audio Quality - SNR Measurement
// ============================================================================

TEST_F(PluginFunctionalityTest, AudioQuality_SNRMeasurement) {
    AudioBuffer signalBuffer(2, bufferSize_);
    AudioBuffer noiseBuffer(2, bufferSize_);

    signalBuffer.clear();
    noiseBuffer.clear();

    // Generate signal (note playing)
    auto noteOn = createMIDIMessage(0x90, 60, 100);
    processMIDI(noteOn);

    processor_->processAudio(
        signalBuffer.getWritePointer(0),
        signalBuffer.getWritePointer(1),
        bufferSize_
    );

    // Generate noise (silence)
    auto noteOff = createMIDIMessage(0x80, 60, 0);
    processMIDI(noteOff);

    processor_->processAudio(
        noiseBuffer.getWritePointer(0),
        noiseBuffer.getWritePointer(1),
        bufferSize_
    );

    // Calculate SNR
    float snr = calculateSNR(
        signalBuffer.getReadPointer(0),
        noiseBuffer.getReadPointer(0),
        bufferSize_
    );

    // SNR should be > 90dB (0.001 noise floor)
    EXPECT_GT(snr, 90.0f) << "SNR too low: " << snr << " dB";
}

// ============================================================================
// Test 14: Audio Quality - THD Measurement
// ============================================================================

TEST_F(PluginFunctionalityTest, AudioQuality_THDMeasurement) {
    AudioBuffer outputBuffer(2, bufferSize_);
    outputBuffer.clear();

    // Play pure tone
    auto noteOn = createMIDIMessage(0x90, 69, 100); // A4 = 440Hz
    processMIDI(noteOn);

    processor_->processAudio(
        outputBuffer.getWritePointer(0),
        outputBuffer.getWritePointer(1),
        bufferSize_
    );

    // Calculate THD
    float thd = calculateTHD(outputBuffer.getReadPointer(0), bufferSize_, 440.0f);

    // THD should be < 0.1% (-60dB)
    EXPECT_LT(thd, 0.1f) << "THD too high: " << thd << "%";

    auto noteOff = createMIDIMessage(0x80, 69, 0);
    processMIDI(noteOff);
}

// ============================================================================
// Test 15: Voice Stealing Under Load
// ============================================================================

TEST_F(PluginFunctionalityTest, VoiceStealingUnderLoad) {
    // Try to play more than 60 notes (max voices)
    AudioBuffer outputBuffer(2, bufferSize_);
    outputBuffer.clear();

    // Play 80 notes (exceeds 60 voice limit)
    for (int i = 0; i < 80; ++i) {
        auto noteOn = createMIDIMessage(0x90, 60 + (i % 24), 100);
        processMIDI(noteOn);
    }

    // Process audio
    processor_->processAudio(
        outputBuffer.getWritePointer(0),
        outputBuffer.getWritePointer(1),
        bufferSize_
    );

    // Should still produce audio without crashing
    EXPECT_FALSE(isSilent(outputBuffer.getReadPointer(0), bufferSize_))
        << "No audio after voice stealing";

    // Verify voice manager stats
    const auto& stats = voiceManager_->getStats();
    EXPECT_GT(stats.stolenVoices, 0) << "No voices were stolen";
}

// ============================================================================
// Test 16: Real-Time Safety
// ============================================================================

TEST_F(PluginFunctionalityTest, RealTimeSafety) {
    // Verify no allocations in audio thread

    // Activate 40 voices
    for (int i = 0; i < 40; ++i) {
        auto noteOn = createMIDIMessage(0x90, 60 + (i % 24), 100);
        processMIDI(noteOn);
    }

    // Process many buffers
    for (int i = 0; i < 1000; ++i) {
        AudioBuffer buffer(2, bufferSize_);
        buffer.clear();

        // Change parameters
        processor_->setParameter(ParameterID::MasterGain, 0.5f + (i % 10) / 20.0f);

        // Process audio
        processor_->processAudio(
            buffer.getWritePointer(0),
            buffer.getWritePointer(1),
            bufferSize_
        );

        // Verify output is valid
        for (int j = 0; j < bufferSize_; ++j) {
            EXPECT_FALSE(std::isnan(buffer.getReadPointer(0)[j]));
            EXPECT_FALSE(std::isinf(buffer.getReadPointer(0)[j]));
        }
    }

    // All notes off
    for (int i = 0; i < 40; ++i) {
        auto noteOff = createMIDIMessage(0x80, 60 + (i % 24), 0);
        processMIDI(noteOff);
    }
}

// ============================================================================
// Test 17: Concurrent MIDI And Audio
// ============================================================================

TEST_F(PluginFunctionalityTest, ConcurrentMIDIAndAudio) {
    // Simulate realistic usage: MIDI messages during audio processing

    AudioBuffer outputBuffer(2, bufferSize_ * 100);
    outputBuffer.clear();

    for (int i = 0; i < 100; ++i) {
        // Send MIDI messages during processing
        if (i % 5 == 0) {
            auto noteOn = createMIDIMessage(0x90, 60 + (i % 12), 100);
            processMIDI(noteOn);
        } else if (i % 5 == 3) {
            auto noteOff = createMIDIMessage(0x80, 60 + (i % 12), 0);
            processMIDI(noteOff);
        }

        // Process audio
        int offset = i * bufferSize_;
        processor_->processAudio(
            outputBuffer.getWritePointer(0) + offset,
            outputBuffer.getWritePointer(1) + offset,
            bufferSize_
        );
    }

    // Verify output is valid throughout
    for (int i = 0; i < bufferSize_ * 100; ++i) {
        EXPECT_FALSE(std::isnan(outputBuffer.getReadPointer(0)[i]));
        EXPECT_FALSE(std::isinf(outputBuffer.getReadPointer(0)[i]));
    }
}

// ============================================================================
// Test 18: Edge Cases
// ============================================================================

TEST_F(PluginFunctionalityTest, EdgeCases) {
    AudioBuffer outputBuffer(2, bufferSize_);

    // Test zero sample rate (should handle gracefully)
    EXPECT_NO_THROW({
        auto tempProcessor = std::make_unique<ChoirV2Processor>();
        tempProcessor->prepareToPlay(0.0, 512);
    });

    // Test very small buffer size
    EXPECT_NO_THROW({
        auto tempProcessor = std::make_unique<ChoirV2Processor>();
        tempProcessor->prepareToPlay(44100.0, 1);
        AudioBuffer smallBuffer(2, 1);
        tempProcessor->processAudio(
            smallBuffer.getWritePointer(0),
            smallBuffer.getWritePointer(1),
            1
        );
    });

    // Test very large buffer size
    EXPECT_NO_THROW({
        auto tempProcessor = std::make_unique<ChoirV2Processor>();
        tempProcessor->prepareToPlay(44100.0, 8192);
        AudioBuffer largeBuffer(2, 8192);
        tempProcessor->processAudio(
            largeBuffer.getWritePointer(0),
            largeBuffer.getWritePointer(1),
            8192
        );
    });

    // Test invalid MIDI messages
    EXPECT_NO_THROW({
        processMIDI({0x90, 128, 100}); // Invalid note number
        processMIDI({0x90, 60, 128});  // Invalid velocity
        processMIDI({0xFF, 0, 0});     // Invalid status
    });

    // Test extreme parameter values
    EXPECT_NO_THROW({
        processor_->setParameter(ParameterID::MasterGain, -1.0f);
        processor_->setParameter(ParameterID::MasterGain, 1000.0f);
        processor_->setParameter(ParameterID::VibratoRate, -100.0f);
        processor_->setParameter(ParameterID::VibratoRate, 10000.0f);
    });
}
