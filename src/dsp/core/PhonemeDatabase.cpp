/*
  ==============================================================================

    PhonemeDatabase.cpp
    Created: January 18, 2026
    Author:  Bret Bouchard

    PureDSP phoneme database implementation - No JUCE dependencies

    Implementation of universal phoneme database with JSON loading,
    thread-safe querying, and diphone synthesis support.

  ==============================================================================
*/

#include "dsp/core/PhonemeDatabase.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <nlohmann/json.hpp>

namespace DSP {

using json = nlohmann::json;

//==============================================================================
// PhonemeCategory Conversion Functions
//==============================================================================

std::string phonemeCategoryToString(PhonemeCategory category) {
    switch(category) {
        case PhonemeCategory::Vowel: return "vowel";
        case PhonemeCategory::Consonant: return "consonant";
        case PhonemeCategory::Drone: return "drone";
        case PhonemeCategory::Formant: return "formant";
        case PhonemeCategory::Subharmonic: return "subharmonic";
        case PhonemeCategory::Pulsed: return "pulsed";
        default:
            std::cerr << "Warning: Unknown PhonemeCategory value, defaulting to 'vowel'" << std::endl;
            return "vowel";
    }
}

PhonemeCategory stringToPhonemeCategory(const std::string& str) {
    if (str == "vowel") return PhonemeCategory::Vowel;
    if (str == "consonant") return PhonemeCategory::Consonant;
    if (str == "drone") return PhonemeCategory::Drone;
    if (str == "formant") return PhonemeCategory::Formant;
    if (str == "subharmonic") return PhonemeCategory::Subharmonic;
    if (str == "pulsed") return PhonemeCategory::Pulsed;

    // Default to vowel for unknown strings
    std::cerr << "Warning: Unknown phoneme category string '" << str
              << "', defaulting to Vowel" << std::endl;
    return PhonemeCategory::Vowel;
}

//==============================================================================
// PhonemeDatabase Implementation
//==============================================================================

PhonemeDatabase::PhonemeDatabase() {
    // Constructor - nothing to initialize yet
}

PhonemeDatabase::~PhonemeDatabase() {
    clear();
}

bool PhonemeDatabase::loadLanguage(const std::string& language_file) {
    // Try to open the file
    std::ifstream file(language_file);
    if (!file.is_open()) {
        std::cerr << "Failed to open language file: " << language_file << std::endl;
        return false;
    }

    // Read file content
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string json_content = buffer.str();
    file.close();

    // Parse JSON using nlohmann/json
    json json_obj;
    try {
        json_obj = json::parse(json_content);
    } catch (const json::parse_error& e) {
        std::cerr << "JSON parse error in " << language_file << ": " << e.what() << std::endl;
        return false;
    }

    if (!json_obj.is_object()) {
        std::cerr << "Invalid JSON in language file: " << language_file << std::endl;
        return false;
    }

    // Get phonemes object
    if (!json_obj.contains("phonemes") || !json_obj["phonemes"].is_object()) {
        std::cerr << "No phonemes found in language file: " << language_file << std::endl;
        return false;
    }

    auto phonemes_obj = json_obj["phonemes"];

    std::lock_guard<std::mutex> lock(mutex_);

    // Parse each phoneme
    int loaded_count = 0;
    for (auto it = phonemes_obj.begin(); it != phonemes_obj.end(); ++it) {
        const std::string symbol = it.key();

        auto phoneme = std::make_shared<Phoneme>();
        phoneme->id = symbol;

        if (parsePhonemeObject(it.value(), phoneme)) {
            // Add to symbol map
            symbol_map_[symbol] = phoneme;

            // Add to IPA map
            ipa_map_[phoneme->ipa] = phoneme;

            // Add to category map - CONVERT ENUM TO STRING
            std::string category_str = phonemeCategoryToString(phoneme->category);
            category_map_[category_str].push_back(phoneme);

            loaded_count++;
        } else {
            std::cerr << "Failed to parse phoneme: " << symbol << std::endl;
        }
    }

    std::cout << "Loaded " << loaded_count << " phonemes from "
              << language_file << std::endl;
    return loaded_count > 0;
}

std::shared_ptr<Phoneme> PhonemeDatabase::getPhoneme(const std::string& symbol) const {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = symbol_map_.find(symbol);
    if (it != symbol_map_.end()) {
        return it->second;
    }
    return nullptr;
}

std::shared_ptr<Phoneme> PhonemeDatabase::getPhonemeByIPA(const std::string& ipa) const {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = ipa_map_.find(ipa);
    if (it != ipa_map_.end()) {
        return it->second;
    }
    return nullptr;
}

std::vector<std::shared_ptr<Phoneme>> PhonemeDatabase::getByCategory(
    const std::string& category
) const {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = category_map_.find(category);
    if (it != category_map_.end()) {
        return it->second;
    }
    return {};
}

std::vector<std::shared_ptr<Phoneme>> PhonemeDatabase::getAllPhonemes() const {
    std::lock_guard<std::mutex> lock(mutex_);

    std::vector<std::shared_ptr<Phoneme>> all;
    all.reserve(symbol_map_.size());

    for (const auto& entry : symbol_map_) {
        all.push_back(entry.second);
    }

    return all;
}

size_t PhonemeDatabase::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return symbol_map_.size();
}

void PhonemeDatabase::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    symbol_map_.clear();
    ipa_map_.clear();
    category_map_.clear();
}

bool PhonemeDatabase::hasPhoneme(const std::string& symbol) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return symbol_map_.find(symbol) != symbol_map_.end();
}

std::vector<std::string> PhonemeDatabase::getCategories() const {
    std::lock_guard<std::mutex> lock(mutex_);

    std::vector<std::string> categories;
    categories.reserve(category_map_.size());

    for (const auto& entry : category_map_) {
        categories.push_back(entry.first);
    }

    return categories;
}

FormantData PhonemeDatabase::createDiphone(
    const Phoneme& from,
    const Phoneme& to,
    float t
) const {
    // Clamp t to [0, 1]
    t = std::max(0.0f, std::min(1.0f, t));

    // Linear interpolation between formant values
    return lerpFormants(from.formants, to.formants, t);
}

//==============================================================================
// JSON Parsing Helpers
//==============================================================================

bool PhonemeDatabase::parsePhonemeObject(
    const json& phoneme_obj,
    std::shared_ptr<Phoneme>& phoneme
) {
    if (!phoneme_obj.is_object()) {
        return false;
    }

    // Parse basic properties
    if (phoneme_obj.contains("ipa") && phoneme_obj["ipa"].is_string()) {
        phoneme->ipa = phoneme_obj["ipa"].get<std::string>();
    }

    // Parse category - CONVERT STRING TO ENUM
    if (phoneme_obj.contains("category") && phoneme_obj["category"].is_string()) {
        std::string category_str = phoneme_obj["category"].get<std::string>();
        phoneme->category = stringToPhonemeCategory(category_str);
    }

    // Parse nested objects
    if (phoneme_obj.contains("formants") && phoneme_obj["formants"].is_object()) {
        phoneme->formants = parseFormantData(phoneme_obj["formants"]);
    }

    if (phoneme_obj.contains("articulatory") && phoneme_obj["articulatory"].is_object()) {
        phoneme->articulatory = parseArticulatoryFeatures(phoneme_obj["articulatory"]);
    }

    if (phoneme_obj.contains("temporal") && phoneme_obj["temporal"].is_object()) {
        phoneme->temporal = parseTemporalFeatures(phoneme_obj["temporal"]);
    }

    return true;
}

FormantData PhonemeDatabase::parseFormantData(const json& formants_obj) {
    FormantData data;

    if (!formants_obj.is_object()) {
        return data;
    }

    // Parse formant frequencies - support both array and f1/f2/f3/f4 formats
    if (formants_obj.contains("frequencies") && formants_obj["frequencies"].is_array()) {
        auto freqs = formants_obj["frequencies"];
        for (size_t i = 0; i < 4 && i < freqs.size(); ++i) {
            data.frequencies[i] = freqs[i].get<float>();
        }
    } else {
        // Support individual f1, f2, f3, f4 fields for backward compatibility
        if (formants_obj.contains("f1")) data.frequencies[0] = formants_obj["f1"].get<float>();
        if (formants_obj.contains("f2")) data.frequencies[1] = formants_obj["f2"].get<float>();
        if (formants_obj.contains("f3")) data.frequencies[2] = formants_obj["f3"].get<float>();
        if (formants_obj.contains("f4")) data.frequencies[3] = formants_obj["f4"].get<float>();
    }

    // Parse formant bandwidths - support both array and bw1/bw2/bw3/bw4 formats
    if (formants_obj.contains("bandwidths") && formants_obj["bandwidths"].is_array()) {
        auto bws = formants_obj["bandwidths"];
        for (size_t i = 0; i < 4 && i < bws.size(); ++i) {
            data.bandwidths[i] = bws[i].get<float>();
        }
    } else {
        // Support individual bw1, bw2, bw3, bw4 fields for backward compatibility
        if (formants_obj.contains("bw1")) data.bandwidths[0] = formants_obj["bw1"].get<float>();
        if (formants_obj.contains("bw2")) data.bandwidths[1] = formants_obj["bw2"].get<float>();
        if (formants_obj.contains("bw3")) data.bandwidths[2] = formants_obj["bw3"].get<float>();
        if (formants_obj.contains("bw4")) data.bandwidths[3] = formants_obj["bw4"].get<float>();
    }

    return data;
}

ArticulatoryFeatures PhonemeDatabase::parseArticulatoryFeatures(
    const json& articulatory_obj
) {
    ArticulatoryFeatures features;

    if (!articulatory_obj.is_object()) {
        return features;
    }

    if (articulatory_obj.contains("is_nasal"))
        features.is_nasal = articulatory_obj["is_nasal"].get<bool>();
    if (articulatory_obj.contains("is_rounded"))
        features.is_rounded = articulatory_obj["is_rounded"].get<bool>();
    if (articulatory_obj.contains("is_voiced"))
        features.is_voiced = articulatory_obj["is_voiced"].get<bool>();
    if (articulatory_obj.contains("is_lateral"))
        features.is_lateral = articulatory_obj["is_lateral"].get<bool>();
    if (articulatory_obj.contains("is_rhotic"))
        features.is_rhotic = articulatory_obj["is_rhotic"].get<bool>();

    return features;
}

TemporalFeatures PhonemeDatabase::parseTemporalFeatures(
    const json& temporal_obj
) {
    TemporalFeatures features;

    if (!temporal_obj.is_object()) {
        return features;
    }

    if (temporal_obj.contains("min_duration"))
        features.min_duration = temporal_obj["min_duration"].get<int>();
    if (temporal_obj.contains("max_duration"))
        features.max_duration = temporal_obj["max_duration"].get<int>();
    if (temporal_obj.contains("default_duration"))
        features.default_duration = temporal_obj["default_duration"].get<int>();

    return features;
}

//==============================================================================
// Formant Interpolation
//==============================================================================

float PhonemeDatabase::lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

FormantData PhonemeDatabase::lerpFormants(
    const FormantData& a,
    const FormantData& b,
    float t
) const {
    FormantData result;

    for (size_t i = 0; i < 4; ++i) {
        result.frequencies[i] = lerp(a.frequencies[i], b.frequencies[i], t);
        result.bandwidths[i] = lerp(a.bandwidths[i], b.bandwidths[i], t);
    }

    return result;
}

} // namespace DSP
