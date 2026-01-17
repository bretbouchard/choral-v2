/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * DiphoneSynthesis.h - Diphone concatenative synthesis
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#pragma once

#include "ISynthesisMethod.h"
#include <memory>

namespace ChoirV2 {

/**
 * @brief Diphone synthesis method
 *
 * Concatenative synthesis using pre-recorded diphones.
 * Diphones are transitions between pairs of phonemes.
 */
class DiphoneSynthesis : public ISynthesisMethod {
public:
    DiphoneSynthesis();
    ~DiphoneSynthesis() override;

    bool initialize(const SynthesisParams& params) override;
    SynthesisResult synthesizeVoice(Voice* voice, const Phoneme* phoneme,
                                   float* output, int num_samples) override;
    SynthesisResult synthesizeVoicesSIMD(const std::vector<Voice*>& voices,
                                       const std::vector<const Phoneme*>& phonemes,
                                       float* output, int num_samples) override;
    void reset() override;
    std::string getName() const override { return "diphone"; }
    MethodStats getStats() const override;

private:
    SynthesisParams params_;
    MethodStats stats_;
};

} // namespace ChoirV2
