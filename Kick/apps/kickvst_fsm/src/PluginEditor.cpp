/*
  ==============================================================================

    FSM Kick VST3 Plugin - Editor Implementation
    
    Based on FSM Kick Buzz machine by Krzysztof Foltman.
    Original code licensed under LGPL 2.1.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
FSMKickAudioProcessorEditor::FSMKickAudioProcessorEditor (FSMKickAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 400);

    // Set up sliders
    addAndMakeVisible(startFreqSlider);
    startFreqSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    startFreqSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 20);
    startFreqSlider.setRange(20.0, 2000.0, 1.0);
    startFreqSlider.setSkewFactorFromMidPoint(440.0);
    startFreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.state, "start_freq", startFreqSlider);

    addAndMakeVisible(endFreqSlider);
    endFreqSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    endFreqSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 20);
    endFreqSlider.setRange(20.0, 2000.0, 1.0);
    endFreqSlider.setSkewFactorFromMidPoint(440.0);
    endFreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.state, "end_freq", endFreqSlider);

    addAndMakeVisible(toneDecaySlider);
    toneDecaySlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    toneDecaySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 20);
    toneDecaySlider.setRange(0.0, 1.0, 0.01);
    toneDecayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.state, "tone_decay", toneDecaySlider);

    addAndMakeVisible(toneShapeSlider);
    toneShapeSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    toneShapeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 20);
    toneShapeSlider.setRange(0.0, 1.0, 0.01);
    toneShapeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.state, "tone_shape", toneShapeSlider);

    addAndMakeVisible(ampDecaySlider);
    ampDecaySlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    ampDecaySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 20);
    ampDecaySlider.setRange(0.0, 1.0, 0.01);
    ampDecayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.state, "amp_decay", ampDecaySlider);

    addAndMakeVisible(volumeSlider);
    volumeSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    volumeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 20);
    volumeSlider.setRange(0.0, 1.0, 0.01);
    volumeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.state, "volume", volumeSlider);

    // Set up labels
    addAndMakeVisible(startFreqLabel);
    startFreqLabel.setText("Start Freq", juce::dontSendNotification);
    startFreqLabel.setJustificationType(juce::Justification::centred);
    startFreqLabel.attachToComponent(&startFreqSlider, false);

    addAndMakeVisible(endFreqLabel);
    endFreqLabel.setText("End Freq", juce::dontSendNotification);
    endFreqLabel.setJustificationType(juce::Justification::centred);
    endFreqLabel.attachToComponent(&endFreqSlider, false);

    addAndMakeVisible(toneDecayLabel);
    toneDecayLabel.setText("Tone Decay", juce::dontSendNotification);
    toneDecayLabel.setJustificationType(juce::Justification::centred);
    toneDecayLabel.attachToComponent(&toneDecaySlider, false);

    addAndMakeVisible(toneShapeLabel);
    toneShapeLabel.setText("Tone Shape", juce::dontSendNotification);
    toneShapeLabel.setJustificationType(juce::Justification::centred);
    toneShapeLabel.attachToComponent(&toneShapeSlider, false);

    addAndMakeVisible(ampDecayLabel);
    ampDecayLabel.setText("Amp Decay", juce::dontSendNotification);
    ampDecayLabel.setJustificationType(juce::Justification::centred);
    ampDecayLabel.attachToComponent(&ampDecaySlider, false);

    addAndMakeVisible(volumeLabel);
    volumeLabel.setText("Volume", juce::dontSendNotification);
    volumeLabel.setJustificationType(juce::Justification::centred);
    volumeLabel.attachToComponent(&volumeSlider, false);

    // Title and credits
    addAndMakeVisible(titleLabel);
    titleLabel.setText("FSM Kick", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(24.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(creditsLabel);
    creditsLabel.setText("Based on FSM Kick by Krzysztof Foltman (LGPL 2.1)", juce::dontSendNotification);
    creditsLabel.setFont(juce::Font(12.0f));
    creditsLabel.setJustificationType(juce::Justification::centred);
}

FSMKickAudioProcessorEditor::~FSMKickAudioProcessorEditor()
{
}

//==============================================================================
void FSMKickAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Fill background
    g.fillAll(juce::Colours::darkgrey);

    // Draw a border
    g.setColour(juce::Colours::black);
    g.drawRect(getLocalBounds(), 1);

    // Set font for any text
    g.setFont(15.0f);
}

void FSMKickAudioProcessorEditor::resized()
{
    // Layout components
    auto bounds = getLocalBounds().reduced(10);
    
    // Title at top
    titleLabel.setBounds(bounds.removeFromTop(40));
    
    // Credits at bottom
    creditsLabel.setBounds(bounds.removeFromBottom(20));
    
    // Create a grid for the sliders
    auto sliderRow = bounds.removeFromTop(100);
    
    // Row 1
    startFreqSlider.setBounds(sliderRow.removeFromLeft(100));
    endFreqSlider.setBounds(sliderRow.removeFromLeft(100));
    toneDecaySlider.setBounds(sliderRow.removeFromLeft(100));
    
    // Row 2
    sliderRow = bounds.removeFromTop(100);
    toneShapeSlider.setBounds(sliderRow.removeFromLeft(100));
    ampDecaySlider.setBounds(sliderRow.removeFromLeft(100));
    volumeSlider.setBounds(sliderRow.removeFromLeft(100));
}

void FSMKickAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    // Update synth parameters when sliders change
    audioProcessor.kickSynth.setStartFrequency(startFreqSlider.getValue());
    audioProcessor.kickSynth.setEndFrequency(endFreqSlider.getValue());
    audioProcessor.kickSynth.setToneDecay(toneDecaySlider.getValue());
    audioProcessor.kickSynth.setToneShape(toneShapeSlider.getValue());
    audioProcessor.kickSynth.setAmpDecay(ampDecaySlider.getValue());
    audioProcessor.kickSynth.setVolume(volumeSlider.getValue());
}
