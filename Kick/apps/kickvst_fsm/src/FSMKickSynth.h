/*
 * FSM Kick Synthesizer
 * 
 * Based on FSM Kick Buzz machine by Krzysztof Foltman <kfoltman@users.sourceforge.net>
 * Original source: buzzmachines/FSM/Kick/Kick.cpp
 * 
 * This code is derived from work licensed under LGPL 2.1.
 * See: buzzmachines/FSM/COPYING for the original license.
 * 
 * Adapted for JUCE/VST3 by Aapo Kojo
 */

#pragma once

#include <JuceHeader.h>

/**
    FSM Kick Synthesizer - A simple kick drum synthesizer
    
    Based on the original Buzz machine by Krzysztof Foltman (FSM).
    
    The synthesis algorithm:
    - Sine wave oscillator with frequency modulation
    - Frequency sweeps from start to end frequency
    - Tone decay with shape control
    - Amplitude decay envelope
    - Trigger-based activation
 */
class FSMKickSynth
{
public:
    FSMKickSynth();
    ~FSMKickSynth();

    /** Initialize the synthesizer with sample rate */
    void setSampleRate(double newSampleRate);

    /** Set parameters */
    void setStartFrequency(float freq);
    void setEndFrequency(float freq);
    void setToneDecay(float decay);
    void setToneShape(float shape);
    void setAmpDecay(float decay);
    void setVolume(float vol);

    /** Trigger the kick */
    void trigger();

    /** Render audio block */
    void processBlock(juce::AudioBuffer<float>& buffer, int startSample, int numSamples);

    /** Check if currently active */
    bool isActive() const noexcept;

    /** Reset the synthesizer */
    void reset();
    // Synthesis state
    double sampleRate;
    double oscPhase;           // Oscillator phase
    double frequency;          // Current frequency
    double xSin, xCos;         // Sine/cosine state for oscillator
    double dxSin, dxCos;       // Phase increments
    double amp;                // Current amplitude
    double mulAmp;             // Amplitude multiplier for decay
    double antiClick;          // Anti-click protection
    double lastValue;          // Last output value

    int envPhase;              // Envelope phase counter
    int leftOver;              // Samples left in current block

    // Parameters
    float startFreq;
    float endFreq;
    float toneDecay;
    float toneShape;
    float ampDecay;
    float volume;

    // Parameter smoothing (optional)
    float smoothStartFreq;
    float smoothEndFreq;
    float smoothToneDecay;
    float smoothToneShape;
    float smoothAmpDecay;
    float smoothVolume;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FSMKickSynth)
};
