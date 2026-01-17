/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * test_preset_manager.cpp - Unit tests for PresetManager
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#include <gtest/gtest.h>
#include "core/PresetManager.h"
#include <fstream>
#include <filesystem>
#include <random>

using namespace ChoirV2;

class PresetManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create temporary directories for testing
        test_dir_ = std::filesystem::temp_directory_path() /
                    "choir_v2_test_" + std::to_string(std::random_device{}());

        std::filesystem::create_directories(test_dir_);
        std::filesystem::create_directories(test_dir_ / "factory");
        std::filesystem::create_directories(test_dir_ / "user");

        // Create test factory presets
        createTestFactoryPresets();

        // Initialize preset manager
        manager_ = std::make_unique<PresetManager>();
        manager_->initialize(
            (test_dir_ / "user").string(),
            (test_dir_ / "factory").string()
        );
    }

    void TearDown() override {
        manager_->shutdown();
        std::filesystem::remove_all(test_dir_);
    }

    void createTestFactoryPresets() {
        // Create a simple test factory preset
        Preset test_preset;
        test_preset.metadata.name = "Test Factory Preset";
        test_preset.metadata.author = "White Room";
        test_preset.metadata.description = "Test factory preset";
        test_preset.metadata.version = "2.0.0";
        test_preset.metadata.category = "Factory";
        test_preset.is_factory = true;
        test_preset.is_read_only = true;

        // Set default parameters
        test_preset.parameters = PresetParameters{};

        // Save to factory directory
        std::string file_path = (test_dir_ / "factory" / "Test Factory Preset.choirv2").string();
        savePresetToFile(test_preset, file_path);
    }

    void savePresetToFile(const Preset& preset, const std::string& file_path) {
        // Simple JSON serialization
        std::ofstream file(file_path);
        file << "{\n";
        file << "  \"format\": \"choir-v2-preset\",\n";
        file << "  \"version\": \"2.0.0\",\n";
        file << "  \"metadata\": {\n";
        file << "    \"name\": \"" << preset.metadata.name << "\",\n";
        file << "    \"author\": \"" << preset.metadata.author << "\",\n";
        file << "    \"description\": \"" << preset.metadata.description << "\",\n";
        file << "    \"version\": \"" << preset.metadata.version << "\",\n";
        file << "    \"category\": \"" << preset.metadata.category << "\",\n";
        file << "    \"tags\": [],\n";
        file << "    \"createdDate\": \"" << preset.metadata.created_date << "\",\n";
        file << "    \"modifiedDate\": \"" << preset.metadata.modified_date << "\",\n";
        file << "    \"pluginVersion\": \"2.0.0\"\n";
        file << "  },\n";
        file << "  \"parameters\": {\n";
        file << "    \"language\": \"" << preset.parameters.language << "\",\n";
        file << "    \"lyrics\": \"" << preset.parameters.lyrics << "\",\n";
        file << "    \"numVoices\": " << preset.parameters.num_voices << ",\n";
        file << "    \"masterGain\": " << preset.parameters.master_gain << ",\n";
        file << "    \"synthesisMethod\": \"" << preset.parameters.synthesis_method << "\",\n";
        file << "    \"formantMix\": " << preset.parameters.formant_mix << ",\n";
        file << "    \"subharmonicMix\": " << preset.parameters.subharmonic_mix << ",\n";
        file << "    \"stereoWidth\": " << preset.parameters.stereo_width << ",\n";
        file << "    \"vibratoRate\": " << preset.parameters.vibrato_rate << ",\n";
        file << "    \"vibratoDepth\": " << preset.parameters.vibrato_depth << ",\n";
        file << "    \"reverbMix\": " << preset.parameters.reverb_mix << ",\n";
        file << "    \"reverbSize\": " << preset.parameters.reverb_size << ",\n";
        file << "    \"attackTime\": " << preset.parameters.attack_time << ",\n";
        file << "    \"releaseTime\": " << preset.parameters.release_time << ",\n";
        file << "    \"enableAntiAliasing\": " << (preset.parameters.enable_anti_aliasing ? "true" : "false") << ",\n";
        file << "    \"enableSpectralEnhancement\": " << (preset.parameters.enable_spectral_enhancement ? "true" : "false") << ",\n";
        file << "    \"oversamplingFactor\": " << preset.parameters.oversampling_factor << ",\n";
        file << "    \"pitchVariation\": " << preset.parameters.pitch_variation << ",\n";
        file << "    \"timingVariation\": " << preset.parameters.timing_variation << ",\n";
        file << "    \"formantVariation\": " << preset.parameters.formant_variation << ",\n";
        file << "    \"breathiness\": " << preset.parameters.breathiness << ",\n";
        file << "    \"warmth\": " << preset.parameters.warmth << ",\n";
        file << "    \"brightness\": " << preset.parameters.brightness << "\n";
        file << "  }\n";
        file << "}\n";
    }

    Preset createTestPreset(const std::string& name) {
        Preset preset;
        preset.metadata.name = name;
        preset.metadata.author = "Test Author";
        preset.metadata.description = "Test description";
        preset.metadata.version = "2.0.0";
        preset.metadata.category = "User";
        preset.metadata.created_date = "2026-01-17T00:00:00Z";
        preset.metadata.modified_date = "2026-01-17T00:00:00Z";
        preset.metadata.plugin_version = "2.0.0";

        preset.parameters = PresetParameters{};
        preset.parameters.num_voices = 16;
        preset.parameters.language = "english";
        preset.parameters.lyrics = "test lyrics";

        return preset;
    }

    std::filesystem::path test_dir_;
    std::unique_ptr<PresetManager> manager_;
};

// ========== Initialization Tests ==========

TEST_F(PresetManagerTest, InitializeCreatesDirectories) {
    EXPECT_TRUE(std::filesystem::exists(test_dir_ / "user"));
    EXPECT_TRUE(std::filesystem::exists(test_dir_ / "factory"));
}

TEST_F(PresetManagerTest, InitializeLoadsFactoryPresets) {
    auto factory_presets = manager_->getFactoryPresets();
    EXPECT_GT(factory_presets.size(), 0);
}

// ========== Preset Loading & Saving Tests ==========

TEST_F(PresetManagerTest, LoadValidPreset) {
    Preset preset = createTestPreset("Load Test");
    std::string file_path = (test_dir_ / "load_test.choirv2").string();
    savePresetToFile(preset, file_path);

    auto loaded_preset = manager_->loadPreset(file_path);
    ASSERT_NE(loaded_preset, nullptr);
    EXPECT_EQ(loaded_preset->metadata.name, "Load Test");
    EXPECT_EQ(loaded_preset->parameters.num_voices, 16);
}

TEST_F(PresetManagerTest, LoadInvalidPreset) {
    std::string file_path = (test_dir_ / "invalid.choirv2").string();
    {
        std::ofstream file(file_path);
        file << "invalid json";
    }

    auto loaded_preset = manager_->loadPreset(file_path);
    EXPECT_EQ(loaded_preset, nullptr);
}

TEST_F(PresetManagerTest, LoadNonexistentPreset) {
    auto loaded_preset = manager_->loadPreset("nonexistent.choirv2");
    EXPECT_EQ(loaded_preset, nullptr);
}

TEST_F(PresetManagerTest, SaveAndLoadPreset) {
    Preset preset = createTestPreset("Save Load Test");
    std::string file_path = (test_dir_ / "save_load_test.choirv2").string();

    EXPECT_TRUE(manager_->savePreset(preset, file_path));

    auto loaded_preset = manager_->loadPreset(file_path);
    ASSERT_NE(loaded_preset, nullptr);
    EXPECT_EQ(loaded_preset->metadata.name, "Save Load Test");
    EXPECT_EQ(loaded_preset->parameters.num_voices, 16);
}

// ========== Factory Presets Tests ==========

TEST_F(PresetManagerTest, GetFactoryPresets) {
    auto factory_presets = manager_->getFactoryPresets();
    EXPECT_GT(factory_presets.size(), 0);
}

TEST_F(PresetManagerTest, GetFactoryPresetByName) {
    auto preset = manager_->getFactoryPreset("Test Factory Preset");
    ASSERT_NE(preset, nullptr);
    EXPECT_EQ(preset->metadata.name, "Test Factory Preset");
    EXPECT_TRUE(preset->is_factory);
    EXPECT_TRUE(preset->is_read_only);
}

TEST_F(PresetManagerTest, GetNonexistentFactoryPreset) {
    auto preset = manager_->getFactoryPreset("Nonexistent Preset");
    EXPECT_EQ(preset, nullptr);
}

// ========== User Presets Tests ==========

TEST_F(PresetManagerTest, SaveUserPreset) {
    Preset preset = createTestPreset("User Preset Test");

    EXPECT_TRUE(manager_->saveUserPreset(preset));

    auto user_presets = manager_->getUserPresets();
    EXPECT_GT(user_presets.size(), 0);
}

TEST_F(PresetManagerTest, DeleteUserPreset) {
    Preset preset = createTestPreset("Delete Test");
    manager_->saveUserPreset(preset);

    EXPECT_TRUE(manager_->deleteUserPreset("Delete Test"));

    auto user_presets = manager_->getUserPresets();
    bool found = false;
    for (const auto& p : user_presets) {
        if (p.metadata.name == "Delete Test") {
            found = true;
            break;
        }
    }
    EXPECT_FALSE(found);
}

TEST_F(PresetManagerTest, RenameUserPreset) {
    Preset preset = createTestPreset("Old Name");
    manager_->saveUserPreset(preset);

    EXPECT_TRUE(manager_->renameUserPreset("Old Name", "New Name"));

    auto old_preset = manager_->getFactoryPreset("Old Name");
    EXPECT_EQ(old_preset, nullptr);

    auto user_presets = manager_->getUserPresets();
    bool found = false;
    for (const auto& p : user_presets) {
        if (p.metadata.name == "New Name") {
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found);
}

TEST_F(PresetManagerTest, DuplicateUserPreset) {
    Preset preset = createTestPreset("Original");
    manager_->saveUserPreset(preset);

    auto duplicated = manager_->duplicateUserPreset("Original", "Copy");
    ASSERT_NE(duplicated, nullptr);
    EXPECT_EQ(duplicated->metadata.name, "Copy");

    auto user_presets = manager_->getUserPresets();
    int count = 0;
    for (const auto& p : user_presets) {
        if (p.metadata.name == "Original" || p.metadata.name == "Copy") {
            count++;
        }
    }
    EXPECT_EQ(count, 2);
}

// ========== Preset Browsing Tests ==========

TEST_F(PresetManagerTest, SearchPresetsByName) {
    Preset preset = createTestPreset("Searchable Name");
    manager_->saveUserPreset(preset);

    auto results = manager_->searchPresets("Searchable");
    EXPECT_GT(results.size(), 0);
}

TEST_F(PresetManagerTest, SearchPresetsByDescription) {
    Preset preset = createTestPreset("Search Test");
    preset.metadata.description = "This is a searchable description";
    manager_->saveUserPreset(preset);

    auto results = manager_->searchPresets("searchable");
    EXPECT_GT(results.size(), 0);
}

TEST_F(PresetManagerTest, SearchPresetsByTag) {
    Preset preset = createTestPreset("Tagged Preset");
    preset.metadata.tags = {"test", "searchable", "custom"};
    manager_->saveUserPreset(preset);

    auto results = manager_->searchPresets("searchable");
    EXPECT_GT(results.size(), 0);
}

TEST_F(PresetManagerTest, GetPresetsByCategory) {
    auto factory_presets = manager_->getPresetsByCategory(PresetCategory::Factory);
    EXPECT_GT(factory_presets.size(), 0);
}

TEST_F(PresetManagerTest, GetAllTags) {
    Preset preset = createTestPreset("Tagged Preset");
    preset.metadata.tags = {"tag1", "tag2", "tag3"};
    manager_->saveUserPreset(preset);

    auto tags = manager_->getAllTags();
    EXPECT_GT(tags.size(), 0);
}

// ========== Import/Export Tests ==========

TEST_F(PresetManagerTest, ImportPreset) {
    Preset preset = createTestPreset("Imported Preset");
    std::string file_path = (test_dir_ / "import_test.choirv2").string();
    savePresetToFile(preset, file_path);

    auto imported = manager_->importPreset(file_path);
    ASSERT_NE(imported, nullptr);
    EXPECT_EQ(imported->metadata.name, "Imported Preset");
}

TEST_F(PresetManagerTest, ExportPreset) {
    Preset preset = createTestPreset("Exported Preset");
    std::string file_path = (test_dir_ / "export_test.choirv2").string();

    EXPECT_TRUE(manager_->exportPreset(preset, file_path));
    EXPECT_TRUE(std::filesystem::exists(file_path));
}

// ========== Validation Tests ==========

TEST_F(PresetManagerTest, ValidateValidPreset) {
    Preset preset = createTestPreset("Valid Preset");

    auto result = manager_->validatePreset(preset);
    EXPECT_TRUE(result.is_valid);
    EXPECT_TRUE(result.is_compatible);
}

TEST_F(PresetManagerTest, ValidateInvalidPreset) {
    Preset preset;
    preset.metadata.name = ""; // Invalid: empty name
    preset.parameters.num_voices = 100; // Invalid: > 60

    auto result = manager_->validatePreset(preset);
    EXPECT_FALSE(result.is_valid);
    EXPECT_GT(result.errors.size(), 0);
}

TEST_F(PresetManagerTest, ValidatePresetFile) {
    Preset preset = createTestPreset("File Validation Test");
    std::string file_path = (test_dir_ / "validate_test.choirv2").string();
    savePresetToFile(preset, file_path);

    auto result = manager_->validatePresetFile(file_path);
    EXPECT_TRUE(result.is_valid);
    EXPECT_TRUE(result.is_compatible);
}

// ========== Favorites & Recent Tests ==========

TEST_F(PresetManagerTest, AddFavorite) {
    EXPECT_TRUE(manager_->addFavorite("Test Factory Preset"));
    EXPECT_TRUE(manager_->isFavorite("Test Factory Preset"));
}

TEST_F(PresetManagerTest, RemoveFavorite) {
    manager_->addFavorite("Test Factory Preset");
    EXPECT_TRUE(manager_->removeFavorite("Test Factory Preset"));
    EXPECT_FALSE(manager_->isFavorite("Test Factory Preset"));
}

TEST_F(PresetManagerTest, GetFavorites) {
    manager_->addFavorite("Test Factory Preset");

    auto favorites = manager_->getFavorites();
    EXPECT_GT(favorites.size(), 0);
}

TEST_F(PresetManagerTest, AddToRecent) {
    Preset preset = createTestPreset("Recent Preset");
    manager_->addToRecent(preset);

    auto recent = manager_->getRecentPresets(10);
    EXPECT_GT(recent.size(), 0);
}

TEST_F(PresetManagerTest, RecentPresetsLimit) {
    for (int i = 0; i < 20; i++) {
        Preset preset = createTestPreset("Recent " + std::to_string(i));
        manager_->addToRecent(preset);
    }

    auto recent = manager_->getRecentPresets(10);
    EXPECT_LE(recent.size(), 10);
}

// ========== Current Preset Tests ==========

TEST_F(PresetManagerTest, ApplyPreset) {
    Preset preset = createTestPreset("Apply Test");
    preset.parameters.num_voices = 24;

    EXPECT_TRUE(manager_->applyPreset(preset));

    auto current_params = manager_->getCurrentParameters();
    EXPECT_EQ(current_params.num_voices, 24);
}

TEST_F(PresetManagerTest, SetCurrentParameters) {
    PresetParameters params;
    params.num_voices = 32;
    params.master_gain = -6.0f;

    manager_->setCurrentParameters(params);

    auto current_params = manager_->getCurrentParameters();
    EXPECT_EQ(current_params.num_voices, 32);
    EXPECT_FLOAT_EQ(current_params.master_gain, -6.0f);
}

TEST_F(PresetManagerTest, ApplyInvalidPreset) {
    Preset preset;
    preset.metadata.name = ""; // Invalid
    preset.parameters.num_voices = 100; // Invalid

    EXPECT_FALSE(manager_->applyPreset(preset));
}

// ========== Callback Tests ==========

TEST_F(PresetManagerTest, PresetChangedCallback) {
    bool callback_called = false;
    Preset received_preset;

    auto callback_id = manager_->registerPresetChangedCallback(
        [&callback_called, &received_preset](const Preset& preset) {
            callback_called = true;
            received_preset = preset;
        }
    );

    Preset preset = createTestPreset("Callback Test");
    manager_->applyPreset(preset);

    EXPECT_TRUE(callback_called);
    EXPECT_EQ(received_preset.metadata.name, "Callback Test");

    manager_->unregisterPresetChangedCallback(callback_id);
}

TEST_F(PresetManagerTest, MultipleCallbacks) {
    int callback1_count = 0;
    int callback2_count = 0;

    manager_->registerPresetChangedCallback(
        [&callback1_count](const Preset&) { callback1_count++; }
    );

    manager_->registerPresetChangedCallback(
        [&callback2_count](const Preset&) { callback2_count++; }
    );

    Preset preset = createTestPreset("Multi Callback Test");
    manager_->applyPreset(preset);

    EXPECT_EQ(callback1_count, 1);
    EXPECT_EQ(callback2_count, 1);
}

// ========== PresetParameters Equality Tests ==========

TEST_F(PresetManagerTest, PresetParametersEquality) {
    PresetParameters params1;
    params1.num_voices = 16;
    params1.master_gain = -3.0f;

    PresetParameters params2;
    params2.num_voices = 16;
    params2.master_gain = -3.0f;

    EXPECT_EQ(params1, params2);
}

TEST_F(PresetManagerTest, PresetParametersInequality) {
    PresetParameters params1;
    params1.num_voices = 16;

    PresetParameters params2;
    params2.num_voices = 24;

    EXPECT_NE(params1, params2);
}

// ========== Edge Cases Tests ==========

TEST_F(PresetManagerTest, EmptyPresetName) {
    Preset preset;
    preset.metadata.name = "";

    auto errors = preset.getValidationErrors();
    EXPECT_GT(errors.size(), 0);
}

TEST_F(PresetManagerTest, InvalidVoiceCount) {
    Preset preset = createTestPreset("Invalid Voices");
    preset.parameters.num_voices = 100; // > 60

    auto errors = preset.getValidationErrors();
    EXPECT_GT(errors.size(), 0);
}

TEST_F(PresetManagerTest, InvalidMasterGain) {
    Preset preset = createTestPreset("Invalid Gain");
    preset.parameters.master_gain = 10.0f; // > 0 dB

    auto errors = preset.getValidationErrors();
    EXPECT_GT(errors.size(), 0);
}

TEST_F(PresetManagerTest, InvalidSynthesisMethod) {
    Preset preset = createTestPreset("Invalid Method");
    preset.parameters.synthesis_method = "invalid_method";

    auto errors = preset.getValidationErrors();
    EXPECT_GT(errors.size(), 0);
}

TEST_F(PresetManagerTest, InvalidOversamplingFactor) {
    Preset preset = createTestPreset("Invalid Oversampling");
    preset.parameters.oversampling_factor = 3.0f; // Not 1x, 2x, or 4x

    auto errors = preset.getValidationErrors();
    EXPECT_GT(errors.size(), 0);
}

// ========== Default Values Tests ==========

TEST_F(PresetManagerTest, PresetParametersDefaults) {
    PresetParameters params;

    EXPECT_EQ(params.num_voices, 8);
    EXPECT_FLOAT_EQ(params.master_gain, -3.0f);
    EXPECT_EQ(params.language, "english");
    EXPECT_EQ(params.lyrics, "ah ee ih oh oo");
    EXPECT_EQ(params.synthesis_method, "formant");
    EXPECT_FLOAT_EQ(params.formant_mix, 80.0f);
    EXPECT_FLOAT_EQ(params.subharmonic_mix, 20.0f);
    EXPECT_FLOAT_EQ(params.stereo_width, 75.0f);
    EXPECT_FLOAT_EQ(params.vibrato_rate, 6.0f);
    EXPECT_FLOAT_EQ(params.vibrato_depth, 30.0f);
    EXPECT_FLOAT_EQ(params.reverb_mix, 25.0f);
    EXPECT_FLOAT_EQ(params.reverb_size, 50.0f);
    EXPECT_FLOAT_EQ(params.attack_time, 50.0f);
    EXPECT_FLOAT_EQ(params.release_time, 200.0f);
    EXPECT_TRUE(params.enable_anti_aliasing);
    EXPECT_TRUE(params.enable_spectral_enhancement);
    EXPECT_FLOAT_EQ(params.oversampling_factor, 1.0f);
    EXPECT_FLOAT_EQ(params.pitch_variation, 10.0f);
    EXPECT_FLOAT_EQ(params.timing_variation, 5.0f);
    EXPECT_FLOAT_EQ(params.formant_variation, 15.0f);
    EXPECT_FLOAT_EQ(params.breathiness, 10.0f);
    EXPECT_FLOAT_EQ(params.warmth, 20.0f);
    EXPECT_FLOAT_EQ(params.brightness, 50.0f);
}

// ========== File Format Tests ==========

TEST_F(PresetManagerTest, PresetFileExtension) {
    EXPECT_STREQ(PresetManager::getPresetFileExtension(), ".choirv2");
}

TEST_F(PresetManagerTest, FormatVersion) {
    EXPECT_STREQ(PresetManager::getFormatVersion(), "2.0.0");
}

// ========== Persistence Tests ==========

TEST_F(PresetManagerTest, ShutdownAndReinitialize) {
    Preset preset = createTestPreset("Persistence Test");
    manager_->saveUserPreset(preset);
    manager_->addFavorite("Persistence Test");

    // Shutdown and reinitialize
    manager_->shutdown();
    manager_->initialize(
        (test_dir_ / "user").string(),
        (test_dir_ / "factory").string()
    );

    // Check that user preset persists
    auto user_presets = manager_->getUserPresets();
    bool found = false;
    for (const auto& p : user_presets) {
        if (p.metadata.name == "Persistence Test") {
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found);
}

// ========== Performance Tests ==========

TEST_F(PresetManagerTest, LoadManyPresets) {
    // Create 100 test presets
    for (int i = 0; i < 100; i++) {
        Preset preset = createTestPreset("Performance Test " + std::to_string(i));
        manager_->saveUserPreset(preset);
    }

    auto user_presets = manager_->getUserPresets();
    EXPECT_GE(user_presets.size(), 100);
}

TEST_F(PresetManagerTest, SearchManyPresets) {
    // Create 50 presets with "Searchable" in name
    for (int i = 0; i < 50; i++) {
        Preset preset = createTestPreset("Searchable " + std::to_string(i));
        manager_->saveUserPreset(preset);
    }

    auto results = manager_->searchPresets("Searchable");
    EXPECT_GE(results.size(), 50);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
