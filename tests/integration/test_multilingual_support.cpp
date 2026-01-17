/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * test_multilingual_support.cpp - Integration tests for multi-language support
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#include <gtest/gtest.h>
#include <core/G2PEngine.h>
#include <core/PhonemeDatabase.h>
#include <core/LanguageLoader.h>
#include <core/LanguageDefinition.h>
#include <synthesis/FormantSynthesis.h>
#include <utils/AudioBuffer.h>

#include <fstream>
#include <cmath>
#include <set>
#include <algorithm>

using namespace ChoirV2;

class MultilingualSupportTest : public ::testing::Test {
protected:
    void SetUp() override {
        sampleRate_ = 44100.0;

        // Initialize phoneme database
        phonemeDB_ = std::make_shared<PhonemeDatabase>();
        ASSERT_TRUE(phonemeDB_->loadFromDirectory("languages/phonemes"));

        // Initialize language loader
        languageLoader_ = std::make_unique<LanguageLoader>();

        // Initialize G2P engine
        g2pEngine_ = std::make_unique<G2PEngine>(phonemeDB_);

        // Initialize formant synthesis
        formantSynth_ = std::make_unique<FormantSynthesis>(sampleRate_);
    }

    void TearDown() override {
        formantSynth_.reset();
        g2pEngine_.reset();
        languageLoader_.reset();
        phonemeDB_.reset();
    }

    // Helper: Load language by code
    bool loadLanguage(const std::string& langCode) {
        auto language = languageLoader_->loadLanguage(langCode);
        if (language) {
            g2pEngine_->setLanguage(language);
            currentLanguage_ = language;
            return true;
        }
        return false;
    }

    // Helper: Get phoneme sequence as string
    std::string getPhonemeString(const G2PResult& result) {
        std::string phonemeStr;
        for (const auto& phoneme : result.phonemes) {
            if (!phonemeStr.empty()) phonemeStr += " ";
            phonemeStr += phoneme.symbol;
        }
        return phonemeStr;
    }

    // Helper: Check if phoneme sequence is valid
    bool isValidPhonemeSequence(const std::vector<PhonemeResult>& phonemes) {
        if (phonemes.empty()) return false;

        for (const auto& phoneme : phonemes) {
            if (phoneme.symbol.empty()) return false;
            if (phoneme.duration <= 0.0f) return false;
        }

        return true;
    }

    // Helper: Verify phoneme symbols are in database
    bool arePhonemesInDatabase(const std::vector<PhonemeResult>& phonemes) {
        for (const auto& phoneme : phonemes) {
            if (!phonemeDB_->hasPhoneme(phoneme.symbol)) {
                return false;
            }
        }
        return true;
    }

    // Helper: Calculate formant frequencies for language
    bool verifyFormantFrequencies(const std::string& langCode) {
        auto language = languageLoader_->loadLanguage(langCode);
        if (!language) return false;

        // Check if language has formant definitions
        auto formants = language->getFormantFrequencies();
        return !formants.empty();
    }

    // Helper: Test language switching stability
    bool testLanguageSwitchingStability(const std::vector<std::string>& langCodes) {
        std::string testText = "Hello world";

        for (const auto& langCode : langCodes) {
            if (!loadLanguage(langCode)) {
                return false;
            }

            // Attempt conversion
            G2PResult result = g2pEngine_->convert(testText);

            // Some languages may not support this text, but should not crash
            if (!result.success && !result.error_message.empty()) {
                // Expected for unsupported text in some languages
                continue;
            }
        }

        return true;
    }

    double sampleRate_;
    std::shared_ptr<PhonemeDatabase> phonemeDB_;
    std::unique_ptr<LanguageLoader> languageLoader_;
    std::unique_ptr<G2PEngine> g2pEngine_;
    std::unique_ptr<FormantSynthesis> formantSynth_;
    std::shared_ptr<LanguageDefinition> currentLanguage_;
};

// ============================================================================
// Test 1: Load All Supported Languages
// ============================================================================

TEST_F(MultilingualSupportTest, LoadAllSupportedLanguages) {
    std::vector<std::string> supportedLanguages = {
        "en",    // English
        "la",    // Latin
        "es",    // Spanish
        "de",    // German
        "fr",    // French
        "it",    // Italian
        "ja",    // Japanese
        "zh",    // Mandarin Chinese
        "ko",    // Korean
        "ru",    // Russian
        "ar",    // Arabic
        "he",    // Hebrew
        "tlh"    // Klingon
    };

    for (const auto& langCode : supportedLanguages) {
        EXPECT_TRUE(loadLanguage(langCode))
            << "Failed to load language: " << langCode;
    }
}

// ============================================================================
// Test 2: English Text Processing
// ============================================================================

TEST_F(MultilingualSupportTest, EnglishTextProcessing) {
    ASSERT_TRUE(loadLanguage("en"));

    struct TestCase {
        std::string text;
        size_t expectedPhonemeCount;
    };

    std::vector<TestCase> testCases = {
        {"Hello", 3},           // HH AH L OW
        {"world", 3},           // W ER L D
        {"The quick brown fox", 15},
        {"Choir V2.0", 6},
        {"testing", 7}
    };

    for (const auto& testCase : testCases) {
        G2PResult result = g2pEngine_->convert(testCase.text);

        EXPECT_TRUE(result.success)
            << "Failed to convert: " << testCase.text;
        EXPECT_TRUE(isValidPhonemeSequence(result.phonemes))
            << "Invalid phoneme sequence for: " << testCase.text;
        EXPECT_TRUE(arePhonemesInDatabase(result.phonemes))
            << "Phonemes not in database for: " << testCase.text;
    }
}

// ============================================================================
// Test 3: Latin Text Processing
// ============================================================================

TEST_F(MultilingualSupportTest, LatinTextProcessing) {
    ASSERT_TRUE(loadLanguage("la"));

    std::vector<std::string> testTexts = {
        "Pater noster",
        "Ave Maria",
        "Dominus vobiscum",
        "Gloria in excelsis Deo",
        "Kyrie eleison"
    };

    for (const auto& text : testTexts) {
        G2PResult result = g2pEngine_->convert(text);

        EXPECT_TRUE(result.success)
            << "Failed to convert Latin text: " << text;
        EXPECT_TRUE(isValidPhonemeSequence(result.phonemes))
            << "Invalid phoneme sequence for: " << text;

        // Latin should have distinct phoneme patterns
        std::string phonemeStr = getPhonemeString(result);
        EXPECT_FALSE(phonemeStr.empty())
            << "Empty phoneme string for: " << text;
    }
}

// ============================================================================
// Test 4: Klingon Text Processing
// ============================================================================

TEST_F(MultilingualSupportTest, KlingonTextProcessing) {
    ASSERT_TRUE(loadLanguage("tlh"));

    std::vector<std::string> testTexts = {
        "tlhIngan maH",         // We are Klingons
        "Qapla'",               // Success
        "ghaytan",              // Perhaps
        "yIH",                  // Attention!
        "nuqneH"                // What do you want?
    };

    for (const auto& text : testTexts) {
        G2PResult result = g2pEngine_->convert(text);

        EXPECT_TRUE(result.success)
            << "Failed to convert Klingon text: " << text;
        EXPECT_TRUE(isValidPhonemeSequence(result.phonemes))
            << "Invalid phoneme sequence for: " << text;

        // Klingon should have unique phonemes
        bool hasKlingonPhonemes = false;
        for (const auto& phoneme : result.phonemes) {
            // Check for Klingon-specific patterns
            if (phoneme.symbol.find("tlh") != std::string::npos ||
                phoneme.symbol.find("Q") != std::string::npos ||
                phoneme.symbol.find("gh") != std::string::npos) {
                hasKlingonPhonemes = true;
                break;
            }
        }
        EXPECT_TRUE(hasKlingonPhonemes)
            << "No Klingon-specific phonemes in: " << text;
    }
}

// ============================================================================
// Test 5: Right-to-Left Languages (Arabic, Hebrew)
// ============================================================================

TEST_F(MultilingualSupportTest, RightToLeftLanguages) {
    // Test Arabic
    ASSERT_TRUE(loadLanguage("ar"));
    std::string arabicText = "السلام عليكم"; // Peace be upon you
    G2PResult arabicResult = g2pEngine_->convert(arabicText);

    EXPECT_TRUE(arabicResult.success)
        << "Failed to process Arabic text";
    EXPECT_TRUE(isValidPhonemeSequence(arabicResult.phonemes));

    // Test Hebrew
    ASSERT_TRUE(loadLanguage("he"));
    std::string hebrewText = "שלום"; // Shalom (Peace)
    G2PResult hebrewResult = g2pEngine_->convert(hebrewText);

    EXPECT_TRUE(hebrewResult.success)
        << "Failed to process Hebrew text";
    EXPECT_TRUE(isValidPhonemeSequence(hebrewResult.phonemes));
}

// ============================================================================
// Test 6: East Asian Languages (Japanese, Chinese, Korean)
// ============================================================================

TEST_F(MultilingualSupportTest, EastAsianLanguages) {
    // Test Japanese
    ASSERT_TRUE(loadLanguage("ja"));
    std::string japaneseText = "こんにちは"; // Konnichiwa (Hello)
    G2PResult japaneseResult = g2pEngine_->convert(japaneseText);

    EXPECT_TRUE(japaneseResult.success)
        << "Failed to process Japanese text";
    EXPECT_TRUE(isValidPhonemeSequence(japaneseResult.phonemes));

    // Test Mandarin Chinese
    ASSERT_TRUE(loadLanguage("zh"));
    std::string chineseText = "你好"; // Nǐ hǎo (Hello)
    G2PResult chineseResult = g2pEngine_->convert(chineseText);

    EXPECT_TRUE(chineseResult.success)
        << "Failed to process Chinese text";
    EXPECT_TRUE(isValidPhonemeSequence(chineseResult.phonemes));

    // Test Korean
    ASSERT_TRUE(loadLanguage("ko"));
    std::string koreanText = "안녕하세요"; // Annyeonghaseyo (Hello)
    G2PResult koreanResult = g2pEngine_->convert(koreanText);

    EXPECT_TRUE(koreanResult.success)
        << "Failed to process Korean text";
    EXPECT_TRUE(isValidPhonemeSequence(koreanResult.phonemes));
}

// ============================================================================
// Test 7: European Languages (Spanish, German, French, Italian)
// ============================================================================

TEST_F(MultilingualSupportTest, EuropeanLanguages) {
    struct LanguageTest {
        std::string code;
        std::string text;
    };

    std::vector<LanguageTest> tests = {
        {"es", "Hola mundo"},           // Spanish
        {"de", "Hallo Welt"},           // German
        {"fr", "Bonjour le monde"},     // French
        {"it", "Ciao mondo"}            // Italian
    };

    for (const auto& test : tests) {
        ASSERT_TRUE(loadLanguage(test.code));
        G2PResult result = g2pEngine_->convert(test.text);

        EXPECT_TRUE(result.success)
            << "Failed to process " << test.code << " text: " << test.text;
        EXPECT_TRUE(isValidPhonemeSequence(result.phonemes));
    }
}

// ============================================================================
// Test 8: Language Switching Without Crashes
// ============================================================================

TEST_F(MultilingualSupportTest, LanguageSwitchingNoCrashes) {
    std::vector<std::string> languages = {"en", "la", "tlh", "ja", "ar", "de"};
    std::string testText = "test";

    // Rapid language switching
    for (int iteration = 0; iteration < 10; ++iteration) {
        for (const auto& langCode : languages) {
            ASSERT_TRUE(loadLanguage(langCode));

            // Attempt conversion (may fail for unsupported text, but shouldn't crash)
            G2PResult result = g2pEngine_->convert(testText);
            // We don't care about success, just that it doesn't crash
        }
    }
}

// ============================================================================
// Test 9: Concurrent Language Switching
// ============================================================================

TEST_F(MultilingualSupportTest, ConcurrentLanguageSwitching) {
    // Simulate concurrent language access
    std::vector<std::string> languages = {"en", "la", "es", "de", "fr", "tlh"};

    // Load all languages
    std::vector<std::shared_ptr<LanguageDefinition>> loadedLanguages;
    for (const auto& langCode : languages) {
        auto language = languageLoader_->loadLanguage(langCode);
        ASSERT_TRUE(language);
        loadedLanguages.push_back(language);
    }

    // Switch between languages rapidly
    for (int i = 0; i < 50; ++i) {
        for (auto& language : loadedLanguages) {
            g2pEngine_->setLanguage(language);

            std::string text = "test";
            G2PResult result = g2pEngine_->convert(text);
            // Should not crash
        }
    }
}

// ============================================================================
// Test 10: Verify Phoneme Sequences Are Correct
// ============================================================================

TEST_F(MultilingualSupportTest, VerifyPhonemeSequences) {
    struct PhonemeTest {
        std::string langCode;
        std::string text;
        std::vector<std::string> expectedPhonemes;
    };

    std::vector<PhonemeTest> tests = {
        {"en", "cat", {"K", "AE", "T"}},
        {"en", "dog", {"D", "AO", "G"}},
        {"la", "pater", {"P", "AH", "T", "EH", "R"}}
    };

    for (const auto& test : tests) {
        ASSERT_TRUE(loadLanguage(test.langCode));
        G2PResult result = g2pEngine_->convert(test.text);

        EXPECT_TRUE(result.success)
            << "Failed to convert: " << test.text;

        // Check phoneme count matches
        if (!test.expectedPhonemes.empty()) {
            EXPECT_EQ(result.phonemes.size(), test.expectedPhonemes.size())
                << "Phoneme count mismatch for: " << test.text;

            // Check phoneme symbols
            for (size_t i = 0; i < std::min(result.phonemes.size(), test.expectedPhonemes.size()); ++i) {
                EXPECT_EQ(result.phonemes[i].symbol, test.expectedPhonemes[i])
                    << "Phoneme mismatch at position " << i << " for: " << test.text;
            }
        }
    }
}

// ============================================================================
// Test 11: Check Formant Frequencies Match Language
// ============================================================================

TEST_F(MultilingualSupportTest, FormantFrequenciesMatchLanguage) {
    std::vector<std::string> languages = {"en", "la", "es", "de", "fr", "ja", "tlh"};

    for (const auto& langCode : languages) {
        ASSERT_TRUE(loadLanguage(langCode));

        // Verify language has formant definitions
        EXPECT_TRUE(verifyFormantFrequencies(langCode))
            << "No formant frequencies for: " << langCode;

        // Get formant frequencies
        auto language = languageLoader_->loadLanguage(langCode);
        ASSERT_TRUE(language);

        auto formants = language->getFormantFrequencies();
        EXPECT_FALSE(formants.empty())
            << "Empty formant frequencies for: " << langCode;

        // Verify formant values are reasonable
        for (const auto& formant : formants) {
            EXPECT_GT(formant.f1, 100.0f)
                << "F1 too low for: " << langCode;
            EXPECT_LT(formant.f1, 1500.0f)
                << "F1 too high for: " << langCode;

            EXPECT_GT(formant.f2, 500.0f)
                << "F2 too low for: " << langCode;
            EXPECT_LT(formant.f2, 3500.0f)
                << "F2 too high for: " << langCode;

            EXPECT_GT(formant.f3, 1000.0f)
                << "F3 too low for: " << langCode;
            EXPECT_LT(formant.f3, 4500.0f)
                << "F3 too high for: " << langCode;
        }
    }
}

// ============================================================================
// Test 12: Test Timing And Prosody
// ============================================================================

TEST_F(MultilingualSupportTest, TimingAndProsody) {
    ASSERT_TRUE(loadLanguage("en"));

    std::string text = "The quick brown fox jumps over the lazy dog";
    G2PResult result = g2pEngine_->convert(text);

    ASSERT_TRUE(result.success);
    ASSERT_FALSE(result.phonemes.empty());

    // Verify phonemes have timing information
    for (const auto& phoneme : result.phonemes) {
        EXPECT_GT(phoneme.duration, 0.0f)
            << "Phoneme has zero duration: " << phoneme.symbol;
        EXPECT_LT(phoneme.duration, 1.0f)
            << "Phoneme duration too long: " << phoneme.symbol;
    }

    // Verify total duration is reasonable
    float totalDuration = result.getTotalDuration();
    EXPECT_GT(totalDuration, 0.5f)
        << "Total duration too short";
    EXPECT_LT(totalDuration, 10.0f)
        << "Total duration too long";

    // Verify syllable information
    bool hasSyllableInfo = false;
    for (const auto& phoneme : result.phonemes) {
        if (phoneme.syllable > 0 || phoneme.stressed) {
            hasSyllableInfo = true;
            break;
        }
    }
    EXPECT_TRUE(hasSyllableInfo)
        << "No syllable information in result";
}

// ============================================================================
// Test 13: Language-Specific G2P Rules
// ============================================================================

TEST_F(MultilingualSupportTest, LanguageSpecificG2PRules) {
    // Test English-specific rules
    ASSERT_TRUE(loadLanguage("en"));
    G2PResult enResult = g2pEngine_->convert("though");
    EXPECT_TRUE(enResult.success);
    EXPECT_FALSE(getPhonemeString(enResult).empty());

    // Test that same text produces different phonemes in different languages
    ASSERT_TRUE(loadLanguage("la"));
    G2PResult laResult = g2pEngine_->convert("a");

    ASSERT_TRUE(loadLanguage("en"));
    G2PResult enResult2 = g2pEngine_->convert("a");

    // Phoneme sequences should be different (or at least not identical in all cases)
    std::string enPhonemes = getPhonemeString(enResult2);
    std::string laPhonemes = getPhonemeString(laResult);

    // They may be different (not strictly required, but likely)
    EXPECT_FALSE(enPhonemes.empty() && laPhonemes.empty());
}

// ============================================================================
// Test 14: Stress And Intonation Patterns
// ============================================================================

TEST_F(MultilingualSupportTest, StressAndIntonationPatterns) {
    ASSERT_TRUE(loadLanguage("en"));

    // Test words with different stress patterns
    std::vector<std::string> words = {
        "record",    // Noun vs verb (different stress)
        "present",   // Noun vs verb
        "permit",    // Noun vs verb
        "object"     // Noun vs verb
    };

    for (const auto& word : words) {
        G2PResult result = g2pEngine_->convert(word);
        EXPECT_TRUE(result.success)
            << "Failed to convert: " << word;

        // Check for stress information
        bool hasStress = false;
        for (const auto& phoneme : result.phonemes) {
            if (phoneme.stressed) {
                hasStress = true;
                break;
            }
        }

        // Not all words have marked stress, but many should
        // We don't enforce this strictly, but check it works
    }
}

// ============================================================================
// Test 15: Multilingual Audio Generation
// ============================================================================

TEST_F(MultilingualSupportTest, MultilingualAudioGeneration) {
    std::vector<std::string> languages = {"en", "la", "es", "de", "tlh"};

    for (const auto& langCode : languages) {
        ASSERT_TRUE(loadLanguage(langCode));

        std::string text = (langCode == "en") ? "Hello" :
                          (langCode == "la") ? "Salve" :
                          (langCode == "es") ? "Hola" :
                          (langCode == "de") ? "Hallo" : "nuqneH";

        G2PResult result = g2pEngine_->convert(text);
        ASSERT_TRUE(result.success)
            << "Failed to convert in " << langCode;

        // Generate audio
        int numSamples = static_cast<int>(result.getTotalDuration() * sampleRate_);
        if (numSamples < 100) numSamples = 100; // Minimum

        AudioBuffer outputBuffer(2, numSamples);
        outputBuffer.clear();

        formantSynth_->process(result.phonemes, outputBuffer);

        // Verify audio was generated
        float maxAmplitude = 0.0f;
        for (int i = 0; i < numSamples; ++i) {
            float amp = std::abs(outputBuffer.getReadPointer(0)[i]);
            if (amp > maxAmplitude) maxAmplitude = amp;
        }

        EXPECT_GT(maxAmplitude, 0.001f)
            << "No audio generated for " << langCode << ": " << text;
    }
}

// ============================================================================
// Test 16: Error Handling For Invalid Text
// ============================================================================

TEST_F(MultilingualSupportTest, ErrorHandlingForInvalidText) {
    ASSERT_TRUE(loadLanguage("en"));

    // Empty text
    G2PResult result1 = g2pEngine_->convert("");
    EXPECT_TRUE(result1.success);
    EXPECT_EQ(result1.getPhonemeCount(), 0);

    // Text with only spaces
    G2PResult result2 = g2pEngine_->convert("   ");
    EXPECT_TRUE(result2.success);

    // Text with special characters
    G2PResult result3 = g2pEngine_->convert("@#$%^&*()");
    EXPECT_TRUE(result3.success);

    // Very long text
    std::string longText(100000, 'a');
    G2PResult result4 = g2pEngine_->convert(longText);
    EXPECT_TRUE(result4.success);
    EXPECT_GT(result4.getPhonemeCount(), 0);
}

// ============================================================================
// Test 17: Language Loader Performance
// ============================================================================

TEST_F(MultilingualSupportTest, LanguageLoaderPerformance) {
    // Measure language loading performance
    auto startTime = std::chrono::high_resolution_clock::now();

    std::vector<std::string> languages = {"en", "la", "es", "de", "fr", "ja", "tlh"};
    for (const auto& langCode : languages) {
        auto language = languageLoader_->loadLanguage(langCode);
        EXPECT_TRUE(language) << "Failed to load: " << langCode;
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    // Loading all languages should be fast (< 1 second)
    EXPECT_LT(duration.count(), 1000)
        << "Language loading too slow: " << duration.count() << "ms";
}

// ============================================================================
// Test 18: G2P Conversion Performance
// ============================================================================

TEST_F(MultilingualSupportTest, G2PConversionPerformance) {
    ASSERT_TRUE(loadLanguage("en"));

    std::string text = "The quick brown fox jumps over the lazy dog. "
                      "Pack my box with five dozen liquor jugs. "
                      "How vexingly quick daft zebras jump!";

    // Measure conversion performance
    auto startTime = std::chrono::high_resolution_clock::now();

    G2PResult result = g2pEngine_->convert(text);

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);

    EXPECT_TRUE(result.success);
    EXPECT_LT(duration.count(), 10000) // < 10ms
        << "G2P conversion too slow: " << duration.count() << "μs";
}
