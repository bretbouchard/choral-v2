/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * PresetManager.cpp - Preset management system implementation
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#include "PresetManager.h"
#include "nlohmann/json.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <filesystem>
#include <regex>
#include <unordered_set>

#ifdef _WIN32
#include <shlobj.h>
#include <windows.h>
#else
#include <sys/stat.h>
#include <pwd.h>
#include <unistd.h>
#endif

namespace ChoirV2 {

using json = nlohmann::json;

// ========== PresetParameters Implementation ==========

bool PresetParameters::operator==(const PresetParameters& other) const {
    return num_voices == other.num_voices &&
           master_gain == other.master_gain &&
           language == other.language &&
           lyrics == other.lyrics &&
           synthesis_method == other.synthesis_method &&
           formant_mix == other.formant_mix &&
           subharmonic_mix == other.subharmonic_mix &&
           stereo_width == other.stereo_width &&
           vibrato_rate == other.vibrato_rate &&
           vibrato_depth == other.vibrato_depth &&
           reverb_mix == other.reverb_mix &&
           reverb_size == other.reverb_size &&
           attack_time == other.attack_time &&
           release_time == other.release_time &&
           enable_anti_aliasing == other.enable_anti_aliasing &&
           enable_spectral_enhancement == other.enable_spectral_enhancement &&
           oversampling_factor == other.oversampling_factor &&
           pitch_variation == other.pitch_variation &&
           timing_variation == other.timing_variation &&
           formant_variation == other.formant_variation &&
           breathiness == other.breathiness &&
           warmth == other.warmth &&
           brightness == other.brightness;
}

bool PresetParameters::operator!=(const PresetParameters& other) const {
    return !(*this == other);
}

// ========== Preset Implementation ==========

bool Preset::isValid() const {
    return getValidationErrors().empty();
}

std::vector<std::string> Preset::getValidationErrors() const {
    std::vector<std::string> errors;

    // Validate metadata
    if (metadata.name.empty()) {
        errors.push_back("Preset name cannot be empty");
    }

    if (metadata.version.empty()) {
        errors.push_back("Preset version cannot be empty");
    }

    // Validate parameters
    if (parameters.num_voices < 1 || parameters.num_voices > 60) {
        errors.push_back("Number of voices must be between 1 and 60");
    }

    if (parameters.master_gain < -60.0f || parameters.master_gain > 0.0f) {
        errors.push_back("Master gain must be between -60 and 0 dB");
    }

    if (parameters.formant_mix < 0.0f || parameters.formant_mix > 100.0f) {
        errors.push_back("Formant mix must be between 0 and 100");
    }

    if (parameters.subharmonic_mix < 0.0f || parameters.subharmonic_mix > 100.0f) {
        errors.push_back("Subharmonic mix must be between 0 and 100");
    }

    if (parameters.stereo_width < 0.0f || parameters.stereo_width > 100.0f) {
        errors.push_back("Stereo width must be between 0 and 100");
    }

    if (parameters.vibrato_rate < 0.1f || parameters.vibrato_rate > 15.0f) {
        errors.push_back("Vibrato rate must be between 0.1 and 15.0 Hz");
    }

    if (parameters.vibrato_depth < 0.0f || parameters.vibrato_depth > 100.0f) {
        errors.push_back("Vibrato depth must be between 0 and 100");
    }

    if (parameters.reverb_mix < 0.0f || parameters.reverb_mix > 100.0f) {
        errors.push_back("Reverb mix must be between 0 and 100");
    }

    if (parameters.reverb_size < 0.0f || parameters.reverb_size > 100.0f) {
        errors.push_back("Reverb size must be between 0 and 100");
    }

    if (parameters.attack_time < 0.0f || parameters.attack_time > 5000.0f) {
        errors.push_back("Attack time must be between 0 and 5000 ms");
    }

    if (parameters.release_time < 0.0f || parameters.release_time > 10000.0f) {
        errors.push_back("Release time must be between 0 and 10000 ms");
    }

    if (parameters.oversampling_factor != 1.0f &&
        parameters.oversampling_factor != 2.0f &&
        parameters.oversampling_factor != 4.0f) {
        errors.push_back("Oversampling factor must be 1x, 2x, or 4x");
    }

    if (parameters.synthesis_method != "formant" &&
        parameters.synthesis_method != "diphone" &&
        parameters.synthesis_method != "subharmonic") {
        errors.push_back("Synthesis method must be 'formant', 'diphone', or 'subharmonic'");
    }

    if (parameters.language.empty()) {
        errors.push_back("Language cannot be empty");
    }

    return errors;
}

// ========== PresetManager::Impl ==========

struct PresetManager::Impl {
    std::string user_preset_dir;
    std::string factory_preset_dir;

    // Preset storage
    std::unordered_map<std::string, Preset> factory_presets;
    std::unordered_map<std::string, Preset> user_presets;
    std::unordered_map<std::string, Preset> imported_presets;

    // Current preset
    PresetParameters current_parameters;

    // Favorites
    std::unordered_set<std::string> favorites;
    std::vector<std::pair<std::string, std::chrono::system_clock::time_point>> recent_presets;

    // Callbacks
    std::vector<std::pair<size_t, PresetChangedCallback>> callbacks;
    size_t next_callback_id = 1;

    // Initialized flag
    bool initialized = false;

    // Helper methods
    std::string getCurrentTimestamp() const {
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);

        std::ostringstream oss;
        oss << std::put_time(std::gmtime(&time_t_now), "%Y-%m-%dT%H:%M:%SZ");
        return oss.str();
    }

    void notifyPresetChanged(const Preset& preset) {
        for (const auto& [id, callback] : callbacks) {
            if (callback) {
                callback(preset);
            }
        }
    }

    json parametersToJson(const PresetParameters& params) const {
        json j;
        j["numVoices"] = params.num_voices;
        j["masterGain"] = params.master_gain;
        j["language"] = params.language;
        j["lyrics"] = params.lyrics;
        j["synthesisMethod"] = params.synthesis_method;
        j["formantMix"] = params.formant_mix;
        j["subharmonicMix"] = params.subharmonic_mix;
        j["stereoWidth"] = params.stereo_width;
        j["vibratoRate"] = params.vibrato_rate;
        j["vibratoDepth"] = params.vibrato_depth;
        j["reverbMix"] = params.reverb_mix;
        j["reverbSize"] = params.reverb_size;
        j["attackTime"] = params.attack_time;
        j["releaseTime"] = params.release_time;
        j["enableAntiAliasing"] = params.enable_anti_aliasing;
        j["enableSpectralEnhancement"] = params.enable_spectral_enhancement;
        j["oversamplingFactor"] = params.oversampling_factor;
        j["pitchVariation"] = params.pitch_variation;
        j["timingVariation"] = params.timing_variation;
        j["formantVariation"] = params.formant_variation;
        j["breathiness"] = params.breathiness;
        j["warmth"] = params.warmth;
        j["brightness"] = params.brightness;
        return j;
    }

    PresetParameters jsonToParameters(const json& j) const {
        PresetParameters params;

        if (j.contains("numVoices")) params.num_voices = j["numVoices"];
        if (j.contains("masterGain")) params.master_gain = j["masterGain"];
        if (j.contains("language")) params.language = j["language"];
        if (j.contains("lyrics")) params.lyrics = j["lyrics"];
        if (j.contains("synthesisMethod")) params.synthesis_method = j["synthesisMethod"];
        if (j.contains("formantMix")) params.formant_mix = j["formantMix"];
        if (j.contains("subharmonicMix")) params.subharmonic_mix = j["subharmonicMix"];
        if (j.contains("stereoWidth")) params.stereo_width = j["stereoWidth"];
        if (j.contains("vibratoRate")) params.vibrato_rate = j["vibratoRate"];
        if (j.contains("vibratoDepth")) params.vibrato_depth = j["vibratoDepth"];
        if (j.contains("reverbMix")) params.reverb_mix = j["reverbMix"];
        if (j.contains("reverbSize")) params.reverb_size = j["reverbSize"];
        if (j.contains("attackTime")) params.attack_time = j["attackTime"];
        if (j.contains("releaseTime")) params.release_time = j["releaseTime"];
        if (j.contains("enableAntiAliasing")) params.enable_anti_aliasing = j["enableAntiAliasing"];
        if (j.contains("enableSpectralEnhancement")) params.enable_spectral_enhancement = j["enableSpectralEnhancement"];
        if (j.contains("oversamplingFactor")) params.oversampling_factor = j["oversamplingFactor"];
        if (j.contains("pitchVariation")) params.pitch_variation = j["pitchVariation"];
        if (j.contains("timingVariation")) params.timing_variation = j["timingVariation"];
        if (j.contains("formantVariation")) params.formant_variation = j["formantVariation"];
        if (j.contains("breathiness")) params.breathiness = j["breathiness"];
        if (j.contains("warmth")) params.warmth = j["warmth"];
        if (j.contains("brightness")) params.brightness = j["brightness"];

        return params;
    }

    json metadataToJson(const PresetMetadata& metadata) const {
        json j;
        j["name"] = metadata.name;
        j["author"] = metadata.author;
        j["description"] = metadata.description;
        j["version"] = metadata.version;
        j["category"] = metadata.category;
        j["tags"] = metadata.tags;
        j["createdDate"] = metadata.created_date;
        j["modifiedDate"] = metadata.modified_date;
        j["pluginVersion"] = metadata.plugin_version;
        return j;
    }

    PresetMetadata jsonToMetadata(const json& j) const {
        PresetMetadata metadata;

        if (j.contains("name")) metadata.name = j["name"];
        if (j.contains("author")) metadata.author = j["author"];
        if (j.contains("description")) metadata.description = j["description"];
        if (j.contains("version")) metadata.version = j["version"];
        if (j.contains("category")) metadata.category = j["category"];
        if (j.contains("tags")) metadata.tags = j["tags"].get<std::vector<std::string>>();
        if (j.contains("createdDate")) metadata.created_date = j["createdDate"];
        if (j.contains("modifiedDate")) metadata.modified_date = j["modifiedDate"];
        if (j.contains("pluginVersion")) metadata.plugin_version = j["pluginVersion"];

        return metadata;
    }
};

// ========== PresetManager Implementation ==========

PresetManager::PresetManager()
    : impl_(std::make_unique<Impl>()) {
}

PresetManager::~PresetManager() {
    shutdown();
}

bool PresetManager::initialize(
    const std::string& user_preset_dir,
    const std::string& factory_preset_dir
) {
    if (impl_->initialized) {
        return true; // Already initialized
    }

    // Set directories
    impl_->user_preset_dir = user_preset_dir.empty()
        ? getDefaultUserPresetDir()
        : user_preset_dir;

    impl_->factory_preset_dir = factory_preset_dir.empty()
        ? getDefaultFactoryPresetDir()
        : factory_preset_dir;

    // Ensure directories exist
    ensurePresetDirectoriesExist();

    // Load factory presets
    reloadFactoryPresets();

    // Load user presets
    loadUserPresets();

    // Load favorites
    loadFavorites();

    // Load recent presets
    loadRecentPresets();

    impl_->initialized = true;
    return true;
}

void PresetManager::shutdown() {
    if (!impl_->initialized) {
        return;
    }

    // Save favorites
    saveFavorites();

    // Save recent presets
    saveRecentPresets();

    // Clear callbacks
    impl_->callbacks.clear();

    impl_->initialized = false;
}

std::shared_ptr<Preset> PresetManager::loadPreset(const std::string& file_path) {
    try {
        std::ifstream file(file_path);
        if (!file.is_open()) {
            return nullptr;
        }

        json j;
        file >> j;

        // Validate format
        if (!j.contains("format") || j["format"] != "choir-v2-preset") {
            return nullptr;
        }

        // Create preset
        auto preset = std::make_shared<Preset>();

        // Load metadata
        if (j.contains("metadata")) {
            preset->metadata = impl_->jsonToMetadata(j["metadata"]);
        }

        // Load parameters
        if (j.contains("parameters")) {
            preset->parameters = impl_->jsonToParameters(j["parameters"]);
        }

        // Set file path
        preset->file_path = file_path;

        // Check if factory preset
        preset->is_factory = file_path.find(impl_->factory_preset_dir) == 0;

        return preset;

    } catch (const std::exception& e) {
        return nullptr;
    }
}

bool PresetManager::savePreset(
    const Preset& preset,
    const std::string& file_path,
    const PresetExportOptions& options
) {
    try {
        json j;

        // Format info
        j["format"] = "choir-v2-preset";
        j["version"] = PresetManager::getFormatVersion();

        // Metadata
        if (options.include_metadata) {
            j["metadata"] = impl_->metadataToJson(preset.metadata);
        }

        // Parameters
        j["parameters"] = impl_->parametersToJson(preset.parameters);

        // Backup existing file
        if (options.backup_original) {
            std::filesystem::path path(file_path);
            if (std::filesystem::exists(path)) {
                std::filesystem::path backup_path = path;
                backup_path += ".backup";
                std::filesystem::copy_file(
                    path,
                    backup_path,
                    std::filesystem::copy_options::overwrite_existing
                );
            }
        }

        // Write file
        std::ofstream file(file_path);
        if (!file.is_open()) {
            return false;
        }

        if (options.compact_format) {
            file << j.dump();
        } else {
            file << j.dump(2); // Pretty print with 2-space indent
        }

        return true;

    } catch (const std::exception& e) {
        return false;
    }
}

Preset PresetManager::createPresetFromCurrent(
    const std::string& name,
    const std::string& description
) {
    Preset preset;

    // Metadata
    preset.metadata.name = name;
    preset.metadata.author = "User";
    preset.metadata.description = description;
    preset.metadata.version = PresetManager::getFormatVersion();
    preset.metadata.category = "User";
    preset.metadata.plugin_version = "2.0.0";
    preset.metadata.created_date = impl_->getCurrentTimestamp();
    preset.metadata.modified_date = impl_->getCurrentTimestamp();

    // Parameters
    preset.parameters = impl_->current_parameters;

    return preset;
}

std::vector<Preset> PresetManager::getFactoryPresets() const {
    std::vector<Preset> presets;
    presets.reserve(impl_->factory_presets.size());

    for (const auto& [name, preset] : impl_->factory_presets) {
        presets.push_back(preset);
    }

    return presets;
}

std::shared_ptr<Preset> PresetManager::getFactoryPreset(const std::string& name) const {
    auto it = impl_->factory_presets.find(name);
    if (it != impl_->factory_presets.end()) {
        return std::make_shared<Preset>(it->second);
    }
    return nullptr;
}

int PresetManager::reloadFactoryPresets() {
    impl_->factory_presets.clear();

    if (!std::filesystem::exists(impl_->factory_preset_dir)) {
        return 0;
    }

    int count = 0;
    for (const auto& entry : std::filesystem::directory_iterator(impl_->factory_preset_dir)) {
        if (entry.path().extension() == getPresetFileExtension()) {
            auto preset = loadPreset(entry.path().string());
            if (preset && preset->isValid()) {
                preset->is_factory = true;
                preset->is_read_only = true;
                impl_->factory_presets[preset->metadata.name] = *preset;
                count++;
            }
        }
    }

    return count;
}

std::vector<Preset> PresetManager::getUserPresets() const {
    std::vector<Preset> presets;
    presets.reserve(impl_->user_presets.size());

    for (const auto& [name, preset] : impl_->user_presets) {
        presets.push_back(preset);
    }

    return presets;
}

bool PresetManager::saveUserPreset(const Preset& preset) {
    // Create file path
    std::string file_name = sanitizeFilename(preset.metadata.name) + getPresetFileExtension();
    std::string file_path = impl_->user_preset_dir + "/" + file_name;

    // Create mutable copy
    Preset preset_to_save = preset;
    preset_to_save.file_path = file_path;
    preset_to_save.metadata.modified_date = impl_->getCurrentTimestamp();

    // Save file
    if (!savePreset(preset_to_save, file_path)) {
        return false;
    }

    // Add to storage
    impl_->user_presets[preset.metadata.name] = preset_to_save;

    // Notify callbacks
    impl_->notifyPresetChanged(preset_to_save);

    return true;
}

bool PresetManager::deleteUserPreset(const std::string& preset_name) {
    auto it = impl_->user_presets.find(preset_name);
    if (it == impl_->user_presets.end()) {
        return false;
    }

    // Delete file
    if (!it->second.file_path.empty()) {
        std::filesystem::remove(it->second.file_path);
    }

    // Remove from storage
    impl_->user_presets.erase(it);

    // Remove from favorites
    impl_->favorites.erase(preset_name);

    return true;
}

bool PresetManager::renameUserPreset(
    const std::string& old_name,
    const std::string& new_name
) {
    auto it = impl_->user_presets.find(old_name);
    if (it == impl_->user_presets.end()) {
        return false;
    }

    // Create new file path
    std::string new_file_name = sanitizeFilename(new_name) + getPresetFileExtension();
    std::string new_file_path = impl_->user_preset_dir + "/" + new_file_name;

    // Rename file
    if (!it->second.file_path.empty()) {
        std::filesystem::rename(it->second.file_path, new_file_path);
    }

    // Update preset
    Preset preset = it->second;
    preset.metadata.name = new_name;
    preset.file_path = new_file_path;
    preset.metadata.modified_date = impl_->getCurrentTimestamp();

    // Re-save with new name
    savePreset(preset, new_file_path);

    // Update storage
    impl_->user_presets.erase(it);
    impl_->user_presets[new_name] = preset;

    // Update favorites
    if (impl_->favorites.count(old_name)) {
        impl_->favorites.erase(old_name);
        impl_->favorites.insert(new_name);
    }

    return true;
}

std::shared_ptr<Preset> PresetManager::duplicateUserPreset(
    const std::string& preset_name,
    const std::string& new_name
) {
    auto it = impl_->user_presets.find(preset_name);
    if (it == impl_->user_presets.end()) {
        return nullptr;
    }

    // Create duplicate
    Preset duplicate = it->second;
    duplicate.metadata.name = new_name;
    duplicate.metadata.created_date = impl_->getCurrentTimestamp();
    duplicate.metadata.modified_date = impl_->getCurrentTimestamp();
    duplicate.file_path.clear(); // Will be set by saveUserPreset

    // Save duplicate
    if (saveUserPreset(duplicate)) {
        auto dup_it = impl_->user_presets.find(new_name);
        if (dup_it != impl_->user_presets.end()) {
            return std::make_shared<Preset>(dup_it->second);
        }
    }

    return nullptr;
}

std::vector<Preset> PresetManager::searchPresets(
    const std::string& query,
    bool include_factory,
    bool include_user
) const {
    std::vector<Preset> results;

    // Convert query to lowercase for case-insensitive search
    std::string query_lower = toLower(query);

    // Search factory presets
    if (include_factory) {
        for (const auto& [name, preset] : impl_->factory_presets) {
            if (matchesSearchQuery(preset, query_lower)) {
                results.push_back(preset);
            }
        }
    }

    // Search user presets
    if (include_user) {
        for (const auto& [name, preset] : impl_->user_presets) {
            if (matchesSearchQuery(preset, query_lower)) {
                results.push_back(preset);
            }
        }
    }

    return results;
}

std::vector<Preset> PresetManager::getPresetsByCategory(PresetCategory category) const {
    std::vector<Preset> presets;

    switch (category) {
        case PresetCategory::Factory:
            return getFactoryPresets();
        case PresetCategory::User:
            return getUserPresets();
        case PresetCategory::Imported: {
            for (const auto& [name, preset] : impl_->imported_presets) {
                presets.push_back(preset);
            }
            return presets;
        }
    }

    return presets;
}

std::vector<std::string> PresetManager::getAllTags() const {
    std::unordered_set<std::string> tags_set;

    // Collect tags from factory presets
    for (const auto& [name, preset] : impl_->factory_presets) {
        for (const auto& tag : preset.metadata.tags) {
            tags_set.insert(tag);
        }
    }

    // Collect tags from user presets
    for (const auto& [name, preset] : impl_->user_presets) {
        for (const auto& tag : preset.metadata.tags) {
            tags_set.insert(tag);
        }
    }

    return std::vector<std::string>(tags_set.begin(), tags_set.end());
}

std::shared_ptr<Preset> PresetManager::importPreset(
    const std::string& file_path,
    const PresetImportOptions& options
) {
    // Load preset
    auto preset = loadPreset(file_path);
    if (!preset || !preset->isValid()) {
        return nullptr;
    }

    // Validate compatibility
    if (options.validate_compatibility) {
        auto validation = validatePreset(*preset);
        if (!validation.is_compatible) {
            return nullptr;
        }
    }

    // Add to imported presets
    if (options.copy_to_user_presets) {
        preset->metadata.category = options.import_category;

        // Generate unique name if exists
        std::string base_name = preset->metadata.name;
        std::string unique_name = base_name;
        int counter = 1;

        while (impl_->user_presets.count(unique_name)) {
            unique_name = base_name + " " + std::to_string(counter++);
        }

        preset->metadata.name = unique_name;

        // Save to user presets
        if (!saveUserPreset(*preset)) {
            return nullptr;
        }

        // Add to imported presets tracking
        impl_->imported_presets[unique_name] = *preset;
    } else {
        impl_->imported_presets[preset->metadata.name] = *preset;
    }

    return preset;
}

bool PresetManager::exportPreset(
    const Preset& preset,
    const std::string& file_path,
    const PresetExportOptions& options
) {
    return savePreset(preset, file_path, options);
}

bool PresetManager::exportAllUserPresets(const std::string& zip_path) {
    // TODO: Implement zip archive creation
    // This requires a zip library like miniz or zlib
    return false;
}

PresetValidationResult PresetManager::validatePreset(const Preset& preset) const {
    PresetValidationResult result;

    // Check format version
    result.format_version = preset.metadata.version;

    // Validate preset
    auto errors = preset.getValidationErrors();
    result.errors = errors;
    result.is_valid = errors.empty();

    // Check compatibility
    result.is_compatible = true;
    // TODO: Add version compatibility checking

    return result;
}

PresetValidationResult PresetManager::validatePresetFile(const std::string& file_path) const {
    PresetValidationResult result;

    try {
        std::ifstream file(file_path);
        if (!file.is_open()) {
            result.errors.push_back("Cannot open file");
            return result;
        }

        json j;
        file >> j;

        // Validate format
        if (!j.contains("format") || j["format"] != "choir-v2-preset") {
            result.errors.push_back("Invalid preset format");
            return result;
        }

        // Check version
        if (j.contains("version")) {
            result.format_version = j["version"];
        }

        // Validate structure
        if (!j.contains("metadata")) {
            result.warnings.push_back("Missing metadata");
        }

        if (!j.contains("parameters")) {
            result.errors.push_back("Missing parameters");
            return result;
        }

        result.is_valid = true;
        result.is_compatible = true;

    } catch (const std::exception& e) {
        result.errors.push_back(std::string("JSON parsing error: ") + e.what());
    }

    return result;
}

bool PresetManager::addFavorite(const std::string& preset_name) {
    return impl_->favorites.insert(preset_name).second;
}

bool PresetManager::removeFavorite(const std::string& preset_name) {
    return impl_->favorites.erase(preset_name) > 0;
}

bool PresetManager::isFavorite(const std::string& preset_name) const {
    return impl_->favorites.count(preset_name) > 0;
}

std::vector<Preset> PresetManager::getFavorites() const {
    std::vector<Preset> favorites;

    for (const auto& name : impl_->favorites) {
        // Check factory presets
        auto it = impl_->factory_presets.find(name);
        if (it != impl_->factory_presets.end()) {
            favorites.push_back(it->second);
            continue;
        }

        // Check user presets
        it = impl_->user_presets.find(name);
        if (it != impl_->user_presets.end()) {
            favorites.push_back(it->second);
        }
    }

    return favorites;
}

std::vector<Preset> PresetManager::getRecentPresets(int max_count) const {
    std::vector<Preset> recent;

    for (const auto& [name, time] : impl_->recent_presets) {
        // Find preset
        auto it = impl_->factory_presets.find(name);
        if (it == impl_->factory_presets.end()) {
            it = impl_->user_presets.find(name);
        }

        if (it != impl_->factory_presets.end() && it != impl_->user_presets.end()) {
            continue; // Not found
        }

        if (it != impl_->factory_presets.end()) {
            recent.push_back(it->second);
        } else if (it != impl_->user_presets.end()) {
            recent.push_back(it->second);
        }

        if (recent.size() >= max_count) {
            break;
        }
    }

    return recent;
}

void PresetManager::addToRecent(const Preset& preset) {
    // Remove existing entry
    auto it = std::find_if(
        impl_->recent_presets.begin(),
        impl_->recent_presets.end(),
        [&preset](const auto& entry) {
            return entry.first == preset.metadata.name;
        }
    );

    if (it != impl_->recent_presets.end()) {
        impl_->recent_presets.erase(it);
    }

    // Add to front
    impl_->recent_presets.emplace_front(
        preset.metadata.name,
        std::chrono::system_clock::now()
    );

    // Limit size
    while (impl_->recent_presets.size() > 20) {
        impl_->recent_presets.pop_back();
    }
}

bool PresetManager::applyPreset(const Preset& preset) {
    if (!preset.isValid()) {
        return false;
    }

    // Apply parameters
    impl_->current_parameters = preset.parameters;

    // Add to recent
    addToRecent(preset);

    // Notify callbacks
    impl_->notifyPresetChanged(preset);

    return true;
}

PresetParameters PresetManager::getCurrentParameters() const {
    return impl_->current_parameters;
}

void PresetManager::setCurrentParameters(const PresetParameters& params) {
    impl_->current_parameters = params;
}

size_t PresetManager::registerPresetChangedCallback(PresetChangedCallback callback) {
    size_t id = impl_->next_callback_id++;
    impl_->callbacks.emplace_back(id, callback);
    return id;
}

void PresetManager::unregisterPresetChangedCallback(size_t callback_id) {
    auto it = std::find_if(
        impl_->callbacks.begin(),
        impl_->callbacks.end(),
        [callback_id](const auto& entry) {
            return entry.first == callback_id;
        }
    );

    if (it != impl_->callbacks.end()) {
        impl_->callbacks.erase(it);
    }
}

std::string PresetManager::getDefaultUserPresetDir() {
    std::string preset_dir;

#ifdef _WIN32
    CHAR path[MAX_PATH];
    if (SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, path) == S_OK) {
        preset_dir = std::string(path) + "\\ChoirV2\\presets\\user";
    }
#else
    const char* home_dir = getenv("HOME");
    if (!home_dir) {
        home_dir = getpwuid(getuid())->pw_dir;
    }

    #ifdef __APPLE__
        preset_dir = std::string(home_dir) + "/Library/Application Support/ChoirV2/presets/user";
    #else
        preset_dir = std::string(home_dir) + "/.local/share/ChoirV2/presets/user";
    #endif
#endif

    return preset_dir;
}

std::string PresetManager::getDefaultFactoryPresetDir() {
    // Factory presets are typically bundled with the plugin
    // This is a placeholder - actual implementation would use the plugin's resource path
    return "/usr/local/share/ChoirV2/presets/factory";
}

void PresetManager::ensurePresetDirectoriesExist() {
    std::filesystem::create_directories(impl_->user_preset_dir);

    // Create favorites file
    std::string favorites_path = impl_->user_preset_dir + "/favorites.json";
    if (!std::filesystem::exists(favorites_path)) {
        std::ofstream file(favorites_path);
        json j = json::array();
        file << j.dump(2);
    }

    // Create recent presets file
    std::string recent_path = impl_->user_preset_dir + "/recent.json";
    if (!std::filesystem::exists(recent_path)) {
        std::ofstream file(recent_path);
        json j = json::array();
        file << j.dump(2);
    }
}

// ========== Private Helper Methods ==========

void PresetManager::loadUserPresets() {
    if (!std::filesystem::exists(impl_->user_preset_dir)) {
        return;
    }

    for (const auto& entry : std::filesystem::directory_iterator(impl_->user_preset_dir)) {
        if (entry.path().extension() == getPresetFileExtension()) {
            auto preset = loadPreset(entry.path().string());
            if (preset && preset->isValid()) {
                preset->is_factory = false;
                preset->is_read_only = false;
                impl_->user_presets[preset->metadata.name] = *preset;
            }
        }
    }
}

void PresetManager::loadFavorites() {
    std::string favorites_path = impl_->user_preset_dir + "/favorites.json";

    try {
        std::ifstream file(favorites_path);
        if (!file.is_open()) {
            return;
        }

        json j;
        file >> j;

        if (j.is_array()) {
            for (const auto& item : j) {
                if (item.is_string()) {
                    impl_->favorites.insert(item.get<std::string>());
                }
            }
        }
    } catch (...) {
        // Ignore errors
    }
}

void PresetManager::saveFavorites() {
    std::string favorites_path = impl_->user_preset_dir + "/favorites.json";

    try {
        json j = json::array();
        for (const auto& favorite : impl_->favorites) {
            j.push_back(favorite);
        }

        std::ofstream file(favorites_path);
        file << j.dump(2);
    } catch (...) {
        // Ignore errors
    }
}

void PresetManager::loadRecentPresets() {
    std::string recent_path = impl_->user_preset_dir + "/recent.json";

    try {
        std::ifstream file(recent_path);
        if (!file.is_open()) {
            return;
        }

        json j;
        file >> j;

        if (j.is_array()) {
            for (const auto& item : j) {
                if (item.contains("name") && item.contains("timestamp")) {
                    impl_->recent_presets.emplace_back(
                        item["name"].get<std::string>(),
                        std::chrono::system_clock::from_time_t(item["timestamp"].get<long long>())
                    );
                }
            }
        }
    } catch (...) {
        // Ignore errors
    }
}

void PresetManager::saveRecentPresets() {
    std::string recent_path = impl_->user_preset_dir + "/recent.json";

    try {
        json j = json::array();
        for (const auto& [name, time] : impl_->recent_presets) {
            json item;
            item["name"] = name;
            item["timestamp"] = std::chrono::system_clock::to_time_t(time);
            j.push_back(item);
        }

        std::ofstream file(recent_path);
        file << j.dump(2);
    } catch (...) {
        // Ignore errors
    }
}

bool PresetManager::matchesSearchQuery(const Preset& preset, const std::string& query) const {
    // Search in name
    if (toLower(preset.metadata.name).find(query) != std::string::npos) {
        return true;
    }

    // Search in description
    if (toLower(preset.metadata.description).find(query) != std::string::npos) {
        return true;
    }

    // Search in tags
    for (const auto& tag : preset.metadata.tags) {
        if (toLower(tag).find(query) != std::string::npos) {
            return true;
        }
    }

    return false;
}

std::string PresetManager::toLower(const std::string& str) const {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

std::string PresetManager::sanitizeFilename(const std::string& name) const {
    std::string result = name;

    // Replace invalid characters with underscores
    std::regex invalid_chars(R"([<>:"/\\|?*])");
    result = std::regex_replace(result, invalid_chars, "_");

    // Remove leading/trailing spaces and dots
    result.erase(0, result.find_first_not_of(" ."));
    result.erase(result.find_last_not_of(" .") + 1);

    return result;
}

} // namespace ChoirV2
