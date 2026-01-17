/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * SubharmonicSynthesis.h - Sub-harmonic synthesis method
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#pragma once

#include "ISynthesisMethod.h"
#include <memory>

namespace ChoirV2 {

class SubharmonicGenerator;

/**
 * @brief Sub-harmonic synthesis method
 *
 * Generates vocal sounds with sub-harmonic frequencies
 * for throat singing and related techniques.
 */
class SubharmonicSynthesis : public ISynthesisMethod {
public:
    SubharmonicSynthesis();
    ~SubharmonicSynthesis() override;

    bool initialize(const SynthesisParams& params) override;
    SynthesisResult synthesizeVoice(Voice* voice, const Phoneme* phoneme,
                                   float* output, int num_samples) override;
    SynthesisResult synthesizeVoicesSIMD(const std::vector<Voice*>& voices,
                                       const std::vector<const Phoneme*>& phonemes,
                                       float* output, int num_samples) override;
    void reset() override;
    std::string getName() const override { return "subharmonic"; }
    MethodStats getStats() const override;

private:
    std::unique_ptr<SubharmonicGenerator> subharmonic_gen_;
    SynthesisParams params_;
    MethodStats stats_;
};

} // namespace ChoirV2
