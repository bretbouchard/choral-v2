/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * LanguageLoader.h - Data-driven language definition loader
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#pragma once

#include <string>
#include <vector>
#include <memory>

namespace ChoirV2 {

// Forward declarations
class PhonemeDatabase;
class G2PEngine;

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
 * @brief Language definition loader
 *
 * Loads language definitions from JSON files in languages/ directory.
 * Each language is self-contained and can be added/modified independently.
 */
class LanguageLoader {
public:
    LanguageLoader(std::shared_ptr<PhonemeDatabase> db);
    ~LanguageLoader();

    /**
     * @brief Load a language definition
     * @param language_file Path to JSON file (e.g., "languages/english.json")
     * @return Language definition, or nullptr if loading failed
     */
    std::shared_ptr<LanguageDefinition> loadLanguage(
        const std::string& language_file
    );

    /**
     * @brief Get all available languages
     * @return List of language codes and names
     */
    std::vector<std::pair<std::string, std::string>> getAvailableLanguages() const;

    /**
     * @brief Reload a language (hot-reload for development)
     * @param language_file Path to JSON file
     * @return true if reloaded successfully
     */
    bool reloadLanguage(const std::string& language_file);

private:
    std::shared_ptr<PhonemeDatabase> phoneme_db_;
    std::unordered_map<std::string, std::shared_ptr<LanguageDefinition>> loaded_languages_;

    bool validateLanguage(const LanguageDefinition& lang);
    bool parseJSON(const std::string& json_content, LanguageDefinition& lang);
};

} // namespace ChoirV2
