/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * test_voice_allocator.cpp - Unit tests for VoiceAllocator
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#include <gtest/gtest.h>
#include "core/VoiceAllocator.h"

using namespace ChoirV2;

class VoiceAllocatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create allocator with 10 voices for testing
        allocator_ = std::make_unique<VoiceAllocator>(10);
    }

    void TearDown() override {
        allocator_.reset();
    }

    std::unique_ptr<VoiceAllocator> allocator_;
};

/**
 * Test: Voice allocation when voices are available
 */
TEST_F(VoiceAllocatorTest, AllocateVoice_WhenVoicesAvailable_Success) {
    // Allocate first voice
    auto result = allocator_->allocateVoice(60, 100.0f);

    EXPECT_TRUE(result.success);
    EXPECT_GE(result.voiceId, 0);
    EXPECT_LT(result.voiceId, 10);  // maxVoices = 10
    EXPECT_FALSE(result.stolen);  // Should not steal when voices available
    EXPECT_EQ(result.stolenFromId, -1);

    // Verify voice is active
    VoiceRecord* voice = allocator_->getVoice(result.voiceId);
    ASSERT_NE(voice, nullptr);
    EXPECT_TRUE(voice->active);
    EXPECT_EQ(voice->midiNote, 60);
    EXPECT_FLOAT_EQ(voice->velocity, 100.0f);
    EXPECT_FLOAT_EQ(voice->amplitude, 100.0f / 127.0f);
}

/**
 * Test: Voice allocation with invalid MIDI note
 */
TEST_F(VoiceAllocatorTest, AllocateVoice_InvalidMidiNote_Failure) {
    // Test MIDI note too low
    auto result1 = allocator_->allocateVoice(-1, 100.0f);
    EXPECT_FALSE(result1.success);

    // Test MIDI note too high
    auto result2 = allocator_->allocateVoice(128, 100.0f);
    EXPECT_FALSE(result2.success);
}

/**
 * Test: Voice allocation with invalid velocity
 */
TEST_F(VoiceAllocatorTest, AllocateVoice_InvalidVelocity_Failure) {
    // Test velocity too low
    auto result1 = allocator_->allocateVoice(60, -1.0f);
    EXPECT_FALSE(result1.success);

    // Test velocity too high
    auto result2 = allocator_->allocateVoice(60, 128.0f);
    EXPECT_FALSE(result2.success);
}

/**
 * Test: Voice stealing when all voices are active
 */
TEST_F(VoiceAllocatorTest, AllocateVoice_AllVoicesActive_StealsLowestPriority) {
    // Allocate all 10 voices
    std::vector<int> voiceIds;
    for (int i = 0; i < 10; ++i) {
        auto result = allocator_->allocateVoice(60 + i, 50.0f);  // Low velocity
        ASSERT_TRUE(result.success);
        voiceIds.push_back(result.voiceId);
    }

    // Verify all voices are active
    EXPECT_EQ(allocator_->getActiveVoiceCount(), 10);

    // Allocate one more voice with high priority
    auto result = allocator_->allocateVoice(72, 127.0f);  // High velocity

    EXPECT_TRUE(result.success);
    EXPECT_TRUE(result.stolen);  // Should have stolen a voice
    EXPECT_NE(result.stolenFromId, -1);

    // Verify still 10 active voices (one was stolen and re-used)
    EXPECT_EQ(allocator_->getActiveVoiceCount(), 10);

    // Verify stolen voice is now playing new note
    VoiceRecord* voice = allocator_->getVoice(result.voiceId);
    ASSERT_NE(voice, nullptr);
    EXPECT_TRUE(voice->active);
    EXPECT_EQ(voice->midiNote, 72);
    EXPECT_FLOAT_EQ(voice->velocity, 127.0f);
}

/**
 * Test: Priority calculation with velocity
 */
TEST_F(VoiceAllocatorTest, CalculatePriority_HigherVelocity_HigherPriority) {
    // Allocate two voices with different velocities
    auto result1 = allocator_->allocateVoice(60, 50.0f);
    auto result2 = allocator_->allocateVoice(61, 100.0f);

    ASSERT_TRUE(result1.success);
    ASSERT_TRUE(result2.success);

    VoiceRecord* voice1 = allocator_->getVoice(result1.voiceId);
    VoiceRecord* voice2 = allocator_->getVoice(result2.voiceId);

    ASSERT_NE(voice1, nullptr);
    ASSERT_NE(voice2, nullptr);

    // Voice with higher velocity should have higher priority
    EXPECT_GT(voice2->priority, voice1->priority);
}

/**
 * Test: Priority calculation with age
 */
TEST_F(VoiceAllocatorTest, CalculatePriority_OlderVoice_HigherPriority) {
    // Allocate two voices with same velocity
    auto result1 = allocator_->allocateVoice(60, 80.0f);
    auto result2 = allocator_->allocateVoice(61, 80.0f);

    ASSERT_TRUE(result1.success);
    ASSERT_TRUE(result2.success);

    VoiceRecord* voice1 = allocator_->getVoice(result1.voiceId);
    VoiceRecord* voice2 = allocator_->getVoice(result2.voiceId);

    ASSERT_NE(voice1, nullptr);
    ASSERT_NE(voice2, nullptr);

    int initialPriority1 = voice1->priority;
    int initialPriority2 = voice2->priority;

    // Update priorities (increases age)
    allocator_->updatePriorities(0.1);
    allocator_->updatePriorities(0.1);

    // Priorities should have increased (due to age)
    EXPECT_GT(voice1->priority, initialPriority1);
    EXPECT_GT(voice2->priority, initialPriority2);
}

/**
 * Test: LRU behavior (newest voices stolen first)
 */
TEST_F(VoiceAllocatorTest, VoiceStealing_NewestVoiceStolenFirst) {
    // Allocate all 10 voices
    std::vector<int> voiceIds;
    for (int i = 0; i < 10; ++i) {
        auto result = allocator_->allocateVoice(60 + i, 80.0f);
        ASSERT_TRUE(result.success);
        voiceIds.push_back(result.voiceId);
    }

    // Update priorities to age all voices
    for (int i = 0; i < 10; ++i) {
        allocator_->updatePriorities(0.1);
    }

    // Allocate one more voice (should steal newest due to age tiebreaker)
    auto result = allocator_->allocateVoice(72, 80.0f);
    ASSERT_TRUE(result.success);
    EXPECT_TRUE(result.stolen);

    // The stolen voice should be the last allocated (newest)
    EXPECT_EQ(result.stolenFromId, voiceIds.back());
}

/**
 * Test: Voice freeing
 */
TEST_F(VoiceAllocatorTest, FreeVoice_VoiceBecomesInactive) {
    // Allocate a voice
    auto result = allocator_->allocateVoice(60, 100.0f);
    ASSERT_TRUE(result.success);

    EXPECT_EQ(allocator_->getActiveVoiceCount(), 1);

    // Free the voice
    allocator_->freeVoice(result.voiceId);

    EXPECT_EQ(allocator_->getActiveVoiceCount(), 0);

    // Verify voice is inactive
    VoiceRecord* voice = allocator_->getVoice(result.voiceId);
    ASSERT_NE(voice, nullptr);
    EXPECT_FALSE(voice->active);
    EXPECT_EQ(voice->midiNote, 0);
    EXPECT_FLOAT_EQ(voice->velocity, 0.0f);
}

/**
 * Test: Freeing invalid voice ID (no-op)
 */
TEST_F(VoiceAllocatorTest, FreeVoice_InvalidVoiceId_NoEffect) {
    // Allocate a voice
    auto result = allocator_->allocateVoice(60, 100.0f);
    ASSERT_TRUE(result.success);

    int activeCount = allocator_->getActiveVoiceCount();

    // Try to free invalid voice IDs
    allocator_->freeVoice(-1);
    allocator_->freeVoice(100);
    allocator_->freeVoice(999);

    // Active count should not change
    EXPECT_EQ(allocator_->getActiveVoiceCount(), activeCount);
}

/**
 * Test: Getting voice by invalid ID
 */
TEST_F(VoiceAllocatorTest, GetVoice_InvalidId_ReturnsNullptr) {
    VoiceRecord* voice1 = allocator_->getVoice(-1);
    EXPECT_EQ(voice1, nullptr);

    VoiceRecord* voice2 = allocator_->getVoice(100);
    EXPECT_EQ(voice2, nullptr);
}

/**
 * Test: Reset all voices
 */
TEST_F(VoiceAllocatorTest, ResetAll_AllVoicesInactive) {
    // Allocate all voices
    for (int i = 0; i < 10; ++i) {
        auto result = allocator_->allocateVoice(60 + i, 80.0f);
        ASSERT_TRUE(result.success);
    }

    EXPECT_EQ(allocator_->getActiveVoiceCount(), 10);

    // Reset all
    allocator_->resetAll();

    EXPECT_EQ(allocator_->getActiveVoiceCount(), 0);

    // Verify all voices are inactive
    for (int i = 0; i < 10; ++i) {
        VoiceRecord* voice = allocator_->getVoice(i);
        ASSERT_NE(voice, nullptr);
        EXPECT_FALSE(voice->active);
    }
}

/**
 * Test: Stealing statistics tracking
 */
TEST_F(VoiceAllocatorTest, StealingStats_TracksCorrectly) {
    // Allocate all voices
    for (int i = 0; i < 10; ++i) {
        auto result = allocator_->allocateVoice(60 + i, 50.0f);
        ASSERT_TRUE(result.success);
    }

    auto stats = allocator_->getStealingStats();
    EXPECT_EQ(stats.totalAllocations, 10);
    EXPECT_EQ(stats.stolenVoices, 0);

    // Steal a voice
    auto result = allocator_->allocateVoice(72, 100.0f);
    ASSERT_TRUE(result.success);

    stats = allocator_->getStealingStats();
    EXPECT_EQ(stats.totalAllocations, 11);
    EXPECT_EQ(stats.stolenVoices, 1);
    EXPECT_GT(stats.lowPriorityStolen, 0);  // Stolen low priority voice
}

/**
 * Test: MIDI to frequency conversion
 */
TEST_F(VoiceAllocatorTest, AllocateVoice_ConvertsMidiToFrequency) {
    // A4 (MIDI note 69) should be 440 Hz
    auto result = allocator_->allocateVoice(69, 100.0f);
    ASSERT_TRUE(result.success);

    VoiceRecord* voice = allocator_->getVoice(result.voiceId);
    ASSERT_NE(voice, nullptr);

    EXPECT_NEAR(voice->frequency, 440.0f, 0.1f);
}

/**
 * Test: Real-time safety (no dynamic allocation during audio processing)
 */
TEST_F(VoiceAllocatorTest, RealTimeSafety_NoAllocationDuringProcessing) {
    // Allocate all voices
    std::vector<int> voiceIds;
    for (int i = 0; i < 10; ++i) {
        auto result = allocator_->allocateVoice(60 + i, 80.0f);
        ASSERT_TRUE(result.success);
        voiceIds.push_back(result.voiceId);
    }

    // These operations should not allocate memory (real-time safe)
    allocator_->updatePriorities(0.1);
    allocator_->getActiveVoiceCount();
    allocator_->getVoice(voiceIds[0]);
    allocator_->getStealingStats();

    // Steal voice (also should not allocate)
    auto result = allocator_->allocateVoice(72, 100.0f);
    ASSERT_TRUE(result.success);

    // Free voice (also should not allocate)
    allocator_->freeVoice(result.voiceId);

    // If we get here without crashing, real-time safety is maintained
    SUCCEED();
}

/**
 * Test: Priority clamping to valid range
 */
TEST_F(VoiceAllocatorTest, CalculatePriority_ClampedToValidRange) {
    // Test various combinations
    auto result1 = allocator_->allocateVoice(60, 0.0f);   // Minimum velocity
    auto result2 = allocator_->allocateVoice(61, 127.0f); // Maximum velocity

    ASSERT_TRUE(result1.success);
    ASSERT_TRUE(result2.success);

    VoiceRecord* voice1 = allocator_->getVoice(result1.voiceId);
    VoiceRecord* voice2 = allocator_->getVoice(result2.voiceId);

    ASSERT_NE(voice1, nullptr);
    ASSERT_NE(voice2, nullptr);

    // Priorities should be in valid range
    EXPECT_GE(voice1->priority, 0);
    EXPECT_LE(voice1->priority, 100);
    EXPECT_GE(voice2->priority, 0);
    EXPECT_LE(voice2->priority, 100);
}

/**
 * Test: Multiple allocate/free cycles
 */
TEST_F(VoiceAllocatorTest, MultipleCycles_MaintainsConsistency) {
    std::vector<int> voiceIds;

    // Allocate and free voices multiple times
    for (int cycle = 0; cycle < 5; ++cycle) {
        // Allocate all voices
        voiceIds.clear();
        for (int i = 0; i < 10; ++i) {
            auto result = allocator_->allocateVoice(60 + i, 80.0f);
            ASSERT_TRUE(result.success);
            voiceIds.push_back(result.voiceId);
        }

        EXPECT_EQ(allocator_->getActiveVoiceCount(), 10);

        // Free all voices
        for (int voiceId : voiceIds) {
            allocator_->freeVoice(voiceId);
        }

        EXPECT_EQ(allocator_->getActiveVoiceCount(), 0);
    }

    // Allocator should still work after multiple cycles
    auto result = allocator_->allocateVoice(60, 100.0f);
    EXPECT_TRUE(result.success);
}

/**
 * Test: Pan position defaults to center
 */
TEST_F(VoiceAllocatorTest, AllocateVoice_PanDefaultsToCenter) {
    auto result = allocator_->allocateVoice(60, 100.0f);
    ASSERT_TRUE(result.success);

    VoiceRecord* voice = allocator_->getVoice(result.voiceId);
    ASSERT_NE(voice, nullptr);

    EXPECT_FLOAT_EQ(voice->pan, 0.0f);  // Center
}

/**
 * Test: Get max voices
 */
TEST_F(VoiceAllocatorTest, GetMaxVoices_ReturnsCorrectValue) {
    EXPECT_EQ(allocator_->getMaxVoices(), 10);
}

/**
 * Test: Age capping at 100
 */
TEST_F(VoiceAllocatorTest, UpdatePriorities_AgeCapsAt100) {
    auto result = allocator_->allocateVoice(60, 80.0f);
    ASSERT_TRUE(result.success);

    VoiceRecord* voice = allocator_->getVoice(result.voiceId);
    ASSERT_NE(voice, nullptr);

    // Update priorities many times (should cap age at 100)
    for (int i = 0; i < 200; ++i) {
        allocator_->updatePriorities(0.1);
    }

    // Age should be capped at 100
    EXPECT_LE(voice->age, 100);

    // Priority should not exceed maximum
    EXPECT_LE(voice->priority, 100);
}
