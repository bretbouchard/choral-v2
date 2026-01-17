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
#include <unordered_map>
#include <mutex>
#include <nlohmann/json.hpp>
#include "Phoneme.h"

namespace ChoirV2 {

/**
 * @brief Conversion functions for PhonemeCategory enum
 */
std::string phonemeCategoryToString(PhonemeCategory category);
PhonemeCategory stringToPhonemeCategory(const std::string& str);

/**
 * @brief Universal phoneme database with JSON loading
 *
 * Manages the collection of all phonemes across all languages.
 * Phonemes are stored in JSON format in languages/ directory.
 *
 * Thread-safe for concurrent read access.
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
     * @param symbol Phoneme symbol (e.g., "AA", "IY", "K")
     * @return Phoneme data, or nullptr if not found
     */
    std::shared_ptr<Phoneme> getPhoneme(const std::string& symbol) const;

    /**
     * @brief Get a phoneme by IPA symbol
     * @param ipa IPA symbol (e.g., "/ɑ/", "/i/", "/k/")
     * @return Phoneme data, or nullptr if not found
     */
    std::shared_ptr<Phoneme> getPhonemeByIPA(const std::string& ipa) const;

    /**
     * @brief Get all phonemes for a category
     * @param category Category name (e.g., "vowel", "consonant", "plosive")
     * @return Vector of matching phonemes
     */
    std::vector<std::shared_ptr<Phoneme>> getByCategory(
        const std::string& category
    ) const;

    /**
     * @brief Get all loaded phonemes
     * @return Vector of all phonemes
     */
    std::vector<std::shared_ptr<Phoneme>> getAllPhonemes() const;

    /**
     * @brief Get number of phonemes in database
     */
    size_t size() const;

    /**
     * @brief Clear all loaded phonemes
     */
    void clear();

    /**
     * @brief Check if a phoneme exists
     * @param symbol Phoneme symbol
     * @return true if phoneme exists in database
     */
    bool hasPhoneme(const std::string& symbol) const;

    /**
     * @brief Get all categories present in database
     * @return Vector of category names
     */
    std::vector<std::string> getCategories() const;

    /**
     * @brief Create diphone transition data
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
    std::unordered_map<std::string, std::shared_ptr<Phoneme>> symbol_map_;
    std::unordered_map<std::string, std::shared_ptr<Phoneme>> ipa_map_;
    std::unordered_map<std::string, std::vector<std::shared_ptr<Phoneme>>> category_map_;
    mutable std::mutex mutex_;  // For thread-safe read access

    // JSON parsing helpers
    bool parsePhonemeObject(
        const nlohmann::json& phoneme_obj,
        std::shared_ptr<Phoneme>& phoneme
    );

    FormantData parseFormantData(const nlohmann::json& formants_obj);
    ArticulatoryFeatures parseArticulatoryFeatures(const nlohmann::json& articulatory_obj);
    TemporalFeatures parseTemporalFeatures(const nlohmann::json& temporal_obj);

    // Formant interpolation for diphones
    static float lerp(float a, float b, float t);
    FormantData lerpFormants(const FormantData& a, const FormantData& b, float t) const;
};

} // namespace ChoirV2
