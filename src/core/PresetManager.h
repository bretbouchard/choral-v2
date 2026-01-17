/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * PresetManager.h - Preset management system
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>

namespace ChoirV2 {

/**
 * @brief Complete preset parameters for Choir V2.0
 *
 * Contains all parameters needed to recreate a specific vocal sound.
 */
struct PresetParameters {
    // Voice configuration
    int num_voices = 8;              // Number of simultaneous voices (1-60)
    float master_gain = -3.0f;       // Master volume (dB)

    // Language and lyrics
    std::string language = "english"; // Language code
    std::string lyrics = "ah ee ih oh oo"; // Lyrics text

    // Synthesis method
    std::string synthesis_method = "formant"; // "formant", "diphone", "subharmonic"

    // Formant parameters
    float formant_mix = 80.0f;       // Formant mix percentage (0-100)
    float subharmonic_mix = 20.0f;   // Subharmonic mix percentage (0-100)

    // Stereo imaging
    float stereo_width = 75.0f;      // Stereo width percentage (0-100)

    // Vibrato
    float vibrato_rate = 6.0f;       // Vibrato rate (Hz)
    float vibrato_depth = 30.0f;     // Vibrato depth (percentage)

    // Reverb
    float reverb_mix = 25.0f;        // Reverb mix (percentage)
    float reverb_size = 50.0f;       // Reverb size/room size (percentage)

    // Envelope
    float attack_time = 50.0f;       // Attack time (ms)
    float release_time = 200.0f;     // Release time (ms)

    // Quality settings
    bool enable_anti_aliasing = true;
    bool enable_spectral_enhancement = true;
    float oversampling_factor = 1.0f; // 1x, 2x, 4x

    // Voice characteristics (per-voice randomization ranges)
    float pitch_variation = 10.0f;   // Pitch variation (cents)
    float timing_variation = 5.0f;   // Timing variation (percentage)
    float formant_variation = 15.0f; // Formant variation (percentage)

    // Advanced synthesis
    float breathiness = 10.0f;       // Breathiness amount (percentage)
    float warmth = 20.0f;            // Warmth/coloration (percentage)
    float brightness = 50.0f;        // Brightness (percentage)

    // Equality operators
    bool operator==(const PresetParameters& other) const;
    bool operator!=(const PresetParameters& other) const;
};

/**
 * @brief Preset metadata
 */
struct PresetMetadata {
    std::string name;                // Preset name
    std::string author;              // Preset author
    std::string description;         // Preset description
    std::string version = "2.0.0";   // Preset format version
    std::string category = "Custom"; // Preset category
    std::vector<std::string> tags;   // Preset tags

    // Creation info
    std::string created_date;        // ISO 8601 date string
    std::string modified_date;       // ISO 8601 date string
    std::string plugin_version;      // Plugin version when created
};

/**
 * @brief Complete preset containing metadata and parameters
 */
struct Preset {
    PresetMetadata metadata;
    PresetParameters parameters;

    // File information
    std::string file_path;           // Path to preset file (if loaded from disk)
    bool is_factory = false;         // Whether this is a factory preset
    bool is_read_only = false;       // Whether preset can be modified

    // Validation
    bool isValid() const;
    std::vector<std::string> getValidationErrors() const;
};

/**
 * @brief Preset category for organization
 */
enum class PresetCategory {
    Factory,         // Factory presets (shipped with plugin)
    User,            // User-created presets
    Imported         // Imported from third-party
};

/**
 * @brief Preset validation result
 */
struct PresetValidationResult {
    bool is_valid = false;
    std::string format_version;      // Preset format version
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
    std::vector<std::string> info;

    // Compatibility check
    bool is_compatible = true;
    std::string min_plugin_version;
    std::string max_plugin_version;
};

/**
 * @brief Preset import/export options
 */
struct PresetImportOptions {
    bool overwrite_existing = false;
    bool validate_compatibility = true;
    bool copy_to_user_presets = true;
    std::string import_category = "Imported";
};

struct PresetExportOptions {
    bool include_metadata = true;
    bool compact_format = false;     // Use compact JSON formatting
    bool backup_original = true;     // Backup existing file when overwriting
};

/**
 * @brief Callback type for preset change notifications
 */
using PresetChangedCallback = std::function<void(const Preset& preset)>;

/**
 * @brief Comprehensive preset management system
 *
 * Features:
 * - Load/save preset files (.choirv2 JSON format)
 * - Manage factory presets (shipped with plugin)
 * - Manage user presets (user-created)
 * - Preset browser with search and filtering
 * - Import/export presets
 * - Preset validation and compatibility checking
 * - Real-time preset change notifications
 * - Undo/redo support for preset changes
 * - Preset favorites and recent presets
 *
 * Usage:
 * ```cpp
 * PresetManager manager;
 * manager.initialize();
 *
 * // Load a preset
 * auto preset = manager.loadPreset("path/to/preset.choirv2");
 * if (preset && preset->isValid()) {
 *     manager.applyPreset(*preset);
 * }
 *
 * // Save current settings as preset
 * Preset current = manager.createPresetFromCurrent("My Preset");
 * manager.saveUserPreset(current);
 *
 * // Browse presets
 * auto factory_presets = manager.getFactoryPresets();
 * auto user_presets = manager.getUserPresets();
 *
 * // Search presets
 * auto results = manager.searchPresets("choir");
 * ```
 */
class PresetManager {
public:
    PresetManager();
    ~PresetManager();

    /**
     * @brief Initialize the preset manager
     * @param user_preset_dir Optional custom directory for user presets
     * @param factory_preset_dir Optional custom directory for factory presets
     * @return true if initialized successfully
     */
    bool initialize(
        const std::string& user_preset_dir = "",
        const std::string& factory_preset_dir = ""
    );

    /**
     * @brief Shutdown the preset manager
     */
    void shutdown();

    // ========== Preset Loading & Saving ==========

    /**
     * @brief Load a preset from file
     * @param file_path Path to .choirv2 file
     * @return Preset object if loaded successfully, nullptr otherwise
     */
    std::shared_ptr<Preset> loadPreset(const std::string& file_path);

    /**
     * @brief Save a preset to file
     * @param preset Preset to save
     * @param file_path Destination file path
     * @param options Export options
     * @return true if saved successfully
     */
    bool savePreset(
        const Preset& preset,
        const std::string& file_path,
        const PresetExportOptions& options = {}
    );

    /**
     * @brief Create preset from current engine parameters
     * @param name Preset name
     * @param description Optional description
     * @return Preset object with current parameters
     */
    Preset createPresetFromCurrent(
        const std::string& name,
        const std::string& description = ""
    );

    // ========== Factory Presets ==========

    /**
     * @brief Get all factory presets
     * @return Vector of factory presets
     */
    std::vector<Preset> getFactoryPresets() const;

    /**
     * @brief Get factory preset by name
     * @param name Preset name
     * @return Preset object if found, nullptr otherwise
     */
    std::shared_ptr<Preset> getFactoryPreset(const std::string& name) const;

    /**
     * @brief Reload factory presets from disk
     * @return Number of factory presets loaded
     */
    int reloadFactoryPresets();

    // ========== User Presets ==========

    /**
     * @brief Get all user presets
     * @return Vector of user presets
     */
    std::vector<Preset> getUserPresets() const;

    /**
     * @brief Save a user preset
     * @param preset Preset to save
     * @return true if saved successfully
     */
    bool saveUserPreset(const Preset& preset);

    /**
     * @brief Delete a user preset
     * @param preset_name Name of preset to delete
     * @return true if deleted successfully
     */
    bool deleteUserPreset(const std::string& preset_name);

    /**
     * @brief Rename a user preset
     * @param old_name Current preset name
     * @param new_name New preset name
     * @return true if renamed successfully
     */
    bool renameUserPreset(
        const std::string& old_name,
        const std::string& new_name
    );

    /**
     * @brief Duplicate a user preset
     * @param preset_name Name of preset to duplicate
     * @param new_name Name for duplicated preset
     * @return Duplicated preset if successful, nullptr otherwise
     */
    std::shared_ptr<Preset> duplicateUserPreset(
        const std::string& preset_name,
        const std::string& new_name
    );

    // ========== Preset Browsing ==========

    /**
     * @brief Search presets by name, description, or tags
     * @param query Search query
     * @param include_factory Include factory presets in search
     * @param include_user Include user presets in search
     * @return Vector of matching presets
     */
    std::vector<Preset> searchPresets(
        const std::string& query,
        bool include_factory = true,
        bool include_user = true
    ) const;

    /**
     * @brief Get presets by category
     * @param category Preset category
     * @return Vector of presets in category
     */
    std::vector<Preset> getPresetsByCategory(PresetCategory category) const;

    /**
     * @brief Get preset tags
     * @return Vector of all unique tags across presets
     */
    std::vector<std::string> getAllTags() const;

    // ========== Import/Export ==========

    /**
     * @brief Import a preset file
     * @param file_path Path to preset file
     * @param options Import options
     * @return Imported preset if successful, nullptr otherwise
     */
    std::shared_ptr<Preset> importPreset(
        const std::string& file_path,
        const PresetImportOptions& options = {}
    );

    /**
     * @brief Export a preset to file
     * @param preset Preset to export
     * @param file_path Destination file path
     * @param options Export options
     * @return true if exported successfully
     */
    bool exportPreset(
        const Preset& preset,
        const std::string& file_path,
        const PresetExportOptions& options = {}
    );

    /**
     * @brief Export all user presets to a zip archive
     * @param zip_path Path to zip file
     * @return true if exported successfully
     */
    bool exportAllUserPresets(const std::string& zip_path);

    // ========== Validation ==========

    /**
     * @brief Validate a preset
     * @param preset Preset to validate
     * @return Validation result
     */
    PresetValidationResult validatePreset(const Preset& preset) const;

    /**
     * @brief Validate a preset file without loading it
     * @param file_path Path to preset file
     * @return Validation result
     */
    PresetValidationResult validatePresetFile(const std::string& file_path) const;

    // ========== Favorites & Recent ==========

    /**
     * @brief Add preset to favorites
     * @param preset_name Name of preset to favorite
     * @return true if added successfully
     */
    bool addFavorite(const std::string& preset_name);

    /**
     * @brief Remove preset from favorites
     * @param preset_name Name of preset to unfavorite
     * @return true if removed successfully
     */
    bool removeFavorite(const std::string& preset_name);

    /**
     * @brief Check if preset is favorited
     * @param preset_name Name of preset to check
     * @return true if preset is favorited
     */
    bool isFavorite(const std::string& preset_name) const;

    /**
     * @brief Get all favorite presets
     * @return Vector of favorite presets
     */
    std::vector<Preset> getFavorites() const;

    /**
     * @brief Get recent presets
     * @param max_count Maximum number of recent presets to return
     * @return Vector of recent presets
     */
    std::vector<Preset> getRecentPresets(int max_count = 10) const;

    /**
     * @brief Add preset to recent list
     * @param preset Preset to add to recent list
     */
    void addToRecent(const Preset& preset);

    // ========== Current Preset ==========

    /**
     * @brief Apply a preset to the current engine state
     * @param preset Preset to apply
     * @return true if applied successfully
     */
    bool applyPreset(const Preset& preset);

    /**
     * @brief Get current preset parameters
     * @return Current preset parameters
     */
    PresetParameters getCurrentParameters() const;

    /**
     * @brief Set current preset parameters
     * @param params Parameters to set
     */
    void setCurrentParameters(const PresetParameters& params);

    // ========== Notifications ==========

    /**
     * @brief Register callback for preset changes
     * @param callback Function to call when preset changes
     * @return Callback ID for unregistering later
     */
    size_t registerPresetChangedCallback(PresetChangedCallback callback);

    /**
     * @brief Unregister preset change callback
     * @param callback_id Callback ID returned from registerPresetChangedCallback
     */
    void unregisterPresetChangedCallback(size_t callback_id);

    // ========== Utilities ==========

    /**
     * @brief Get default preset directory for user presets
     * @return Path to user preset directory
     */
    static std::string getDefaultUserPresetDir();

    /**
     * @brief Get default preset directory for factory presets
     * @return Path to factory preset directory
     */
    static std::string getDefaultFactoryPresetDir();

    /**
     * @brief Ensure preset directories exist
     */
    void ensurePresetDirectoriesExist();

    /**
     * @brief Get preset file extension
     * @return Preset file extension (including dot)
     */
    static constexpr const char* getPresetFileExtension() { return ".choirv2"; }

    /**
     * @brief Get current preset format version
     * @return Format version string
     */
    static constexpr const char* getFormatVersion() { return "2.0.0"; }

private:
    // Implementation details
    struct Impl;
    std::unique_ptr<Impl> impl_;

    // Private helper methods
    void loadUserPresets();
    void loadFavorites();
    void saveFavorites();
    void loadRecentPresets();
    void saveRecentPresets();
    bool matchesSearchQuery(const Preset& preset, const std::string& query) const;
    std::string toLower(const std::string& str) const;
    std::string sanitizeFilename(const std::string& name) const;
};

} // namespace ChoirV2
