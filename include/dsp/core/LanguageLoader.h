/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * LanguageLoader.h - Data-driven language definition loader (PureDSP)
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#pragma once

#include "PhonemeTypes.h"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include <mutex>

namespace DSP {

// Use nlohmann::json for JSON parsing
using json = nlohmann::json;

/**
 * @brief Language definition from JSON
 *
 * Stores all language-specific parameters loaded from JSON files.
 * This allows adding new languages without code changes.
 */
struct LanguageDefinition {
    // Metadata
    std::string name;             // Language name (e.g., "English (US)")
    std::string code;             // ISO 639-1 code (e.g., "en")
    std::string region;           // Region variant (e.g., "US")
    std::string version;          // Definition version

    // Phoneme inventory
    std::vector<std::string> phonemes;  // Phonemes used in this language

    // Phonotactics (rules about phoneme combinations)
    std::string syllable_structure;     // e.g., "CVC", "CVV", "(C)V(C)"
    std::vector<std::string> onset_clusters;   // Valid consonant clusters
    std::vector<std::string> coda_clusters;    // Valid ending clusters

    // Prosody
    float speech_rate;            // Syllables per second (typical)
    float pause_duration;         // Typical pause duration (seconds)
    std::vector<float> pitch_contours;  // F0 contours (JSON-encoded)

    // Grapheme-to-phoneme rules
    std::string g2p_rules;        // G2P rule set (JSON or XML format)

    // Common words and phrases (for testing)
    std::vector<std::string> test_words;
    std::vector<std::string> test_phrases;

    // Vocal technique parameters (for non-standard techniques)
    std::string technique_params;  // e.g., subharmonic, throat singing
};

/**
 * @brief Phoneme database interface
 *
 * Abstract interface for phoneme storage and retrieval.
 * Implementations can be thread-safe or single-threaded.
 */
class IPhonemeDatabase {
public:
    virtual ~IPhonemeDatabase() = default;

    /**
     * @brief Get a phoneme by symbol
     * @param symbol Phoneme symbol (e.g., "AA", "IY", "K")
     * @return Phoneme data, or nullptr if not found
     */
    virtual std::shared_ptr<Phoneme> getPhoneme(const std::string& symbol) const = 0;

    /**
     * @brief Check if a phoneme exists
     * @param symbol Phoneme symbol
     * @return true if phoneme exists in database
     */
    virtual bool hasPhoneme(const std::string& symbol) const = 0;

    /**
     * @brief Get number of phonemes in database
     */
    virtual size_t size() const = 0;
};

/**
 * @brief Language definition loader (PureDSP version)
 *
 * Loads language definitions from JSON files in languages/ directory.
 * Each language is self-contained and can be added/modified independently.
 *
 * Key differences from ChoirV2 version:
 * - Uses DSP namespace instead of ChoirV2
 * - No JUCE dependencies (uses nlohmann/json only)
 * - Interface-based phoneme database (IPhonemeDatabase)
 * - Real-time safe: JSON loading happens in prepare(), not process()
 * - Lightweight caching for fast lookup
 */
class LanguageLoader {
public:
    /**
     * @brief Constructor
     * @param db Phoneme database (can be nullptr for validation-only mode)
     */
    explicit LanguageLoader(std::shared_ptr<IPhonemeDatabase> db = nullptr);
    ~LanguageLoader();

    /**
     * @brief Load a language definition
     * @param language_file Path to JSON file (e.g., "languages/english.json")
     * @return Language definition, or nullptr if loading failed
     *
     * NOTE: This function performs file I/O and JSON parsing.
     * Do NOT call from real-time audio thread. Call during prepare() only.
     */
    std::shared_ptr<LanguageDefinition> loadLanguage(
        const std::string& language_file
    );

    /**
     * @brief Get all available languages
     * @return List of language codes and names
     *
     * Real-time safe: returns cached data, no file I/O.
     */
    std::vector<std::pair<std::string, std::string>> getAvailableLanguages() const;

    /**
     * @brief Reload a language (hot-reload for development)
     * @param language_file Path to JSON file
     * @return true if reloaded successfully
     *
     * NOTE: This function performs file I/O and JSON parsing.
     * Do NOT call from real-time audio thread. Call during prepare() only.
     */
    bool reloadLanguage(const std::string& language_file);

    /**
     * @brief Clear all cached languages
     *
     * Real-time safe: can be called from audio thread if no other operations
     * are in progress (use with caution).
     */
    void clearCache();

    /**
     * @brief Get a loaded language by code
     * @param language_code ISO 639-1 code (e.g., "en")
     * @return Language definition, or nullptr if not loaded
     *
     * Real-time safe: returns cached data, no file I/O.
     */
    std::shared_ptr<LanguageDefinition> getLanguage(const std::string& language_code) const;

private:
    std::shared_ptr<IPhonemeDatabase> phoneme_db_;
    std::unordered_map<std::string, std::shared_ptr<LanguageDefinition>> loaded_languages_;
    mutable std::mutex cache_mutex_;  // Protect cache for thread safety

    bool validateLanguage(const LanguageDefinition& lang);
    bool parseJSON(const std::string& json_content, LanguageDefinition& lang);
    void parsePhoneme(const std::string& phoneme_id, const json& phoneme_data);
};

} // namespace DSP
