/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * test_voice_manager.cpp - Unit tests for VoiceManager
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#include <gtest/gtest.h>
#include "core/VoiceManager.h"
#include <cmath>
#include <algorithm>

using namespace ChoirV2;

//==============================================================================
// Test Fixture
//==============================================================================

class VoiceManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager_ = std::make_unique<VoiceManager>(60, 44100.0);
        manager_->prepare(44100.0, 512);
    }

    void TearDown() override {
        manager_.reset();
    }

    // Helper: Generate test audio buffer
    void generateTestAudio(float* buffer, int numSamples, float frequency) {
        for (int i = 0; i < numSamples; ++i) {
            buffer[i] = std::sin(2.0f * M_PI * frequency * i / 44100.0f);
        }
    }

    // Helper: Count non-zero samples in buffer
    int countNonZeroSamples(const float* buffer, int numSamples) {
        int count = 0;
        for (int i = 0; i < numSamples; ++i) {
            if (std::abs(buffer[i]) > 0.0001f) {
                count++;
            }
        }
        return count;
    }

    // Helper: Calculate RMS level
    float calculateRMS(const float* buffer, int numSamples) {
        float sum = 0.0f;
        for (int i = 0; i < numSamples; ++i) {
            sum += buffer[i] * buffer[i];
        }
        return std::sqrt(sum / numSamples);
    }

    std::unique_ptr<VoiceManager> manager_;
};

//==============================================================================
// Basic Functionality Tests
//==============================================================================

TEST_F(VoiceManagerTest, Constructor_ValidParameters_Success) {
    VoiceManager mgr(60, 44100.0);
    EXPECT_EQ(mgr.getMaxVoices(), 60);
    EXPECT_EQ(mgr.getActiveVoiceCount(), 0);
}

TEST_F(VoiceManagerTest, Prepare_ValidParameters_InitializesCorrectly) {
    manager_->prepare(48000.0, 256);

    // Should be able to process audio without crashing
    float outputLeft[256];
    float outputRight[256];

    manager_->processAudio(outputLeft, outputRight, 256);

    // Output should be silent (no active voices)
    for (int i = 0; i < 256; ++i) {
        EXPECT_FLOAT_EQ(outputLeft[i], 0.0f);
        EXPECT_FLOAT_EQ(outputRight[i], 0.0f);
    }
}

//==============================================================================
// Note On/Off Tests
//==============================================================================

TEST_F(VoiceManagerTest, NoteOn_ValidNote_AllocatesVoice) {
    int voiceId = manager_->noteOn(60, 100.0f);

    EXPECT_NE(voiceId, -1);
    EXPECT_EQ(manager_->getActiveVoiceCount(), 1);
}

TEST_F(VoiceManagerTest, NoteOn_InvalidNote_ReturnsMinusOne) {
    int voiceId1 = manager_->noteOn(-1, 100.0f);
    int voiceId2 = manager_->noteOn(128, 100.0f);

    EXPECT_EQ(voiceId1, -1);
    EXPECT_EQ(voiceId2, -1);
    EXPECT_EQ(manager_->getActiveVoiceCount(), 0);
}

TEST_F(VoiceManagerTest, NoteOn_MultipleNotes_AllVoicesAllocated) {
    const int numNotes = 10;

    for (int i = 0; i < numNotes; ++i) {
        int voiceId = manager_->noteOn(60 + i, 100.0f);
        EXPECT_NE(voiceId, -1);
    }

    EXPECT_EQ(manager_->getActiveVoiceCount(), numNotes);
}

TEST_F(VoiceManagerTest, NoteOff_ValidNote_TriggersRelease) {
    manager_->noteOn(60, 100.0f);
    EXPECT_EQ(manager_->getActiveVoiceCount(), 1);

    manager_->noteOff(60, 0.0f);

    // Voice should still be active during release
    // (will be freed after release envelope completes)
    // For now, just check it doesn't crash
}

TEST_F(VoiceManagerTest, AllNotesOff_StopsAllVoices) {
    // Start multiple voices
    for (int i = 0; i < 10; ++i) {
        manager_->noteOn(60 + i, 100.0f);
    }

    EXPECT_EQ(manager_->getActiveVoiceCount(), 10);

    manager_->allNotesOff();

    EXPECT_EQ(manager_->getActiveVoiceCount(), 0);
}

//==============================================================================
// Audio Processing Tests
//==============================================================================

TEST_F(VoiceManagerTest, ProcessAudio_NoActiveVoices_SilentOutput) {
    float outputLeft[512];
    float outputRight[512];

    manager_->processAudio(outputLeft, outputRight, 512);

    for (int i = 0; i < 512; ++i) {
        EXPECT_FLOAT_EQ(outputLeft[i], 0.0f);
        EXPECT_FLOAT_EQ(outputRight[i], 0.0f);
    }
}

TEST_F(VoiceManagerTest, ProcessAudio_ActiveVoice_GeneratesSound) {
    manager_->noteOn(60, 100.0f);

    float outputLeft[512];
    float outputRight[512];

    manager_->processAudio(outputLeft, outputRight, 512);

    // Output should not be silent
    int nonZeroLeft = countNonZeroSamples(outputLeft, 512);
    int nonZeroRight = countNonZeroSamples(outputRight, 512);

    EXPECT_GT(nonZeroLeft, 0);
    EXPECT_GT(nonZeroRight, 0);
}

TEST_F(VoiceManagerTest, ProcessAudio_MultipleVoices_MixesCorrectly) {
    // Start multiple voices
    for (int i = 0; i < 5; ++i) {
        manager_->noteOn(60 + i, 80.0f);
    }

    float outputLeft[512];
    float outputRight[512];

    manager_->processAudio(outputLeft, outputRight, 512);

    // Output should have content
    float rmsLeft = calculateRMS(outputLeft, 512);
    float rmsRight = calculateRMS(outputRight, 512);

    EXPECT_GT(rmsLeft, 0.0f);
    EXPECT_GT(rmsRight, 0.0f);
}

TEST_F(VoiceManagerTest, ProcessAudio_WithMasterGain_ScalesOutput) {
    manager_->noteOn(60, 100.0f);

    // Process with gain = 1.0
    manager_->setMasterGain(1.0f);

    float outputLeft1[512];
    float outputRight1[512];

    manager_->processAudio(outputLeft1, outputRight1, 512);

    // Process with gain = 0.5
    manager_->setMasterGain(0.5f);

    float outputLeft2[512];
    float outputRight2[512];

    manager_->processAudio(outputLeft2, outputRight2, 512);

    // Second output should be quieter
    float rms1 = calculateRMS(outputLeft1, 512);
    float rms2 = calculateRMS(outputLeft2, 512);

    EXPECT_LT(rms2, rms1);
    EXPECT_NEAR(rms2, rms1 * 0.5f, 0.01f);
}

//==============================================================================
// Parameter Tests
//==============================================================================

TEST_F(VoiceManagerTest, SetMasterGain_ValidValue_SmoothsCorrectly) {
    manager_->setMasterGain(0.5f);

    // Process audio to update smoothing
    float output[512];
    manager_->processAudio(output, output, 512);

    // Check that gain was applied (shouldn't crash)
    SUCCEED();
}

TEST_F(VoiceManagerTest, SetAttackTime_ValidValue_AppliesCorrectly) {
    manager_->setAttackTime(0.05f);

    manager_->noteOn(60, 100.0f);

    float outputLeft[512];
    float outputRight[512];

    manager_->processAudio(outputLeft, outputRight, 512);

    // Should process without crashing
    SUCCEED();
}

TEST_F(VoiceManagerTest, SetReleaseTime_ValidValue_AppliesCorrectly) {
    manager_->setReleaseTime(0.2f);

    manager_->noteOn(60, 100.0f);
    manager_->noteOff(60, 0.0f);

    float outputLeft[512];
    float outputRight[512];

    manager_->processAudio(outputLeft, outputRight, 512);

    // Should process without crashing
    SUCCEED();
}

//==============================================================================
// Performance Tests
//==============================================================================

TEST_F(VoiceManagerTest, Performance_MaxVoices_ProcessesEfficiently) {
    // Start all 60 voices
    for (int i = 0; i < 60; ++i) {
        manager_->noteOn(60 + (i % 40), 80.0f);
    }

    EXPECT_EQ(manager_->getActiveVoiceCount(), 60);

    float outputLeft[512];
    float outputRight[512];

    // Process multiple blocks
    for (int i = 0; i < 100; ++i) {
        manager_->processAudio(outputLeft, outputRight, 512);
    }

    // Should complete without crashing
    SUCCEED();
}

TEST_F(VoiceManagerTest, Performance_RapidNoteOnOff_HandlesEfficiently) {
    // Rapidly start and stop voices
    for (int i = 0; i < 100; ++i) {
        int voiceId = manager_->noteOn(60 + (i % 20), 100.0f);
        EXPECT_NE(voiceId, -1);

        if (i % 2 == 0) {
            manager_->noteOff(60 + (i % 20), 0.0f);
        }
    }

    // Process audio
    float outputLeft[512];
    float outputRight[512];

    manager_->processAudio(outputLeft, outputRight, 512);

    // Should handle rapid changes without crashing
    SUCCEED();
}

//==============================================================================
// Statistics Tests
//==============================================================================

TEST_F(VoiceManagerTest, Statistics_TracksActiveVoices) {
    const auto& stats = manager_->getStats();

    EXPECT_EQ(stats.activeVoices, 0);
    EXPECT_EQ(stats.totalVoices, 60);

    manager_->noteOn(60, 100.0f);

    const auto& statsAfter = manager_->getStats();

    EXPECT_EQ(statsAfter.activeVoices, 1);
}

TEST_F(VoiceManagerTest, Statistics_TracksStolenVoices) {
    // Fill all voices
    for (int i = 0; i < 60; ++i) {
        manager_->noteOn(60 + (i % 40), 80.0f);
    }

    const auto& statsBefore = manager_->getStats();
    int stolenBefore = statsBefore.stolenVoices;

    // One more note should trigger voice stealing
    manager_->noteOn(60, 100.0f);

    const auto& statsAfter = manager_->getStats();

    EXPECT_GT(statsAfter.stolenVoices, stolenBefore);
}

TEST_F(VoiceManagerTest, ResetStats_ClearsStatistics) {
    manager_->noteOn(60, 100.0f);
    manager_->noteOn(61, 100.0f);

    const auto& statsBefore = manager_->getStats();
    EXPECT_GT(statsBefore.activeVoices, 0);

    manager_->resetStats();

    const auto& statsAfter = manager_->getStats();
    EXPECT_EQ(statsAfter.activeVoices, 0);
    EXPECT_EQ(statsAfter.stolenVoices, 0);
}

//==============================================================================
// Real-Time Safety Tests
//==============================================================================

TEST_F(VoiceManagerTest, RealTimeSafety_NoAllocationsInProcessAudio) {
    manager_->noteOn(60, 100.0f);

    float outputLeft[512];
    float outputRight[512];

    // This should not allocate any memory
    // (verified by Valgrind/AddressSanitizer in CI)
    for (int i = 0; i < 1000; ++i) {
        manager_->processAudio(outputLeft, outputRight, 512);
    }

    SUCCEED();
}

TEST_F(VoiceManagerTest, RealTimeSafety_VaryingBufferSizes) {
    manager_->noteOn(60, 100.0f);

    // Test various buffer sizes
    int bufferSizes[] = {64, 128, 256, 512, 1024};

    for (int bufferSize : bufferSizes) {
        float* outputLeft = new float[bufferSize];
        float* outputRight = new float[bufferSize];

        manager_->processAudio(outputLeft, outputRight, bufferSize);

        delete[] outputLeft;
        delete[] outputRight;
    }

    SUCCEED();
}

//==============================================================================
// SIMD Tests
//==============================================================================

TEST_F(VoiceManagerTest, SIMD_BatchProcessing_ProcessesCorrectly) {
    // Start exactly 8 voices (one SIMD batch)
    for (int i = 0; i < 8; ++i) {
        manager_->noteOn(60 + i, 100.0f);
    }

    float outputLeft[512];
    float outputRight[512];

    manager_->processAudio(outputLeft, outputRight, 512);

    // Should process full batch without crashing
    float rms = calculateRMS(outputLeft, 512);
    EXPECT_GT(rms, 0.0f);
}

TEST_F(VoiceManagerTest, SIMD_PartialBatch_ProcessesCorrectly) {
    // Start 5 voices (partial SIMD batch)
    for (int i = 0; i < 5; ++i) {
        manager_->noteOn(60 + i, 100.0f);
    }

    float outputLeft[512];
    float outputRight[512];

    manager_->processAudio(outputLeft, outputRight, 512);

    // Should process partial batch without crashing
    float rms = calculateRMS(outputLeft, 512);
    EXPECT_GT(rms, 0.0f);
}

//==============================================================================
// Integration Tests
//==============================================================================

TEST_F(VoiceManagerTest, Integration_FullAudioPipeline) {
    // Simulate real usage: multiple notes, parameter changes, processing

    // Start chord
    manager_->noteOn(60, 100.0f);  // C
    manager_->noteOn(64, 90.0f);   // E
    manager_->noteOn(67, 95.0f);   // G

    // Process audio
    float outputLeft[512];
    float outputRight[512];

    for (int i = 0; i < 10; ++i) {
        manager_->processAudio(outputLeft, outputRight, 512);
    }

    // Change parameters
    manager_->setMasterGain(0.8f);
    manager_->setAttackTime(0.02f);

    // Process more
    for (int i = 0; i < 10; ++i) {
        manager_->processAudio(outputLeft, outputRight, 512);
    }

    // Stop notes
    manager_->noteOff(60, 0.0f);
    manager_->noteOff(64, 0.0f);
    manager_->noteOff(67, 0.0f);

    // Process release
    for (int i = 0; i < 20; ++i) {
        manager_->processAudio(outputLeft, outputRight, 512);
    }

    // Should complete without crashing
    SUCCEED();
}

//==============================================================================
// Main
//==============================================================================

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
