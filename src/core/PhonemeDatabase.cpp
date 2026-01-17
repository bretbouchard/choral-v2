/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * PhonemeDatabase.cpp - Implementation of phoneme database with JSON loading
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#include "PhonemeDatabase.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>

namespace ChoirV2 {

// ============================================================================
// Phoneme Helper Methods
// ============================================================================

float Phoneme::getFormantFrequency(int index) const {
    switch (index) {
        case 0: return formants.f1;
        case 1: return formants.f2;
        case 2: return formants.f3;
        case 3: return formants.f4;
        default: return 0.0f;
    }
}

float Phoneme::getFormantBandwidth(int index) const {
    switch (index) {
        case 0: return formants.bw1;
        case 1: return formants.bw2;
        case 2: return formants.bw3;
        case 3: return formants.bw4;
        default: return 0.0f;
    }
}

bool Phoneme::hasFormant(int index) const {
    return index >= 0 && index <= 3;
}

// ============================================================================
// PhonemeDatabase Implementation
// ============================================================================

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
    nlohmann::json json_obj;
    try {
        json_obj = nlohmann::json::parse(json_content);
    } catch (const nlohmann::json::parse_error& e) {
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

            // Add to category map
            category_map_[phoneme->category].push_back(phoneme);

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

// ============================================================================
// JSON Parsing Helpers
// ============================================================================

bool PhonemeDatabase::parsePhonemeObject(
    const nlohmann::json& phoneme_obj,
    std::shared_ptr<Phoneme>& phoneme
) {
    if (!phoneme_obj.is_object()) {
        return false;
    }

    // Parse basic properties
    if (phoneme_obj.contains("ipa") && phoneme_obj["ipa"].is_string()) {
        phoneme->ipa = phoneme_obj["ipa"].get<std::string>();
    }

    if (phoneme_obj.contains("category") && phoneme_obj["category"].is_string()) {
        phoneme->category = phoneme_obj["category"].get<std::string>();
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

FormantData PhonemeDatabase::parseFormantData(const nlohmann::json& formants_obj) {
    FormantData data;

    if (!formants_obj.is_object()) {
        return data;
    }

    // Parse formant frequencies
    if (formants_obj.contains("f1")) data.f1 = formants_obj["f1"].get<float>();
    if (formants_obj.contains("f2")) data.f2 = formants_obj["f2"].get<float>();
    if (formants_obj.contains("f3")) data.f3 = formants_obj["f3"].get<float>();
    if (formants_obj.contains("f4")) data.f4 = formants_obj["f4"].get<float>();

    // Parse formant bandwidths
    if (formants_obj.contains("bw1")) data.bw1 = formants_obj["bw1"].get<float>();
    if (formants_obj.contains("bw2")) data.bw2 = formants_obj["bw2"].get<float>();
    if (formants_obj.contains("bw3")) data.bw3 = formants_obj["bw3"].get<float>();
    if (formants_obj.contains("bw4")) data.bw4 = formants_obj["bw4"].get<float>();

    return data;
}

ArticulatoryFeatures PhonemeDatabase::parseArticulatoryFeatures(
    const nlohmann::json& articulatory_obj
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
    const nlohmann::json& temporal_obj
) {
    TemporalFeatures features;

    if (!temporal_obj.is_object()) {
        return features;
    }

    if (temporal_obj.contains("min_duration"))
        features.min_duration = temporal_obj["min_duration"].get<float>();
    if (temporal_obj.contains("max_duration"))
        features.max_duration = temporal_obj["max_duration"].get<float>();
    if (temporal_obj.contains("default_duration"))
        features.default_duration = temporal_obj["default_duration"].get<float>();

    return features;
}

// ============================================================================
// Formant Interpolation
// ============================================================================

float PhonemeDatabase::lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

FormantData PhonemeDatabase::lerpFormants(
    const FormantData& a,
    const FormantData& b,
    float t
) const {
    FormantData result;

    result.f1 = lerp(a.f1, b.f1, t);
    result.f2 = lerp(a.f2, b.f2, t);
    result.f3 = lerp(a.f3, b.f3, t);
    result.f4 = lerp(a.f4, b.f4, t);

    result.bw1 = lerp(a.bw1, b.bw1, t);
    result.bw2 = lerp(a.bw2, b.bw2, t);
    result.bw3 = lerp(a.bw3, b.bw3, t);
    result.bw4 = lerp(a.bw4, b.bw4, t);

    return result;
}

} // namespace ChoirV2
