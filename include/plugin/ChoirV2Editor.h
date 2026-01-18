#pragma once

#include <JuceHeader.h>
#include "plugin/ChoirV2Processor.h"

/**
 * ChoirV2Editor: Comprehensive UI for ChoirV2 choral synthesis instrument
 *
 * Features:
 * - Professional dark theme with organized sections
 * - All 33 parameters displayed with intuitive controls
 * - Real-time visual feedback
 * - Preset management system
 * - Responsive layout with proper spacing
 *
 * UI Sections:
 * 1. Master Section - Volume, polyphony, text input, phoneme display
 * 2. Vowel Control (3D) - X, Y, Z sliders with visualization
 * 3. Formants - Scale and shift controls
 * 4. Breath - Mix and color controls
 * 5. Vibrato - Rate, depth, and delay
 * 6. Ensemble - Tightness, size, and spread
 * 7. ADSR Envelope - Attack, decay, sustain, release
 * 8. SATB Blend - Individual section levels
 * 9. Effects - Reverb controls
 * 10. Spectral - Enhancement and harmonics boost
 * 11. Subharmonic - Mix, depth, and ratio
 * 12. Diphone - Crossfade and smoothing
 * 13. Synthesis Method - Mode selection
 */
class ChoirV2Editor  : public juce::AudioProcessorEditor,
                       private juce::Timer
{
public:
    ChoirV2Editor(ChoirV2Processor&);
    ~ChoirV2Editor() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void timerCallback() override;

    // UI Components - Master Section (4 controls)
    std::unique_ptr<juce::Slider> masterVolumeSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> masterVolumeAttachment;
    std::unique_ptr<juce::Label> masterVolumeLabel;

    std::unique_ptr<juce::Slider> polyphonySlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> polyphonyAttachment;
    std::unique_ptr<juce::Label> polyphonyLabel;

    std::unique_ptr<juce::TextEditor> textInputEditor;
    std::unique_ptr<juce::Label> textInputLabel;

    std::unique_ptr<juce::Label> phonemeDisplayLabel;
    std::unique_ptr<juce::Label> phonemeDisplayValue;

    // UI Components - Vowel Control 3D (3 controls)
    std::unique_ptr<juce::Slider> vowelXSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> vowelXAttachment;
    std::unique_ptr<juce::Label> vowelXLabel;

    std::unique_ptr<juce::Slider> vowelYSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> vowelYAttachment;
    std::unique_ptr<juce::Label> vowelYLabel;

    std::unique_ptr<juce::Slider> vowelZSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> vowelZAttachment;
    std::unique_ptr<juce::Label> vowelZLabel;

    std::unique_ptr<juce::Component> vowelSpaceDisplay; // Visual display of vowel space

    // UI Components - Formants (2 controls)
    std::unique_ptr<juce::Slider> formantScaleSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> formantScaleAttachment;
    std::unique_ptr<juce::Label> formantScaleLabel;

    std::unique_ptr<juce::Slider> formantShiftSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> formantShiftAttachment;
    std::unique_ptr<juce::Label> formantShiftLabel;

    // UI Components - Breath (2 controls)
    std::unique_ptr<juce::Slider> breathMixSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> breathMixAttachment;
    std::unique_ptr<juce::Label> breathMixLabel;

    std::unique_ptr<juce::Slider> breathColorSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> breathColorAttachment;
    std::unique_ptr<juce::Label> breathColorLabel;

    // UI Components - Vibrato (3 controls)
    std::unique_ptr<juce::Slider> vibratoRateSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> vibratoRateAttachment;
    std::unique_ptr<juce::Label> vibratoRateLabel;

    std::unique_ptr<juce::Slider> vibratoDepthSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> vibratoDepthAttachment;
    std::unique_ptr<juce::Label> vibratoDepthLabel;

    std::unique_ptr<juce::Slider> vibratoDelaySlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> vibratoDelayAttachment;
    std::unique_ptr<juce::Label> vibratoDelayLabel;

    // UI Components - Ensemble (3 controls)
    std::unique_ptr<juce::Slider> tightnessSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> tightnessAttachment;
    std::unique_ptr<juce::Label> tightnessLabel;

    std::unique_ptr<juce::Slider> ensembleSizeSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ensembleSizeAttachment;
    std::unique_ptr<juce::Label> ensembleSizeLabel;

    std::unique_ptr<juce::Slider> voiceSpreadSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> voiceSpreadAttachment;
    std::unique_ptr<juce::Label> voiceSpreadLabel;

    // UI Components - ADSR Envelope (4 controls)
    std::unique_ptr<juce::Slider> attackSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackAttachment;
    std::unique_ptr<juce::Label> attackLabel;

    std::unique_ptr<juce::Slider> decaySlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> decayAttachment;
    std::unique_ptr<juce::Label> decayLabel;

    std::unique_ptr<juce::Slider> sustainSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sustainAttachment;
    std::unique_ptr<juce::Label> sustainLabel;

    std::unique_ptr<juce::Slider> releaseSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> releaseAttachment;
    std::unique_ptr<juce::Label> releaseLabel;

    // UI Components - SATB Blend (4 controls)
    std::unique_ptr<juce::Slider> sopranoLevelSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sopranoLevelAttachment;
    std::unique_ptr<juce::Label> sopranoLevelLabel;

    std::unique_ptr<juce::Slider> altoLevelSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> altoLevelAttachment;
    std::unique_ptr<juce::Label> altoLevelLabel;

    std::unique_ptr<juce::Slider> tenorLevelSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> tenorLevelAttachment;
    std::unique_ptr<juce::Label> tenorLevelLabel;

    std::unique_ptr<juce::Slider> bassLevelSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bassLevelAttachment;
    std::unique_ptr<juce::Label> bassLevelLabel;

    // UI Components - Effects (3 controls)
    std::unique_ptr<juce::Slider> reverbMixSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> reverbMixAttachment;
    std::unique_ptr<juce::Label> reverbMixLabel;

    std::unique_ptr<juce::Slider> reverbDecaySlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> reverbDecayAttachment;
    std::unique_ptr<juce::Label> reverbDecayLabel;

    std::unique_ptr<juce::Slider> reverbPreDelaySlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> reverbPreDelayAttachment;
    std::unique_ptr<juce::Label> reverbPreDelayLabel;

    // UI Components - Spectral (2 controls)
    std::unique_ptr<juce::Slider> spectralEnhanceSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> spectralEnhanceAttachment;
    std::unique_ptr<juce::Label> spectralEnhanceLabel;

    std::unique_ptr<juce::Slider> harmonicsBoostSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> harmonicsBoostAttachment;
    std::unique_ptr<juce::Label> harmonicsBoostLabel;

    // UI Components - Subharmonic (3 controls)
    std::unique_ptr<juce::Slider> subharmonicMixSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> subharmonicMixAttachment;
    std::unique_ptr<juce::Label> subharmonicMixLabel;

    std::unique_ptr<juce::Slider> subharmonicDepthSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> subharmonicDepthAttachment;
    std::unique_ptr<juce::Label> subharmonicDepthLabel;

    std::unique_ptr<juce::Slider> subharmonicRatioSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> subharmonicRatioAttachment;
    std::unique_ptr<juce::Label> subharmonicRatioLabel;

    // UI Components - Diphone (2 controls)
    std::unique_ptr<juce::Slider> crossfadeDurationSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> crossfadeDurationAttachment;
    std::unique_ptr<juce::Label> crossfadeDurationLabel;

    std::unique_ptr<juce::Slider> formantSmoothingSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> formantSmoothingAttachment;
    std::unique_ptr<juce::Label> formantSmoothingLabel;

    // UI Components - Synthesis Method (1 control)
    std::unique_ptr<juce::ComboBox> synthesisMethodCombo;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> synthesisMethodAttachment;
    std::unique_ptr<juce::Label> synthesisMethodLabel;

    // Preset Management
    std::unique_ptr<juce::TextButton> presetLoadButton;
    std::unique_ptr<juce::TextButton> presetSaveButton;
    std::unique_ptr<juce::ComboBox> presetComboBox;

    // Section Groups (for visual organization)
    std::unique_ptr<juce::GroupComponent> masterGroup;
    std::unique_ptr<juce::GroupComponent> vowelGroup;
    std::unique_ptr<juce::GroupComponent> formantGroup;
    std::unique_ptr<juce::GroupComponent> breathGroup;
    std::unique_ptr<juce::GroupComponent> vibratoGroup;
    std::unique_ptr<juce::GroupComponent> ensembleGroup;
    std::unique_ptr<juce::GroupComponent> adsrGroup;
    std::unique_ptr<juce::GroupComponent> satbGroup;
    std::unique_ptr<juce::GroupComponent> effectsGroup;
    std::unique_ptr<juce::GroupComponent> spectralGroup;
    std::unique_ptr<juce::GroupComponent> subharmonicGroup;
    std::unique_ptr<juce::GroupComponent> diphoneGroup;
    std::unique_ptr<juce::GroupComponent> synthesisGroup;

    // Look and Feel
    std::unique_ptr<juce::LookAndFeel> lookAndFeel;

    ChoirV2Processor& processorRef;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChoirV2Editor)
};
