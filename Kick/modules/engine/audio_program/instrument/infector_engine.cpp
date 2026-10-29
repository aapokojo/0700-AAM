/*
 * FSM Infector Instrument Engine Implementation
 * 
 * Based on FSM Infector Buzz machine by Krzysztof Foltman <kfoltman@users.sourceforge.net>
 * Original code: buzzmachines/FSM/Infector/Infector.cpp
 * Original license: LGPL 2.1 (see buzzmachines/FSM/COPYING)
 * 
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "infector_engine.h"
#include <cmath>
#include <algorithm>

namespace mt
{

InfectorEngine::InfectorEngine()
    : sampleRate(44100.0)
    , noteActive(false)
    , currentNote(60)
    , noteVelocity(1.0f)
    , oscPhaseA(0.0f)
    , oscPhaseB(0.0f)
    , oscPhaseSub(0.0f)
    , freqA(0.0f)
    , freqB(0.0f)
    , freqSub(0.0f)
    , filterCutoff(1000.0f)
    , filterResonance(0.5f)
    , ampEnvPhase(0)
    , ampEnvValue(0.0f)
    , ampAttack(0.01f)
    , ampDecay(0.5f)
    , ampSustain(0.5f)
    , ampRelease(0.5f)
    , volume(0.7f)
    , oscMix(0.5f)
    , subOscWave(0)
    , subOscVolume(0.0f)
    , waveformA(0)
    , waveformB(0)
{
    // Simple parameter initialization without StringArray to avoid crash
    params.push_back({ "volume", "Volume", "", 0.0f, 1.0f, 0.7f });
    params.push_back({ "waveform_a", "OSC1 Waveform", "", 0.0f, 22.0f, 0.0f });
    params.push_back({ "waveform_b", "OSC2 Waveform", "", 0.0f, 22.0f, 0.0f });
    params.push_back({ "osc_mix", "OSC Mix", "", 0.0f, 1.0f, 0.5f });
    params.push_back({ "sub_osc_wave", "SubOsc Waveform", "", 0.0f, 20.0f, 0.0f });
    params.push_back({ "sub_osc_vol", "SubOsc Volume", "", 0.0f, 1.0f, 0.0f });
    params.push_back({ "filter_type", "Filter Type", "", 0.0f, 17.0f, 0.0f });
    params.push_back({ "filter_cutoff", "Filter Cutoff", "Hz", 20.0f, 20000.0f, 1000.0f });
    params.push_back({ "filter_resonance", "Filter Resonance", "", 0.0f, 1.0f, 0.5f });
    params.push_back({ "amp_attack", "Amp Attack", "s", 0.001f, 4.0f, 0.01f });
    params.push_back({ "amp_decay", "Amp Decay", "s", 0.001f, 4.0f, 0.5f });
    params.push_back({ "amp_sustain", "Amp Sustain", "", 0.0f, 1.0f, 0.5f });
    params.push_back({ "amp_release", "Amp Release", "s", 0.001f, 4.0f, 0.5f });

    // Initialize all values
    for (int i = 0; i < kNumParams; i++)
        values[i] = params[i].defaultValue;
    
    // Update derived parameters
    updateParameters();
}

void InfectorEngine::prepare(double newSampleRate, int maxBlockSize)
{
    sampleRate = newSampleRate;
}

void InfectorEngine::reset()
{
    noteActive = false;
    oscPhaseA = 0.0f;
    oscPhaseB = 0.0f;
    oscPhaseSub = 0.0f;
    ampEnvPhase = 0;
    ampEnvValue = 0.0f;
}

void InfectorEngine::noteOn(int note, float velocity)
{
    noteActive = true;
    currentNote = note;
    noteVelocity = velocity;

    // Calculate frequencies
    float baseFreq = noteToFreq(note);
    freqA = baseFreq;
    freqB = baseFreq;
    freqSub = baseFreq * 0.5f;

    // Reset envelopes
    ampEnvPhase = 0;
    ampEnvValue = 0.0f;
}

void InfectorEngine::noteOff(int note)
{
    if (currentNote == note)
        noteActive = false;
}

void InfectorEngine::allNotesOff()
{
    noteActive = false;
    ampEnvPhase = 0;
}

void InfectorEngine::updateParameters()
{
    // Volume
    volume = values[kVolume];

    // OSC waveforms
    waveformA = static_cast<int>(values[kWaveformA]);
    waveformA = std::max(0, std::min(22, waveformA));
    waveformB = static_cast<int>(values[kWaveformB]);
    waveformB = std::max(0, std::min(22, waveformB));

    oscMix = values[kOscMix];

    // Sub oscillator
    subOscWave = static_cast<int>(values[kSubOscWave]);
    subOscWave = std::max(0, std::min(20, subOscWave));
    subOscVolume = values[kSubOscVolume];

    // Filter
    filterCutoff = 20.0f + (values[kFilterCutoff] / 240.0f) * (20000.0f - 20.0f);
    filterResonance = values[kFilterResonance];

    // Amp envelope
    ampAttack = 0.001f + (values[kAmpAttack] / 240.0f) * (4.0f - 0.001f);
    ampDecay = 0.001f + (values[kAmpDecay] / 240.0f) * (4.0f - 0.001f);
    ampSustain = values[kAmpSustain];
    ampRelease = 0.001f + (values[kAmpRelease] / 240.0f) * (4.0f - 0.001f);
}

float InfectorEngine::noteToFreq(int note)
{
    return 440.0f * std::pow(2.0f, (note - 69.0f) / 12.0f);
}

// Simple waveform generators for different waveforms
float InfectorEngine::getWaveformValue(int waveform, float phase)
{
    // Normalize phase to [0, 1)
    phase = std::fmod(phase, 1.0f);
    if (phase < 0.0f)
        phase += 1.0f;

    float value = 0.0f;
    float t = phase * 2.0f * static_cast<float>(M_PI);

    switch (waveform)
    {
        case 0:  // Sine
            value = std::sin(t);
            break;
        case 1:  // Triangle
            value = 2.0f * std::fabs(phase - 0.5f) - 1.0f;
            break;
        case 2:  // SuperSaw
            value = std::sin(t) + std::sin(t * 1.01f) * 0.5f + std::sin(t * 1.02f) * 0.25f;
            value *= 0.5f;
            break;
        case 3:  // PWM Sqr
            value = (phase < 0.5f) ? 1.0f : -1.0f;
            break;
        case 4:  // Dbl Sqr
            value = std::sin(t) + std::sin(t * 2.0f) * 0.5f;
            break;
        case 5:  // Hexagon
            value = std::sin(t) + std::sin(t * 2.0f) * 0.3f + std::sin(t * 3.0f) * 0.2f;
            break;
        case 15: // Saw
            value = 2.0f * (phase - 0.5f);
            break;
        default: // Fallback to sine for others
            value = std::sin(t);
            break;
    }

    return value * 0.7f; // Scale down to prevent clipping
}

void InfectorEngine::setParam(int index, float value)
{
    if (index >= 0 && index < kNumParams)
    {
        values[index] = value;
        updateParameters();
    }
}

void InfectorEngine::process(float* const* out, int numChannels, int numSamples)
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

    // Calculate current frequencies
    float baseFreq = noteToFreq(currentNote);
    freqA = baseFreq;
    freqB = baseFreq;
    freqSub = baseFreq * 0.5f;

    for (int i = 0; i < numSamples; i++)
    {
        // Update amplitude envelope (AHDSR)
        if (ampEnvPhase < static_cast<int>(ampAttack * sampleRate))
        {
            ampEnvValue = static_cast<float>(ampEnvPhase) / (ampAttack * sampleRate);
        }
        else if (ampEnvPhase < static_cast<int>((ampAttack + ampDecay) * sampleRate))
        {
            float decayPos = static_cast<float>(ampEnvPhase - ampAttack * sampleRate) / sampleRate;
            ampEnvValue = 1.0f - decayPos / ampDecay;
        }
        else
        {
            ampEnvValue = ampSustain;
        }
        ampEnvPhase++;
        if (ampEnvPhase >= static_cast<int>((ampAttack + ampDecay + ampRelease) * sampleRate))
            ampEnvPhase = 0;

        // Generate waveforms
        float sampleA = getWaveformValue(waveformA, oscPhaseA);
        float sampleB = getWaveformValue(waveformB, oscPhaseB);
        float sampleSub = getWaveformValue(subOscWave, oscPhaseSub) * subOscVolume;

        // Mix oscillators
        float mixed = (sampleA * (1.0f - oscMix) + sampleB * oscMix) * 0.5f + sampleSub * 0.5f;

        // Simple lowpass filter
        static float filterState = 0.0f;
        float filterCoef = 1.0f - std::exp(-2.0f * static_cast<float>(M_PI) * filterCutoff / static_cast<float>(sampleRate));
        mixed = filterState + filterCoef * (mixed - filterState);
        filterState = mixed;

        // Apply amplitude envelope and volume
        float finalSample = mixed * ampEnvValue * volume * noteVelocity;

        // Output
        outLeft[i] = finalSample;
        if (outRight)
            outRight[i] = finalSample;

        // Update oscillator phases
        oscPhaseA += freqA / static_cast<float>(sampleRate);
        oscPhaseB += freqB / static_cast<float>(sampleRate);
        oscPhaseSub += freqSub / static_cast<float>(sampleRate);

        // Keep phases in range
        if (oscPhaseA >= 1.0f)
            oscPhaseA -= 1.0f;
        if (oscPhaseB >= 1.0f)
            oscPhaseB -= 1.0f;
        if (oscPhaseSub >= 1.0f)
            oscPhaseSub -= 1.0f;
    }
}

} // namespace mt
