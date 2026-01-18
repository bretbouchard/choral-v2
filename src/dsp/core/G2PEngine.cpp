/*
  ==============================================================================

    G2PEngine.cpp - PureDSP Grapheme-to-Phoneme Conversion Engine Implementation

    Copyright (c) 2026 Bret Bouchard
    All rights reserved.

    This file is part of the Choir V2 PureDSP implementation.

  ==============================================================================
*/

#include "dsp/core/G2PEngine.h"
#include "dsp/core/PhonemeDatabase.h"
#include "dsp/core/LanguageLoader.h"
#include <algorithm>
#include <cctype>
#include <chrono>
#include <sstream>
#include <regex>

namespace DSP {

//==============================================================================
// G2PResult Implementation
//==============================================================================

float G2PResult::getTotalDuration() const {
    float total = 0.0f;
    for (const auto& phoneme : phonemes) {
        total += phoneme.duration;
    }
    return total;
}

std::string G2PResult::getPhonemeString() const {
    std::stringstream ss;
    ss << "/";
    for (size_t i = 0; i < phonemes.size(); ++i) {
        ss << phonemes[i].symbol;
        if (i < phonemes.size() - 1) {
            ss << " ";
        }
    }
    ss << "/";
    return ss.str();
}

//==============================================================================
// G2PEngine Implementation
//==============================================================================

G2PEngine::G2PEngine(std::shared_ptr<PhonemeDatabase> db)
    : phoneme_db_(db)
    , current_language_(nullptr)
{
    // Initialize statistics
    last_stats_ = G2PConversionStats{};
}

G2PEngine::~G2PEngine() = default;

void G2PEngine::setLanguage(std::shared_ptr<LanguageDefinition> language) {
    current_language_ = language;

    // Load G2P rules from language definition
    if (language && !language->g2p_rules.empty()) {
        // Parse G2P rules from JSON
        // For now, we'll use a simple placeholder implementation
        // In production, this would parse the JSON and populate rules_
    }
}

G2PResult G2PEngine::convert(const std::string& text) {
    // Use default speech rate from language or 4.0 syllables/sec
    float speech_rate = 4.0f;
    if (current_language_) {
        speech_rate = current_language_->speech_rate;
    }

    return convertWithTiming(text, speech_rate);
}

G2PResult G2PEngine::convertWithTiming(
    const std::string& text,
    float speech_rate
) {
    auto start_time = std::chrono::high_resolution_clock::now();

    G2PResult result;
    result.success = false;

    // Reset statistics
    last_stats_ = G2PConversionStats{};

    if (!phoneme_db_) {
        result.error_message = "Phoneme database not initialized";
        return result;
    }

    // Tokenize input text
    std::vector<WordToken> tokens = tokenize(text);

    // Process each word
    std::vector<PhonemeResult> all_phonemes;
    std::vector<std::string> processed_words;

    for (size_t i = 0; i < tokens.size(); ++i) {
        const auto& token = tokens[i];

        // Skip punctuation and whitespace
        if (token.punctuation || token.whitespace) {
            continue;
        }

        processed_words.push_back(token.text);

        // Check dictionary first
        std::string word_lower = toLowerCase(token.text);
        auto dict_result = lookupDictionary(word_lower);

        if (dict_result.success) {
            // Found in dictionary
            last_stats_.dictionary_hits++;

            // Extract phoneme symbols from PhonemeResult
            std::vector<std::string> phoneme_symbols;
            for (const auto& pr : dict_result.phonemes) {
                phoneme_symbols.push_back(pr.symbol);
            }

            auto timed_phonemes = addTimingAndProsody(
                phoneme_symbols,
                speech_rate
            );
            all_phonemes.insert(
                all_phonemes.end(),
                timed_phonemes.begin(),
                timed_phonemes.end()
            );
        } else {
            // Apply G2P rules
            auto word_phonemes = applyRulesToWord(token.text, i);
            all_phonemes.insert(
                all_phonemes.end(),
                word_phonemes.begin(),
                word_phonemes.end()
            );
        }
    }

    // Detect stress patterns
    detectStress(all_phonemes);

    result.phonemes = all_phonemes;
    result.words = processed_words;
    result.success = true;

    // Calculate processing time
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        end_time - start_time
    );
    last_stats_.processing_time_ms = duration.count() / 1000.0f;

    return result;
}

void G2PEngine::addRule(const G2PRule& rule) {
    custom_rules_.push_back(rule);
}

void G2PEngine::addDictionaryEntry(
    const std::string& word,
    const std::vector<std::string>& phonemes
) {
    custom_dictionary_[toLowerCase(word)] = phonemes;
}

void G2PEngine::clearCustomRules() {
    custom_rules_.clear();
    custom_dictionary_.clear();
}

G2PResult G2PEngine::lookupDictionary(const std::string& word) {
    G2PResult result;
    result.success = false;

    // Check custom dictionary first (higher priority)
    auto it = custom_dictionary_.find(word);
    if (it != custom_dictionary_.end()) {
        result.phonemes.clear();
        // Convert to PhonemeResult format
        for (const auto& symbol : it->second) {
            PhonemeResult pr;
            pr.symbol = symbol;
            pr.duration = 0.0f;  // Will be set by timing logic
            pr.pitch_target = 0.0f;
            pr.stressed = false;
            pr.position = 0;
            pr.syllable = 0;
            result.phonemes.push_back(pr);
        }
        result.success = true;
        return result;
    }

    // Check main dictionary
    it = dictionary_.find(word);
    if (it != dictionary_.end()) {
        result.phonemes.clear();
        for (const auto& symbol : it->second) {
            PhonemeResult pr;
            pr.symbol = symbol;
            pr.duration = 0.0f;
            pr.pitch_target = 0.0f;
            pr.stressed = false;
            pr.position = 0;
            pr.syllable = 0;
            result.phonemes.push_back(pr);
        }
        result.success = true;
    }

    return result;
}

std::vector<WordToken> G2PEngine::tokenize(const std::string& text) {
    std::vector<WordToken> tokens;

    [[maybe_unused]] size_t pos = 0;
    size_t start = 0;
    bool in_word = false;

    for (size_t i = 0; i < text.size(); ++i) {
        char c = text[i];

        if (std::isalpha(static_cast<unsigned char>(c))) {
            if (!in_word) {
                start = i;
                in_word = true;
            }
        } else {
            if (in_word) {
                // End of word
                WordToken token;
                token.text = text.substr(start, i - start);
                token.start_pos = start;
                token.end_pos = i;
                token.punctuation = false;
                token.whitespace = false;
                tokens.push_back(token);
                in_word = false;
            }

            // Add punctuation/whitespace tokens
            if (std::isspace(static_cast<unsigned char>(c))) {
                WordToken token;
                token.text = std::string(1, c);
                token.start_pos = i;
                token.end_pos = i + 1;
                token.punctuation = false;
                token.whitespace = true;
                tokens.push_back(token);
            } else if (std::ispunct(static_cast<unsigned char>(c))) {
                WordToken token;
                token.text = std::string(1, c);
                token.start_pos = i;
                token.end_pos = i + 1;
                token.punctuation = true;
                token.whitespace = false;
                tokens.push_back(token);
            }
        }
    }

    // Handle last word
    if (in_word) {
        WordToken token;
        token.text = text.substr(start);
        token.start_pos = start;
        token.end_pos = text.size();
        token.punctuation = false;
        token.whitespace = false;
        tokens.push_back(token);
    }

    return tokens;
}

std::vector<PhonemeResult> G2PEngine::applyRulesToWord(
    const std::string& word,
    [[maybe_unused]] size_t word_position
) {
    std::vector<PhonemeResult> phonemes;
    std::string word_lower = toLowerCase(word);
    size_t pos = 0;

    // Combine custom rules (higher priority) with language rules
    std::vector<G2PRule> all_rules = custom_rules_;
    all_rules.insert(all_rules.end(), rules_.begin(), rules_.end());

    // Sort by priority (highest first)
    std::sort(all_rules.begin(), all_rules.end(),
        [](const G2PRule& a, const G2PRule& b) {
            return a.priority > b.priority;
        }
    );

    while (pos < word_lower.size()) {
        bool matched = false;

        // Try to match rules (longest match first)
        for (const auto& rule : all_rules) {
            size_t pattern_len = rule.pattern.size();

            // Check if pattern fits
            if (pos + pattern_len > word_lower.size()) {
                continue;
            }

            // Extract substring
            std::string substr = word_lower.substr(pos, pattern_len);

            // Check pattern match (using std::regex for PureDSP compatibility)
            std::regex pattern_regex(rule.pattern);
            if (!std::regex_search(substr, pattern_regex)) {
                continue;
            }

            // Check context constraints
            if (!matchesContext(rule, word_lower, pos)) {
                continue;
            }

            // Apply rule
            for (const auto& phoneme : rule.phonemes) {
                PhonemeResult pr;
                pr.symbol = phoneme;
                pr.duration = 0.0f;  // Will be set by timing logic
                pr.pitch_target = 0.0f;
                pr.stressed = false;
                pr.position = pos;
                pr.syllable = 0;
                phonemes.push_back(pr);
            }

            pos += pattern_len;
            matched = true;
            last_stats_.rule_matches++;
            break;
        }

        // If no rule matched, use character-to-phoneme mapping
        if (!matched) {
            // Default: use character as phoneme symbol
            PhonemeResult pr;
            pr.symbol = std::string(1, word_lower[pos]);
            pr.duration = 0.0f;
            pr.pitch_target = 0.0f;
            pr.stressed = false;
            pr.position = pos;
            pr.syllable = 0;
            phonemes.push_back(pr);
            pos++;
        }
    }

    return phonemes;
}

bool G2PEngine::matchesContext(
    const G2PRule& rule,
    const std::string& word,
    size_t match_pos
) {
    // Check word start constraint
    if (rule.word_start && match_pos != 0) {
        return false;
    }

    // Check word end constraint
    if (rule.word_end && match_pos + rule.pattern.size() != word.size()) {
        return false;
    }

    // Check preceding context
    if (!rule.preceding.empty()) {
        if (match_pos < rule.preceding.size()) {
            return false;
        }
        std::string actual_preceding = word.substr(
            match_pos - rule.preceding.size(),
            rule.preceding.size()
        );
        if (actual_preceding != rule.preceding) {
            return false;
        }
    }

    // Check following context
    if (!rule.following.empty()) {
        size_t following_pos = match_pos + rule.pattern.size();
        if (following_pos + rule.following.size() > word.size()) {
            return false;
        }
        std::string actual_following = word.substr(
            following_pos,
            rule.following.size()
        );
        if (actual_following != rule.following) {
            return false;
        }
    }

    // Check character class constraints
    if (!rule.char_class.empty()) {
        // Implement character class checks
        if (rule.char_class == "vowel") {
            // Check if preceding character is a vowel
            if (match_pos > 0 && !isVowel(word[match_pos - 1])) {
                return false;
            }
        } else if (rule.char_class == "consonant") {
            // Check if preceding character is a consonant
            if (match_pos > 0 && !isConsonant(word[match_pos - 1])) {
                return false;
            }
        }
    }

    return true;
}

std::vector<PhonemeResult> G2PEngine::addTimingAndProsody(
    const std::vector<std::string>& phonemes,
    float speech_rate
) {
    std::vector<PhonemeResult> results;

    [[maybe_unused]] float syllable_duration = 1.0f / speech_rate;  // seconds per syllable

    for (size_t i = 0; i < phonemes.size(); ++i) {
        PhonemeResult pr;
        pr.symbol = phonemes[i];
        pr.duration = estimatePhonemeDuration(phonemes[i]);
        pr.pitch_target = 0.0f;  // Will be set by prosody module
        pr.stressed = false;     // Will be set by stress detection
        pr.position = i;
        pr.syllable = 0;         // Will be set by syllable detection

        // Scale duration by speech rate
        pr.duration *= (4.0f / speech_rate);

        results.push_back(pr);
    }

    return results;
}

float G2PEngine::estimatePhonemeDuration(const std::string& phoneme) {
    // Check if phoneme exists in database
    if (phoneme_db_) {
        auto p = phoneme_db_->getPhoneme(phoneme);
        if (p) {
            // Convert milliseconds to seconds
            return p->temporal.default_duration / 1000.0f;
        }
    }

    // Default duration estimates based on phoneme type
    // Vowels: longer (100-150ms)
    // Consonants: shorter (50-100ms)
    if (phoneme.size() == 1) {
        char c = phoneme[0];
        if (isVowel(c)) {
            return 0.12f;  // 120ms for vowels
        } else {
            return 0.07f;  // 70ms for consonants
        }
    }

    // Multi-character phonemes (diphthongs, affricates, etc.)
    return 0.15f;  // 150ms default
}

void G2PEngine::detectStress(std::vector<PhonemeResult>& phonemes) {
    if (phonemes.empty()) {
        return;
    }

    // Simple stress detection: mark first vowel as stressed
    // In production, this would use language-specific stress rules
    bool found_vowel = false;

    for (auto& phoneme : phonemes) {
        // Check if phoneme is a vowel
        if (phoneme.symbol.size() == 1 && isVowel(phoneme.symbol[0])) {
            if (!found_vowel) {
                phoneme.stressed = true;
                found_vowel = true;
            } else {
                phoneme.stressed = false;
            }
        } else {
            phoneme.stressed = false;
        }
    }

    // If no vowels found, mark nothing as stressed
    if (!found_vowel) {
        for (auto& phoneme : phonemes) {
            phoneme.stressed = false;
        }
    }
}

std::string G2PEngine::toLowerCase(const std::string& text) const {
    std::string result = text;
    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c) { return std::tolower(c); }
    );
    return result;
}

bool G2PEngine::isVowel(char c) const {
    c = std::tolower(static_cast<unsigned char>(c));
    return c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u';
}

bool G2PEngine::isConsonant(char c) const {
    c = std::tolower(static_cast<unsigned char>(c));
    return std::isalpha(static_cast<unsigned char>(c)) && !isVowel(c);
}

std::string G2PEngine::getSyllableStressPattern([[maybe_unused]] const std::string& word) {
    // Placeholder for syllable stress detection
    // In production, this would implement language-specific stress rules
    return "";  // Return stress pattern (e.g., "010", "01", etc.)
}

bool G2PEngine::validatePhonemes(const std::vector<std::string>& phonemes) {
    if (!phoneme_db_) {
        return false;
    }

    for (const auto& phoneme : phonemes) {
        if (!phoneme_db_->hasPhoneme(phoneme)) {
            return false;
        }
    }

    return true;
}

} // namespace DSP
