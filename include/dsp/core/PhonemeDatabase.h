/*
  ==============================================================================

    PhonemeDatabase.h
    Created: January 18, 2026
    Author:  Bret Bouchard

    PureDSP phoneme database - No JUCE dependencies

    Universal phoneme database with JSON loading for multilingual
    vocal synthesis. Thread-safe for concurrent read access.

  ==============================================================================
*/

#pragma once

#include "PhonemeTypes.h"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <mutex>

namespace DSP {

using json = nlohmann::json;

/**
 * @brief Universal phoneme database with JSON loading
 *
 * Manages the collection of all phonemes across all languages.
 * Phonemes are stored in JSON format in languages/ directory.
 *
 * Thread-safe for concurrent read access.
 *
 * Key Features:
 * - Load phonemes from JSON files
 * - Query by symbol or IPA
 * - Filter by category
 * - Real-time safe (no allocations in audio path after loading)
 * - Thread-safe concurrent reads
 *
 * Supported Languages (25+):
 * English, Spanish, French, German, Italian, Portuguese, Dutch,
 * Swedish, Norwegian, Danish, Finnish, Polish, Czech, Hungarian,
 * Romanian, Turkish, Russian, Japanese, Mandarin Chinese, Korean,
 * Arabic, Hebrew, Hindi, Thai, Vietnamese
 */
class PhonemeDatabase {
public:
    //==========================================================================
    // Construction/Destruction
    //==========================================================================

    PhonemeDatabase();
    ~PhonemeDatabase();

    //==========================================================================
    // Database Loading
    //==========================================================================

    /**
     * @brief Load phonemes from a language file
     *
     * Parses JSON file containing phoneme definitions and adds them
     * to the database. Thread-safe with exclusive locking.
     *
     * @param language_file Path to JSON file (e.g., "languages/english.json")
     * @return true if loaded successfully
     *
     * JSON Format:
     * ```json
     * {
     *   "phonemes": {
     *     "AA": {
     *       "ipa": "/ɑ/",
     *       "category": "vowel",
     *       "formants": {
     *         "frequencies": [750, 1150, 2500, 3500],
     *         "bandwidths": [80, 90, 120, 130]
     *       },
     *       "articulatory": {
     *         "is_nasal": false,
     *         "is_rounded": true,
     *         "is_voiced": true
     *       },
     *       "temporal": {
     *         "min_duration": 50,
     *         "max_duration": 300,
     *         "default_duration": 150
     *       }
     *     }
     *   }
     * }
     * ```
     */
    bool loadLanguage(const std::string& language_file);

    //==========================================================================
    // Phoneme Query
    //==========================================================================

    /**
     * @brief Get a phoneme by symbol
     *
     * Thread-safe read access. Returns shared_ptr for safe concurrent access.
     *
     * @param symbol Phoneme symbol (e.g., "AA", "IY", "K")
     * @return Phoneme data, or nullptr if not found
     */
    std::shared_ptr<Phoneme> getPhoneme(const std::string& symbol) const;

    /**
     * @brief Get a phoneme by IPA symbol
     *
     * Thread-safe read access.
     *
     * @param ipa IPA symbol (e.g., "/ɑ/", "/i/", "/k/")
     * @return Phoneme data, or nullptr if not found
     */
    std::shared_ptr<Phoneme> getPhonemeByIPA(const std::string& ipa) const;

    /**
     * @brief Get all phonemes for a category
     *
     * Thread-safe read access.
     *
     * @param category Category name (e.g., "vowel", "consonant", "plosive")
     * @return Vector of matching phonemes
     */
    std::vector<std::shared_ptr<Phoneme>> getByCategory(
        const std::string& category
    ) const;

    /**
     * @brief Get all loaded phonemes
     *
     * Thread-safe read access.
     *
     * @return Vector of all phonemes
     */
    std::vector<std::shared_ptr<Phoneme>> getAllPhonemes() const;

    //==========================================================================
    // Database Management
    //==========================================================================

    /**
     * @brief Get number of phonemes in database
     *
     * Thread-safe read access.
     *
     * @return Number of phonemes
     */
    size_t size() const;

    /**
     * @brief Clear all loaded phonemes
     *
     * Thread-safe with exclusive locking.
     */
    void clear();

    /**
     * @brief Check if a phoneme exists
     *
     * Thread-safe read access.
     *
     * @param symbol Phoneme symbol
     * @return true if phoneme exists in database
     */
    bool hasPhoneme(const std::string& symbol) const;

    /**
     * @brief Get all categories present in database
     *
     * Thread-safe read access.
     *
     * @return Vector of category names
     */
    std::vector<std::string> getCategories() const;

    //==========================================================================
    // Diphone Synthesis
    //==========================================================================

    /**
     * @brief Create diphone transition data
     *
     * Interpolates between two phonemes for smooth coarticulation.
     * Real-time safe (no allocations).
     *
     * @param from Phoneme to transition from
     * @param to Phoneme to transition to
     * @param t Transition point (0.0 = from, 1.0 = to)
     * @return Interpolated formant data
     */
    FormantData createDiphone(
        const Phoneme& from,
        const Phoneme& to,
        float t
    ) const;

private:
    //==========================================================================
    // Internal Data
    //==========================================================================

    std::unordered_map<std::string, std::shared_ptr<Phoneme>> symbol_map_;
    std::unordered_map<std::string, std::shared_ptr<Phoneme>> ipa_map_;
    std::unordered_map<std::string, std::vector<std::shared_ptr<Phoneme>>> category_map_;
    mutable std::mutex mutex_;  // For thread-safe read access

    //==========================================================================
    // JSON Parsing Helpers
    //==========================================================================

    /**
     * @brief Parse phoneme object from JSON
     *
     * Extracts all phoneme data from JSON object.
     *
     * @param phoneme_obj JSON object containing phoneme data
     * @param phoneme Output phoneme structure
     * @return true if parsing succeeded
     */
    bool parsePhonemeObject(
        const json& phoneme_obj,
        std::shared_ptr<Phoneme>& phoneme
    );

    /**
     * @brief Parse formant data from JSON
     *
     * Extracts formant frequencies and bandwidths.
     * Supports both array and individual field formats.
     *
     * @param formants_obj JSON object with formant data
     * @return FormantData structure
     */
    FormantData parseFormantData(const json& formants_obj);

    /**
     * @brief Parse articulatory features from JSON
     *
     * Extracts boolean articulatory flags.
     *
     * @param articulatory_obj JSON object with articulatory data
     * @return ArticulatoryFeatures structure
     */
    ArticulatoryFeatures parseArticulatoryFeatures(const json& articulatory_obj);

    /**
     * @brief Parse temporal features from JSON
     *
     * Extracts duration parameters.
     *
     * @param temporal_obj JSON object with temporal data
     * @return TemporalFeatures structure
     */
    TemporalFeatures parseTemporalFeatures(const json& temporal_obj);

    //==========================================================================
    // Formant Interpolation
    //==========================================================================

    /**
     * @brief Linear interpolation helper
     *
     * Real-time safe linear interpolation.
     *
     * @param a Start value
     * @param b End value
     * @param t Interpolation factor (0.0 to 1.0)
     * @return Interpolated value
     */
    static float lerp(float a, float b, float t);

    /**
     * @brief Interpolate formant data
     *
     * Real-time safe formant interpolation for diphone synthesis.
     *
     * @param a Start formant data
     * @param b End formant data
     * @param t Interpolation factor (0.0 to 1.0)
     * @return Interpolated formant data
     */
    FormantData lerpFormants(const FormantData& a, const FormantData& b, float t) const;
};

} // namespace DSP
