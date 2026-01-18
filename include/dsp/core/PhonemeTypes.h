/*
  ==============================================================================

    PhonemeTypes.h
    Created: January 18, 2026
    Author:  Bret Bouchard

    PureDSP phoneme data structures - No JUCE dependencies

    Phoneme representation for vocal synthesis with formant data,
    articulatory features, and temporal characteristics.

  ==============================================================================
*/

#pragma once

#include <array>
#include <string>

namespace DSP {

/**
 * @brief Phoneme category classification
 */
enum class PhonemeCategory {
    Vowel,
    Consonant,
    Drone,
    Formant,
    Subharmonic,
    Pulsed
};

/**
 * @brief Conversion functions for PhonemeCategory enum
 */
std::string phonemeCategoryToString(PhonemeCategory category);
PhonemeCategory stringToPhonemeCategory(const std::string& str);

/**
 * @brief Articulatory features for phoneme synthesis
 *
 * Describes the physical articulation properties that affect
 * the sound quality and timbre.
 */
struct ArticulatoryFeatures {
    bool is_nasal = false;      // Nasal consonant (m, n, ng)
    bool is_rounded = false;    // Rounded lips (o, u, w)
    bool is_voiced = true;      // Voiced sound (vs unvoiced)
    bool is_lateral = false;    // Lateral consonant (l)
    bool is_rhotic = false;     // Rhotic vowel (r-colored)
};

/**
 * @brief Temporal features for phoneme duration
 *
 * Defines minimum, maximum, and default duration for phonemes
 * in milliseconds.
 */
struct TemporalFeatures {
    int min_duration = 50;        // Minimum duration in ms
    int max_duration = 500;       // Maximum duration in ms
    int default_duration = 200;   // Default duration in ms
};

/**
 * @brief Subharmonic-specific parameters
 *
 * Parameters for subharmonic synthesis used in bass and
 * chest voice generation.
 */
struct SubharmonicParams {
    float fundamental_freq = 110.0f;
    float subharmonic_ratio = 2.0f;
    float subharmonic_amplitude = 0.5f;
    float formant_center_freq = 0.0f;
    float formant_bandwidth = 0.0f;
    float formant_amplitude = 0.0f;
    float pulse_rate = 0.0f;
    float pulse_depth = 0.0f;
    bool ventricular_folds = false;
    bool chest_voice = false;
    bool formant_modulation = false;
    bool sharp_resonance = false;
};

/**
 * @brief Formant frequencies and bandwidths
 *
 * Represents the 4-formant model (F1-F4) with their respective
 * bandwidths (B1-B4) for vowel synthesis.
 */
struct FormantData {
    std::array<float, 4> frequencies = {500, 1500, 2500, 3500};  // F1, F2, F3, F4 in Hz
    std::array<float, 4> bandwidths = {50, 80, 120, 150};         // B1, B2, B3, B4 in Hz

    /**
     * @brief Get formant frequency by index
     * @param index Formant index (0-3)
     * @return Frequency in Hz
     */
    float getFrequency(int index) const {
        if (index >= 0 && index < 4) {
            return frequencies[index];
        }
        return 0.0f;
    }

    /**
     * @brief Get formant bandwidth by index
     * @param index Formant index (0-3)
     * @return Bandwidth in Hz
     */
    float getBandwidth(int index) const {
        if (index >= 0 && index < 4) {
            return bandwidths[index];
        }
        return 0.0f;
    }
};

/**
 * @brief Phoneme definition
 *
 * Represents a single speech sound or vocal technique element.
 * Contains all information needed for synthesis including formants,
 * articulatory features, and temporal characteristics.
 */
struct Phoneme {
    std::string id;                              // Unique ID (e.g., "AA", "IY")
    std::string ipa;                             // IPA symbol (e.g., "/ɑ/", "/i/")
    PhonemeCategory category;                    // Phoneme category
    FormantData formants;                        // Formant frequencies/bandwidths
    ArticulatoryFeatures articulatory;           // Articulatory features
    TemporalFeatures temporal;                   // Temporal features
    SubharmonicParams subharmonic_params;        // Subharmonic-specific params

    /**
     * @brief Get formant frequency by index
     * @param index Formant index (0-3)
     * @return Frequency in Hz
     */
    float getFormantFrequency(int index) const {
        return formants.getFrequency(index);
    }

    /**
     * @brief Get formant bandwidth by index
     * @param index Formant index (0-3)
     * @return Bandwidth in Hz
     */
    float getFormantBandwidth(int index) const {
        return formants.getBandwidth(index);
    }
};

} // namespace DSP
