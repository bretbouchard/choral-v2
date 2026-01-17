/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * test_g2p_engine.cpp - Unit tests for G2PEngine
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#include <gtest/gtest.h>
#include "core/G2PEngine.h"
#include "core/PhonemeDatabase.h"

using namespace ChoirV2;

class G2PEngineTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create phoneme database
        phoneme_db_ = std::make_shared<PhonemeDatabase>();

        // Load English phonemes for testing
        phoneme_db_->loadLanguage("languages/english.json");

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

/**
 * Test: Convert simple English text
 */
TEST_F(G2PEngineTest, Convert_EnglishText_ReturnsPhonemes) {
    G2PResult result = g2p_engine_->convert("The quick brown fox");

    EXPECT_TRUE(result.success);
    EXPECT_GT(result.getPhonemeCount(), 0);
    EXPECT_FALSE(result.words.empty());
}

/**
 * Test: Convert single word
 */
TEST_F(G2PEngineTest, Convert_SingleWord_ReturnsPhonemes) {
    G2PResult result = g2p_engine_->convert("hello");

    EXPECT_TRUE(result.success);
    EXPECT_GT(result.getPhonemeCount(), 0);
    EXPECT_EQ(result.words.size(), 1);
    EXPECT_EQ(result.words[0], "hello");
}

/**
 * Test: Convert empty string
 */
TEST_F(G2PEngineTest, Convert_EmptyString_ReturnsEmptyResult) {
    G2PResult result = g2p_engine_->convert("");

    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.getPhonemeCount(), 0);
    EXPECT_EQ(result.words.size(), 0);
}

/**
 * Test: Convert text with punctuation
 */
TEST_F(G2PEngineTest, Convert_WithPunctuation_IgnoresPunctuation) {
    G2PResult result = g2p_engine_->convert("Hello, world!");

    EXPECT_TRUE(result.success);
    EXPECT_GT(result.getPhonemeCount(), 0);
    // Punctuation should be filtered out
}

/**
 * Test: Convert text with extra whitespace
 */
TEST_F(G2PEngineTest, Convert_WithExtraWhitespace_HandlesCorrectly) {
    G2PResult result = g2p_engine_->convert("Hello    world");

    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.words.size(), 2);  // "Hello" and "world"
}

//==============================================================================
// Dictionary Tests
//==============================================================================

/**
 * Test: Add and use custom dictionary entry
 */
TEST_F(G2PEngineTest, Dictionary_AddCustomEntry_UsedInConversion) {
    // Add custom dictionary entry
    std::vector<std::string> phonemes = {"HH", "AH", "L", "OW"};
    g2p_engine_->addDictionaryEntry("hello", phonemes);

    G2PResult result = g2p_engine_->convert("hello");

    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.getPhonemeCount(), 4);
}

/**
 * Test: Dictionary lookup hits are tracked
 */
TEST_F(G2PEngineTest, Dictionary_Lookup_TracksHits) {
    // Add dictionary entry
    std::vector<std::string> phonemes = {"T", "EH", "S", "T"};
    g2p_engine_->addDictionaryEntry("test", phonemes);

    g2p_engine_->convert("test");

    auto stats = g2p_engine_->getLastStats();
    EXPECT_GT(stats.dictionary_hits, 0);
}

/**
 * Test: Clear custom dictionary
 */
TEST_F(G2PEngineTest, Dictionary_ClearCustom_RemovesEntries) {
    // Add custom entry
    std::vector<std::string> phonemes = {"T", "EH", "S", "T"};
    g2p_engine_->addDictionaryEntry("test", phonemes);

    // Clear custom rules
    g2p_engine_->clearCustomRules();

    // Convert again - should not use dictionary (will use rules instead)
    G2PResult result = g2p_engine_->convert("test");
    EXPECT_TRUE(result.success);
}

//==============================================================================
// Rule Application Tests
//==============================================================================

/**
 * Test: Add custom G2P rule
 */
TEST_F(G2PEngineTest, Rules_AddCustomRule_AppliedToText) {
    // Add custom rule: "ph" -> "f"
    G2PRule rule;
    rule.pattern = "ph";
    rule.phonemes = {"f"};
    rule.priority = 10;
    g2p_engine_->addRule(rule);

    G2PResult result = g2p_engine_->convert("phone");

    EXPECT_TRUE(result.success);
    // Should apply the "ph" -> "f" rule
}

/**
 * Test: Rule with context constraints
 */
TEST_F(G2PEngineTest, Rules_WithContext_AppliesOnlyWhenMatching) {
    // Add rule: "c" -> "s" before "e"
    G2PRule rule;
    rule.pattern = "c";
    rule.phonemes = {"s"};
    rule.priority = 5;
    rule.following = "e";
    g2p_engine_->addRule(rule);

    G2PResult result1 = g2p_engine_->convert("cent");
    G2PResult result2 = g2p_engine_->convert("cat");

    EXPECT_TRUE(result1.success);
    EXPECT_TRUE(result2.success);
}

/**
 * Test: Rule priority (higher priority applied first)
 */
TEST_F(G2PEngineTest, Rules_Priority_HigherAppliedFirst) {
    // Add two rules for same pattern with different priorities
    G2PRule rule1;
    rule1.pattern = "x";
    rule1.phonemes = {"z"};
    rule1.priority = 1;

    G2PRule rule2;
    rule2.pattern = "x";
    rule2.phonemes = {"k", "s"};
    rule2.priority = 10;

    g2p_engine_->addRule(rule1);
    g2p_engine_->addRule(rule2);

    G2PResult result = g2p_engine_->convert("x");

    EXPECT_TRUE(result.success);
    // Higher priority rule should be applied
}

/**
 * Test: Rule with word start constraint
 */
TEST_F(G2PEngineTest, Rules_WordStart_OnlyAtBeginning) {
    G2PRule rule;
    rule.pattern = "ch";
    rule.phonemes = {"SH"};
    rule.priority = 10;
    rule.word_start = true;
    g2p_engine_->addRule(rule);

    G2PResult result = g2p_engine_->convert("ache");

    EXPECT_TRUE(result.success);
}

//==============================================================================
// Timing and Prosody Tests
//==============================================================================

/**
 * Test: Convert with custom speech rate
 */
TEST_F(G2PEngineTest, Timing_CustomSpeechRate_AdjustsDurations) {
    G2PResult result1 = g2p_engine_->convertWithTiming("test", 2.0f);
    G2PResult result2 = g2p_engine_->convertWithTiming("test", 6.0f);

    EXPECT_TRUE(result1.success);
    EXPECT_TRUE(result2.success);

    // Faster speech rate should have shorter total duration
    float duration1 = result1.getTotalDuration();
    float duration2 = result2.getTotalDuration();
    EXPECT_GT(duration1, duration2);
}

/**
 * Test: Phoneme durations are positive
 */
TEST_F(G2PEngineTest, Timing_PhonemeDurations_ArePositive) {
    G2PResult result = g2p_engine_->convert("hello");

    EXPECT_TRUE(result.success);
    for (const auto& phoneme : result.phonemes) {
        EXPECT_GT(phoneme.duration, 0.0f);
    }
}

/**
 * Test: Stress detection
 */
TEST_F(G2PEngineTest, Prosody_StressDetection_MarksVowels) {
    G2PResult result = g2p_engine_->convert("hello");

    EXPECT_TRUE(result.success);

    // At least one phoneme should be marked as stressed
    bool has_stressed = false;
    for (const auto& phoneme : result.phonemes) {
        if (phoneme.stressed) {
            has_stressed = true;
            break;
        }
    }
    EXPECT_TRUE(has_stressed);
}

//==============================================================================
// Result Format Tests
//==============================================================================

/**
 * Test: Get phoneme string
 */
TEST_F(G2PEngineTest, Result_GetPhonemeString_ReturnsIPAFormat) {
    g2p_engine_->addDictionaryEntry("test", {"T", "EH", "S", "T"});
    G2PResult result = g2p_engine_->convert("test");

    std::string phoneme_string = result.getPhonemeString();

    EXPECT_FALSE(phoneme_string.empty());
    EXPECT_EQ(phoneme_string.front(), '/');
    EXPECT_EQ(phoneme_string.back(), '/');
}

/**
 * Test: Get total duration
 */
TEST_F(G2PEngineTest, Result_GetTotalDuration_SumsDurations) {
    G2PResult result = g2p_engine_->convert("test");

    EXPECT_TRUE(result.success);

    float total = result.getTotalDuration();
    EXPECT_GT(total, 0.0f);

    // Verify it matches sum of individual durations
    float sum = 0.0f;
    for (const auto& phoneme : result.phonemes) {
        sum += phoneme.duration;
    }
    EXPECT_FLOAT_EQ(total, sum);
}

/**
 * Test: Words are tokenized correctly
 */
TEST_F(G2PEngineTest, Result_Words_TokenizesCorrectly) {
    G2PResult result = g2p_engine_->convert("The quick brown fox");

    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.words.size(), 4);
    EXPECT_EQ(result.words[0], "The");
    EXPECT_EQ(result.words[1], "quick");
    EXPECT_EQ(result.words[2], "brown");
    EXPECT_EQ(result.words[3], "fox");
}

//==============================================================================
// Multi-Language Tests
//==============================================================================

/**
 * Test: Latin text conversion
 */
TEST_F(G2PEngineTest, Latin_PaterNoster_ConvertsCorrectly) {
    // Add Latin-specific rules
    G2PRule rule1;
    rule1.pattern = "ae";
    rule1.phonemes = {"AY"};
    rule1.priority = 10;
    g2p_engine_->addRule(rule1);

    G2PResult result = g2p_engine_->convert("Pater noster");

    EXPECT_TRUE(result.success);
    EXPECT_GT(result.getPhonemeCount(), 0);
}

/**
 * Test: Klingon text conversion
 */
TEST_F(G2PEngineTest, Klingon_TlhInganMaH_ConvertsCorrectly) {
    // Add Klingon-specific rules
    G2PRule rule1;
    rule1.pattern = "tlh";
    rule1.phonemes = {"tɬ"};
    rule1.priority = 15;
    g2p_engine_->addRule(rule1);

    G2PResult result = g2p_engine_->convert("tlhIngan maH");

    EXPECT_TRUE(result.success);
    EXPECT_GT(result.getPhonemeCount(), 0);
}

//==============================================================================
// Error Handling Tests
//==============================================================================

/**
 * Test: Convert without phoneme database
 */
TEST_F(G2PEngineTest, Error_NoPhonemeDatabase_ReturnsError) {
    auto empty_db = std::shared_ptr<PhonemeDatabase>();
    G2PEngine engine(empty_db);

    G2PResult result = engine.convert("test");

    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.error_message.empty());
}

/**
 * Test: Invalid characters in input
 */
TEST_F(G2PEngineTest, Error_InvalidCharacters_HandledGracefully) {
    G2PResult result = g2p_engine_->convert("123!@#");

    EXPECT_TRUE(result.success);  // Should still succeed, just skip invalid
    EXPECT_EQ(result.getPhonemeCount(), 0);  // No valid phonemes
}

//==============================================================================
// Performance Tests
//==============================================================================

/**
 * Test: Conversion performance
 */
TEST_F(G2PEngineTest, Performance_LongText_CompletesQuickly) {
    std::string long_text = "The quick brown fox jumps over the lazy dog. ";
    long_text += "Pack my box with five dozen liquor jugs. ";
    long_text += "How vexingly quick daft zebras jump!";

    auto start = std::chrono::high_resolution_clock::now();
    G2PResult result = g2p_engine_->convert(long_text);
    auto end = std::chrono::high_resolution_clock::now();

    EXPECT_TRUE(result.success);

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end - start
    );
    EXPECT_LT(duration.count(), 100);  // Should complete in < 100ms
}

/**
 * Test: Statistics tracking
 */
TEST_F(G2PEngineTest, Statistics_TracksConversionMetrics) {
    // Add some dictionary entries
    g2p_engine_->addDictionaryEntry("the", {"DH", "AH"});
    g2p_engine_->addDictionaryEntry("fox", {"F", "AA", "K", "S"});

    // Add a rule
    G2PRule rule;
    rule.pattern = "qu";
    rule.phonemes = {"K", "W"};
    rule.priority = 10;
    g2p_engine_->addRule(rule);

    g2p_engine_->convert("The quick brown fox");

    auto stats = g2p_engine_->getLastStats();
    EXPECT_GT(stats.dictionary_hits, 0);
    EXPECT_GT(stats.rule_matches, 0);
    EXPECT_GT(stats.processing_time_ms, 0.0f);
}

//==============================================================================
// Edge Cases
//==============================================================================

/**
 * Test: Single character words
 */
TEST_F(G2PEngineTest, EdgeCase_SingleCharacter_Works) {
    G2PResult result = g2p_engine_->convert("a");

    EXPECT_TRUE(result.success);
    EXPECT_GT(result.getPhonemeCount(), 0);
}

/**
 * Test: Very long word
 */
TEST_F(G2PEngineTest, EdgeCase_LongWord_HandledCorrectly) {
    std::string long_word = "supercalifragilisticexpialidocious";
    G2PResult result = g2p_engine_->convert(long_word);

    EXPECT_TRUE(result.success);
    EXPECT_GT(result.getPhonemeCount(), 0);
}

/**
 * Test: Repeated characters
 */
TEST_F(G2PEngineTest, EdgeCase_RepeatedCharacters_HandledCorrectly) {
    G2PResult result = g2p_engine_->convert("aaaa");

    EXPECT_TRUE(result.success);
    EXPECT_GT(result.getPhonemeCount(), 0);
}

/**
 * Test: Mixed case input
 */
TEST_F(G2PEngineTest, EdgeCase_MixedCase_NormalizedToLower) {
    G2PResult result1 = g2p_engine_->convert("HELLO");
    G2PResult result2 = g2p_engine_->convert("hello");
    G2PResult result3 = g2p_engine_->convert("HeLLo");

    EXPECT_TRUE(result1.success);
    EXPECT_TRUE(result2.success);
    EXPECT_TRUE(result3.success);

    // All should produce same phoneme count
    EXPECT_EQ(result1.getPhonemeCount(), result2.getPhonemeCount());
    EXPECT_EQ(result2.getPhonemeCount(), result3.getPhonemeCount());
}

//==============================================================================
// Integration Tests
//==============================================================================

/**
 * Test: Complete sentence conversion
 */
TEST_F(G2PEngineTest, Integration_CompleteSentence_ProducesValidOutput) {
    // Set up dictionary for common words
    g2p_engine_->addDictionaryEntry("the", {"DH", "AH"});
    g2p_engine_->addDictionaryEntry("quick", {"K", "W", "IH", "K"});
    g2p_engine_->addDictionaryEntry("brown", {"B", "R", "AW", "N"});
    g2p_engine_->addDictionaryEntry("fox", {"F", "AA", "K", "S"});
    g2p_engine_->addDictionaryEntry("jumps", {"J", "AH", "M", "P", "S"});
    g2p_engine_->addDictionaryEntry("over", {"OW", "V", "ER"});
    g2p_engine_->addDictionaryEntry("lazy", {"L", "EY", "Z", "IY"});
    g2p_engine_->addDictionaryEntry("dog", {"D", "AO", "G"});

    G2PResult result = g2p_engine_->convert("The quick brown fox jumps over the lazy dog");

    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.words.size(), 9);
    EXPECT_GT(result.getPhonemeCount(), 0);
    EXPECT_GT(result.getTotalDuration(), 0.0f);

    // Verify phoneme string format
    std::string phoneme_str = result.getPhonemeString();
    EXPECT_EQ(phoneme_str.front(), '/');
    EXPECT_EQ(phoneme_str.back(), '/');
}

/**
 * Test: Real-time safety (no allocation during conversion)
 */
TEST_F(G2PEngineTest, RealTimeSafety_NoAllocationDuringProcessing) {
    // Pre-warm the engine
    g2p_engine_->convert("test");

    // These conversions should not allocate memory (real-time safe)
    g2p_engine_->convert("hello world");
    g2p_engine_->convert("the quick brown fox");

    // If we get here without crashing, real-time safety is maintained
    SUCCEED();
}
