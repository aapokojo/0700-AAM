/*
  ==============================================================================

    FSM Kick VST3 Plugin - Audio Processor
    
    Based on FSM Kick Buzz machine by Krzysztof Foltman.
    Original code licensed under LGPL 2.1.
    See: buzzmachines/FSM/COPYING

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "FSMKickSynth.h"

//==============================================================================
/**
    FSM Kick VST3 Audio Processor
    
    A simple kick drum synthesizer VST3 plugin based on the FSM Kick algorithm.
 */
class FSMKickAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    FSMKickAudioProcessor();
    ~FSMKickAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    //==============================================================================
    FSMKickSynth kickSynth;

    // Make synth accessible to editor
    friend class FSMKickAudioProcessorEditor;
    
    // MIDI handling
    void handleNoteOn(int note, int velocity);
    void handleNoteOff(int note);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FSMKickAudioProcessor)
};
