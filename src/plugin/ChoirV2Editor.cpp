#include "plugin/ChoirV2Editor.h"

//==============================================================================
class ChoirV2LookAndFeel : public juce::LookAndFeel_V4
{
public:
    ChoirV2LookAndFeel()
    {
        // Dark theme colors
        setColour(juce::ResizableWindow::backgroundColourId, juce::Colour(30, 30, 35));
        setColour(juce::Slider::backgroundColourId, juce::Colour(45, 45, 50));
        setColour(juce::Slider::trackColourId, juce::Colour(70, 70, 80));
        setColour(juce::Slider::thumbColourId, juce::Colour(100, 150, 200));
        setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(100, 150, 200));
        setColour(juce::Label::textColourId, juce::Colour(220, 220, 220));
        setColour(juce::Label::backgroundColourId, juce::Colour(30, 30, 35));
        setColour(juce::GroupComponent::textColourId, juce::Colour(180, 180, 180));
        setColour(juce::GroupComponent::outlineColourId, juce::Colour(60, 60, 70));
        setColour(juce::ComboBox::backgroundColourId, juce::Colour(45, 45, 50));
        setColour(juce::ComboBox::textColourId, juce::Colour(220, 220, 220));
        setColour(juce::TextButton::buttonColourId, juce::Colour(70, 70, 80));
        setColour(juce::TextButton::textColourOnId, juce::Colour(220, 220, 220));
    }
};

//==============================================================================
ChoirV2Editor::ChoirV2Editor(ChoirV2Processor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    // Set custom look and feel
    lookAndFeel = std::make_unique<ChoirV2LookAndFeel>();
    setLookAndFeel(lookAndFeel.get());

    // Set editor size
    setSize(1000, 900);

    // Initialize all UI components
    auto& params = processorRef.getValueTreeState();

    //==========================================================================
    // MASTER SECTION
    //==========================================================================
    masterGroup = std::make_unique<juce::GroupComponent>("MasterGroup", "MASTER");
    addAndMakeVisible(*masterGroup);

    // Master Volume
    masterVolumeSlider = std::make_unique<juce::Slider>(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow);
    masterVolumeSlider->setRange(0.0, 1.0, 0.001);
    masterVolumeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, DSP::ChoirV2PureDSP::ParameterID::masterVolume, *masterVolumeSlider);
    addAndMakeVisible(*masterVolumeSlider);

    masterVolumeLabel = std::make_unique<juce::Label>("MasterVolumeLabel", "Volume");
    masterVolumeLabel->attachToComponent(masterVolumeSlider.get(), false);
    masterVolumeLabel->setJustificationType(juce::Justification::centred);
    addAndMakeVisible(*masterVolumeLabel);

    // Polyphony
    polyphonySlider = std::make_unique<juce::Slider>(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow);
    polyphonySlider->setRange(1.0, 64.0, 1.0);
    polyphonyAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, DSP::ChoirV2PureDSP::ParameterID::polyphony, *polyphonySlider);
    addAndMakeVisible(*polyphonySlider);

    polyphonyLabel = std::make_unique<juce::Label>("PolyphonyLabel", "Voices");
    polyphonyLabel->attachToComponent(polyphonySlider.get(), false);
    polyphonyLabel->setJustificationType(juce::Justification::centred);
    addAndMakeVisible(*polyphonyLabel);

    // Text Input
    textInputLabel = std::make_unique<juce::Label>("TextInputLabel", "Text:");
    addAndMakeVisible(*textInputLabel);

    textInputEditor = std::make_unique<juce::TextEditor>("TextInputEditor");
    textInputEditor->setMultiLine(false);
    textInputEditor->setReturnKeyStartsNewLine(false);
    textInputEditor->setReadOnly(false);
    textInputEditor->setScrollbarsShown(false);
    textInputEditor->setCaretVisible(true);
    textInputEditor->setPopupMenuEnabled(true);
    addAndMakeVisible(*textInputEditor);

    // Phoneme Display
    phonemeDisplayLabel = std::make_unique<juce::Label>("PhonemeDisplayLabel", "Phoneme:");
    addAndMakeVisible(*phonemeDisplayLabel);

    phonemeDisplayValue = std::make_unique<juce::Label>("PhonemeDisplayValue", "AH");
    phonemeDisplayValue->setFont(juce::Font(juce::FontOptions()));
    phonemeDisplayValue->setJustificationType(juce::Justification::centred);
    phonemeDisplayValue->setColour(juce::Label::textColourId, juce::Colour(100, 200, 150));
    addAndMakeVisible(*phonemeDisplayValue);

    //==========================================================================
    // VOWEL CONTROL (3D)
    //==========================================================================
    vowelGroup = std::make_unique<juce::GroupComponent>("VowelGroup", "VOWEL CONTROL (3D)");
    addAndMakeVisible(*vowelGroup);

    vowelXSlider = std::make_unique<juce::Slider>(juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight);
    vowelXSlider->setRange(-1.0, 1.0, 0.01);
    vowelXAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, DSP::ChoirV2PureDSP::ParameterID::vowelX, *vowelXSlider);
    addAndMakeVisible(*vowelXSlider);

    vowelXLabel = std::make_unique<juce::Label>("VowelXLabel", "X (Front/Back)");
    vowelXLabel->attachToComponent(vowelXSlider.get(), true);
    addAndMakeVisible(*vowelXLabel);

    vowelYSlider = std::make_unique<juce::Slider>(juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight);
    vowelYSlider->setRange(-1.0, 1.0, 0.01);
    vowelYAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, DSP::ChoirV2PureDSP::ParameterID::vowelY, *vowelYSlider);
    addAndMakeVisible(*vowelYSlider);

    vowelYLabel = std::make_unique<juce::Label>("VowelYLabel", "Y (Open/Closed)");
    vowelYLabel->attachToComponent(vowelYSlider.get(), true);
    addAndMakeVisible(*vowelYLabel);

    vowelZSlider = std::make_unique<juce::Slider>(juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight);
    vowelZSlider->setRange(-1.0, 1.0, 0.01);
    vowelZAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, DSP::ChoirV2PureDSP::ParameterID::vowelZ, *vowelZSlider);
    addAndMakeVisible(*vowelZSlider);

    vowelZLabel = std::make_unique<juce::Label>("VowelZLabel", "Z (Rounded/Spread)");
    vowelZLabel->attachToComponent(vowelZSlider.get(), true);
    addAndMakeVisible(*vowelZLabel);

    //==========================================================================
    // FORMANTS
    //==========================================================================
    formantGroup = std::make_unique<juce::GroupComponent>("FormantGroup", "FORMANTS");
    addAndMakeVisible(*formantGroup);

    formantScaleSlider = std::make_unique<juce::Slider>(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow);
    formantScaleSlider->setRange(0.5, 2.0, 0.01);
    formantScaleAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, DSP::ChoirV2PureDSP::ParameterID::formantScale, *formantScaleSlider);
    addAndMakeVisible(*formantScaleSlider);

    formantScaleLabel = std::make_unique<juce::Label>("FormantScaleLabel", "Scale");
    formantScaleLabel->attachToComponent(formantScaleSlider.get(), false);
    formantScaleLabel->setJustificationType(juce::Justification::centred);
    addAndMakeVisible(*formantScaleLabel);

    formantShiftSlider = std::make_unique<juce::Slider>(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow);
    formantShiftSlider->setRange(-12.0, 12.0, 0.1);
    formantShiftAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, DSP::ChoirV2PureDSP::ParameterID::formantShift, *formantShiftSlider);
    addAndMakeVisible(*formantShiftSlider);

    formantShiftLabel = std::make_unique<juce::Label>("FormantShiftLabel", "Shift");
    formantShiftLabel->attachToComponent(formantShiftSlider.get(), false);
    formantShiftLabel->setJustificationType(juce::Justification::centred);
    addAndMakeVisible(*formantShiftLabel);

    //==========================================================================
    // BREATH
    //==========================================================================
    breathGroup = std::make_unique<juce::GroupComponent>("BreathGroup", "BREATH");
    addAndMakeVisible(*breathGroup);

    breathMixSlider = std::make_unique<juce::Slider>(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow);
    breathMixSlider->setRange(0.0, 1.0, 0.01);
    breathMixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, DSP::ChoirV2PureDSP::ParameterID::breathMix, *breathMixSlider);
    addAndMakeVisible(*breathMixSlider);

    breathMixLabel = std::make_unique<juce::Label>("BreathMixLabel", "Mix");
    breathMixLabel->attachToComponent(breathMixSlider.get(), false);
    breathMixLabel->setJustificationType(juce::Justification::centred);
    addAndMakeVisible(*breathMixLabel);

    breathColorSlider = std::make_unique<juce::Slider>(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow);
    breathColorSlider->setRange(0.0, 1.0, 0.01);
    breathColorAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, DSP::ChoirV2PureDSP::ParameterID::breathColor, *breathColorSlider);
    addAndMakeVisible(*breathColorSlider);

    breathColorLabel = std::make_unique<juce::Label>("BreathColorLabel", "Color");
    breathColorLabel->attachToComponent(breathColorSlider.get(), false);
    breathColorLabel->setJustificationType(juce::Justification::centred);
    addAndMakeVisible(*breathColorLabel);

    //==========================================================================
    // VIBRATO
    //==========================================================================
    vibratoGroup = std::make_unique<juce::GroupComponent>("VibratoGroup", "VIBRATO");
    addAndMakeVisible(*vibratoGroup);

    vibratoRateSlider = std::make_unique<juce::Slider>(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow);
    vibratoRateSlider->setRange(0.0, 10.0, 0.1);
    vibratoRateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, DSP::ChoirV2PureDSP::ParameterID::vibratoRate, *vibratoRateSlider);
    addAndMakeVisible(*vibratoRateSlider);

    vibratoRateLabel = std::make_unique<juce::Label>("VibratoRateLabel", "Rate");
    vibratoRateLabel->attachToComponent(vibratoRateSlider.get(), false);
    vibratoRateLabel->setJustificationType(juce::Justification::centred);
    addAndMakeVisible(*vibratoRateLabel);

    vibratoDepthSlider = std::make_unique<juce::Slider>(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow);
    vibratoDepthSlider->setRange(0.0, 1.0, 0.01);
    vibratoDepthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, DSP::ChoirV2PureDSP::ParameterID::vibratoDepth, *vibratoDepthSlider);
    addAndMakeVisible(*vibratoDepthSlider);

    vibratoDepthLabel = std::make_unique<juce::Label>("VibratoDepthLabel", "Depth");
    vibratoDepthLabel->attachToComponent(vibratoDepthSlider.get(), false);
    vibratoDepthLabel->setJustificationType(juce::Justification::centred);
    addAndMakeVisible(*vibratoDepthLabel);

    vibratoDelaySlider = std::make_unique<juce::Slider>(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow);
    vibratoDelaySlider->setRange(0.0, 1.0, 0.01);
    vibratoDelayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, DSP::ChoirV2PureDSP::ParameterID::vibratoDelay, *vibratoDelaySlider);
    addAndMakeVisible(*vibratoDelaySlider);

    vibratoDelayLabel = std::make_unique<juce::Label>("VibratoDelayLabel", "Delay");
    vibratoDelayLabel->attachToComponent(vibratoDelaySlider.get(), false);
    vibratoDelayLabel->setJustificationType(juce::Justification::centred);
    addAndMakeVisible(*vibratoDelayLabel);

    //==========================================================================
    // ENSEMBLE
    //==========================================================================
    ensembleGroup = std::make_unique<juce::GroupComponent>("EnsembleGroup", "ENSEMBLE");
    addAndMakeVisible(*ensembleGroup);

    tightnessSlider = std::make_unique<juce::Slider>(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow);
    tightnessSlider->setRange(0.0, 1.0, 0.01);
    tightnessAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, DSP::ChoirV2PureDSP::ParameterID::tightness, *tightnessSlider);
    addAndMakeVisible(*tightnessSlider);

    tightnessLabel = std::make_unique<juce::Label>("TightnessLabel", "Tightness");
    tightnessLabel->attachToComponent(tightnessSlider.get(), false);
    tightnessLabel->setJustificationType(juce::Justification::centred);
    addAndMakeVisible(*tightnessLabel);

    ensembleSizeSlider = std::make_unique<juce::Slider>(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow);
    ensembleSizeSlider->setRange(1.0, 32.0, 1.0);
    ensembleSizeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, DSP::ChoirV2PureDSP::ParameterID::ensembleSize, *ensembleSizeSlider);
    addAndMakeVisible(*ensembleSizeSlider);

    ensembleSizeLabel = std::make_unique<juce::Label>("EnsembleSizeLabel", "Size");
    ensembleSizeLabel->attachToComponent(ensembleSizeSlider.get(), false);
    ensembleSizeLabel->setJustificationType(juce::Justification::centred);
    addAndMakeVisible(*ensembleSizeLabel);

    voiceSpreadSlider = std::make_unique<juce::Slider>(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow);
    voiceSpreadSlider->setRange(0.0, 1.0, 0.01);
    voiceSpreadAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, DSP::ChoirV2PureDSP::ParameterID::voiceSpread, *voiceSpreadSlider);
    addAndMakeVisible(*voiceSpreadSlider);

    voiceSpreadLabel = std::make_unique<juce::Label>("VoiceSpreadLabel", "Spread");
    voiceSpreadLabel->attachToComponent(voiceSpreadSlider.get(), false);
    voiceSpreadLabel->setJustificationType(juce::Justification::centred);
    addAndMakeVisible(*voiceSpreadLabel);

    //==========================================================================
    // ADSR ENVELOPE
    //==========================================================================
    adsrGroup = std::make_unique<juce::GroupComponent>("ADSRGroup", "ENVELOPE");
    addAndMakeVisible(*adsrGroup);

    attackSlider = std::make_unique<juce::Slider>(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow);
    attackSlider->setRange(0.0, 2.0, 0.01);
    attackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, DSP::ChoirV2PureDSP::ParameterID::attack, *attackSlider);
    addAndMakeVisible(*attackSlider);

    attackLabel = std::make_unique<juce::Label>("AttackLabel", "A");
    attackLabel->attachToComponent(attackSlider.get(), false);
    attackLabel->setJustificationType(juce::Justification::centred);
    addAndMakeVisible(*attackLabel);

    decaySlider = std::make_unique<juce::Slider>(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow);
    decaySlider->setRange(0.0, 2.0, 0.01);
    decayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, DSP::ChoirV2PureDSP::ParameterID::decay, *decaySlider);
    addAndMakeVisible(*decaySlider);

    decayLabel = std::make_unique<juce::Label>("DecayLabel", "D");
    decayLabel->attachToComponent(decaySlider.get(), false);
    decayLabel->setJustificationType(juce::Justification::centred);
    addAndMakeVisible(*decayLabel);

    sustainSlider = std::make_unique<juce::Slider>(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow);
    sustainSlider->setRange(0.0, 1.0, 0.01);
    sustainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, DSP::ChoirV2PureDSP::ParameterID::sustain, *sustainSlider);
    addAndMakeVisible(*sustainSlider);

    sustainLabel = std::make_unique<juce::Label>("SustainLabel", "S");
    sustainLabel->attachToComponent(sustainSlider.get(), false);
    sustainLabel->setJustificationType(juce::Justification::centred);
    addAndMakeVisible(*sustainLabel);

    releaseSlider = std::make_unique<juce::Slider>(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow);
    releaseSlider->setRange(0.01, 5.0, 0.01);
    releaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, DSP::ChoirV2PureDSP::ParameterID::release, *releaseSlider);
    addAndMakeVisible(*releaseSlider);

    releaseLabel = std::make_unique<juce::Label>("ReleaseLabel", "R");
    releaseLabel->attachToComponent(releaseSlider.get(), false);
    releaseLabel->setJustificationType(juce::Justification::centred);
    addAndMakeVisible(*releaseLabel);

    //==========================================================================
    // SATB BLEND
    //==========================================================================
    satbGroup = std::make_unique<juce::GroupComponent>("SATBGroup", "SATB BLEND");
    addAndMakeVisible(*satbGroup);

    sopranoLevelSlider = std::make_unique<juce::Slider>(juce::Slider::LinearVertical, juce::Slider::TextBoxBelow);
    sopranoLevelSlider->setRange(0.0, 1.0, 0.01);
    sopranoLevelAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, DSP::ChoirV2PureDSP::ParameterID::sopranoLevel, *sopranoLevelSlider);
    addAndMakeVisible(*sopranoLevelSlider);

    sopranoLevelLabel = std::make_unique<juce::Label>("SopranoLevelLabel", "Soprano");
    sopranoLevelLabel->attachToComponent(sopranoLevelSlider.get(), false);
    sopranoLevelLabel->setJustificationType(juce::Justification::centred);
    addAndMakeVisible(*sopranoLevelLabel);

    altoLevelSlider = std::make_unique<juce::Slider>(juce::Slider::LinearVertical, juce::Slider::TextBoxBelow);
    altoLevelSlider->setRange(0.0, 1.0, 0.01);
    altoLevelAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, DSP::ChoirV2PureDSP::ParameterID::altoLevel, *altoLevelSlider);
    addAndMakeVisible(*altoLevelSlider);

    altoLevelLabel = std::make_unique<juce::Label>("AltoLevelLabel", "Alto");
    altoLevelLabel->attachToComponent(altoLevelSlider.get(), false);
    altoLevelLabel->setJustificationType(juce::Justification::centred);
    addAndMakeVisible(*altoLevelLabel);

    tenorLevelSlider = std::make_unique<juce::Slider>(juce::Slider::LinearVertical, juce::Slider::TextBoxBelow);
    tenorLevelSlider->setRange(0.0, 1.0, 0.01);
    tenorLevelAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, DSP::ChoirV2PureDSP::ParameterID::tenorLevel, *tenorLevelSlider);
    addAndMakeVisible(*tenorLevelSlider);

    tenorLevelLabel = std::make_unique<juce::Label>("TenorLevelLabel", "Tenor");
    tenorLevelLabel->attachToComponent(tenorLevelSlider.get(), false);
    tenorLevelLabel->setJustificationType(juce::Justification::centred);
    addAndMakeVisible(*tenorLevelLabel);

    bassLevelSlider = std::make_unique<juce::Slider>(juce::Slider::LinearVertical, juce::Slider::TextBoxBelow);
    bassLevelSlider->setRange(0.0, 1.0, 0.01);
    bassLevelAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, DSP::ChoirV2PureDSP::ParameterID::bassLevel, *bassLevelSlider);
    addAndMakeVisible(*bassLevelSlider);

    bassLevelLabel = std::make_unique<juce::Label>("BassLevelLabel", "Bass");
    bassLevelLabel->attachToComponent(bassLevelSlider.get(), false);
    bassLevelLabel->setJustificationType(juce::Justification::centred);
    addAndMakeVisible(*bassLevelLabel);

    //==========================================================================
    // EFFECTS
    //==========================================================================
    effectsGroup = std::make_unique<juce::GroupComponent>("EffectsGroup", "REVERB");
    addAndMakeVisible(*effectsGroup);

    reverbMixSlider = std::make_unique<juce::Slider>(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow);
    reverbMixSlider->setRange(0.0, 1.0, 0.01);
    reverbMixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, DSP::ChoirV2PureDSP::ParameterID::reverbMix, *reverbMixSlider);
    addAndMakeVisible(*reverbMixSlider);

    reverbMixLabel = std::make_unique<juce::Label>("ReverbMixLabel", "Mix");
    reverbMixLabel->attachToComponent(reverbMixSlider.get(), false);
    reverbMixLabel->setJustificationType(juce::Justification::centred);
    addAndMakeVisible(*reverbMixLabel);

    reverbDecaySlider = std::make_unique<juce::Slider>(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow);
    reverbDecaySlider->setRange(0.1, 10.0, 0.1);
    reverbDecayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, DSP::ChoirV2PureDSP::ParameterID::reverbDecay, *reverbDecaySlider);
    addAndMakeVisible(*reverbDecaySlider);

    reverbDecayLabel = std::make_unique<juce::Label>("ReverbDecayLabel", "Decay");
    reverbDecayLabel->attachToComponent(reverbDecaySlider.get(), false);
    reverbDecayLabel->setJustificationType(juce::Justification::centred);
    addAndMakeVisible(*reverbDecayLabel);

    reverbPreDelaySlider = std::make_unique<juce::Slider>(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow);
    reverbPreDelaySlider->setRange(0.0, 0.1, 0.001);
    reverbPreDelayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, DSP::ChoirV2PureDSP::ParameterID::reverbPredelay, *reverbPreDelaySlider);
    addAndMakeVisible(*reverbPreDelaySlider);

    reverbPreDelayLabel = std::make_unique<juce::Label>("ReverbPreDelayLabel", "Pre-Delay");
    reverbPreDelayLabel->attachToComponent(reverbPreDelaySlider.get(), false);
    reverbPreDelayLabel->setJustificationType(juce::Justification::centred);
    addAndMakeVisible(*reverbPreDelayLabel);

    //==========================================================================
    // SPECTRAL
    //==========================================================================
    spectralGroup = std::make_unique<juce::GroupComponent>("SpectralGroup", "SPECTRAL");
    addAndMakeVisible(*spectralGroup);

    spectralEnhanceSlider = std::make_unique<juce::Slider>(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow);
    spectralEnhanceSlider->setRange(0.0, 1.0, 0.01);
    spectralEnhanceAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, DSP::ChoirV2PureDSP::ParameterID::spectralEnhancement, *spectralEnhanceSlider);
    addAndMakeVisible(*spectralEnhanceSlider);

    spectralEnhanceLabel = std::make_unique<juce::Label>("SpectralEnhanceLabel", "Enhance");
    spectralEnhanceLabel->attachToComponent(spectralEnhanceSlider.get(), false);
    spectralEnhanceLabel->setJustificationType(juce::Justification::centred);
    addAndMakeVisible(*spectralEnhanceLabel);

    harmonicsBoostSlider = std::make_unique<juce::Slider>(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow);
    harmonicsBoostSlider->setRange(0.0, 1.0, 0.01);
    harmonicsBoostAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, DSP::ChoirV2PureDSP::ParameterID::harmonicsBoost, *harmonicsBoostSlider);
    addAndMakeVisible(*harmonicsBoostSlider);

    harmonicsBoostLabel = std::make_unique<juce::Label>("HarmonicsBoostLabel", "Harmonics");
    harmonicsBoostLabel->attachToComponent(harmonicsBoostSlider.get(), false);
    harmonicsBoostLabel->setJustificationType(juce::Justification::centred);
    addAndMakeVisible(*harmonicsBoostLabel);

    //==========================================================================
    // SUBHARMONIC
    //==========================================================================
    subharmonicGroup = std::make_unique<juce::GroupComponent>("SubharmonicGroup", "SUBHARMONIC");
    addAndMakeVisible(*subharmonicGroup);

    subharmonicMixSlider = std::make_unique<juce::Slider>(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow);
    subharmonicMixSlider->setRange(0.0, 1.0, 0.01);
    subharmonicMixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, DSP::ChoirV2PureDSP::ParameterID::subharmonicMix, *subharmonicMixSlider);
    addAndMakeVisible(*subharmonicMixSlider);

    subharmonicMixLabel = std::make_unique<juce::Label>("SubharmonicMixLabel", "Mix");
    subharmonicMixLabel->attachToComponent(subharmonicMixSlider.get(), false);
    subharmonicMixLabel->setJustificationType(juce::Justification::centred);
    addAndMakeVisible(*subharmonicMixLabel);

    subharmonicDepthSlider = std::make_unique<juce::Slider>(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow);
    subharmonicDepthSlider->setRange(0.0, 1.0, 0.01);
    subharmonicDepthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, DSP::ChoirV2PureDSP::ParameterID::subharmonicDepth, *subharmonicDepthSlider);
    addAndMakeVisible(*subharmonicDepthSlider);

    subharmonicDepthLabel = std::make_unique<juce::Label>("SubharmonicDepthLabel", "Depth");
    subharmonicDepthLabel->attachToComponent(subharmonicDepthSlider.get(), false);
    subharmonicDepthLabel->setJustificationType(juce::Justification::centred);
    addAndMakeVisible(*subharmonicDepthLabel);

    subharmonicRatioSlider = std::make_unique<juce::Slider>(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow);
    subharmonicRatioSlider->setRange(0.5, 2.0, 0.01);
    subharmonicRatioAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, DSP::ChoirV2PureDSP::ParameterID::subharmonicRatio, *subharmonicRatioSlider);
    addAndMakeVisible(*subharmonicRatioSlider);

    subharmonicRatioLabel = std::make_unique<juce::Label>("SubharmonicRatioLabel", "Ratio");
    subharmonicRatioLabel->attachToComponent(subharmonicRatioSlider.get(), false);
    subharmonicRatioLabel->setJustificationType(juce::Justification::centred);
    addAndMakeVisible(*subharmonicRatioLabel);

    //==========================================================================
    // DIPHONE
    //==========================================================================
    diphoneGroup = std::make_unique<juce::GroupComponent>("DiphoneGroup", "DIPHONE");
    addAndMakeVisible(*diphoneGroup);

    crossfadeDurationSlider = std::make_unique<juce::Slider>(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow);
    crossfadeDurationSlider->setRange(0.01, 1.0, 0.01);
    crossfadeDurationAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, DSP::ChoirV2PureDSP::ParameterID::diphoneCrossfadeDuration, *crossfadeDurationSlider);
    addAndMakeVisible(*crossfadeDurationSlider);

    crossfadeDurationLabel = std::make_unique<juce::Label>("CrossfadeDurationLabel", "Crossfade");
    crossfadeDurationLabel->attachToComponent(crossfadeDurationSlider.get(), false);
    crossfadeDurationLabel->setJustificationType(juce::Justification::centred);
    addAndMakeVisible(*crossfadeDurationLabel);

    formantSmoothingSlider = std::make_unique<juce::Slider>(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow);
    formantSmoothingSlider->setRange(0.0, 1.0, 0.01);
    formantSmoothingAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, DSP::ChoirV2PureDSP::ParameterID::diphoneFormantSmoothing, *formantSmoothingSlider);
    addAndMakeVisible(*formantSmoothingSlider);

    formantSmoothingLabel = std::make_unique<juce::Label>("FormantSmoothingLabel", "Smoothing");
    formantSmoothingLabel->attachToComponent(formantSmoothingSlider.get(), false);
    formantSmoothingLabel->setJustificationType(juce::Justification::centred);
    addAndMakeVisible(*formantSmoothingLabel);

    //==========================================================================
    // SYNTHESIS METHOD
    //==========================================================================
    synthesisGroup = std::make_unique<juce::GroupComponent>("SynthesisGroup", "SYNTHESIS");
    addAndMakeVisible(*synthesisGroup);

    synthesisMethodCombo = std::make_unique<juce::ComboBox>("SynthesisMethodCombo");
    synthesisMethodCombo->addItem("Formant", 1);
    synthesisMethodCombo->addItem("Subharmonic", 2);
    synthesisMethodCombo->addItem("Diphone", 3);
    synthesisMethodCombo->setSelectedId(1);
    synthesisMethodAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        params, DSP::ChoirV2PureDSP::ParameterID::synthesisMethod, *synthesisMethodCombo);
    addAndMakeVisible(*synthesisMethodCombo);

    synthesisMethodLabel = std::make_unique<juce::Label>("SynthesisMethodLabel", "Method");
    synthesisMethodLabel->attachToComponent(synthesisMethodCombo.get(), true);
    addAndMakeVisible(*synthesisMethodLabel);

    //==========================================================================
    // PRESET MANAGEMENT
    //==========================================================================
    presetLoadButton = std::make_unique<juce::TextButton>("PresetLoadButton", "Load");
    presetLoadButton->setButtonText("Load");
    addAndMakeVisible(*presetLoadButton);

    presetSaveButton = std::make_unique<juce::TextButton>("PresetSaveButton", "Save");
    presetSaveButton->setButtonText("Save");
    addAndMakeVisible(*presetSaveButton);

    presetComboBox = std::make_unique<juce::ComboBox>("PresetComboBox");
    presetComboBox->addItem("Init", 1);
    presetComboBox->addItem("Pad", 2);
    presetComboBox->addItem("Ensemble", 3);
    presetComboBox->addItem("Ethereal", 4);
    presetComboBox->setSelectedId(1);
    addAndMakeVisible(*presetComboBox);

    // Start timer for UI updates (30Hz)
    startTimerHz(30);
}

ChoirV2Editor::~ChoirV2Editor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

//==============================================================================
void ChoirV2Editor::paint(juce::Graphics& g)
{
    // Background
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    // Title
    g.setColour(juce::Colour(220, 220, 220));
    g.setFont(juce::Font(juce::FontOptions()));
    g.drawText("Choir V2", getLocalBounds().removeFromTop(40), juce::Justification::centred, true);

    // Subtitle
    g.setFont(juce::Font(juce::FontOptions()));
    g.drawText("Choral Synthesis Instrument", getLocalBounds().removeFromTop(60), juce::Justification::centred, true);
}

void ChoirV2Editor::resized()
{
    auto bounds = getLocalBounds();
    bounds.removeFromTop(70); // Space for title

    // Master Section (top full width)
    auto masterBounds = bounds.removeFromTop(120);
    masterGroup->setBounds(masterBounds);

    auto masterInner = masterBounds.reduced(10, 20);
    auto masterControls = masterInner.removeFromLeft(masterInner.getWidth() / 2);

    masterVolumeSlider->setBounds(masterControls.removeFromLeft(80).reduced(5));
    polyphonySlider->setBounds(masterControls.removeFromLeft(80).reduced(5));

    auto masterText = masterInner;
    textInputLabel->setBounds(masterText.removeFromLeft(50).reduced(5));
    textInputEditor->setBounds(masterText.removeFromLeft(150).reduced(5));
    phonemeDisplayLabel->setBounds(masterText.removeFromLeft(70).reduced(5));
    phonemeDisplayValue->setBounds(masterText.removeFromLeft(80).reduced(5));

    // Preset management
    auto presetBounds = masterInner.removeFromRight(100).reduced(5);
    presetComboBox->setBounds(presetBounds.removeFromTop(25));
    presetLoadButton->setBounds(presetBounds.removeFromTop(25));
    presetSaveButton->setBounds(presetBounds.removeFromTop(25));

    // Two column layout
    auto leftColumn = bounds.removeFromLeft(bounds.getWidth() / 2 - 5);
    auto rightColumn = bounds;

    // LEFT COLUMN
    // Vowel Control
    auto vowelBounds = leftColumn.removeFromTop(140);
    vowelGroup->setBounds(vowelBounds);
    auto vowelInner = vowelBounds.reduced(10, 25);
    vowelXSlider->setBounds(vowelInner.removeFromTop(30));
    vowelYSlider->setBounds(vowelInner.removeFromTop(30));
    vowelZSlider->setBounds(vowelInner.removeFromTop(30));

    // Formants
    auto formantBounds = leftColumn.removeFromTop(130);
    formantGroup->setBounds(formantBounds);
    auto formantInner = formantBounds.reduced(10, 25);
    formantScaleSlider->setBounds(formantInner.removeFromLeft(formantInner.getWidth() / 2).reduced(5));
    formantShiftSlider->setBounds(formantInner.reduced(5));

    // Breath
    auto breathBounds = leftColumn.removeFromTop(130);
    breathGroup->setBounds(breathBounds);
    auto breathInner = breathBounds.reduced(10, 25);
    breathMixSlider->setBounds(breathInner.removeFromLeft(breathInner.getWidth() / 2).reduced(5));
    breathColorSlider->setBounds(breathInner.reduced(5));

    // Vibrato
    auto vibratoBounds = leftColumn.removeFromTop(150);
    vibratoGroup->setBounds(vibratoBounds);
    auto vibratoInner = vibratoBounds.reduced(10, 25);
    vibratoRateSlider->setBounds(vibratoInner.removeFromLeft(vibratoInner.getWidth() / 3).reduced(5));
    vibratoDepthSlider->setBounds(vibratoInner.removeFromLeft(vibratoInner.getWidth() / 2).reduced(5));
    vibratoDelaySlider->setBounds(vibratoInner.reduced(5));

    // Ensemble
    auto ensembleBounds = leftColumn.removeFromTop(150);
    ensembleGroup->setBounds(ensembleBounds);
    auto ensembleInner = ensembleBounds.reduced(10, 25);
    tightnessSlider->setBounds(ensembleInner.removeFromLeft(ensembleInner.getWidth() / 3).reduced(5));
    ensembleSizeSlider->setBounds(ensembleInner.removeFromLeft(ensembleInner.getWidth() / 2).reduced(5));
    voiceSpreadSlider->setBounds(ensembleInner.reduced(5));

    // ADSR
    auto adsrBounds = leftColumn.removeFromTop(150);
    adsrGroup->setBounds(adsrBounds);
    auto adsrInner = adsrBounds.reduced(10, 25);
    attackSlider->setBounds(adsrInner.removeFromLeft(adsrInner.getWidth() / 4).reduced(5));
    decaySlider->setBounds(adsrInner.removeFromLeft(adsrInner.getWidth() / 3).reduced(5));
    sustainSlider->setBounds(adsrInner.removeFromLeft(adsrInner.getWidth() / 2).reduced(5));
    releaseSlider->setBounds(adsrInner.reduced(5));

    // RIGHT COLUMN
    // SATB
    auto satbBounds = rightColumn.removeFromTop(200);
    satbGroup->setBounds(satbBounds);
    auto satbInner = satbBounds.reduced(10, 25);
    auto satbWidth = satbInner.getWidth() / 4;
    sopranoLevelSlider->setBounds(satbInner.removeFromLeft(satbWidth).reduced(5));
    altoLevelSlider->setBounds(satbInner.removeFromLeft(satbWidth).reduced(5));
    tenorLevelSlider->setBounds(satbInner.removeFromLeft(satbWidth).reduced(5));
    bassLevelSlider->setBounds(satbInner.reduced(5));

    // Reverb
    auto reverbBounds = rightColumn.removeFromTop(150);
    effectsGroup->setBounds(reverbBounds);
    auto reverbInner = reverbBounds.reduced(10, 25);
    reverbMixSlider->setBounds(reverbInner.removeFromLeft(reverbInner.getWidth() / 3).reduced(5));
    reverbDecaySlider->setBounds(reverbInner.removeFromLeft(reverbInner.getWidth() / 2).reduced(5));
    reverbPreDelaySlider->setBounds(reverbInner.reduced(5));

    // Spectral
    auto spectralBounds = rightColumn.removeFromTop(130);
    spectralGroup->setBounds(spectralBounds);
    auto spectralInner = spectralBounds.reduced(10, 25);
    spectralEnhanceSlider->setBounds(spectralInner.removeFromLeft(spectralInner.getWidth() / 2).reduced(5));
    harmonicsBoostSlider->setBounds(spectralInner.reduced(5));

    // Subharmonic
    auto subharmonicBounds = rightColumn.removeFromTop(150);
    subharmonicGroup->setBounds(subharmonicBounds);
    auto subharmonicInner = subharmonicBounds.reduced(10, 25);
    subharmonicMixSlider->setBounds(subharmonicInner.removeFromLeft(subharmonicInner.getWidth() / 3).reduced(5));
    subharmonicDepthSlider->setBounds(subharmonicInner.removeFromLeft(subharmonicInner.getWidth() / 2).reduced(5));
    subharmonicRatioSlider->setBounds(subharmonicInner.reduced(5));

    // Diphone
    auto diphoneBounds = rightColumn.removeFromTop(130);
    diphoneGroup->setBounds(diphoneBounds);
    auto diphoneInner = diphoneBounds.reduced(10, 25);
    crossfadeDurationSlider->setBounds(diphoneInner.removeFromLeft(diphoneInner.getWidth() / 2).reduced(5));
    formantSmoothingSlider->setBounds(diphoneInner.reduced(5));

    // Synthesis Method
    auto synthBounds = rightColumn.removeFromTop(80);
    synthesisGroup->setBounds(synthBounds);
    auto synthInner = synthBounds.reduced(10, 25);
    synthesisMethodCombo->setBounds(synthInner.withSizeKeepingCentre(200, 25));
}

void ChoirV2Editor::timerCallback()
{
    // Update phoneme display from processor
    // This can be used to show real-time updates
    // For now, just a placeholder for dynamic UI updates
}
