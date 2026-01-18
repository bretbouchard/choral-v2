/*
  ==============================================================================

    test_puredsp_g2p.cpp - Unit tests for PureDSP G2PEngine

    Copyright (c) 2026 Bret Bouchard
    All rights reserved.

  ==============================================================================
*/

#include <gtest/gtest.h>
#include "dsp/core/G2PEngine.h"
#include "dsp/core/PhonemeDatabase.h"
#include "dsp/core/LanguageLoader.h"
#include <memory>

using namespace DSP;

//==============================================================================
// Test Fixture
//==============================================================================

class G2PEngineTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create phoneme database
        phoneme_db_ = std::make_shared<PhonemeDatabase>();

        // Create G2P engine
        g2p_engine_ = std::make_unique<G2PEngine>(phoneme_db_);
    }

    void TearDown() override {
        g2p_engine_.reset();
        phoneme_db_.reset();
    }

    std::shared_ptr<PhonemeDatabase> phoneme_db_;
    std::unique_ptr<G2PEngine> g2p_engine_;
};

//==============================================================================
// Basic Conversion Tests
//==============================================================================

TEST_F(G2PEngineTest, ConvertEmptyString) {
    G2PResult result = g2p_engine_->convert("");

    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.getPhonemeCount(), 0);
    EXPECT_EQ(result.getTotalDuration(), 0.0f);
}

TEST_F(G2PEngineTest, ConvertSimpleWord) {
    // Add dictionary entry
    g2p_engine_->addDictionaryEntry("hello", {"HH", "AH", "L", "OW"});

    G2PResult result = g2p_engine_->convert("hello");

    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.getPhonemeCount(), 4);
    EXPECT_EQ(result.phonemes[0].symbol, "HH");
    EXPECT_EQ(result.phonemes[1].symbol, "AH");
    EXPECT_EQ(result.phonemes[2].symbol, "L");
    EXPECT_EQ(result.phonemes[3].symbol, "OW");
}

TEST_F(G2PEngineTest, ConvertMultipleWords) {
    // Add dictionary entries
    g2p_engine_->addDictionaryEntry("hello", {"HH", "AH", "L", "OW"});
    g2p_engine_->addDictionaryEntry("world", {"W", "ER", "L", "D"});

    G2PResult result = g2p_engine_->convert("hello world");

    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.getPhonemeCount(), 8);
    EXPECT_EQ(result.words.size(), 2);
    EXPECT_EQ(result.words[0], "hello");
    EXPECT_EQ(result.words[1], "world");
}

//==============================================================================
// Dictionary Tests
//==============================================================================

TEST_F(G2PEngineTest, DictionaryLookup) {
    g2p_engine_->addDictionaryEntry("test", {"T", "EH", "S", "T"});

    G2PResult result = g2p_engine_->convert("test");

    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.getPhonemeCount(), 4);

    // Check statistics
    auto stats = g2p_engine_->getLastStats();
    EXPECT_EQ(stats.dictionary_hits, 1);
}

TEST_F(G2PEngineTest, CustomDictionaryOverridesMain) {
    // Add main dictionary entry
    g2p_engine_->addDictionaryEntry("test", {"T", "EH", "S", "T"});

    // Override with custom entry
    g2p_engine_->addDictionaryEntry("test", {"T", "IY", "S", "T"});

    G2PResult result = g2p_engine_->convert("test");

    EXPECT_TRUE(result.success);
    // Should use custom entry
    EXPECT_EQ(result.phonemes[1].symbol, "IY");
}

//==============================================================================
// Rule Tests
//==============================================================================

TEST_F(G2PEngineTest, AddCustomRule) {
    G2PRule rule;
    rule.pattern = "ing";
    rule.phonemes = {"IH", "N", "G"};
    rule.priority = 10;

    g2p_engine_->addRule(rule);

    G2PResult result = g2p_engine_->convert("running");

    EXPECT_TRUE(result.success);
    // Rule should match "ing" at the end
}

TEST_F(G2PEngineTest, ClearCustomRules) {
    G2PRule rule;
    rule.pattern = "test";
    rule.phonemes = {"T", "EH", "S", "T"};
    rule.priority = 10;

    g2p_engine_->addRule(rule);
    g2p_engine_->clearCustomRules();

    // After clearing, rule should not apply
    G2PResult result = g2p_engine_->convert("test");

    EXPECT_TRUE(result.success);
    // Should use default character-to-phoneme mapping
}

//==============================================================================
// Timing and Prosody Tests
//==============================================================================

TEST_F(G2PEngineTest, ConvertWithTiming) {
    g2p_engine_->addDictionaryEntry("test", {"T", "EH", "S", "T"});

    G2PResult result = g2p_engine_->convertWithTiming("test", 4.0f);

    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.getPhonemeCount(), 4);

    // Check that durations are set
    for (const auto& phoneme : result.phonemes) {
        EXPECT_GT(phoneme.duration, 0.0f);
    }

    // Check total duration
    float total_duration = result.getTotalDuration();
    EXPECT_GT(total_duration, 0.0f);
}

TEST_F(G2PEngineTest, SpeechRateAffectsTiming) {
    g2p_engine_->addDictionaryEntry("test", {"T", "EH", "S", "T"});

    G2PResult result_slow = g2p_engine_->convertWithTiming("test", 2.0f);
    G2PResult result_fast = g2p_engine_->convertWithTiming("test", 8.0f);

    EXPECT_TRUE(result_slow.success);
    EXPECT_TRUE(result_fast.success);

    // Slower speech rate should produce longer duration
    EXPECT_GT(result_slow.getTotalDuration(), result_fast.getTotalDuration());
}

//==============================================================================
// Tokenization Tests
//==============================================================================

TEST_F(G2PEngineTest, HandlesPunctuation) {
    g2p_engine_->addDictionaryEntry("hello", {"HH", "AH", "L", "OW"});

    G2PResult result = g2p_engine_->convert("hello!");

    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.getPhonemeCount(), 4);
}

TEST_F(G2PEngineTest, HandlesWhitespace) {
    g2p_engine_->addDictionaryEntry("hello", {"HH", "AH", "L", "OW"});
    g2p_engine_->addDictionaryEntry("world", {"W", "ER", "L", "D"});

    G2PResult result = g2p_engine_->convert("hello  world");

    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.getPhonemeCount(), 8);
}

//==============================================================================
// Phoneme String Tests
//==============================================================================

TEST_F(G2PEngineTest, GetPhonemeString) {
    g2p_engine_->addDictionaryEntry("hello", {"HH", "AH", "L", "OW"});

    G2PResult result = g2p_engine_->convert("hello");

    EXPECT_TRUE(result.success);
    std::string phoneme_string = result.getPhonemeString();
    EXPECT_EQ(phoneme_string, "/HH AH L OW/");
}

//==============================================================================
// Statistics Tests
//==============================================================================

TEST_F(G2PEngineTest, StatisticsTracking) {
    g2p_engine_->addDictionaryEntry("hello", {"HH", "AH", "L", "OW"});

    G2PResult result = g2p_engine_->convert("hello");

    EXPECT_TRUE(result.success);

    auto stats = g2p_engine_->getLastStats();
    EXPECT_GT(stats.processing_time_ms, 0.0f);
}

//==============================================================================
// Main
//==============================================================================

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
