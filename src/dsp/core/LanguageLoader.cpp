/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * LanguageLoader.cpp - Data-driven language definition loader (PureDSP)
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#include "dsp/core/LanguageLoader.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <mutex>

namespace DSP {

// Use nlohmann::json for JSON parsing
using json = nlohmann::json;

//==============================================================================
// LanguageLoader implementation
//==============================================================================

LanguageLoader::LanguageLoader(std::shared_ptr<IPhonemeDatabase> db)
    : phoneme_db_(db) {
}

LanguageLoader::~LanguageLoader() = default;

std::shared_ptr<LanguageDefinition> LanguageLoader::loadLanguage(
    const std::string& language_file
) {
    // Check cache first (real-time safe if cache hit)
    {
        std::lock_guard<std::mutex> lock(cache_mutex_);
        auto cached = loaded_languages_.find(language_file);
        if (cached != loaded_languages_.end()) {
            return cached->second;
        }
    }

    // Read file (NOT real-time safe - file I/O)
    std::ifstream file(language_file);
    if (!file.is_open()) {
        std::cerr << "LanguageLoader: Failed to open " << language_file << std::endl;
        return nullptr;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string json_content = buffer.str();

    // Parse JSON (NOT real-time safe - JSON parsing)
    auto lang_def = std::make_shared<LanguageDefinition>();
    if (!parseJSON(json_content, *lang_def)) {
        std::cerr << "LanguageLoader: Failed to parse " << language_file << std::endl;
        return nullptr;
    }

    // Validate language definition (real-time safe)
    if (!validateLanguage(*lang_def)) {
        std::cerr << "LanguageLoader: Validation failed for " << language_file << std::endl;
        return nullptr;
    }

    // Cache the loaded language (real-time safe with mutex)
    {
        std::lock_guard<std::mutex> lock(cache_mutex_);
        loaded_languages_[language_file] = lang_def;
    }

    return lang_def;
}

std::vector<std::pair<std::string, std::string>>
LanguageLoader::getAvailableLanguages() const {
    std::vector<std::pair<std::string, std::string>> languages;

    // Lock mutex for thread-safe cache access
    std::lock_guard<std::mutex> lock(cache_mutex_);

    for (const auto& entry : loaded_languages_) {
        languages.push_back({entry.second->code, entry.second->name});
    }

    // Sort by language code
    std::sort(languages.begin(), languages.end(),
        [](const auto& a, const auto& b) {
            return a.first < b.first;
        });

    return languages;
}

bool LanguageLoader::reloadLanguage(const std::string& language_file) {
    // Remove from cache (real-time safe with mutex)
    {
        std::lock_guard<std::mutex> lock(cache_mutex_);
        loaded_languages_.erase(language_file);
    }

    // Reload (NOT real-time safe - file I/O + JSON parsing)
    auto lang = loadLanguage(language_file);
    return lang != nullptr;
}

void LanguageLoader::clearCache() {
    // Clear all cached languages (real-time safe with mutex)
    std::lock_guard<std::mutex> lock(cache_mutex_);
    loaded_languages_.clear();
}

std::shared_ptr<LanguageDefinition> LanguageLoader::getLanguage(
    const std::string& language_code
) const {
    // Lock mutex for thread-safe cache access
    std::lock_guard<std::mutex> lock(cache_mutex_);

    for (const auto& entry : loaded_languages_) {
        if (entry.second->code == language_code) {
            return entry.second;
        }
    }

    return nullptr;
}

bool LanguageLoader::validateLanguage(const LanguageDefinition& lang) {
    // Check required metadata
    if (lang.name.empty()) {
        std::cerr << "LanguageLoader: Missing language name" << std::endl;
        return false;
    }

    if (lang.code.empty()) {
        std::cerr << "LanguageLoader: Missing language code" << std::endl;
        return false;
    }

    if (lang.phonemes.empty()) {
        std::cerr << "LanguageLoader: No phonemes defined" << std::endl;
        return false;
    }

    // Validate phoneme inventory against phoneme database
    if (phoneme_db_) {
        for (const auto& phoneme_id : lang.phonemes) {
            if (!phoneme_db_->getPhoneme(phoneme_id)) {
                std::cerr << "LanguageLoader: Phoneme " << phoneme_id
                         << " not found in database" << std::endl;
                return false;
            }
        }
    }

    // Check G2P rules
    if (lang.g2p_rules.empty()) {
        std::cerr << "LanguageLoader: No G2P rules defined" << std::endl;
        return false;
    }

    return true;
}

bool LanguageLoader::parseJSON(
    const std::string& json_content,
    LanguageDefinition& lang
) {
    try {
        auto json = nlohmann::json::parse(json_content);

        // Extract metadata
        if (json.contains("language_id")) {
            lang.code = json["language_id"].get<std::string>();
        }

        if (json.contains("language_name")) {
            lang.name = json["language_name"].get<std::string>();
        }

        if (json.contains("version")) {
            lang.version = json["version"].get<std::string>();
        }

        // Extract region if available
        if (json.contains("region")) {
            lang.region = json["region"].get<std::string>();
        }

        // Extract phonemes
        if (json.contains("phonemes")) {
            auto phonemes_json = json["phonemes"];
            if (phonemes_json.is_object()) {
                for (auto& [phoneme_id, phoneme_data] : phonemes_json.items()) {
                    lang.phonemes.push_back(phoneme_id);

                    // Load phoneme data into phoneme database
                    if (phoneme_db_) {
                        parsePhoneme(phoneme_id, phoneme_data);
                    }
                }
            }
        }

        // Extract G2P rules
        if (json.contains("grapheme_to_phoneme_rules")) {
            auto g2p_json = json["grapheme_to_phoneme_rules"];
            if (g2p_json.is_array()) {
                std::stringstream g2p_ss;
                for (const auto& rule : g2p_json) {
                    if (rule.is_object()) {
                        if (rule.contains("pattern")) {
                            g2p_ss << rule["pattern"].get<std::string>();
                        }
                        if (rule.contains("replacement")) {
                            g2p_ss << " -> " << rule["replacement"].get<std::string>();
                        }
                        if (rule.contains("context")) {
                            g2p_ss << " (" << rule["context"].get<std::string>() << ")";
                        }
                        if (rule.contains("priority")) {
                            g2p_ss << " [p=" << rule["priority"].get<int>() << "]";
                        }
                        g2p_ss << "\n";
                    }
                }
                lang.g2p_rules = g2p_ss.str();
            }
        }

        // Extract stress rules
        if (json.contains("stress_rules")) {
            auto stress_json = json["stress_rules"];
            if (stress_json.is_array()) {
                lang.pitch_contours.push_back(0.5f); // Default neutral contour
            }
        }

        // Extract test words
        if (json.contains("test_words")) {
            auto test_json = json["test_words"];
            if (test_json.is_array()) {
                for (const auto& word : test_json) {
                    if (word.is_string()) {
                        lang.test_words.push_back(word.get<std::string>());
                    }
                }
            }
        }

        // Extract synthesis parameters
        if (json.contains("synthesis_params")) {
            auto synth_json = json["synthesis_params"];
            if (synth_json.is_object()) {
                if (synth_json.contains("default_pitch")) {
                    lang.pitch_contours.push_back(
                        synth_json["default_pitch"].get<float>()
                    );
                }

                if (synth_json.contains("speech_rate")) {
                    lang.speech_rate = synth_json["speech_rate"].get<float>();
                } else {
                    lang.speech_rate = 4.0f;
                }

                if (synth_json.contains("pause_duration")) {
                    lang.pause_duration = synth_json["pause_duration"].get<float>();
                } else {
                    lang.pause_duration = 0.2f;
                }
            }
        }

        // Extract syllable structure
        if (json.contains("syllable_structure")) {
            lang.syllable_structure = json["syllable_structure"].get<std::string>();
        } else {
            lang.syllable_structure = "(C)V(C)";
        }

        // Extract technique parameters for subharmonic methods
        if (json.contains("synthesis_method")) {
            auto method = json["synthesis_method"].get<std::string>();
            if (method == "subharmonic" && json.contains("presets")) {
                lang.technique_params = json["presets"].dump();
            }
        }

        return true;
    } catch (const std::exception& e) {
        std::cerr << "LanguageLoader: JSON parsing error: " << e.what() << std::endl;
        return false;
    }
}

void LanguageLoader::parsePhoneme(
    const std::string& phoneme_id,
    const nlohmann::json& phoneme_data
) {
    if (!phoneme_data.is_object()) {
        return;
    }

    // Note: In PureDSP version, we don't store phonemes directly
    // The phoneme_db_ is an interface, and implementations handle storage
    // This function just validates the phoneme structure

    // Extract IPA symbol
    if (!phoneme_data.contains("ipa")) {
        std::cerr << "LanguageLoader: Phoneme " << phoneme_id
                 << " missing IPA symbol" << std::endl;
        return;
    }

    // Extract category - CONVERT STRING TO ENUM
    if (phoneme_data.contains("category")) {
        std::string category_str = phoneme_data["category"].get<std::string>();
        auto category = stringToPhonemeCategory(category_str);

        // Validate category
        if (category == PhonemeCategory::Vowel && category_str != "vowel") {
            std::cerr << "LanguageLoader: Unknown category '" << category_str
                     << "' for phoneme " << phoneme_id << std::endl;
        }
    }

    // Extract formants (basic validation)
    if (phoneme_data.contains("formants")) {
        auto formants = phoneme_data["formants"];
        if (formants.is_object()) {
            // Validate formant structure
            if (formants.contains("frequencies") && formants["frequencies"].is_array()) {
                auto freqs = formants["frequencies"];
                if (freqs.size() < 4) {
                    std::cerr << "LanguageLoader: Phoneme " << phoneme_id
                             << " has insufficient formant frequencies" << std::endl;
                }
            }
        }
    }

    // Note: Actual phoneme storage is handled by IPhonemeDatabase implementation
    // This version focuses on validation and structure checking
}

} // namespace DSP
