/*
  ==============================================================================

    FSM Kick VST3 Plugin - Audio Processor Implementation
    
    Based on FSM Kick Buzz machine by Krzysztof Foltman.
    Original code licensed under LGPL 2.1.
    See: buzzmachines/FSM/COPYING

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
FSMKickAudioProcessor::FSMKickAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    // Initialize synth
    kickSynth.setSampleRate(44100.0);
    kickSynth.setStartFrequency(150.0f);
    kickSynth.setEndFrequency(50.0f);
    kickSynth.setToneDecay(0.5f);
    kickSynth.setToneShape(0.5f);
    kickSynth.setAmpDecay(0.5f);
    kickSynth.setVolume(0.8f);
}

FSMKickAudioProcessor::~FSMKickAudioProcessor()
{
}

//==============================================================================
const juce::String FSMKickAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool FSMKickAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool FSMKickAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool FSMKickAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double FSMKickAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int FSMKickAudioProcessor::getNumPrograms()
{
    return 1;
}

int FSMKickAudioProcessor::getCurrentProgram()
{
    return 0;
}

void FSMKickAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String FSMKickAudioProcessor::getProgramName (int index)
{
    return {};
}

void FSMKickAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void FSMKickAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    kickSynth.setSampleRate(sampleRate);
}

void FSMKickAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool FSMKickAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // Support mono or stereo
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // Input must match output for effects
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void FSMKickAudioProcessor::handleNoteOn(int note, int velocity)
{
    // Trigger the kick on any note-on
    kickSynth.trigger();
}

void FSMKickAudioProcessor::handleNoteOff(int note)
{
    // Could stop the envelope, but kick drums typically don't need this
}

void FSMKickAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    
    // Process MIDI messages
    for (const auto& metadata : midiMessages)
    {
        const auto& msg = metadata.getMessage();
        
        if (msg.isNoteOn())
            handleNoteOn(msg.getNoteNumber(), msg.getVelocity());
        else if (msg.isNoteOff())
            handleNoteOff(msg.getNoteNumber());
    }

    // Clear output if no input (for synth mode)
    #if JucePlugin_IsSynth
    buffer.clear();
    #endif

    // Process audio
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear unused output channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Process the kick synth
    kickSynth.processBlock(buffer, 0, buffer.getNumSamples());
}

//==============================================================================
bool FSMKickAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* FSMKickAudioProcessor::createEditor()
{
    return new FSMKickAudioProcessorEditor (*this);
}

//==============================================================================
void FSMKickAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // For now, no state to save
}

void FSMKickAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // For now, no state to load
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FSMKickAudioProcessor();
}
