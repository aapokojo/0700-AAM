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
                       ),
    state(*this, nullptr, juce::Identifier("FSMKickParameters"), 
          createParameterLayout())
#endif
{
    // Get parameter pointers from state
    startFreqParam = dynamic_cast<juce::AudioParameterFloat*>(state.getParameter("start_freq"));
    endFreqParam = dynamic_cast<juce::AudioParameterFloat*>(state.getParameter("end_freq"));
    toneDecayParam = dynamic_cast<juce::AudioParameterFloat*>(state.getParameter("tone_decay"));
    toneShapeParam = dynamic_cast<juce::AudioParameterFloat*>(state.getParameter("tone_shape"));
    ampDecayParam = dynamic_cast<juce::AudioParameterFloat*>(state.getParameter("amp_decay"));
    volumeParam = dynamic_cast<juce::AudioParameterFloat*>(state.getParameter("volume"));

    jassert(startFreqParam != nullptr);
    jassert(endFreqParam != nullptr);
    jassert(toneDecayParam != nullptr);
    jassert(toneShapeParam != nullptr);
    jassert(ampDecayParam != nullptr);
    jassert(volumeParam != nullptr);

    // Initialize synth with default values
    kickSynth.setSampleRate(44100.0);
    kickSynth.setStartFrequency(startFreqParam->get());
    kickSynth.setEndFrequency(endFreqParam->get());
    kickSynth.setToneDecay(toneDecayParam->get());
    kickSynth.setToneShape(toneShapeParam->get());
    kickSynth.setAmpDecay(ampDecayParam->get());
    kickSynth.setVolume(volumeParam->get());
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
    kickSynth.setStartFrequency(startFreqParam->get());
    kickSynth.setEndFrequency(endFreqParam->get());
    kickSynth.setToneDecay(toneDecayParam->get());
    kickSynth.setToneShape(toneShapeParam->get());
    kickSynth.setAmpDecay(ampDecayParam->get());
    kickSynth.setVolume(volumeParam->get());
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
    // ValueTreeState handles state saving automatically
    auto stateTree = state.copyState();
    std::unique_ptr<juce::XmlElement> xml(stateTree.createXml());
    copyXmlToBinary(*xml, destData);
}

void FSMKickAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // ValueTreeState handles state loading automatically
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
    {
        auto stateTree = juce::ValueTree::fromXml(*xmlState);
        state.replaceState(stateTree);
    }
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout FSMKickAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "start_freq", "Start Frequency",
        juce::NormalisableRange<float>(20.0f, 2000.0f, 440.0f),
        150.0f, "Hz",
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value, 1) + " Hz"; }
    ));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "end_freq", "End Frequency",
        juce::NormalisableRange<float>(20.0f, 2000.0f, 440.0f),
        50.0f, "Hz",
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value, 1) + " Hz"; }
    ));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "tone_decay", "Tone Decay",
        juce::NormalisableRange<float>(0.0f, 1.0f),
        0.5f,
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value * 100.0f, 0) + "%"; }
    ));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "tone_shape", "Tone Shape",
        juce::NormalisableRange<float>(0.0f, 1.0f),
        0.5f,
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value * 100.0f, 0) + "%"; }
    ));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "amp_decay", "Amp Decay",
        juce::NormalisableRange<float>(0.0f, 1.0f),
        0.5f,
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value * 100.0f, 0) + "%"; }
    ));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "volume", "Volume",
        juce::NormalisableRange<float>(0.0f, 1.0f),
        0.8f,
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value * 100.0f, 0) + "%"; }
    ));
    
    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FSMKickAudioProcessor();
}
