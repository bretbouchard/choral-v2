/*
  ==============================================================================

    test_phonedatabase_puredsp.cpp
    Created: January 18, 2026
    Author:  Bret Bouchard

    Unit tests for PureDSP PhonemeDatabase

  ==============================================================================
*/

#include <gtest/gtest.h>
#include "dsp/core/PhonemeDatabase.h"
#include <fstream>
#include <cstdio>

using namespace DSP;

class PhonemeDatabaseTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a test JSON file
        test_json_file_ = "/tmp/test_phonemes.json";

        std::ofstream file(test_json_file_);
        file << R"({
  "phonemes": {
    "AA": {
      "ipa": "/ɑ/",
      "category": "vowel",
      "formants": {
        "frequencies": [750, 1150, 2500, 3500],
        "bandwidths": [80, 90, 120, 130]
      },
      "articulatory": {
        "is_nasal": false,
        "is_rounded": true,
        "is_voiced": true
      },
      "temporal": {
        "min_duration": 50,
        "max_duration": 300,
        "default_duration": 150
      }
    },
    "IY": {
      "ipa": "/i/",
      "category": "vowel",
      "formants": {
        "frequencies": [300, 2200, 3000, 3500],
        "bandwidths": [50, 80, 100, 120]
      },
      "articulatory": {
        "is_nasal": false,
        "is_rounded": false,
        "is_voiced": true
      },
      "temporal": {
        "min_duration": 40,
        "max_duration": 250,
        "default_duration": 100
      }
    }
  }
})";
        file.close();
    }

    void TearDown() override {
        // Clean up test file
        std::remove(test_json_file_.c_str());
    }

    std::string test_json_file_;
};

TEST_F(PhonemeDatabaseTest, LoadLanguage) {
    PhonemeDatabase db;

    // Load language file
    bool success = db.loadLanguage(test_json_file_);
    EXPECT_TRUE(success);
    EXPECT_EQ(db.size(), 2);
}

TEST_F(PhonemeDatabaseTest, GetPhonemeBySymbol) {
    PhonemeDatabase db;
    db.loadLanguage(test_json_file_);

    // Get phoneme by symbol
    auto phoneme = db.getPhoneme("AA");
    ASSERT_NE(phoneme, nullptr);
    EXPECT_EQ(phoneme->id, "AA");
    EXPECT_EQ(phoneme->ipa, "/ɑ/");
    EXPECT_EQ(phoneme->category, PhonemeCategory::Vowel);
}

TEST_F(PhonemeDatabaseTest, GetPhonemeByIPA) {
    PhonemeDatabase db;
    db.loadLanguage(test_json_file_);

    // Get phoneme by IPA
    auto phoneme = db.getPhonemeByIPA("/i/");
    ASSERT_NE(phoneme, nullptr);
    EXPECT_EQ(phoneme->id, "IY");
    EXPECT_EQ(phoneme->ipa, "/i/");
}

TEST_F(PhonemeDatabaseTest, GetByCategory) {
    PhonemeDatabase db;
    db.loadLanguage(test_json_file_);

    // Get all vowels
    auto vowels = db.getByCategory("vowel");
    EXPECT_EQ(vowels.size(), 2);
}

TEST_F(PhonemeDatabaseTest, FormantData) {
    PhonemeDatabase db;
    db.loadLanguage(test_json_file_);

    auto phoneme = db.getPhoneme("AA");
    ASSERT_NE(phoneme, nullptr);

    // Check formant frequencies
    EXPECT_FLOAT_EQ(phoneme->formants.frequencies[0], 750.0f);
    EXPECT_FLOAT_EQ(phoneme->formants.frequencies[1], 1150.0f);
    EXPECT_FLOAT_EQ(phoneme->formants.frequencies[2], 2500.0f);
    EXPECT_FLOAT_EQ(phoneme->formants.frequencies[3], 3500.0f);

    // Check formant bandwidths
    EXPECT_FLOAT_EQ(phoneme->formants.bandwidths[0], 80.0f);
    EXPECT_FLOAT_EQ(phoneme->formants.bandwidths[1], 90.0f);
    EXPECT_FLOAT_EQ(phoneme->formants.bandwidths[2], 120.0f);
    EXPECT_FLOAT_EQ(phoneme->formants.bandwidths[3], 130.0f);
}

TEST_F(PhonemeDatabaseTest, DiphoneInterpolation) {
    PhonemeDatabase db;
    db.loadLanguage(test_json_file_);

    auto aa = db.getPhoneme("AA");
    auto iy = db.getPhoneme("IY");

    ASSERT_NE(aa, nullptr);
    ASSERT_NE(iy, nullptr);

    // Create diphone at midpoint
    auto diphone = db.createDiphone(*aa, *iy, 0.5f);

    // Check interpolation (should be average)
    float expected_f0 = (aa->formants.frequencies[0] + iy->formants.frequencies[0]) / 2.0f;
    EXPECT_FLOAT_EQ(diphone.frequencies[0], expected_f0);
}

TEST_F(PhonemeDatabaseTest, ArticulatoryFeatures) {
    PhonemeDatabase db;
    db.loadLanguage(test_json_file_);

    auto phoneme = db.getPhoneme("AA");
    ASSERT_NE(phoneme, nullptr);

    // Check articulatory features
    EXPECT_FALSE(phoneme->articulatory.is_nasal);
    EXPECT_TRUE(phoneme->articulatory.is_rounded);
    EXPECT_TRUE(phoneme->articulatory.is_voiced);
}

TEST_F(PhonemeDatabaseTest, TemporalFeatures) {
    PhonemeDatabase db;
    db.loadLanguage(test_json_file_);

    auto phoneme = db.getPhoneme("AA");
    ASSERT_NE(phoneme, nullptr);

    // Check temporal features
    EXPECT_EQ(phoneme->temporal.min_duration, 50);
    EXPECT_EQ(phoneme->temporal.max_duration, 300);
    EXPECT_EQ(phoneme->temporal.default_duration, 150);
}

TEST_F(PhonemeDatabaseTest, HasPhoneme) {
    PhonemeDatabase db;
    db.loadLanguage(test_json_file_);

    EXPECT_TRUE(db.hasPhoneme("AA"));
    EXPECT_TRUE(db.hasPhoneme("IY"));
    EXPECT_FALSE(db.hasPhoneme("XX"));
}

TEST_F(PhonemeDatabaseTest, GetCategories) {
    PhonemeDatabase db;
    db.loadLanguage(test_json_file_);

    auto categories = db.getCategories();
    EXPECT_EQ(categories.size(), 1);
    EXPECT_EQ(categories[0], "vowel");
}

TEST_F(PhonemeDatabaseTest, GetAllPhonemes) {
    PhonemeDatabase db;
    db.loadLanguage(test_json_file_);

    auto all = db.getAllPhonemes();
    EXPECT_EQ(all.size(), 2);
}

TEST_F(PhonemeDatabaseTest, Clear) {
    PhonemeDatabase db;
    db.loadLanguage(test_json_file_);

    EXPECT_EQ(db.size(), 2);

    db.clear();

    EXPECT_EQ(db.size(), 0);
    EXPECT_FALSE(db.hasPhoneme("AA"));
}

TEST_F(PhonemeDatabaseTest, PhonemeCategoryConversion) {
    EXPECT_EQ(phonemeCategoryToString(PhonemeCategory::Vowel), "vowel");
    EXPECT_EQ(phonemeCategoryToString(PhonemeCategory::Consonant), "consonant");

    EXPECT_EQ(stringToPhonemeCategory("vowel"), PhonemeCategory::Vowel);
    EXPECT_EQ(stringToPhonemeCategory("consonant"), PhonemeCategory::Consonant);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
