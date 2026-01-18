/*
  ==============================================================================

    ChoirV2PureDSP_Parameters.cpp
    Parameter and Preset Management Implementation

    This file contains the complete implementation of parameter management
    and JSON preset save/load functionality for ChoirV2PureDSP.

  ==============================================================================
*/

#include "dsp/ChoirV2PureDSP.h"
#include <cstring>
#include <cstdio>
#include <cctype>

namespace DSP {

//==============================================================================
// Parameter Management Implementation
//==============================================================================

float ChoirV2PureDSP::getParameter(const char* paramId) const {
    if (paramId == nullptr) {
        return 0.0f;
    }

    // Master parameters
    if (std::strcmp(paramId, ChoirV2Parameters::MASTER_VOLUME) == 0) {
        return params_.masterVolume;
    }
    if (std::strcmp(paramId, ChoirV2Parameters::STEREO_WIDTH) == 0) {
        return params_.stereoWidth;
    }
    if (std::strcmp(paramId, ChoirV2Parameters::POLPHONY) == 0) {
        return static_cast<float>(params_.polyphony);
    }

    // Vowel (3D control)
    if (std::strcmp(paramId, ChoirV2Parameters::VOWEL_X) == 0) {
        return params_.vowelX;
    }
    if (std::strcmp(paramId, ChoirV2Parameters::VOWEL_Y) == 0) {
        return params_.vowelY;
    }
    if (std::strcmp(paramId, ChoirV2Parameters::VOWEL_Z) == 0) {
        return params_.vowelZ;
    }

    // Formants
    if (std::strcmp(paramId, ChoirV2Parameters::FORMANT_SCALE) == 0) {
        return params_.formantScale;
    }
    if (std::strcmp(paramId, ChoirV2Parameters::FORMANT_SHIFT) == 0) {
        return params_.formantShift;
    }

    // Breath
    if (std::strcmp(paramId, ChoirV2Parameters::BREATH_MIX) == 0) {
        return params_.breathMix;
    }
    if (std::strcmp(paramId, ChoirV2Parameters::BREATH_COLOR) == 0) {
        return params_.breathColor;
    }

    // Vibrato
    if (std::strcmp(paramId, ChoirV2Parameters::VIBRATO_RATE) == 0) {
        return params_.vibratoRate;
    }
    if (std::strcmp(paramId, ChoirV2Parameters::VIBRATO_DEPTH) == 0) {
        return params_.vibratoDepth;
    }
    if (std::strcmp(paramId, ChoirV2Parameters::VIBRATO_DELAY) == 0) {
        return params_.vibratoDelay;
    }

    // Ensemble
    if (std::strcmp(paramId, ChoirV2Parameters::TIGHTNESS) == 0) {
        return params_.tightness;
    }
    if (std::strcmp(paramId, ChoirV2Parameters::ENSEMBLE_SIZE) == 0) {
        return params_.ensembleSize;
    }
    if (std::strcmp(paramId, ChoirV2Parameters::VOICE_SPREAD) == 0) {
        return params_.voiceSpread;
    }

    // Envelope (ADSR)
    if (std::strcmp(paramId, ChoirV2Parameters::ATTACK) == 0) {
        return params_.attack;
    }
    if (std::strcmp(paramId, ChoirV2Parameters::DECAY) == 0) {
        return params_.decay;
    }
    if (std::strcmp(paramId, ChoirV2Parameters::SUSTAIN) == 0) {
        return params_.sustain;
    }
    if (std::strcmp(paramId, ChoirV2Parameters::RELEASE) == 0) {
        return params_.release;
    }

    // SATB blend
    if (std::strcmp(paramId, ChoirV2Parameters::SOPRANO_LEVEL) == 0) {
        return params_.sopranoLevel;
    }
    if (std::strcmp(paramId, ChoirV2Parameters::ALTO_LEVEL) == 0) {
        return params_.altoLevel;
    }
    if (std::strcmp(paramId, ChoirV2Parameters::TENOR_LEVEL) == 0) {
        return params_.tenorLevel;
    }
    if (std::strcmp(paramId, ChoirV2Parameters::BASS_LEVEL) == 0) {
        return params_.bassLevel;
    }

    // Effects
    if (std::strcmp(paramId, ChoirV2Parameters::REVERB_MIX) == 0) {
        return params_.reverbMix;
    }
    if (std::strcmp(paramId, ChoirV2Parameters::REVERB_DECAY) == 0) {
        return params_.reverbDecay;
    }
    if (std::strcmp(paramId, ChoirV2Parameters::REVERB_PREDELAY) == 0) {
        return params_.reverbPredelay;
    }

    // Spectral Enhancement
    if (std::strcmp(paramId, ChoirV2Parameters::SPECTRAL_ENHANCEMENT) == 0) {
        return params_.spectralEnhancement;
    }
    if (std::strcmp(paramId, ChoirV2Parameters::HARMONICS_BOOST) == 0) {
        return params_.harmonicsBoost;
    }

    // Subharmonic Generation
    if (std::strcmp(paramId, ChoirV2Parameters::SUBHARMONIC_MIX) == 0) {
        return params_.subharmonicMix;
    }
    if (std::strcmp(paramId, ChoirV2Parameters::SUBHARMONIC_DEPTH) == 0) {
        return params_.subharmonicDepth;
    }

    // Diphone/Coarticulation
    if (std::strcmp(paramId, ChoirV2Parameters::COARTICULATION_AMOUNT) == 0) {
        return params_.coarticulationAmount;
    }
    if (std::strcmp(paramId, ChoirV2Parameters::TRANSITION_SPEED) == 0) {
        return params_.transitionSpeed;
    }

    // Unknown parameter
    return 0.0f;
}

void ChoirV2PureDSP::setParameter(const char* paramId, float value) {
    if (paramId == nullptr) {
        return;
    }

    // Master parameters
    if (std::strcmp(paramId, ChoirV2Parameters::MASTER_VOLUME) == 0) {
        params_.masterVolume = std::clamp(value, 0.0f, 1.0f);
        return;
    }
    if (std::strcmp(paramId, ChoirV2Parameters::STEREO_WIDTH) == 0) {
        params_.stereoWidth = std::clamp(value, 0.0f, 1.0f);
        return;
    }
    if (std::strcmp(paramId, ChoirV2Parameters::POLPHONY) == 0) {
        params_.polyphony = static_cast<int>(std::clamp(value, 1.0f, 128.0f));
        return;
    }

    // Vowel (3D control)
    if (std::strcmp(paramId, ChoirV2Parameters::VOWEL_X) == 0) {
        params_.vowelX = std::clamp(value, 0.0f, 1.0f);
        return;
    }
    if (std::strcmp(paramId, ChoirV2Parameters::VOWEL_Y) == 0) {
        params_.vowelY = std::clamp(value, 0.0f, 1.0f);
        return;
    }
    if (std::strcmp(paramId, ChoirV2Parameters::VOWEL_Z) == 0) {
        params_.vowelZ = std::clamp(value, 0.0f, 1.0f);
        return;
    }

    // Formants
    if (std::strcmp(paramId, ChoirV2Parameters::FORMANT_SCALE) == 0) {
        params_.formantScale = std::clamp(value, 0.5f, 2.0f);
        return;
    }
    if (std::strcmp(paramId, ChoirV2Parameters::FORMANT_SHIFT) == 0) {
        params_.formantShift = std::clamp(value, -12.0f, 12.0f);
        return;
    }

    // Breath
    if (std::strcmp(paramId, ChoirV2Parameters::BREATH_MIX) == 0) {
        params_.breathMix = std::clamp(value, 0.0f, 1.0f);
        return;
    }
    if (std::strcmp(paramId, ChoirV2Parameters::BREATH_COLOR) == 0) {
        params_.breathColor = std::clamp(value, 0.0f, 1.0f);
        return;
    }

    // Vibrato
    if (std::strcmp(paramId, ChoirV2Parameters::VIBRATO_RATE) == 0) {
        params_.vibratoRate = std::clamp(value, 1.0f, 15.0f);
        return;
    }
    if (std::strcmp(paramId, ChoirV2Parameters::VIBRATO_DEPTH) == 0) {
        params_.vibratoDepth = std::clamp(value, 0.0f, 1.0f);
        return;
    }
    if (std::strcmp(paramId, ChoirV2Parameters::VIBRATO_DELAY) == 0) {
        params_.vibratoDelay = std::clamp(value, 0.0f, 1.0f);
        return;
    }

    // Ensemble
    if (std::strcmp(paramId, ChoirV2Parameters::TIGHTNESS) == 0) {
        params_.tightness = std::clamp(value, 0.0f, 1.0f);
        return;
    }
    if (std::strcmp(paramId, ChoirV2Parameters::ENSEMBLE_SIZE) == 0) {
        params_.ensembleSize = std::clamp(value, 0.0f, 1.0f);
        return;
    }
    if (std::strcmp(paramId, ChoirV2Parameters::VOICE_SPREAD) == 0) {
        params_.voiceSpread = std::clamp(value, 0.0f, 1.0f);
        return;
    }

    // Envelope (ADSR)
    if (std::strcmp(paramId, ChoirV2Parameters::ATTACK) == 0) {
        params_.attack = std::clamp(value, 0.001f, 5.0f);
        return;
    }
    if (std::strcmp(paramId, ChoirV2Parameters::DECAY) == 0) {
        params_.decay = std::clamp(value, 0.001f, 5.0f);
        return;
    }
    if (std::strcmp(paramId, ChoirV2Parameters::SUSTAIN) == 0) {
        params_.sustain = std::clamp(value, 0.0f, 1.0f);
        return;
    }
    if (std::strcmp(paramId, ChoirV2Parameters::RELEASE) == 0) {
        params_.release = std::clamp(value, 0.001f, 10.0f);
        return;
    }

    // SATB blend
    if (std::strcmp(paramId, ChoirV2Parameters::SOPRANO_LEVEL) == 0) {
        params_.sopranoLevel = std::clamp(value, 0.0f, 1.0f);
        return;
    }
    if (std::strcmp(paramId, ChoirV2Parameters::ALTO_LEVEL) == 0) {
        params_.altoLevel = std::clamp(value, 0.0f, 1.0f);
        return;
    }
    if (std::strcmp(paramId, ChoirV2Parameters::TENOR_LEVEL) == 0) {
        params_.tenorLevel = std::clamp(value, 0.0f, 1.0f);
        return;
    }
    if (std::strcmp(paramId, ChoirV2Parameters::BASS_LEVEL) == 0) {
        params_.bassLevel = std::clamp(value, 0.0f, 1.0f);
        return;
    }

    // Effects
    if (std::strcmp(paramId, ChoirV2Parameters::REVERB_MIX) == 0) {
        params_.reverbMix = std::clamp(value, 0.0f, 1.0f);
        return;
    }
    if (std::strcmp(paramId, ChoirV2Parameters::REVERB_DECAY) == 0) {
        params_.reverbDecay = std::clamp(value, 0.1f, 10.0f);
        return;
    }
    if (std::strcmp(paramId, ChoirV2Parameters::REVERB_PREDELAY) == 0) {
        params_.reverbPredelay = std::clamp(value, 0.0f, 0.1f);
        return;
    }

    // Spectral Enhancement
    if (std::strcmp(paramId, ChoirV2Parameters::SPECTRAL_ENHANCEMENT) == 0) {
        params_.spectralEnhancement = std::clamp(value, 0.0f, 1.0f);
        return;
    }
    if (std::strcmp(paramId, ChoirV2Parameters::HARMONICS_BOOST) == 0) {
        params_.harmonicsBoost = std::clamp(value, 0.0f, 1.0f);
        return;
    }

    // Subharmonic Generation
    if (std::strcmp(paramId, ChoirV2Parameters::SUBHARMONIC_MIX) == 0) {
        params_.subharmonicMix = std::clamp(value, 0.0f, 1.0f);
        return;
    }
    if (std::strcmp(paramId, ChoirV2Parameters::SUBHARMONIC_DEPTH) == 0) {
        params_.subharmonicDepth = std::clamp(value, 0.0f, 1.0f);
        return;
    }

    // Diphone/Coarticulation
    if (std::strcmp(paramId, ChoirV2Parameters::COARTICULATION_AMOUNT) == 0) {
        params_.coarticulationAmount = std::clamp(value, 0.0f, 1.0f);
        return;
    }
    if (std::strcmp(paramId, ChoirV2Parameters::TRANSITION_SPEED) == 0) {
        params_.transitionSpeed = std::clamp(value, 0.0f, 1.0f);
        return;
    }
}

//==============================================================================
// Preset Management Implementation
//==============================================================================

bool ChoirV2PureDSP::savePreset(char* jsonBuffer, int jsonBufferSize) const {
    if (jsonBuffer == nullptr || jsonBufferSize < 64) {
        return false;
    }

    // Initialize buffer
    int offset = 0;
    jsonBuffer[0] = '{';
    offset = 1;

    // Write all parameters to JSON
    bool success = true;

    // Master parameters
    success &= writeJsonParameter(ChoirV2Parameters::MASTER_VOLUME, params_.masterVolume,
                                  jsonBuffer, offset, jsonBufferSize);
    success &= writeJsonParameter(ChoirV2Parameters::STEREO_WIDTH, params_.stereoWidth,
                                  jsonBuffer, offset, jsonBufferSize);
    success &= writeJsonParameter(ChoirV2Parameters::POLPHONY, static_cast<float>(params_.polyphony),
                                  jsonBuffer, offset, jsonBufferSize);

    // Vowel (3D control)
    success &= writeJsonParameter(ChoirV2Parameters::VOWEL_X, params_.vowelX,
                                  jsonBuffer, offset, jsonBufferSize);
    success &= writeJsonParameter(ChoirV2Parameters::VOWEL_Y, params_.vowelY,
                                  jsonBuffer, offset, jsonBufferSize);
    success &= writeJsonParameter(ChoirV2Parameters::VOWEL_Z, params_.vowelZ,
                                  jsonBuffer, offset, jsonBufferSize);

    // Formants
    success &= writeJsonParameter(ChoirV2Parameters::FORMANT_SCALE, params_.formantScale,
                                  jsonBuffer, offset, jsonBufferSize);
    success &= writeJsonParameter(ChoirV2Parameters::FORMANT_SHIFT, params_.formantShift,
                                  jsonBuffer, offset, jsonBufferSize);

    // Breath
    success &= writeJsonParameter(ChoirV2Parameters::BREATH_MIX, params_.breathMix,
                                  jsonBuffer, offset, jsonBufferSize);
    success &= writeJsonParameter(ChoirV2Parameters::BREATH_COLOR, params_.breathColor,
                                  jsonBuffer, offset, jsonBufferSize);

    // Vibrato
    success &= writeJsonParameter(ChoirV2Parameters::VIBRATO_RATE, params_.vibratoRate,
                                  jsonBuffer, offset, jsonBufferSize);
    success &= writeJsonParameter(ChoirV2Parameters::VIBRATO_DEPTH, params_.vibratoDepth,
                                  jsonBuffer, offset, jsonBufferSize);
    success &= writeJsonParameter(ChoirV2Parameters::VIBRATO_DELAY, params_.vibratoDelay,
                                  jsonBuffer, offset, jsonBufferSize);

    // Ensemble
    success &= writeJsonParameter(ChoirV2Parameters::TIGHTNESS, params_.tightness,
                                  jsonBuffer, offset, jsonBufferSize);
    success &= writeJsonParameter(ChoirV2Parameters::ENSEMBLE_SIZE, params_.ensembleSize,
                                  jsonBuffer, offset, jsonBufferSize);
    success &= writeJsonParameter(ChoirV2Parameters::VOICE_SPREAD, params_.voiceSpread,
                                  jsonBuffer, offset, jsonBufferSize);

    // Envelope (ADSR)
    success &= writeJsonParameter(ChoirV2Parameters::ATTACK, params_.attack,
                                  jsonBuffer, offset, jsonBufferSize);
    success &= writeJsonParameter(ChoirV2Parameters::DECAY, params_.decay,
                                  jsonBuffer, offset, jsonBufferSize);
    success &= writeJsonParameter(ChoirV2Parameters::SUSTAIN, params_.sustain,
                                  jsonBuffer, offset, jsonBufferSize);
    success &= writeJsonParameter(ChoirV2Parameters::RELEASE, params_.release,
                                  jsonBuffer, offset, jsonBufferSize);

    // SATB blend
    success &= writeJsonParameter(ChoirV2Parameters::SOPRANO_LEVEL, params_.sopranoLevel,
                                  jsonBuffer, offset, jsonBufferSize);
    success &= writeJsonParameter(ChoirV2Parameters::ALTO_LEVEL, params_.altoLevel,
                                  jsonBuffer, offset, jsonBufferSize);
    success &= writeJsonParameter(ChoirV2Parameters::TENOR_LEVEL, params_.tenorLevel,
                                  jsonBuffer, offset, jsonBufferSize);
    success &= writeJsonParameter(ChoirV2Parameters::BASS_LEVEL, params_.bassLevel,
                                  jsonBuffer, offset, jsonBufferSize);

    // Effects
    success &= writeJsonParameter(ChoirV2Parameters::REVERB_MIX, params_.reverbMix,
                                  jsonBuffer, offset, jsonBufferSize);
    success &= writeJsonParameter(ChoirV2Parameters::REVERB_DECAY, params_.reverbDecay,
                                  jsonBuffer, offset, jsonBufferSize);
    success &= writeJsonParameter(ChoirV2Parameters::REVERB_PREDELAY, params_.reverbPredelay,
                                  jsonBuffer, offset, jsonBufferSize);

    // Spectral Enhancement
    success &= writeJsonParameter(ChoirV2Parameters::SPECTRAL_ENHANCEMENT, params_.spectralEnhancement,
                                  jsonBuffer, offset, jsonBufferSize);
    success &= writeJsonParameter(ChoirV2Parameters::HARMONICS_BOOST, params_.harmonicsBoost,
                                  jsonBuffer, offset, jsonBufferSize);

    // Subharmonic Generation
    success &= writeJsonParameter(ChoirV2Parameters::SUBHARMONIC_MIX, params_.subharmonicMix,
                                  jsonBuffer, offset, jsonBufferSize);
    success &= writeJsonParameter(ChoirV2Parameters::SUBHARMONIC_DEPTH, params_.subharmonicDepth,
                                  jsonBuffer, offset, jsonBufferSize);

    // Diphone/Coarticulation
    success &= writeJsonParameter(ChoirV2Parameters::COARTICULATION_AMOUNT, params_.coarticulationAmount,
                                  jsonBuffer, offset, jsonBufferSize);
    success &= writeJsonParameter(ChoirV2Parameters::TRANSITION_SPEED, params_.transitionSpeed,
                                  jsonBuffer, offset, jsonBufferSize);

    // Close JSON object
    if (offset < jsonBufferSize - 1) {
        jsonBuffer[offset++] = '}';
        jsonBuffer[offset] = '\0';
        return success;
    }

    return false;
}

bool ChoirV2PureDSP::loadPreset(const char* jsonData) {
    if (jsonData == nullptr) {
        return false;
    }

    bool success = true;
    double value;

    // Master parameters
    if (parseJsonParameter(jsonData, ChoirV2Parameters::MASTER_VOLUME, value)) {
        params_.masterVolume = static_cast<float>(value);
    }
    if (parseJsonParameter(jsonData, ChoirV2Parameters::STEREO_WIDTH, value)) {
        params_.stereoWidth = static_cast<float>(value);
    }
    if (parseJsonParameter(jsonData, ChoirV2Parameters::POLPHONY, value)) {
        params_.polyphony = static_cast<int>(value);
    }

    // Vowel (3D control)
    if (parseJsonParameter(jsonData, ChoirV2Parameters::VOWEL_X, value)) {
        params_.vowelX = static_cast<float>(value);
    }
    if (parseJsonParameter(jsonData, ChoirV2Parameters::VOWEL_Y, value)) {
        params_.vowelY = static_cast<float>(value);
    }
    if (parseJsonParameter(jsonData, ChoirV2Parameters::VOWEL_Z, value)) {
        params_.vowelZ = static_cast<float>(value);
    }

    // Formants
    if (parseJsonParameter(jsonData, ChoirV2Parameters::FORMANT_SCALE, value)) {
        params_.formantScale = static_cast<float>(value);
    }
    if (parseJsonParameter(jsonData, ChoirV2Parameters::FORMANT_SHIFT, value)) {
        params_.formantShift = static_cast<float>(value);
    }

    // Breath
    if (parseJsonParameter(jsonData, ChoirV2Parameters::BREATH_MIX, value)) {
        params_.breathMix = static_cast<float>(value);
    }
    if (parseJsonParameter(jsonData, ChoirV2Parameters::BREATH_COLOR, value)) {
        params_.breathColor = static_cast<float>(value);
    }

    // Vibrato
    if (parseJsonParameter(jsonData, ChoirV2Parameters::VIBRATO_RATE, value)) {
        params_.vibratoRate = static_cast<float>(value);
    }
    if (parseJsonParameter(jsonData, ChoirV2Parameters::VIBRATO_DEPTH, value)) {
        params_.vibratoDepth = static_cast<float>(value);
    }
    if (parseJsonParameter(jsonData, ChoirV2Parameters::VIBRATO_DELAY, value)) {
        params_.vibratoDelay = static_cast<float>(value);
    }

    // Ensemble
    if (parseJsonParameter(jsonData, ChoirV2Parameters::TIGHTNESS, value)) {
        params_.tightness = static_cast<float>(value);
    }
    if (parseJsonParameter(jsonData, ChoirV2Parameters::ENSEMBLE_SIZE, value)) {
        params_.ensembleSize = static_cast<float>(value);
    }
    if (parseJsonParameter(jsonData, ChoirV2Parameters::VOICE_SPREAD, value)) {
        params_.voiceSpread = static_cast<float>(value);
    }

    // Envelope (ADSR)
    if (parseJsonParameter(jsonData, ChoirV2Parameters::ATTACK, value)) {
        params_.attack = static_cast<float>(value);
    }
    if (parseJsonParameter(jsonData, ChoirV2Parameters::DECAY, value)) {
        params_.decay = static_cast<float>(value);
    }
    if (parseJsonParameter(jsonData, ChoirV2Parameters::SUSTAIN, value)) {
        params_.sustain = static_cast<float>(value);
    }
    if (parseJsonParameter(jsonData, ChoirV2Parameters::RELEASE, value)) {
        params_.release = static_cast<float>(value);
    }

    // SATB blend
    if (parseJsonParameter(jsonData, ChoirV2Parameters::SOPRANO_LEVEL, value)) {
        params_.sopranoLevel = static_cast<float>(value);
    }
    if (parseJsonParameter(jsonData, ChoirV2Parameters::ALTO_LEVEL, value)) {
        params_.altoLevel = static_cast<float>(value);
    }
    if (parseJsonParameter(jsonData, ChoirV2Parameters::TENOR_LEVEL, value)) {
        params_.tenorLevel = static_cast<float>(value);
    }
    if (parseJsonParameter(jsonData, ChoirV2Parameters::BASS_LEVEL, value)) {
        params_.bassLevel = static_cast<float>(value);
    }

    // Effects
    if (parseJsonParameter(jsonData, ChoirV2Parameters::REVERB_MIX, value)) {
        params_.reverbMix = static_cast<float>(value);
    }
    if (parseJsonParameter(jsonData, ChoirV2Parameters::REVERB_DECAY, value)) {
        params_.reverbDecay = static_cast<float>(value);
    }
    if (parseJsonParameter(jsonData, ChoirV2Parameters::REVERB_PREDELAY, value)) {
        params_.reverbPredelay = static_cast<float>(value);
    }

    // Spectral Enhancement
    if (parseJsonParameter(jsonData, ChoirV2Parameters::SPECTRAL_ENHANCEMENT, value)) {
        params_.spectralEnhancement = static_cast<float>(value);
    }
    if (parseJsonParameter(jsonData, ChoirV2Parameters::HARMONICS_BOOST, value)) {
        params_.harmonicsBoost = static_cast<float>(value);
    }

    // Subharmonic Generation
    if (parseJsonParameter(jsonData, ChoirV2Parameters::SUBHARMONIC_MIX, value)) {
        params_.subharmonicMix = static_cast<float>(value);
    }
    if (parseJsonParameter(jsonData, ChoirV2Parameters::SUBHARMONIC_DEPTH, value)) {
        params_.subharmonicDepth = static_cast<float>(value);
    }

    // Diphone/Coarticulation
    if (parseJsonParameter(jsonData, ChoirV2Parameters::COARTICULATION_AMOUNT, value)) {
        params_.coarticulationAmount = static_cast<float>(value);
    }
    if (parseJsonParameter(jsonData, ChoirV2Parameters::TRANSITION_SPEED, value)) {
        params_.transitionSpeed = static_cast<float>(value);
    }

    return success;
}

//==============================================================================
// Helper Functions Implementation
//==============================================================================

bool ChoirV2PureDSP::writeJsonParameter(const char* name, double value,
                                       char* buffer, int& offset, int bufferSize) const {
    if (name == nullptr || buffer == nullptr || offset < 0) {
        return false;
    }

    // Check if we need to add a comma (not the first parameter)
    bool addComma = (offset > 1 && buffer[offset - 1] != '{');

    // Calculate required space
    int requiredSpace = 100; // Conservative estimate
    if (offset + requiredSpace >= bufferSize) {
        return false; // Buffer too small
    }

    // Write comma if needed
    if (addComma) {
        buffer[offset++] = ',';
    }

    // Write parameter in JSON format: "paramName": value
    int written = std::snprintf(buffer + offset, bufferSize - offset,
                                "\"%s\":%.6g", name, value);

    if (written < 0 || offset + written >= bufferSize) {
        return false; // Write failed or buffer overflow
    }

    offset += written;
    return true;
}

bool ChoirV2PureDSP::parseJsonParameter(const char* json, const char* param, double& value) const {
    if (json == nullptr || param == nullptr) {
        return false;
    }

    // Search for the parameter in JSON string
    // Format: "paramName":value or "paramName": value

    // Build search pattern: "paramName":
    std::string searchPattern = "\"";
    searchPattern += param;
    searchPattern + "\":";

    // Find the pattern in JSON
    const char* patternStart = std::strstr(json, searchPattern.c_str());
    if (patternStart == nullptr) {
        return false; // Parameter not found
    }

    // Move to the value (skip the pattern and any whitespace)
    const char* valueStart = patternStart + searchPattern.length();
    while (*valueStart == ' ' || *valueStart == '\t' || *valueStart == '\n') {
        valueStart++;
    }

    // Parse the value (simple parsing for numbers)
    char* endPtr;
    value = std::strtod(valueStart, &endPtr);

    // Check if parsing succeeded
    if (endPtr == valueStart) {
        return false; // No valid number found
    }

    return true;
}

} // namespace DSP
