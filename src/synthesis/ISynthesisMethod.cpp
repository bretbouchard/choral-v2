/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * ISynthesisMethod.cpp - Synthesis method factory implementation
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#include "ISynthesisMethod.h"
#include "FormantSynthesis.h"
#include "SubharmonicSynthesis.h"
#include "DiphoneSynthesis.h"

namespace ChoirV2 {

std::unique_ptr<ISynthesisMethod> SynthesisMethodFactory::create(const std::string& method_name) {
    if (method_name == "formant" || method_name.empty()) {
        return std::make_unique<FormantSynthesis>();
    }
    else if (method_name == "subharmonic") {
        return std::make_unique<SubharmonicSynthesis>();
    }
    else if (method_name == "diphone") {
        return std::make_unique<DiphoneSynthesis>();
    }
    else {
        // Default to formant synthesis for unknown methods
        return std::make_unique<FormantSynthesis>();
    }
}

std::vector<std::string> SynthesisMethodFactory::getAvailableMethods() {
    return {"formant", "diphone", "subharmonic"};
}

} // namespace ChoirV2
