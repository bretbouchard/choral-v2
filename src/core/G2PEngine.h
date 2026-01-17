/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * G2PEngine.h - Grapheme-to-Phoneme conversion engine
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
class LanguageDefinition;

/**
 * @brief Grapheme-to-Phoneme result
 *
 * Represents the conversion of text to phoneme sequence.
 */
struct G2PResult {
    std::vector<std::string> phonemes;  // Phoneme sequence (IPA symbols)
    std::vector<float> durations;       // Duration for each phoneme
    std::vector<float> pitches;         // Pitch targets for each phoneme
    bool success;                       // Conversion successful?
    std::string error_message;          // Error details if failed
};

/**
 * @brief Grapheme-to-Phoneme conversion engine
 *
 * Converts text (graphemes) to phoneme sequences using language-specific rules.
 * Supports multiple G2P methods:
 * - Rule-based (dictionary + letter-to-sound rules)
 * - Statistical (n-gram models)
 * - Neural (external models, optional)
 */
class G2PEngine {
public:
    G2PEngine(std::shared_ptr<PhonemeDatabase> db);
    ~G2PEngine();

    /**
     * @brief Set the current language
     * @param language Language definition to use
     */
    void setLanguage(std::shared_ptr<LanguageDefinition> language);

    /**
     * @brief Convert text to phonemes
     * @param text Input text (graphemes)
     * @return G2P result with phoneme sequence
     */
    G2PResult convert(const std::string& text);

    /**
     * @brief Convert with custom timing
     * @param text Input text
     * @param speech_rate Syllables per second
     * @return G2P result with custom durations
     */
    G2PResult convertWithTiming(
        const std::string& text,
        float speech_rate
    );

    /**
     * @brief Add a custom pronunciation rule
     * @param grapheme Text pattern
     * @param phonemes Phoneme substitution
     */
    addRule(const std::string& grapheme, const std::vector<std::string>& phonemes);

private:
    std::shared_ptr<PhonemeDatabase> phoneme_db_;
    std::shared_ptr<LanguageDefinition> current_language_;

    // G2P rules (grapheme -> phoneme mapping)
    std::unordered_map<std::string, std::vector<std::string>> rules_;

    // Dictionary for common words
    std::unordered_map<std::string, std::vector<std::string>> dictionary_;

    G2PResult applyRules(const std::string& text);
    G2PResult lookupDictionary(const std::string& word);
    std::vector<std::string> tokenize(const std::string& text);
};

} // namespace ChoirV2
