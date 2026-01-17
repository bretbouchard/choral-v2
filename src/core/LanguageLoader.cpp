/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * LanguageLoader.cpp - Data-driven language definition loader
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#include "LanguageLoader.h"
#include "PhonemeDatabase.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>

namespace ChoirV2 {

LanguageLoader::LanguageLoader(std::shared_ptr<PhonemeDatabase> db)
    : phoneme_db_(db) {
}

LanguageLoader::~LanguageLoader() = default;

std::shared_ptr<LanguageDefinition> LanguageLoader::loadLanguage(
    const std::string& language_file
) {
    // Check cache first
    auto cached = loaded_languages_.find(language_file);
    if (cached != loaded_languages_.end()) {
        return cached->second;
    }

    // Read file
    std::ifstream file(language_file);
    if (!file.is_open()) {
        std::cerr << "LanguageLoader: Failed to open " << language_file << std::endl;
        return nullptr;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string json_content = buffer.str();

    // Parse JSON
    auto lang_def = std::make_shared<LanguageDefinition>();
    if (!parseJSON(json_content, *lang_def)) {
        std::cerr << "LanguageLoader: Failed to parse " << language_file << std::endl;
        return nullptr;
    }

    // Validate language definition
    if (!validateLanguage(*lang_def)) {
        std::cerr << "LanguageLoader: Validation failed for " << language_file << std::endl;
        return nullptr;
    }

    // Cache the loaded language
    loaded_languages_[language_file] = lang_def;

    return lang_def;
}

std::vector<std::pair<std::string, std::string>>
LanguageLoader::getAvailableLanguages() const {
    std::vector<std::pair<std::string, std::string>> languages;

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
    // Remove from cache
    loaded_languages_.erase(language_file);

    // Reload
    auto lang = loadLanguage(language_file);
    return lang != nullptr;
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
    for (const auto& phoneme_id : lang.phonemes) {
        if (!phoneme_db_ || !phoneme_db_->getPhoneme(phoneme_id)) {
            std::cerr << "LanguageLoader: Phoneme " << phoneme_id
                     << " not found in database" << std::endl;
            return false;
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
    if (!phoneme_db_) {
        return;
    }

    if (!phoneme_data.is_object()) {
        return;
    }

    // Create phoneme structure
    Phoneme phoneme;
    phoneme.id = phoneme_id;

    // Extract IPA symbol
    if (phoneme_data.contains("ipa")) {
        phoneme.ipa = phoneme_data["ipa"].get<std::string>();
    }


    // Extract category - CONVERT STRING TO ENUM
    if (phoneme_data.contains("category")) {
        std::string category_str = phoneme_data["category"].get<std::string>();
        phoneme.category = stringToPhonemeCategory(category_str);
    }

    // Extract formants
    if (phoneme_data.contains("formants")) {
        auto formants = phoneme_data["formants"];
        if (formants.is_object()) {
            // Support both array and individual field formats
            if (formants.contains("frequencies") && formants["frequencies"].is_array()) {
                auto freqs = formants["frequencies"];
                for (size_t i = 0; i < 4 && i < freqs.size(); ++i) {
                    phoneme.formants.frequencies[i] = freqs[i].get<float>();
                }
            } else {
                phoneme.formants.frequencies[0] = formants.value("f1", 500.0f);
                phoneme.formants.frequencies[1] = formants.value("f2", 1500.0f);
                phoneme.formants.frequencies[2] = formants.value("f3", 2500.0f);
                phoneme.formants.frequencies[3] = formants.value("f4", 3500.0f);
            }
            
            if (formants.contains("bandwidths") && formants["bandwidths"].is_array()) {
                auto bws = formants["bandwidths"];
                for (size_t i = 0; i < 4 && i < bws.size(); ++i) {
                    phoneme.formants.bandwidths[i] = bws[i].get<float>();
                }
            } else {
                phoneme.formants.bandwidths[0] = formants.value("bw1", 50.0f);
                phoneme.formants.bandwidths[1] = formants.value("bw2", 80.0f);
                phoneme.formants.bandwidths[2] = formants.value("bw3", 120.0f);
                phoneme.formants.bandwidths[3] = formants.value("bw4", 150.0f);
            }
        }
    }

    // Extract articulatory features
    if (phoneme_data.contains("articulatory")) {
        auto articulatory = phoneme_data["articulatory"];
        if (articulatory.is_object()) {
            phoneme.articulatory.is_nasal = articulatory.value("is_nasal", false);
            phoneme.articulatory.is_rounded = articulatory.value("is_rounded", false);
            phoneme.articulatory.is_voiced = articulatory.value("is_voiced", true);
            phoneme.articulatory.is_lateral = articulatory.value("is_lateral", false);
            phoneme.articulatory.is_rhotic = articulatory.value("is_rhotic", false);
        }
    }

    // Extract temporal characteristics
    if (phoneme_data.contains("temporal")) {
        auto temporal = phoneme_data["temporal"];
        if (temporal.is_object()) {
            phoneme.temporal.min_duration = temporal.value("min_duration", 50);
            phoneme.temporal.max_duration = temporal.value("max_duration", 500);
            phoneme.temporal.default_duration = temporal.value("default_duration", 200);
        }
    }

    // Note: PhonemeDatabase would need to be updated to store phonemes
    // For now, we're just validating the structure
}

} // namespace ChoirV2
