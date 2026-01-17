/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * test_language_loader.cpp - Unit tests for LanguageLoader component
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#include <gtest/gtest.h>
#include <core/PhonemeDatabase.h>
#include <core/LanguageLoader.h>
#include <filesystem>
#include <fstream>

using namespace ChoirV2;

class LanguageLoaderTest : public ::testing::Test {
protected:
    void SetUp() override {
        phoneme_db_ = std::make_shared<PhonemeDatabase>();
        loader_ = std::make_unique<LanguageLoader>(phoneme_db_);

        // Get the test directory
        test_dir_ = std::filesystem::current_path().parent_path().parent_path() / "languages";

        // Verify test languages exist
        ASSERT_TRUE(std::filesystem::exists(test_dir_ / "english.json"))
            << "English language file not found at: "
            << (test_dir_ / "english.json").string();
        ASSERT_TRUE(std::filesystem::exists(test_dir_ / "latin.json"))
            << "Latin language file not found at: "
            << (test_dir_ / "latin.json").string();
        ASSERT_TRUE(std::filesystem::exists(test_dir_ / "klingon.json"))
            << "Klingon language file not found at: "
            << (test_dir_ / "klingon.json").string();
        ASSERT_TRUE(std::filesystem::exists(test_dir_ / "throat_singing.json"))
            << "Throat singing language file not found at: "
            << (test_dir_ / "throat_singing.json").string();
    }

    void TearDown() override {
        loader_.reset();
        phoneme_db_.reset();
    }

    std::shared_ptr<PhonemeDatabase> phoneme_db_;
    std::unique_ptr<LanguageLoader> loader_;
    std::filesystem::path test_dir_;
};

// ============================================================================
// English Language Tests
// ============================================================================

TEST_F(LanguageLoaderTest, LoadEnglishLanguage_Success) {
    auto english_path = test_dir_ / "english.json";
    auto lang = loader_->loadLanguage(english_path.string());

    ASSERT_NE(lang, nullptr) << "Failed to load English language";
    EXPECT_EQ(lang->code, "english");
    EXPECT_EQ(lang->name, "English (US)");
    EXPECT_EQ(lang->version, "1.0");
}

TEST_F(LanguageLoaderTest, LoadEnglishLanguage_PhonemeCount) {
    auto english_path = test_dir_ / "english.json";
    auto lang = loader_->loadLanguage(english_path.string());

    ASSERT_NE(lang, nullptr);
    EXPECT_EQ(lang->phonemes.size(), 39) << "English should have 39 phonemes";
}

TEST_F(LanguageLoaderTest, LoadEnglishLanguage_HasRequiredPhonemes) {
    auto english_path = test_dir_ / "english.json";
    auto lang = loader_->loadLanguage(english_path.string());

    ASSERT_NE(lang, nullptr);

    // Check for key vowels
    bool has_aa = false, has_iy = false, has_uw = false;
    for (const auto& phoneme : lang->phonemes) {
        if (phoneme == "AA") has_aa = true;
        if (phoneme == "IY") has_iy = true;
        if (phoneme == "UW") has_uw = true;
    }

    EXPECT_TRUE(has_aa) << "English should have AA phoneme";
    EXPECT_TRUE(has_iy) << "English should have IY phoneme";
    EXPECT_TRUE(has_uw) << "English should have UW phoneme";
}

TEST_F(LanguageLoaderTest, LoadEnglishLanguage_G2PRulesPresent) {
    auto english_path = test_dir_ / "english.json";
    auto lang = loader_->loadLanguage(english_path.string());

    ASSERT_NE(lang, nullptr);
    EXPECT_FALSE(lang->g2p_rules.empty()) << "English should have G2P rules";

    // Check for common patterns
    EXPECT_NE(lang->g2p_rules.find("c[ei]"), std::string::npos)
        << "Should have soft C rule";
    EXPECT_NE(lang->g2p_rules.find("ph"), std::string::npos)
        << "Should have ph -> f rule";
}

TEST_F(LanguageLoaderTest, LoadEnglishLanguage_TestWords) {
    auto english_path = test_dir_ / "english.json";
    auto lang = loader_->loadLanguage(english_path.string());

    ASSERT_NE(lang, nullptr);
    EXPECT_FALSE(lang->test_words.empty()) << "English should have test words";

    // Check for common test words
    bool has_the = false, has_quick = false;
    for (const auto& word : lang->test_words) {
        if (word == "The") has_the = true;
        if (word == "quick") has_quick = true;
    }

    EXPECT_TRUE(has_the) << "Should have 'The' in test words";
    EXPECT_TRUE(has_quick) << "Should have 'quick' in test words";
}

// ============================================================================
// Latin Language Tests
// ============================================================================

TEST_F(LanguageLoaderTest, LoadLatinLanguage_Success) {
    auto latin_path = test_dir_ / "latin.json";
    auto lang = loader_->loadLanguage(latin_path.string());

    ASSERT_NE(lang, nullptr) << "Failed to load Latin language";
    EXPECT_EQ(lang->code, "latin");
    EXPECT_EQ(lang->name, "Latin (Ecclesiastical)");
    EXPECT_EQ(lang->version, "1.0");
}

TEST_F(LanguageLoaderTest, LoadLatinLanguage_PhonemeCount) {
    auto latin_path = test_dir_ / "latin.json";
    auto lang = loader_->loadLanguage(latin_path.string());

    ASSERT_NE(lang, nullptr);
    EXPECT_EQ(lang->phonemes.size(), 31) << "Latin should have 31 phonemes";
}

TEST_F(LanguageLoaderTest, LoadLatinLanguage_HasDiphthongs) {
    auto latin_path = test_dir_ / "latin.json";
    auto lang = loader_->loadLanguage(latin_path.string());

    ASSERT_NE(lang, nullptr);

    // Check for Latin diphthongs
    bool has_ae = false, has_oe = false, has_au = false;
    for (const auto& phoneme : lang->phonemes) {
        if (phoneme == "AE") has_ae = true;
        if (phoneme == "OE") has_oe = true;
        if (phoneme == "AU") has_au = true;
    }

    EXPECT_TRUE(has_ae) << "Latin should have AE diphthong";
    EXPECT_TRUE(has_oe) << "Latin should have OE diphthong";
    EXPECT_TRUE(has_au) << "Latin should have AU diphthong";
}

TEST_F(LanguageLoaderTest, LoadLatinLanguage_SoftAndHardConsonants) {
    auto latin_path = test_dir_ / "latin.json";
    auto lang = loader_->loadLanguage(latin_path.string());

    ASSERT_NE(lang, nullptr);

    // Check for C and G variants
    bool has_c_hard = false, has_c_soft = false;
    bool has_g_hard = false, has_g_soft = false;

    for (const auto& phoneme : lang->phonemes) {
        if (phoneme == "C_HARD") has_c_hard = true;
        if (phoneme == "C_SOFT") has_c_soft = true;
        if (phoneme == "G_HARD") has_g_hard = true;
        if (phoneme == "G_SOFT") has_g_soft = true;
    }

    EXPECT_TRUE(has_c_hard) << "Latin should have C_HARD";
    EXPECT_TRUE(has_c_soft) << "Latin should have C_SOFT";
    EXPECT_TRUE(has_g_hard) << "Latin should have G_HARD";
    EXPECT_TRUE(has_g_soft) << "Latin should have G_SOFT";
}

TEST_F(LanguageLoaderTest, LoadLatinLanguage_TestWords) {
    auto latin_path = test_dir_ / "latin.json";
    auto lang = loader_->loadLanguage(latin_path.string());

    ASSERT_NE(lang, nullptr);
    EXPECT_FALSE(lang->test_words.empty()) << "Latin should have test words";

    // Check for liturgical test words
    bool has_pater = false, has_kyrie = false;
    for (const auto& word : lang->test_words) {
        if (word == "Pater") has_pater = true;
        if (word == "Kyrie") has_kyrie = true;
    }

    EXPECT_TRUE(has_pater) << "Should have 'Pater' in test words";
    EXPECT_TRUE(has_kyrie) << "Should have 'Kyrie' in test words";
}

// ============================================================================
// Klingon Language Tests
// ============================================================================

TEST_F(LanguageLoaderTest, LoadKlingonLanguage_Success) {
    auto klingon_path = test_dir_ / "klingon.json";
    auto lang = loader_->loadLanguage(klingon_path.string());

    ASSERT_NE(lang, nullptr) << "Failed to load Klingon language";
    EXPECT_EQ(lang->code, "klingon");
    EXPECT_EQ(lang->name, "Klingon (tlhIngan Hol)");
    EXPECT_EQ(lang->version, "1.0");
}

TEST_F(LanguageLoaderTest, LoadKlingonLanguage_PhonemeCount) {
    auto klingon_path = test_dir_ / "klingon.json";
    auto lang = loader_->loadLanguage(klingon_path.string());

    ASSERT_NE(lang, nullptr);
    EXPECT_EQ(lang->phonemes.size(), 25) << "Klingon should have 25 phonemes";
}

TEST_F(LanguageLoaderTest, LoadKlingonLanguage_UniqueSounds) {
    auto klingon_path = test_dir_ / "klingon.json";
    auto lang = loader_->loadLanguage(klingon_path.string());

    ASSERT_NE(lang, nullptr);

    // Check for unique Klingon sounds
    bool has_tlh = false, has_Q = false, has_S = false, has_D = false;
    for (const auto& phoneme : lang->phonemes) {
        if (phoneme == "tlh") has_tlh = true;
        if (phoneme == "Q") has_Q = true;
        if (phoneme == "S") has_S = true;
        if (phoneme == "D") has_D = true;
    }

    EXPECT_TRUE(has_tlh) << "Klingon should have tlh phoneme";
    EXPECT_TRUE(has_Q) << "Klingon should have Q phoneme";
    EXPECT_TRUE(has_S) << "Klingon should have S phoneme";
    EXPECT_TRUE(has_D) << "Klingon should have D phoneme";
}

TEST_F(LanguageLoaderTest, LoadKlingonLanguage_GutturalEmphasis) {
    auto klingon_path = test_dir_ / "klingon.json";
    auto lang = loader_->loadLanguage(klingon_path.string());

    ASSERT_NE(lang, nullptr);

    // Check for uvular and guttural sounds
    bool has_q = false, has_gh = false, has_H = false;
    for (const auto& phoneme : lang->phonemes) {
        if (phoneme == "q") has_q = true;
        if (phoneme == "gh") has_gh = true;
        if (phoneme == "H") has_H = true;
    }

    EXPECT_TRUE(has_q) << "Klingon should have q phoneme";
    EXPECT_TRUE(has_gh) << "Klingon should have gh phoneme";
    EXPECT_TRUE(has_H) << "Klingon should have H phoneme";
}

TEST_F(LanguageLoaderTest, LoadKlingonLanguage_TestWords) {
    auto klingon_path = test_dir_ / "klingon.json";
    auto lang = loader_->loadLanguage(klingon_path.string());

    ASSERT_NE(lang, nullptr);
    EXPECT_FALSE(lang->test_words.empty()) << "Klingon should have test words";

    // Check for canonical Klingon words
    bool has_tlhingan = false, has_qapla = false;
    for (const auto& word : lang->test_words) {
        if (word == "tlhIngan") has_tlhingan = true;
        if (word == "Qapla'") has_qapla = true;
    }

    EXPECT_TRUE(has_tlhingan) << "Should have 'tlhIngan' in test words";
    EXPECT_TRUE(has_qapla) << "Should have 'Qapla'' in test words";
}

// ============================================================================
// Throat Singing Tests
// ============================================================================

TEST_F(LanguageLoaderTest, LoadThroatSinging_Success) {
    auto throat_path = test_dir_ / "throat_singing.json";
    auto lang = loader_->loadLanguage(throat_path.string());

    ASSERT_NE(lang, nullptr) << "Failed to load throat singing language";
    EXPECT_EQ(lang->code, "throat_singing");
    EXPECT_EQ(lang->name, "Throat Singing (Vocal Technique)");
}

TEST_F(LanguageLoaderTest, LoadThroatSinging_PhonemeCount) {
    auto throat_path = test_dir_ / "throat_singing.json";
    auto lang = loader_->loadLanguage(throat_path.string());

    ASSERT_NE(lang, nullptr);
    EXPECT_EQ(lang->phonemes.size(), 8) << "Throat singing should have 8 phonemes";
}

TEST_F(LanguageLoaderTest, LoadThroatSinging_SubharmonicMethod) {
    auto throat_path = test_dir_ / "throat_singing.json";
    auto lang = loader_->loadLanguage(throat_path.string());

    ASSERT_NE(lang, nullptr);

    // Check for technique parameters
    EXPECT_FALSE(lang->technique_params.empty())
        << "Throat singing should have technique parameters";

    // Verify subharmonic method is specified
    EXPECT_NE(lang->technique_params.find("subharmonic"), std::string::npos)
        << "Should specify subharmonic synthesis method";
}

TEST_F(LanguageLoaderTest, LoadThroatSinging_Styles) {
    auto throat_path = test_dir_ / "throat_singing.json";
    auto lang = loader_->loadLanguage(throat_path.string());

    ASSERT_NE(lang, nullptr);

    // Check for different throat singing styles
    bool has_drone = false, has_sygyt = false, has_kargyraa = false, has_khoomei = false;
    for (const auto& phoneme : lang->phonemes) {
        if (phoneme == "drone_fundamental") has_drone = true;
        if (phoneme == "sygyt_whistle") has_sygyt = true;
        if (phoneme == "kargyraa_bass") has_kargyraa = true;
        if (phoneme == "khoomei_mid") has_khoomei = true;
    }

    EXPECT_TRUE(has_drone) << "Should have drone_fundamental";
    EXPECT_TRUE(has_sygyt) << "Should have sygyt_whistle";
    EXPECT_TRUE(has_kargyraa) << "Should have kargyraa_bass";
    EXPECT_TRUE(has_khoomei) << "Should have khoomei_mid";
}

TEST_F(LanguageLoaderTest, LoadThroatSinging_Presets) {
    auto throat_path = test_dir_ / "throat_singing.json";
    auto lang = loader_->loadLanguage(throat_path.string());

    ASSERT_NE(lang, nullptr);

    // Verify presets are loaded
    EXPECT_FALSE(lang->technique_params.empty())
        << "Should have preset definitions";

    // Check for style-specific presets
    EXPECT_NE(lang->technique_params.find("tibetan_sygyt"), std::string::npos)
        << "Should have Tibetan Sygyt preset";
    EXPECT_NE(lang->technique_params.find("tuva_kargyraa"), std::string::npos)
        << "Should have Tuva Kargyraa preset";
    EXPECT_NE(lang->technique_params.find("mongolian_khoomei"), std::string::npos)
        << "Should have Mongolian Khoomei preset";
}

// ============================================================================
// Caching and Performance Tests
// ============================================================================

TEST_F(LanguageLoaderTest, CachePerformance_MultipleLoads) {
    auto english_path = test_dir_ / "english.json";

    // Load English twice
    auto lang1 = loader_->loadLanguage(english_path.string());
    auto lang2 = loader_->loadLanguage(english_path.string());

    ASSERT_NE(lang1, nullptr);
    ASSERT_NE(lang2, nullptr);

    // Should return same cached instance
    EXPECT_EQ(lang1.get(), lang2.get())
        << "Multiple loads should return cached instance";
}

TEST_F(LanguageLoaderTest, GetAvailableLanguages_AllFour) {
    // Load all languages
    loader_->loadLanguage((test_dir_ / "english.json").string());
    loader_->loadLanguage((test_dir_ / "latin.json").string());
    loader_->loadLanguage((test_dir_ / "klingon.json").string());
    loader_->loadLanguage((test_dir_ / "throat_singing.json").string());

    auto languages = loader_->getAvailableLanguages();

    EXPECT_EQ(languages.size(), 4) << "Should have 4 languages loaded";

    // Check for all languages
    bool has_english = false, has_latin = false;
    bool has_klingon = false, has_throat = false;

    for (const auto& [code, name] : languages) {
        if (code == "english") has_english = true;
        if (code == "latin") has_latin = true;
        if (code == "klingon") has_klingon = true;
        if (code == "throat_singing") has_throat = true;
    }

    EXPECT_TRUE(has_english) << "Should have English";
    EXPECT_TRUE(has_latin) << "Should have Latin";
    EXPECT_TRUE(has_klingon) << "Should have Klingon";
    EXPECT_TRUE(has_throat) << "Should have throat_singing";
}

TEST_F(LanguageLoaderTest, ReloadLanguage_Success) {
    auto english_path = test_dir_ / "english.json";

    // Load English
    auto lang1 = loader_->loadLanguage(english_path.string());
    ASSERT_NE(lang1, nullptr);

    // Reload
    bool reloaded = loader_->reloadLanguage(english_path.string());
    EXPECT_TRUE(reloaded) << "Reload should succeed";

    auto lang2 = loader_->loadLanguage(english_path.string());
    ASSERT_NE(lang2, nullptr);

    // Should be different instance after reload
    EXPECT_NE(lang1.get(), lang2.get())
        << "Reload should create new instance";
}

// ============================================================================
// Error Handling Tests
// ============================================================================

TEST_F(LanguageLoaderTest, LoadNonExistentFile_ReturnsNull) {
    auto lang = loader_->loadLanguage("languages/nonexistent.json");
    EXPECT_EQ(lang, nullptr) << "Should return nullptr for missing file";
}

TEST_F(LanguageLoaderTest, LoadInvalidJSON_ReturnsNull) {
    // Create invalid JSON file
    auto invalid_path = test_dir_ / "invalid.json";
    std::ofstream file(invalid_path);
    file << "{ invalid json content";
    file.close();

    auto lang = loader_->loadLanguage(invalid_path.string());
    EXPECT_EQ(lang, nullptr) << "Should return nullptr for invalid JSON";

    // Cleanup
    std::filesystem::remove(invalid_path);
}

TEST_F(LanguageLoaderTest, LoadMissingRequiredFields_ReturnsNull) {
    // Create minimal JSON missing required fields
    auto minimal_path = test_dir_ / "minimal.json";
    std::ofstream file(minimal_path);
    file << R"({
      "language_name": "Minimal Language"
    })";
    file.close();

    auto lang = loader_->loadLanguage(minimal_path.string());
    EXPECT_EQ(lang, nullptr) << "Should return nullptr when missing required fields";

    // Cleanup
    std::filesystem::remove(minimal_path);
}

// ============================================================================
// Synthesis Parameters Tests
// ============================================================================

TEST_F(LanguageLoaderTest, EnglishSynthesisParameters_DefaultPitch) {
    auto english_path = test_dir_ / "english.json";
    auto lang = loader_->loadLanguage(english_path.string());

    ASSERT_NE(lang, nullptr);
    ASSERT_FALSE(lang->pitch_contours.empty());

    EXPECT_FLOAT_EQ(lang->pitch_contours[0], 60.0f)
        << "English default pitch should be 60.0 Hz";
}

TEST_F(LanguageLoaderTest, LatinSynthesisParameters_ChoralResonance) {
    auto latin_path = test_dir_ / "latin.json";
    auto lang = loader_->loadLanguage(latin_path.string());

    ASSERT_NE(lang, nullptr);

    // Latin should have choral resonance parameters
    EXPECT_FLOAT_EQ(lang->pitch_contours[0], 65.0f)
        << "Latin default pitch should be 65.0 Hz";
}

TEST_F(LanguageLoaderTest, KlingonSynthesisParameters_Guttural) {
    auto klingon_path = test_dir_ / "klingon.json";
    auto lang = loader_->loadLanguage(klingon_path.string());

    ASSERT_NE(lang, nullptr);

    // Klingon should have guttural emphasis
    EXPECT_FLOAT_EQ(lang->pitch_contours[0], 55.0f)
        << "Klingon default pitch should be 55.0 Hz (lower)";
}

TEST_F(LanguageLoaderTest, ThroatSingingSynthesisParameters_Subharmonic) {
    auto throat_path = test_dir_ / "throat_singing.json";
    auto lang = loader_->loadLanguage(throat_path.string());

    ASSERT_NE(lang, nullptr);

    // Throat singing should use very low pitch
    EXPECT_FLOAT_EQ(lang->pitch_contours[0], 55.0f)
        << "Throat singing default pitch should be 55.0 Hz";
}

// ============================================================================
// Phonotactics Tests
// ============================================================================

TEST_F(LanguageLoaderTest, EnglishSyllableStructure_DefaultCV) {
    auto english_path = test_dir_ / "english.json";
    auto lang = loader_->loadLanguage(english_path.string());

    ASSERT_NE(lang, nullptr);
    EXPECT_EQ(lang->syllable_structure, "(C)V(C)")
        << "English should use default (C)V(C) structure";
}

TEST_F(LanguageLoaderTest, KlingonSyllableStructure_ComplexClusters) {
    auto klingon_path = test_dir_ / "klingon.json";
    auto lang = loader_->loadLanguage(klingon_path.string());

    ASSERT_NE(lang, nullptr);
    // Klingon allows complex consonant clusters
    EXPECT_FALSE(lang->onset_clusters.empty() || lang->syllable_structure == "(C)V(C)")
        << "Klingon should support complex clusters";
}

// ============================================================================
// Main Test Runner
// ============================================================================

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
