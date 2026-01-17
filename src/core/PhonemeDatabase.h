/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * PhonemeDatabase.h - Universal phoneme representation database
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#pragma once

#include <string>
#include <vector>
#include <memory>

namespace ChoirV2 {

/**
 * @brief Phoneme representation with acoustic properties
 *
 * Stores the acoustic characteristics of a single phoneme
 * including formants, duration, and transition parameters.
 */
struct Phoneme {
    std::string symbol;           // IPA symbol
    std::string name;             // Descriptive name
    std::string category;         // vowel, consonant, diphone, etc.

    // Formant frequencies (Hz)
    float f1;                     // First formant
    float f2;                     // Second formant
    float f3;                     // Third formant
    float f4;                     // Fourth formant (optional)

    // Formant bandwidths (Hz)
    float b1;                     // First formant bandwidth
    float b2;                     // Second formant bandwidth
    float b3;                     // Third formant bandwidth

    // Temporal characteristics
    float min_duration;           // Minimum duration (seconds)
    float max_duration;           // Maximum duration (seconds)
    float default_duration;       // Default duration (seconds)

    // Transition parameters
    float transition_time;        // Time to transition to/from this phoneme
    float spectral_continuity;    // How smoothly spectra connect (0-1)

    // Voicing
    bool voiced;                  // Is this a voiced phoneme?
    float pitch_mod;              // Pitch modulation amount (0-1)

    // Language-specific overrides (JSON-based)
    std::string language_overrides; // JSON string with language-specific params
};

/**
 * @brief Universal phoneme database
 *
 * Manages the collection of all phonemes across all languages.
 * Phonemes are stored in JSON format in languages/ directory.
 */
class PhonemeDatabase {
public:
    PhonemeDatabase();
    ~PhonemeDatabase();

    /**
     * @brief Load phonemes from a language file
     * @param language_file Path to JSON file (e.g., "languages/english.json")
     * @return true if loaded successfully
     */
    bool loadLanguage(const std::string& language_file);

    /**
     * @brief Get a phoneme by symbol
     * @param symbol IPA symbol (e.g., "ə", "æ", "θ")
     * @return Phoneme data, or nullptr if not found
     */
    std::shared_ptr<Phoneme> getPhoneme(const std::string& symbol) const;

    /**
     * @brief Get all phonemes for a category
     * @param category Category name (e.g., "vowel", "consonant")
     * @return Vector of matching phonemes
     */
    std::vector<std::shared_ptr<Phoneme>> getByCategory(
        const std::string& category
    ) const;

    /**
     * @brief Get number of phonemes in database
     */
    size_t size() const { return phonemes_.size(); }

private:
    std::vector<std::shared_ptr<Phoneme>> phonemes_;
    std::unordered_map<std::string, std::shared_ptr<Phoneme>> symbol_map_;

    bool parseJSON(const std::string& json_content);
};

} // namespace ChoirV2
