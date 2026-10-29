/*
 * FSM KickXP Instrument Engine Implementation
 * 
 * Based on FSM KickXP Buzz machine by Krzysztof Foltman <kfoltman@users.sourceforge.net>
 * Original code: buzzmachines/FSM/KickXP/KickXP.cpp
 * Original license: LGPL 2.1 (see buzzmachines/FSM/COPYING)
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "kickxp_engine.h"
#include <cmath>
#include <algorithm>

namespace mt
{

KickXPEngine::KickXPEngine()
{
    // Initialize parameters with defaults from Buzz machine
    // Always add Volume as first parameter
    params.push_back({ "volume", "Volume", "", 0.0f, 1.0f, 0.7f });
    params.push_back({ "pitch_limit", "Pitch Limit", "note", 0.0f, 127.0f, 60.0f });
    params.push_back({ "start_freq", "Start Freq", "", 0.0f, 1.0f, 145.0f/240.0f });
    params.push_back({ "end_freq", "End Freq", "", 0.0f, 1.0f, 50.0f/240.0f });
    params.push_back({ "buzz", "Buzz", "", 0.0f, 1.0f, 55.0f/100.0f });
    params.push_back({ "click", "Click", "", 0.0f, 1.0f, 28.0f/100.0f });
    params.push_back({ "punch", "Punch", "", 0.0f, 1.0f, 47.0f/100.0f });
    params.push_back({ "tone_decay", "Tone Decay", "", 0.0f, 1.0f, 30.0f/240.0f });
    params.push_back({ "tone_shape", "Tone Shape", "", 0.0f, 1.0f, 27.0f/240.0f });
    params.push_back({ "b_decay", "B Decay", "", 0.0f, 1.0f, 55.0f/240.0f });
    params.push_back({ "cp_decay", "C+P Decay", "", 0.0f, 1.0f, 55.0f/240.0f });
    params.push_back({ "a_dslope", "A DecSlope", "", 0.0f, 1.0f, 1.0f/240.0f });
    params.push_back({ "a_dtime", "A DecTime", "", 0.0f, 1.0f, 32.0f/240.0f });
    params.push_back({ "a_rslope", "A RelSlope", "", 0.0f, 1.0f, 105.0f/240.0f });
    params.push_back({ "note_delay", "NoteDelay", "", 0.0f, 12.0f, 0.0f });

    // Initialize all values
    for (int i = 0; i < kNumParams; i++)
        values[i] = params[i].defaultValue;

    // Initialize state
    sampleRate = 44100.0;
    noteActive = false;
    currentNote = 0;
    noteVelocity = 0.0f;
    
    oscPhase = 0.0;
    xSin = 0.0;
    xCos = 0.0;
    dxSin = 0.0;
    dxCos = 0.0;
    
    amp = 0.0f;
    decAmp = 0.0f;
    bAmp = 0.0f;
    mulBAmp = 0.0f;
    cAmp = 0.0f;
    mulCAmp = 0.0f;
    frequency = 0.0f;
    
    envPhase = 0;
    samplesToGo = 0;
    retrig = 0;
    retrigCount = 0;
    age = 0;
    leftOver = 32;
    antiClick = 0.0f;
    lastValue = 0.0f;
    volume = 0.7f;
    
    // Generate thump data
    generateThumpData();
    
    // Set default parameter values
    updateParameters();
}

void KickXPEngine::generateThumpData()
{
    for (int i = 0; i < 1024; i++)
        thumpData[i] = static_cast<float>(sin(1.37 * i + 0.1337 * (1024 - i) * sin(1.1 * i)) * pow(1.0 / 256.0, i / 1024.0));
}

void KickXPEngine::prepare(double newSampleRate, int maxBlockSize)
{
    sampleRate = newSampleRate;
}

void KickXPEngine::reset()
{
    noteActive = false;
    amp = 0.0f;
    decAmp = 0.0f;
    bAmp = 0.0f;
    cAmp = 0.0f;
    frequency = 0.0f;
    envPhase = 0;
    samplesToGo = 0;
    retrig = 0;
    retrigCount = 0;
    age = 0;
    leftOver = 32;
    antiClick = 0.0f;
    oscPhase = 0.0;
    xSin = 0.0;
    xCos = 0.0;
    dxSin = 0.0;
    dxCos = 0.0;
}

void KickXPEngine::noteOn(int note, float velocity)
{
    noteActive = true;
    currentNote = note;
    noteVelocity = velocity;
    
    // Convert note to pitch limit
    int v = (note & 15) - 1 + 12 * (note >> 4);
    pitchLimit = static_cast<float>(440.0 * pow(2.0, (v - 69) / 12.0));
    
    trigger();
}

void KickXPEngine::noteOff(int note)
{
    if (currentNote == note)
        noteActive = false;
}

void KickXPEngine::allNotesOff()
{
    noteActive = false;
}

void KickXPEngine::trigger()
{
    if (retrig && retrigCount > 0)
    {
        samplesToGo = retrig;
        retrigCount--;
    }
    else
        samplesToGo = 0;
    
    antiClick = lastValue;
    envPhase = 0;
    oscPhase = clickAmt;
    leftOver = 0;
    age = 0;
    amp = 32.0f;
    
    // These would be updated from current parameter values
    updateParameters();
}

void KickXPEngine::updateParameters()
{
    // Volume is always the first parameter
    volume = values[kVolume];
    
    // Update from normalized values (0-1) to Buzz machine ranges
    float startFrqNorm = values[kStartFreq];
    float endFrqNorm = values[kEndFreq];
    float toneDecayNorm = values[kToneDecay];
    float toneShapeNorm = values[kToneShape];
    float decSlopeNorm = values[kDecSlope];
    float decTimeNorm = values[kDecTime];
    float relSlopeNorm = values[kRelSlope];
    float bDecayNorm = values[kBDecay];
    float cDecayNorm = values[kCDecay];
    
    startFreq = static_cast<float>(33.0 * pow(128.0, startFrqNorm));
    endFreq = static_cast<float>(33.0 * pow(16.0, endFrqNorm));
    toneDecay = toneDecayNorm * ((1.0f / 400.0f) * (44100.0f / sampleRate));
    toneShape = toneShapeNorm;
    decSlope = static_cast<float>(pow(20.0, decSlopeNorm - 1.0) * 25.0 / sampleRate);
    decTime = decTimeNorm * sampleRate / 240.0f;
    relSlope = static_cast<float>(pow(20.0, relSlopeNorm - 1.0) * 25.0 / sampleRate);
    bDecay = bDecayNorm / 240.0f;
    cDecay = cDecayNorm / 240.0f;
    buzzAmt = 3.0f * (values[kBuzzAmt]);  // Already 0-1
    clickAmt = values[kClickAmt];
    punchAmt = values[kPunchAmt];
    
    // Note: removed kTrigger reference, using volume instead
}

void KickXPEngine::setParam(int index, float value)
{
    if (index >= 0 && index < kNumParams)
    {
        values[index] = value;
        updateParameters();
    }
}

void KickXPEngine::process(float* const* out, int numChannels, int numSamples)
{
    float* outLeft = out[0];
    float* outRight = numChannels > 1 ? out[1] : nullptr;
    
    // If no note is active and we're done, output silence
    if (!noteActive && amp <= 0.00001f)
    {
        for (int i = 0; i < numSamples; i++)
        {
            outLeft[i] = 0.0f;
            if (outRight)
                outRight[i] = 0.0f;
        }
        amp = 0.0f;
        return;
    }
    
    // Process samples
    for (int i = 0; i < numSamples; i++)
    {
        // Handle retrigger
        if (samplesToGo == 1)
        {
            trigger();
        }
        
        // Update oscillator state
        oscPhase = fmod(oscPhase, 1.0);
        float ratio = endFreq / startFreq;
        
        // Clamp anti-click
        if (antiClick < -64000.0f) antiClick = -64000.0f;
        if (antiClick >= 64000.0f) antiClick = 64000.0f;
        
        // Only process every 32 samples (leftOver mechanism)
        if (leftOver <= 0)
        {
            leftOver = 32;
            
            double envPoint = envPhase * toneDecay;
            double shapedPoint = pow(envPoint, toneShape * 2.0);
            frequency = startFreq * static_cast<float>(pow(ratio, shapedPoint));
            
            if (frequency > 10000.0f)
                envPhase = 6553600;
            
            if (envPhase < decTime)
            {
                decAmp = decSlope;
                amp = static_cast<float>(1.0 - decAmp * envPhase);
            }
            else
            {
                decAmp = decSlope;
                amp = static_cast<float>(1.0 - decAmp * decTime);
                if (amp > 0.0f)
                {
                    decAmp = relSlope;
                    amp = amp - decAmp * (envPhase - decTime);
                }
            }
            
            if (amp <= 0.0f)
            {
                amp = 0.0f;
                decAmp = 0.0f;
                if (fabs(antiClick) < 0.00012f && samplesToGo == 0)
                {
                    // Note is done
                    if (!noteActive)
                    {
                        outLeft[i] = 0.0f;
                        if (outRight) outRight[i] = 0.0f;
                        continue;
                    }
                }
            }
            
            // Update buzz and click amplitudes
            float odsr = 1.0f / static_cast<float>(sampleRate);
            bAmp = buzzAmt * static_cast<float>(pow(1.0 / 256.0, toneDecay * envPhase * (odsr * 10.0)));
            float cVal = static_cast<float>(pow(1.0 / 256.0, cDecay * envPhase * (odsr * 20.0)));
            cAmp = clickAmt * cVal;
            frequency *= (1.0f + 2.0f * punchAmt * cVal * cVal * cVal);
            
            if (frequency > 10000.0f)
                frequency = 10000.0f;
            if (frequency < pitchLimit)
                frequency = pitchLimit;
            
            mulBAmp = static_cast<float>(pow(1.0 / 256.0, bDecay * (10.0 * odsr)));
            mulCAmp = static_cast<float>(pow(1.0 / 256.0, cDecay * (10.0 * odsr)));
            
            // Update oscillator coefficients
            xSin = static_cast<float>(sin(2.0 * 3.141592653589793 * oscPhase));
            xCos = static_cast<float>(cos(2.0 * 3.141592653589793 * oscPhase));
            dxSin = static_cast<float>(sin(2.0 * 3.141592653589793 * frequency / sampleRate));
            dxCos = static_cast<float>(cos(2.0 * 3.141592653589793 * frequency / sampleRate));
        }
        
        // Process sample
        float sample = 0.0f;
        bool amphigh = amp > 0.00001f;
        float vol = 0.5f * volume;
        
        if (amphigh && vol > 0.0f)
        {
            float oldAmp = amp;
            
            // Main oscillator with buzz
            if (bAmp > 0.01f)
            {
                sample = antiClick + amp * vol * xSin;
                
                if (xSin > 0.0f)
                {
                    float d = amp * vol * bAmp * xSin * xCos;
                    sample -= d;
                }
                
                // Update oscillator
                double xSin2 = xSin * dxCos + xCos * dxSin;
                double xCos2 = xCos * dxCos - xSin * dxSin;
                xSin = static_cast<float>(xSin2);
                xCos = static_cast<float>(xCos2);
                
                amp -= decAmp;
                bAmp *= mulBAmp;
                antiClick *= 0.98f;
            }
            else
            {
                sample = antiClick + amp * vol * xSin;
                
                // Update oscillator
                double xSin2 = xSin * dxCos + xCos * dxSin;
                double xCos2 = xCos * dxCos - xSin * dxSin;
                xSin = static_cast<float>(xSin2);
                xCos = static_cast<float>(xCos2);
                
                amp -= decAmp;
                antiClick *= 0.98f;
            }
            
            if (fabs(antiClick) < 0.0001f)
                antiClick = 0.0001f;
            
            // Add click (thump)
            if (oldAmp > 0.1f && cAmp > 0.001f)
            {
                int max2 = std::min(numSamples - i, 1024 - age);
                for (int j = 0; j < max2; j++)
                {
                    float thump = oldAmp * vol * cAmp * thumpData[age];
                    if (i + j < numSamples)
                    {
                        outLeft[i + j] += thump;
                        if (outRight) outRight[i + j] += thump;
                    }
                    oldAmp -= decAmp;
                    cAmp *= mulCAmp;
                    age++;
                }
                sample += oldAmp * vol * cAmp * thumpData[age - max2];
            }
            
            // Update phase
            oscPhase += frequency / sampleRate;
            envPhase++;
            leftOver--;
            
            if (samplesToGo > 0)
                samplesToGo--;
        }
        
        // Output sample
        outLeft[i] = sample;
        if (outRight)
            outRight[i] = sample;
        
        lastValue = sample;
    }
}

} // namespace mt
