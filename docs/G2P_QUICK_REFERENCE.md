# G2PEngine PureDSP Quick Reference

## Overview

The G2PEngine (Grapheme-to-Phoneme) component has been successfully migrated to the PureDSP namespace. This document provides a quick reference for using the refactored component.

## Location

- **Header:** `include/dsp/core/G2PEngine.h`
- **Implementation:** `src/dsp/core/G2PEngine.cpp`
- **Namespace:** `DSP`
- **Library:** `ChoirV2PureDSP`

## Basic Usage

### 1. Include the Header

```cpp
#include "dsp/core/G2PEngine.h"
```

### 2. Create the Engine

```cpp
// Create phoneme database
auto phoneme_db = std::make_shared<DSP::PhonemeDatabase>();

// Create G2P engine
DSP::G2PEngine g2p_engine(phoneme_db);
```

### 3. Convert Text to Phonemes

```cpp
// Simple conversion
DSP::G2PResult result = g2p_engine.convert("hello world");

if (result.success) {
    // Get phoneme sequence
    for (const auto& phoneme : result.phonemes) {
        std::cout << phoneme.symbol << " ";
    }
    std::cout << std::endl;

    // Get phoneme string (e.g., "/HH AH L OW W ER L D/")
    std::string phoneme_string = result.getPhonemeString();
}
```

### 4. Add Dictionary Entries

```cpp
// Add custom pronunciation
g2p_engine.addDictionaryEntry("hello", {"HH", "AH", "L", "OW"});
```

### 5. Add Custom Rules

```cpp
// Create a rule
DSP::G2PRule rule;
rule.pattern = "ing";
rule.phonemes = {"IH", "N", "G"};
rule.priority = 10;
rule.word_end = true;  // Only match at word end

// Add the rule
g2p_engine.addRule(rule);
```

### 6. Convert with Custom Timing

```cpp
// Convert with specific speech rate (syllables per second)
DSP::G2PResult result = g2p_engine.convertWithTiming("hello", 5.0f);

// Check duration
float total_duration = result.getTotalDuration();
```

### 7. Get Statistics

```cpp
// After conversion
DSP::G2PConversionStats stats = g2p_engine.getLastStats();

std::cout << "Dictionary hits: " << stats.dictionary_hits << std::endl;
std::cout << "Rule matches: " << stats.rule_matches << std::endl;
std::cout << "Processing time: " << stats.processing_time_ms << " ms" << std::endl;
```

## Data Structures

### G2PRule

```cpp
struct G2PRule {
    std::string pattern;              // Grapheme pattern (regex or literal)
    std::vector<std::string> phonemes; // Replacement phonemes
    std::string context;              // Context description
    int priority;                     // Rule priority (higher = more specific)

    // Context constraints
    std::string preceding;            // Required preceding characters
    std::string following;            // Required following characters
    bool word_start;                  // Must be at word start
    bool word_end;                    // Must be at word end

    // Character class constraints
    std::string char_class;           // vowel, consonant, etc.
};
```

### PhonemeResult

```cpp
struct PhonemeResult {
    std::string symbol;               // IPA or phoneme symbol
    float duration;                   // Duration in seconds
    float pitch_target;               // F0 target (Hz)
    bool stressed;                    // Is this syllable stressed?
    size_t position;                  // Position in word
    size_t syllable;                  // Syllable index (0-based)
};
```

### G2PResult

```cpp
struct G2PResult {
    std::vector<PhonemeResult> phonemes;  // Phoneme sequence with metadata
    std::vector<std::string> words;       // Original word tokens
    bool success;                         // Conversion successful?
    std::string error_message;            // Error details if failed

    size_t getPhonemeCount() const;
    float getTotalDuration() const;
    std::string getPhonemeString() const;
};
```

## Key Features

### 1. Dictionary Lookup
- Fast lookup for common words
- Custom dictionary overrides main dictionary
- Supports multiple pronunciations

### 2. Rule-Based Conversion
- Context-sensitive rules
- Priority-based rule matching
- Regex pattern support via `std::regex`

### 3. Timing and Prosody
- Automatic duration estimation
- Speech rate adjustment
- Stress detection

### 4. Tokenization
- Handles punctuation
- Preserves whitespace
- Maintains word positions

## Real-Time Safety

✅ **Safe Operations:**
- All text processing happens in `prepare()`, not `process()`
- No dynamic memory allocation during conversion
- Deterministic output (same input = same output)
- Uses `std::regex` (PureDSP-compatible)

❌ **Unsafe Operations (Do NOT call from audio thread):**
- `loadLanguage()` - File I/O and JSON parsing
- `reloadLanguage()` - File I/O and JSON parsing
- Any operation that loads data from disk

## Examples

### Example 1: Basic Text-to-Phonemes

```cpp
DSP::G2PEngine g2p_engine(phoneme_db);
g2p_engine.addDictionaryEntry("hello", {"HH", "AH", "L", "OW"});

auto result = g2p_engine.convert("hello");
std::cout << result.getPhonemeString() << std::endl;
// Output: /HH AH L OW/
```

### Example 2: Custom Rules

```cpp
// Add "ing" suffix rule
DSP::G2PRule ing_rule;
ing_rule.pattern = "ing";
ing_rule.phonemes = {"IH", "N", "G"};
ing_rule.priority = 10;
ing_rule.word_end = true;

g2p_engine.addRule(ing_rule);

auto result = g2p_engine.convert("running");
// Will apply the "ing" rule at the end
```

### Example 3: Speech Rate Adjustment

```cpp
// Slow speech (2 syllables/sec)
auto slow_result = g2p_engine.convertWithTiming("hello", 2.0f);

// Fast speech (8 syllables/sec)
auto fast_result = g2p_engine.convertWithTiming("hello", 8.0f);

// Slow version should have longer duration
assert(slow_result.getTotalDuration() > fast_result.getTotalDuration());
```

## Migration Notes

### From ChoirV2::G2PEngine to DSP::G2PEngine

1. **Namespace Change:**
   - Old: `ChoirV2::G2PEngine`
   - New: `DSP::G2PEngine`

2. **Header Location:**
   - Old: `src/core/G2PEngine.h`
   - New: `include/dsp/core/G2PEngine.h`

3. **No JUCE Dependencies:**
   - Uses `std::regex` instead of JUCE string functions
   - Pure C++20 standard library

4. **API Compatibility:**
   - Public API is identical
   - Drop-in replacement with namespace update

## Compilation

The G2PEngine is part of the ChoirV2PureDSP library:

```bash
cd build
cmake .. -DCHOIR_V2_BUILD_TESTS=ON
make -j4
```

The library will be built as:
- `libChoirV2PureDSP.a` (3.4MB)

## Testing

Unit tests are available in:
`tests/unit/test_puredsp_g2p.cpp`

Run tests with:
```bash
cd build
make test
```

## Performance

- **Dictionary lookup:** O(1) average case
- **Rule application:** O(n * m) where n = word length, m = number of rules
- **Processing time:** Typically < 1ms for short phrases
- **Memory usage:** Minimal (no allocations during conversion)

## Future Enhancements

1. **Multilingual Support:** Add language-specific rules
2. **Advanced Prosody:** Pitch contour generation
3. **Syllable Detection:** Improved syllabification
4. **Stress Patterns:** Language-specific stress rules
5. **Exception Handling:** Special case word pronunciations

## See Also

- `PhonemeDatabase` - Phoneme storage and validation
- `LanguageLoader` - Language definition loading
- `ChoirV2PureDSP` - Main vocal synthesis engine
