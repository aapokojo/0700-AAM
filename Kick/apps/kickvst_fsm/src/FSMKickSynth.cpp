/*
 * FSM Kick Synthesizer Implementation
 * 
 * Based on FSM Kick Buzz machine by Krzysztof Foltman <kfoltman@users.sourceforge.net>
 * Original source: buzzmachines/FSM/Kick/Kick.cpp
 * 
 * This code is derived from work licensed under LGPL 2.1.
 * See: buzzmachines/FSM/COPYING for the original license.
 * 
 * Adapted for JUCE/VST3 by Aapo Kojo
 */

#include "FSMKickSynth.h"

FSMKickSynth::FSMKickSynth()
    : sampleRate(44100.0),
      oscPhase(0.0),
      frequency(0.0),
      xSin(0.0),
      xCos(0.0),
      dxSin(0.0),
      dxCos(0.0),
      amp(0.0),
      mulAmp(0.0),
      antiClick(0.0),
      lastValue(0.0),
      envPhase(0),
      leftOver(0),
      startFreq(100.0f),
      endFreq(50.0f),
      toneDecay(0.5f),
      toneShape(0.5f),
      ampDecay(0.5f),
      volume(0.8f),
      smoothStartFreq(100.0f),
      smoothEndFreq(50.0f),
      smoothToneDecay(0.5f),
      smoothToneShape(0.5f),
      smoothAmpDecay(0.5f),
      smoothVolume(0.8f)
{
    reset();
}

FSMKickSynth::~FSMKickSynth()
{
}

void FSMKickSynth::setSampleRate(double newSampleRate)
{
    sampleRate = newSampleRate;
}

void FSMKickSynth::setStartFrequency(float freq)
{
    startFreq = juce::jlimit(20.0f, 2000.0f, freq);
}

void FSMKickSynth::setEndFrequency(float freq)
{
    endFreq = juce::jlimit(20.0f, 2000.0f, freq);
}

void FSMKickSynth::setToneDecay(float decay)
{
    toneDecay = juce::jlimit(0.0f, 1.0f, decay);
}

void FSMKickSynth::setToneShape(float shape)
{
    toneShape = juce::jlimit(0.0f, 1.0f, shape);
}

void FSMKickSynth::setAmpDecay(float decay)
{
    ampDecay = juce::jlimit(0.0f, 1.0f, decay);
}

void FSMKickSynth::setVolume(float vol)
{
    volume = juce::jlimit(0.0f, 1.0f, vol);
}

void FSMKickSynth::trigger()
{
    // Save anti-click value
    antiClick = lastValue;
    
    // Reset oscillator
    oscPhase = 0.0;
    xSin = 0.0;
    xCos = 1.0;  // Start at cos=1, sin=0
    
    // Reset envelope
    envPhase = 0;
    leftOver = 0;
    
    // Set initial amplitude
    amp = volume * 0.5f;  // Start at half volume
    
    // Calculate initial frequency
    frequency = startFreq;
}

void FSMKickSynth::processBlock(juce::AudioBuffer<float>& buffer, int startSample, int numSamples)
{
    if (numSamples <= 0)
        return;

    float* const* channelData = buffer.getArrayOfWritePointers();
    const int numChannels = buffer.getNumChannels();

    // Process each channel
    for (int channel = 0; channel < numChannels; ++channel)
    {
        float* out = channelData[channel] + startSample;
        
        int i = 0;
        while (i < numSamples)
        {
            if (leftOver <= 0)
            {
                leftOver = 32;  // Process in chunks of 32 samples
                
                // Update envelope and frequency
                double envPoint = envPhase * toneDecay * 0.0025;
                double shapedPoint = std::pow(envPoint, toneShape * 2.0);
                frequency = startFreq * std::pow(endFreq / std::max(startFreq, 0.1f), shapedPoint);
                
                if (frequency > 10000.0 || amp < 0.001f)
                    amp = 0.0f;
                
                // Update amplitude based on decay
                amp *= std::pow(0.5f, ampDecay * envPhase * 0.002f);
                
                if (amp < 0.001f && std::abs(antiClick) < 0.001f)
                {
                    amp = 0.0f;
                    break;
                }

                oscPhase = std::fmod(oscPhase, 1.0);
                mulAmp = std::pow(0.5f, ampDecay * 0.002f);
                
                xSin = std::sin(2.0 * M_PI * oscPhase);
                xCos = std::cos(2.0 * M_PI * oscPhase);
                dxSin = std::sin(2.0 * M_PI * frequency / sampleRate);
                dxCos = std::cos(2.0 * M_PI * frequency / sampleRate);
                
                lastValue = 0.0f;
            }

            int end = juce::jmin(i + leftOver, numSamples);
            
            if (amp > 0.001f)
            {
                for (int j = i; j < end; ++j)
                {
                    float sample = static_cast<float>(antiClick + amp * xSin);
                    out[j] += sample;
                    
                    // Update oscillator
                    double xSin2 = xSin * dxCos + xCos * dxSin;
                    double xCos2 = xCos * dxCos - xSin * dxSin;
                    xSin = xSin2;
                    xCos = xCos2;
                    
                    amp *= mulAmp;
                    antiClick *= 0.98f;
                }
                lastValue = static_cast<float>(antiClick + amp * xSin);
            }
            else
            {
                // Silence
                for (int j = i; j < end; ++j)
                    out[j] += 0.0f;
            }

            oscPhase += (end - i) * frequency / sampleRate;
            envPhase += end - i;
            leftOver -= end - i;
            i = end;
        }
    }
}

bool FSMKickSynth::isActive() const noexcept
{
    return amp > 0.001f || std::abs(antiClick) > 0.001f;
}

void FSMKickSynth::reset()
{
    oscPhase = 0.0;
    xSin = 0.0;
    xCos = 1.0;
    dxSin = 0.0;
    dxCos = 0.0;
    amp = 0.0f;
    mulAmp = 0.0f;
    antiClick = 0.0f;
    lastValue = 0.0f;
    envPhase = 0;
    leftOver = 0;
    frequency = startFreq;
}
