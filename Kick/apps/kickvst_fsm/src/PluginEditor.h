/*
  ==============================================================================

    FSM Kick VST3 Plugin - Editor
    
    Based on FSM Kick Buzz machine by Krzysztof Foltman.
    Original code licensed under LGPL 2.1.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
    FSM Kick VST3 Plugin Editor
 */
class FSMKickAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                      private juce::Slider::Listener,
                                      private juce::AudioProcessorValueTreeState::Listener
{
public:
    FSMKickAudioProcessorEditor (FSMKickAudioProcessor&);
    ~FSMKickAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    FSMKickAudioProcessor& audioProcessor;

    // UI Components
    juce::Slider startFreqSlider;
    juce::Slider endFreqSlider;
    juce::Slider toneDecaySlider;
    juce::Slider toneShapeSlider;
    juce::Slider ampDecaySlider;
    juce::Slider volumeSlider;

    juce::Label startFreqLabel;
    juce::Label endFreqLabel;
    juce::Label toneDecayLabel;
    juce::Label toneShapeLabel;
    juce::Label ampDecayLabel;
    juce::Label volumeLabel;

    juce::Label titleLabel;
    juce::Label creditsLabel;

    void sliderValueChanged(juce::Slider* slider) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FSMKickAudioProcessorEditor)
};
