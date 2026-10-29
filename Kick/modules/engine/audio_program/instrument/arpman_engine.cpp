/*
 * FSM ArpMan Instrument Engine Implementation
 * 
 * Based on FSM ArpMan Buzz machine by Krzysztof Foltman <kfoltman@users.sourceforge.net>
 * Original code: buzzmachines/FSM/ArpMan/ArpMan.cpp
 * Original license: LGPL 2.1 (see buzzmachines/FSM/COPYING)
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "arpman_engine.h"
#include <cmath>
#include <algorithm>

namespace mt
{

ArpManEngine::ArpManEngine()
{
    // Initialize parameters - Volume first, then rest in order
    params.push_back({ "volume", "Volume", "", 0.0f, 1.0f, 0.7f });
    
    // Oscillator parameters with discrete choices
    juce::StringArray waveformChoices;
    waveformChoices.addArray({ "Sine", "Saw", "Square", "Triangle", "Noise",
                                "Hex 1", "Hex 2", "Octave", "FM 1", "FM 2",
                                "FM 3", "FM 4", "E.Piano 1", "E.Piano 2" });
    
    InstrumentParam waveParamA = { "wave_a", "OSC1 Waveform", "", 0.0f, 13.0f, 0.0f };
    waveParamA.choices = waveformChoices;
    params.push_back(waveParamA);
    
    InstrumentParam waveParamB = { "wave_b", "OSC2 Waveform", "", 0.0f, 13.0f, 0.0f };
    waveParamB.choices = waveformChoices;
    params.push_back(waveParamB);
    params.push_back({ "transpose", "OSC2 Transpose", "st", -36.0f, 36.0f, 0.0f });  // 0 = no transpose
    params.push_back({ "detune", "OSC Detune", "ct", -1200.0f, 1200.0f, 0.0f });  // 0 = no detune
    params.push_back({ "glide", "Glide", "ms", 0.0f, 1000.0f, 0.0f });  // 0 = no glide
    
    // Filter parameters
    InstrumentParam filterTypeParam = { "filter_type", "Filter Type", "", 0.0f, 6.0f, 0.0f };
    juce::StringArray filterTypeChoices;
    filterTypeChoices.addArray({ "Lowpass", "Highpass", "Bandpass", "Notch", "Peak", "Low Shelf", "High Shelf" });
    filterTypeParam.choices = filterTypeChoices;
    params.push_back(filterTypeParam);  // 0 = lowpass
    params.push_back({ "filter_cutoff", "Filter Cutoff", "Hz", 20.0f, 20000.0f, 1000.0f });
    params.push_back({ "filter_reso", "Filter Resonance", "", 0.0f, 1.0f, 0.5f });
    params.push_back({ "filter_mod", "Filter Modulation", "", 0.0f, 1.0f, 0.0f });
    params.push_back({ "filter_att", "Filter Attack", "s", 0.001f, 4.0f, 0.1f });
    params.push_back({ "filter_decay", "Filter Decay", "s", 0.001f, 4.0f, 0.5f });
    params.push_back({ "filter_shape", "Filter Shape", "", 0.0f, 1.0f, 0.5f });
    params.push_back({ "filter_inertia", "Filter Inertia", "", 0.0f, 1.0f, 0.5f });
    
    // LFO parameters
    params.push_back({ "lfo_rate", "LFO Rate", "Hz", 0.01f, 20.0f, 1.0f });
    params.push_back({ "lfo_amt1", "LFO Amount 1", "", 0.0f, 1.0f, 0.0f });
    params.push_back({ "lfo_amt2", "LFO Amount 2", "", 0.0f, 1.0f, 0.0f });
    params.push_back({ "lfo_shape", "LFO Shape", "", 0.0f, 1.0f, 0.0f });
    params.push_back({ "lfo_phase", "LFO Phase", "", 0.0f, 1.0f, 0.0f });
    
    // Amp envelope parameters
    params.push_back({ "amp_att", "Amp Attack", "s", 0.001f, 4.0f, 0.01f });
    params.push_back({ "amp_decay", "Amp Decay", "s", 0.001f, 4.0f, 0.5f });
    
    // Arpeggiator parameters
    params.push_back({ "arp_type", "Arp Type", "", 0.0f, 10.0f, 0.0f });
    params.push_back({ "arp_timing", "Arp Timing", "", 0.0f, 12.0f, 4.0f });
    params.push_back({ "polyphony", "Polyphony", "", 1.0f, 16.0f, 1.0f });
    params.push_back({ "arp_phase", "Arp Phase", "", 0.0f, 1.0f, 0.0f });
    params.push_back({ "arp_note", "Arp Note", "", 0.0f, 127.0f, 60.0f });
    params.push_back({ "arp_accent", "Arp Accent", "", 0.0f, 1.0f, 0.0f });
    params.push_back({ "arp_length", "Arp Length", "", 0.0f, 1.0f, 0.5f });

    // Initialize all values
    for (int i = 0; i < kNumParams; i++)
        values[i] = params[i].defaultValue;

    // Initialize state
    sampleRate = 44100.0;
    noteActive = false;
    currentNote = 60;
    noteVelocity = 1.0f;
    
    oscPhaseA = 0.0;
    oscPhaseB = 0.0;
    freqA = 0.0;
    freqB = 0.0;
    transpose = 0.0f;
    detune = 0.0f;
    glide = 0.0f;
    
    filterType = 0;
    filterCutoff = 1000.0f;
    filterResonance = 0.5f;
    filterModulation = 0.0f;
    filterAttack = 0.1f;
    filterDecay = 0.5f;
    filterShape = 0.5f;
    filterInertia = 0.5f;
    filterEnvPhase = 0;
    filterEnvValue = 0.0f;
    
    lfoPhase = 0.0f;
    lfoRate = 1.0f;
    lfoAmount1 = 0.0f;
    lfoAmount2 = 0.0f;
    lfoShape = 0.0f;
    lfoPhase = 0.0f;
    
    ampEnvPhase = 0;
    ampEnvValue = 0.0f;
    ampAttack = 0.01f;
    ampDecay = 0.5f;
    
    arpType = 0;
    arpTiming = 4;
    arpPhase = 0;
    arpNote = 60;
    arpAccent = 0;
    arpLength = 0.5f;
    arpPatternPos = 0;
    polyphony = 1;
    
    // Generate wavetables
    generateWavetables();
    
    // Update parameters
    updateParameters();
}

void ArpManEngine::generateWavetables()
{
    const int waveSize = 4096;
    
    for (int i = 0; i < waveSize; i++)
    {
        double phase = static_cast<double>(i) / waveSize * 2.0 * M_PI;
        
        // Waveform 0: Sine
        waveTables[0][i] = static_cast<float>(sin(phase));
        
        // Waveform 1: Saw
        waveTables[1][i] = static_cast<float>((2.0 / M_PI) * (phase - sin(phase)));
        
        // Waveform 2: Square
        waveTables[2][i] = static_cast<float>(sin(phase) > 0.0 ? 1.0 : -1.0);
        
        // Waveform 3: Triangle
        waveTables[3][i] = static_cast<float>(fabs(4.0 * (phase / (2.0 * M_PI) - 0.5)) - 1.0);
        
        // Waveform 4: Noise (pseudo-random)
        waveTables[4][i] = static_cast<float>((rand() / static_cast<float>(RAND_MAX)) * 2.0 - 1.0);
        
        // Waveform 5-13: Placeholder for hex 1, hex 2, oct, fm 1-4, epiano 1-2
        // For now, duplicate sine with variations
        waveTables[5][i] = waveTables[0][i];  // hex 1
        waveTables[6][i] = waveTables[0][i];  // hex 2
        waveTables[7][i] = waveTables[0][i];  // oct
        waveTables[8][i] = waveTables[0][i];  // fm 1
        waveTables[9][i] = waveTables[0][i];  // fm 2
        waveTables[10][i] = waveTables[0][i]; // fm 3
        waveTables[11][i] = waveTables[0][i]; // fm 4
        waveTables[12][i] = waveTables[0][i]; // epiano 1
        waveTables[13][i] = waveTables[0][i]; // epiano 2
    }
}

void ArpManEngine::prepare(double newSampleRate, int maxBlockSize)
{
    sampleRate = newSampleRate;
}

void ArpManEngine::reset()
{
    noteActive = false;
    oscPhaseA = 0.0;
    oscPhaseB = 0.0;
    filterEnvPhase = 0;
    filterEnvValue = 0.0f;
    lfoPhase = 0.0f;
    ampEnvPhase = 0;
    ampEnvValue = 0.0f;
    arpPatternPos = 0;
}

void ArpManEngine::noteOn(int note, float velocity)
{
    noteActive = true;
    currentNote = note;
    noteVelocity = velocity;
    
    // Trigger the note
    triggerNote(note, velocity);
}

void ArpManEngine::noteOff(int note)
{
    if (currentNote == note)
        noteActive = false;
}

void ArpManEngine::allNotesOff()
{
    noteActive = false;
}

void ArpManEngine::triggerNote(int note, float velocity)
{
    // Calculate frequencies based on note
    float baseFreq = noteToFreq(note);
    freqA = baseFreq;
    
    // OSC2 with transpose and detune
    float semitones = transpose;
    float cents = detune;
    freqB = baseFreq * powf(2.0f, (semitones + cents / 100.0f) / 12.0f);
    
    // Reset envelopes
    filterEnvPhase = 0;
    filterEnvValue = 0.0f;
    ampEnvPhase = 0;
    ampEnvValue = 1.0f;  // Start at max for immediate sound
    
    // Reset LFO
    lfoPhase = 0.0f;
}

void ArpManEngine::updateParameters()
{
    // Volume
    volume = values[kVolume];
    
    // Oscillators
    int waveA = static_cast<int>(values[kWaveformA]);
    int waveB = static_cast<int>(values[kWaveformB]);
    transpose = values[kTranspose];  // -36 to 36 semitones
    detune = values[kDetune];       // -1200 to 1200 cents
    glide = values[kGlide];         // 0 to 1000ms
    
    // Filter
    filterType = static_cast<int>(values[kFilterType]);
    filterCutoff = values[kFilterCutoff];
    filterResonance = values[kFilterResonance];
    filterModulation = values[kFilterModulation];
    filterAttack = values[kFilterAttack];
    filterDecay = values[kFilterDecay];
    filterShape = values[kFilterShape];
    filterInertia = values[kFilterInertia];
    
    // LFO
    lfoRate = values[kLFORate];
    lfoAmount1 = values[kLFOAmount1];
    lfoAmount2 = values[kLFOAmount2];
    lfoShape = values[kLFOShape];
    lfoPhase = values[kLFOPhase];
    
    // Amp envelope
    ampAttack = values[kAmpAttack];
    ampDecay = values[kAmpDecay];
    
    // Arpeggiator
    arpType = static_cast<int>(values[kArpType]);
    arpTiming = static_cast<int>(values[kArpTiming]);
    polyphony = static_cast<int>(values[kPolyphony]);
    arpPhase = static_cast<int>(values[kArpPhase]);
    arpNote = static_cast<int>(values[kNote]);
    arpAccent = static_cast<int>(values[kAccent]);
    arpLength = values[kLength];
}

void ArpManEngine::setParam(int index, float value)
{
    if (index >= 0 && index < kNumParams)
    {
        values[index] = value;
        updateParameters();
    }
}

float ArpManEngine::getWaveformValue(int waveform, double phase)
{
    // Normalize phase to [0, 1)
    phase = fmod(phase, 1.0);
    if (phase < 0.0)
        phase += 1.0;
    
    // Map phase to wavetable index
    int index = static_cast<int>(phase * 4096.0);
    if (index >= 4096)
        index = 4095;
    
    // Clamp waveform index
    waveform = std::max(0, std::min(13, waveform));
    
    return waveTables[waveform][index];
}

float ArpManEngine::noteToFreq(int note)
{
    return 440.0f * powf(2.0f, (note - 69.0f) / 12.0f);
}

void ArpManEngine::process(float* const* out, int numChannels, int numSamples)
{
    float* outLeft = out[0];
    float* outRight = numChannels > 1 ? out[1] : nullptr;
    
    if (!noteActive)
    {
        // Output silence
        for (int i = 0; i < numSamples; i++)
        {
            outLeft[i] = 0.0f;
            if (outRight) outRight[i] = 0.0f;
        }
        return;
    }
    
    // Get current parameter values (will be smoothed in a real implementation)
    int waveA = static_cast<int>(values[kWaveformA]);
    int waveB = static_cast<int>(values[kWaveformB]);
    
    for (int i = 0; i < numSamples; i++)
    {
        // Update LFO
        lfoPhase += lfoRate / sampleRate;
        if (lfoPhase >= 1.0f)
            lfoPhase -= 1.0f;
        float lfoValue = sinf(lfoPhase * 2.0f * static_cast<float>(M_PI));
        
        // Update filter envelope (simplified)
        if (filterEnvPhase < static_cast<int>(filterAttack * sampleRate))
        {
            filterEnvValue = static_cast<float>(filterEnvPhase) / (filterAttack * sampleRate);
        }
        else
        {
            float decayPos = static_cast<float>(filterEnvPhase - filterAttack * sampleRate) / sampleRate;
            filterEnvValue = 1.0f - decayPos / filterDecay;
            if (filterEnvValue < 0.0f)
                filterEnvValue = 0.0f;
        }
        filterEnvPhase++;
        
        // Calculate modulated filter cutoff
        float modulatedCutoff = filterCutoff * powf(2.0f, filterModulation * filterEnvValue);
        
        // Update amplitude envelope
        if (ampEnvPhase < static_cast<int>(ampAttack * sampleRate))
        {
            ampEnvValue = static_cast<float>(ampEnvPhase) / (ampAttack * sampleRate);
        }
        else
        {
            float decayPos = static_cast<float>(ampEnvPhase - ampAttack * sampleRate) / sampleRate;
            ampEnvValue = 1.0f - decayPos / ampDecay;
            if (ampEnvValue < 0.0f)
                ampEnvValue = 0.0f;
        }
        ampEnvPhase++;
        
        // Simple lowpass filter (placeholder for proper filter implementation)
        float filterCoef = 1.0f - expf(-2.0f * static_cast<float>(M_PI) * modulatedCutoff / sampleRate);
        
        // Generate oscillators
        float sampleA = getWaveformValue(waveA, oscPhaseA) * 0.5f;
        float sampleB = getWaveformValue(waveB, oscPhaseB) * 0.5f;
        float mixed = (sampleA + sampleB) * 0.5f;
        
        // Apply filter (simple one-pole)
        static float filterState = 0.0f;
        mixed = filterState + filterCoef * (mixed - filterState);
        filterState = mixed;
        
        // Apply amplitude envelope and volume
        float finalSample = mixed * ampEnvValue * volume;
        
        // Output
        outLeft[i] = finalSample;
        if (outRight)
            outRight[i] = finalSample;
        
        // Update oscillator phases
        oscPhaseA += freqA / sampleRate;
        oscPhaseB += freqB / sampleRate;
        
        // Keep phases in range
        if (oscPhaseA >= 1.0)
            oscPhaseA -= 1.0;
        if (oscPhaseB >= 1.0)
            oscPhaseB -= 1.0;
    }
}

} // namespace mt
