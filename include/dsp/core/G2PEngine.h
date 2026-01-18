/*
  ==============================================================================

    G2PEngine.h - PureDSP Grapheme-to-Phoneme Conversion Engine

    Copyright (c) 2026 Bret Bouchard
    All rights reserved.

    This file is part of the Choir V2 PureDSP implementation.

    Overview:
    --------
    Converts text (graphemes) to phoneme sequences for vocal synthesis.
    Supports multiple languages and real-time safe text processing.

    Design Principles:
    -----------------
    - Pure C++20 (no JUCE dependencies)
    - Real-time safe (text processing in prepare(), not process())
    - Deterministic output (same input = same output)
    - std::regex for pattern matching (PureDSP-compatible)

  ==============================================================================
*/

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <regex>

namespace DSP {

//==============================================================================
// Forward Declarations
//==============================================================================

class PhonemeDatabase;
struct LanguageDefinition;

//==============================================================================
// G2P Rule with Context Sensitivity
//==============================================================================

/**
 * @brief G2P rule with context sensitivity
 *
 * Represents a single grapheme-to-phoneme conversion rule
 * with optional context constraints.
 */
struct G2PRule {
    std::string pattern;              // Grapheme pattern (regex or literal)
    std::vector<std::string> phonemes; // Replacement phonemes
    std::string context;              // Context description
    int priority;                     // Rule priority (higher = more specific)

    // Context constraints
    std::string preceding;            // Required preceding characters
    std::string following;            // Required following characters
    bool word_start;                  // Must be at word start
    bool word_end;                    // Must be at word end

    // Character class constraints
    std::string char_class;           // vowel, consonant, etc.
};

//==============================================================================
// Phoneme with Timing and Metadata
//==============================================================================

/**
 * @brief Phoneme with timing and metadata
 *
 * Represents a single phoneme in the output sequence with
 * associated timing and prosodic information.
 */
struct PhonemeResult {
    std::string symbol;               // IPA or phoneme symbol
    float duration;                   // Duration in seconds
    float pitch_target;               // F0 target (Hz)
    bool stressed;                    // Is this syllable stressed?
    size_t position;                  // Position in word
    size_t syllable;                  // Syllable index (0-based)
};

//==============================================================================
// G2P Conversion Result
//==============================================================================

/**
 * @brief Grapheme-to-Phoneme conversion result
 *
 * Complete result of text-to-phoneme conversion including
 * phoneme sequence, timing, and metadata.
 */
struct G2PResult {
    std::vector<PhonemeResult> phonemes;  // Phoneme sequence with metadata
    std::vector<std::string> words;       // Original word tokens
    bool success;                         // Conversion successful?
    std::string error_message;            // Error details if failed

    // Convenience methods
    size_t getPhonemeCount() const { return phonemes.size(); }
    float getTotalDuration() const;
    std::string getPhonemeString() const;
};

//==============================================================================
// Word Token with Metadata
//==============================================================================

/**
 * @brief Word token with metadata
 *
 * Represents a single word tokenized from input text
 * with position and context information.
 */
struct WordToken {
    std::string text;               // Word text
    size_t start_pos;              // Start position in original text
    size_t end_pos;                // End position in original text
    bool punctuation;              // Is this punctuation?
    bool whitespace;               // Is this whitespace?
};

//==============================================================================
// G2P Conversion Statistics
//==============================================================================

/**
 * @brief Statistics about last conversion
 */
struct G2PConversionStats {
    size_t dictionary_hits;      // Words found in dictionary
    size_t rule_matches;         // Rules applied
    size_t exceptions_handled;   // Special cases processed
    float processing_time_ms;    // Processing time in milliseconds
};

//==============================================================================
// Main G2P Engine
//==============================================================================

/**
 * @brief Grapheme-to-Phoneme conversion engine
 *
 * Converts text (graphemes) to phoneme sequences using language-specific rules.
 * Supports multiple G2P methods:
 * - Dictionary lookup (for common words)
 * - Rule-based conversion (letter-to-sound rules)
 * - Context-sensitive rules (prefix, suffix, character class)
 * - Exception handling (special case mappings)
 *
 * Algorithm:
 * 1. Tokenize input text into words
 * 2. Apply dictionary lookup (if available)
 * 3. Apply G2P rules left-to-right (longest match first)
 * 4. Handle context-sensitive rules
 * 5. Apply exceptions and special cases
 * 6. Generate timing and prosody estimates
 *
 * Real-Time Safety:
 * - Text processing happens in prepare() phase, not audio thread
 * - No dynamic memory allocation during conversion
 * - All operations are deterministic
 */
class G2PEngine {
public:
    //==========================================================================
    // Construction/Destruction
    //==========================================================================

    /**
     * @brief Constructor
     * @param db Phoneme database for validation
     */
    explicit G2PEngine(std::shared_ptr<PhonemeDatabase> db);

    /**
     * @brief Destructor
     */
    ~G2PEngine();

    //==========================================================================
    // Language Management
    //==========================================================================

    /**
     * @brief Set the current language
     * @param language Language definition to use
     */
    void setLanguage(std::shared_ptr<LanguageDefinition> language);

    //==========================================================================
    // Text Conversion
    //==========================================================================

    /**
     * @brief Convert text to phonemes
     * @param text Input text (graphemes)
     * @return G2P result with phoneme sequence and timing
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

    //==========================================================================
    // Rule Management
    //==========================================================================

    /**
     * @brief Add a custom G2P rule
     * @param rule G2P rule to add
     */
    void addRule(const G2PRule& rule);

    /**
     * @brief Add a dictionary entry
     * @param word Word text
     * @param phonemes Phoneme sequence
     */
    void addDictionaryEntry(
        const std::string& word,
        const std::vector<std::string>& phonemes
    );

    /**
     * @brief Clear all custom rules and dictionary entries
     */
    void clearCustomRules();

    //==========================================================================
    // Statistics
    //==========================================================================

    /**
     * @brief Get statistics about last conversion
     */
    G2PConversionStats getLastStats() const { return last_stats_; }

private:
    //==========================================================================
    // Member Variables
    //==========================================================================

    std::shared_ptr<PhonemeDatabase> phoneme_db_;
    std::shared_ptr<LanguageDefinition> current_language_;

    // G2P rules (loaded from language definition)
    std::vector<G2PRule> rules_;

    // Dictionary for common words
    std::unordered_map<std::string, std::vector<std::string>> dictionary_;

    // Custom rules (user-added, higher priority)
    std::vector<G2PRule> custom_rules_;

    // Custom dictionary entries
    std::unordered_map<std::string, std::vector<std::string>> custom_dictionary_;

    // Statistics
    G2PConversionStats last_stats_;

    //==========================================================================
    // Conversion Methods
    //==========================================================================

    G2PResult applyRules(const std::string& text);
    G2PResult lookupDictionary(const std::string& word);
    std::vector<WordToken> tokenize(const std::string& text);

    // Rule application
    std::vector<PhonemeResult> applyRulesToWord(
        const std::string& word,
        size_t word_position
    );

    bool matchesContext(
        const G2PRule& rule,
        const std::string& word,
        size_t match_pos
    );

    //==========================================================================
    // Timing and Prosody
    //==========================================================================

    std::vector<PhonemeResult> addTimingAndProsody(
        const std::vector<std::string>& phonemes,
        float speech_rate
    );

    float estimatePhonemeDuration(const std::string& phoneme);
    void detectStress(std::vector<PhonemeResult>& phonemes);

    //==========================================================================
    // Helper Methods
    //==========================================================================

    std::string toLowerCase(const std::string& text) const;
    bool isVowel(char c) const;
    bool isConsonant(char c) const;
    std::string getSyllableStressPattern(const std::string& word);

    //==========================================================================
    // Validation
    //==========================================================================

    bool validatePhonemes(const std::vector<std::string>& phonemes);
};

} // namespace DSP
