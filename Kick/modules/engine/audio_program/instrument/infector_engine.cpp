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

// Waveform names for discrete parameter display
static const char* waveformNames[] = {
    "Sine", "Triangle", "SuperSaw", "PWM Sqr", "Dbl Sqr", "Hexagon", "Carrot",
    "Onion", "Tomato", "Cabbage", "Cucumber", "Octave", "1-3", "4-6", "Regs",
    "Saw", "User A", "User AA'", "User B", "User BB'", "User C", "User CC'",
    "User D", "User DD'"
};

static const char* subOscWaveformNames[] = {
    "Sine", "Tri", "Saw", "SuperSaw", "Sqr", "Oct", "1-3", "4-6", "Regs", "Hex",
    "FM", "XT1", "XT2", "User A", "User A'", "User B", "User B'", "User C",
    "User C'", "User D", "User D'"
};

static const char* filterTypeNames[] = {
    "LP 12", "LP 24", "HP 12", "HP 24", "BP 12", "BP 24", "Notch 12", "Notch 24",
    "Peak 12", "Peak 24", "Low Shelf", "High Shelf", "Bandpass", "Comb",
    "Formant", "Allpass", "Reso"
};

static const char* lfoShapeNames[] = {
    "Sine", "Triangle", "Square", "Ramp Up", "Ramp Down", "Random", "Random S&H"
};

static const char* lfoModeNames[] = {
    "Off", "Sync", "Free", "Triggered"
};

InfectorEngine::InfectorEngine()
{
    // Initialize parameters - Volume first, then rest
    params.push_back({ "volume", "Volume", "", 0.0f, 1.0f, 0.7f });

    // OSC1 parameters
    juce::StringArray waveformChoices;
    for (int i = 0; i < 23; i++)
        waveformChoices.add(juce::String(waveformNames[i]));
    
    InstrumentParam waveParamA = { "waveform_a", "OSC1 Waveform", "", 0.0f, 22.0f, 0.0f };
    waveParamA.choices = waveformChoices;
    params.push_back(waveParamA);

    params.push_back({ "pwm_rate_a", "OSC1 PWM Rate", "", 0.0f, 239.0f, 0.0f });
    params.push_back({ "pwm_range_a", "OSC1 PWM Depth", "", 0.0f, 239.0f, 0.0f });
    params.push_back({ "pwm_offset_a", "OSC1 PWM Offset", "%", 0.0f, 240.0f, 120.0f });

    // OSC2 parameters
    InstrumentParam waveParamB = { "waveform_b", "OSC2 Waveform", "", 0.0f, 22.0f, 0.0f };
    waveParamB.choices = waveformChoices;
    params.push_back(waveParamB);

    params.push_back({ "pwm_rate_b", "OSC2 PWM Rate", "", 0.0f, 239.0f, 0.0f });
    params.push_back({ "pwm_range_b", "OSC2 PWM Depth", "", 0.0f, 239.0f, 0.0f });
    params.push_back({ "pwm_offset_b", "OSC2 PWM Offset", "%", 0.0f, 240.0f, 120.0f });

    // Mix parameters
    params.push_back({ "transpose", "OSC2 Transpose", "st", -36.0f, 36.0f, 0.0f });
    params.push_back({ "detune", "OSC Detune", "ct", -1200.0f, 1200.0f, 0.0f });
    params.push_back({ "osc_mix", "OSC Mix", "", 0.0f, 1.0f, 0.5f });

    // Sub oscillator
    juce::StringArray subOscChoices;
    for (int i = 0; i < 21; i++)
        subOscChoices.add(juce::String(subOscWaveformNames[i]));
    
    InstrumentParam subWaveParam = { "sub_osc_wave", "SubOsc Waveform", "", 0.0f, 20.0f, 0.0f };
    subWaveParam.choices = subOscChoices;
    params.push_back(subWaveParam);

    params.push_back({ "sub_osc_vol", "SubOsc Volume", "", 0.0f, 1.0f, 0.0f });
    params.push_back({ "glide", "Glide", "ms", 0.0f, 1000.0f, 0.0f });

    // Filter parameters
    juce::StringArray filterTypeChoices;
    for (int i = 0; i < 18; i++)
        filterTypeChoices.add(juce::String(filterTypeNames[i]));
    
    InstrumentParam filterTypeParam = { "filter_type", "Filter Type", "", 0.0f, 17.0f, 0.0f };
    filterTypeParam.choices = filterTypeChoices;
    params.push_back(filterTypeParam);

    params.push_back({ "filter_cutoff", "Filter Cutoff", "Hz", 20.0f, 20000.0f, 1000.0f });
    params.push_back({ "filter_resonance", "Filter Resonance", "", 0.0f, 1.0f, 0.5f });
    params.push_back({ "filter_modulation", "Filter Modulation", "", 0.0f, 1.0f, 0.0f });
    params.push_back({ "filter_attack", "Filter Attack", "s", 0.001f, 4.0f, 0.1f });
    params.push_back({ "filter_decay", "Filter Decay", "s", 0.001f, 4.0f, 0.5f });
    params.push_back({ "filter_sustain", "Filter Sustain", "", 0.0f, 1.0f, 0.5f });
    params.push_back({ "filter_release", "Filter Release", "s", 0.001f, 4.0f, 0.5f });
    params.push_back({ "filter_shape", "Filter Shape", "", 0.0f, 1.0f, 0.5f });
    params.push_back({ "filter_inertia", "Filter Inertia", "", 0.0f, 1.0f, 0.5f });
    params.push_back({ "filter_track", "Filter Key Tracking", "", 0.0f, 1.0f, 0.5f });

    // LFO1 parameters
    params.push_back({ "lfo_rate", "LFO1 Rate", "Hz", 0.01f, 20.0f, 1.0f });
    params.push_back({ "lfo_amount1", "LFO1 Amount 1", "", -1.0f, 1.0f, 0.0f });
    params.push_back({ "lfo_amount2", "LFO1 Amount 2", "", -1.0f, 1.0f, 0.0f });
    
    juce::StringArray lfoShapeChoices;
    for (int i = 0; i < 7; i++)
        lfoShapeChoices.add(juce::String(lfoShapeNames[i]));
    
    InstrumentParam lfoShapeParam = { "lfo_shape", "LFO1 Shape", "", 0.0f, 6.0f, 0.0f };
    lfoShapeParam.choices = lfoShapeChoices;
    params.push_back(lfoShapeParam);

    // LFO2 parameters
    params.push_back({ "lfo2_rate", "LFO2 Rate", "Hz", 0.01f, 20.0f, 1.0f });
    params.push_back({ "lfo2_amount1", "LFO2 Amount 1", "", -1.0f, 1.0f, 0.0f });
    params.push_back({ "lfo2_amount2", "LFO2 Amount 2", "", -1.0f, 1.0f, 0.0f });
    
    InstrumentParam lfo2ShapeParam = { "lfo2_shape", "LFO2 Shape", "", 0.0f, 6.0f, 0.0f };
    lfo2ShapeParam.choices = lfoShapeChoices;
    params.push_back(lfo2ShapeParam);

    // Amp envelope parameters
    params.push_back({ "amp_attack", "Amp Attack", "s", 0.001f, 4.0f, 0.01f });
    params.push_back({ "amp_decay", "Amp Decay", "s", 0.001f, 4.0f, 0.5f });
    params.push_back({ "amp_sustain", "Amp Sustain", "", 0.0f, 1.0f, 0.5f });
    params.push_back({ "amp_release", "Amp Release", "s", 0.001f, 4.0f, 0.5f });

    // LFO mode
    juce::StringArray lfoModeChoices;
    for (int i = 0; i < 4; i++)
        lfoModeChoices.add(juce::String(lfoModeNames[i]));
    
    InstrumentParam lfoModeParam = { "lfo_mode", "LFO Mode", "", 0.0f, 3.0f, 0.0f };
    lfoModeParam.choices = lfoModeChoices;
    params.push_back(lfoModeParam);

    params.push_back({ "lfo_phase", "LFO Phase", "", 0.0f, 1.0f, 0.0f });

    // Note parameters (typically not exposed in instrument engines)
    params.push_back({ "note", "Note", "", 0.0f, 127.0f, 60.0f });
    params.push_back({ "velocity", "Velocity", "", 0.0f, 1.0f, 0.7f });
    params.push_back({ "length", "Length", "", 0.0f, 1.0f, 0.5f });
    params.push_back({ "command1", "Command 1", "", 0.0f, 255.0f, 0.0f });
    params.push_back({ "argument1", "Argument 1", "", 0.0f, 255.0f, 0.0f });
    params.push_back({ "command2", "Command 2", "", 0.0f, 255.0f, 0.0f });
    params.push_back({ "argument2", "Argument 2", "", 0.0f, 255.0f, 0.0f });
    params.push_back({ "vibrato2", "Vibrato 2", "", 0.0f, 1.0f, 0.0f });

    // Initialize all values
    for (int i = 0; i < kNumParams; i++)
        values[i] = params[i].defaultValue;

    // Initialize state
    sampleRate = 44100.0;
    noteActive = false;
    currentNote = 60;
    noteVelocity = 1.0f;

    oscPhaseA = 0.0f;
    oscPhaseB = 0.0f;
    oscPhaseSub = 0.0f;
    freqA = 0.0f;
    freqB = 0.0f;
    freqSub = 0.0f;
    targetFreqA = 0.0f;
    targetFreqB = 0.0f;

    filterCutoff = 1000.0f;
    filterResonance = 0.5f;
    filterModulation = 0.0f;
    filterAttack = 0.1f;
    filterDecay = 0.5f;
    filterSustain = 0.5f;
    filterRelease = 0.5f;
    filterShape = 0.5f;
    filterInertia = 0.5f;
    filterTrack = 0.5f;
    filterEnvPhase = 0;
    filterEnvValue = 0.0f;

    lfoPhase = 0.0f;
    lfoRate = 1.0f;
    lfoAmount1 = 0.0f;
    lfoAmount2 = 0.0f;
    lfoShape = 0.0f;
    lfo2Phase = 0.0f;
    lfo2Rate = 1.0f;
    lfo2Amount1 = 0.0f;
    lfo2Amount2 = 0.0f;
    lfo2Shape = 0.0f;
    lfoMode = 0;

    ampEnvPhase = 0;
    ampEnvValue = 0.0f;
    ampAttack = 0.01f;
    ampDecay = 0.5f;
    ampSustain = 0.5f;
    ampRelease = 0.5f;

    glide = 0.0f;
    volume = 0.7f;
    transpose = 0.0f;
    detune = 0.0f;
    oscMix = 0.5f;
    subOscWave = 0;
    subOscVolume = 0.0f;
    waveformA = 0;
    waveformB = 0;

    pwmPhaseA = 0.0f;
    pwmPhaseB = 0.0f;

    // Update parameters
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
    filterEnvPhase = 0;
    filterEnvValue = 0.0f;
    lfoPhase = 0.0f;
    lfo2Phase = 0.0f;
    ampEnvPhase = 0;
    ampEnvValue = 0.0f;
    pwmPhaseA = 0.0f;
    pwmPhaseB = 0.0f;
}

void InfectorEngine::noteOn(int note, float velocity)
{
    noteActive = true;
    currentNote = note;
    noteVelocity = velocity;

    // Calculate frequencies
    float baseFreq = noteToFreq(note);
    freqA = baseFreq;
    freqB = baseFreq * std::pow(2.0f, transpose / 12.0f);  // Apply transpose
    
    // Apply detune to OSC2
    freqB *= std::pow(2.0f, detune / 1200.0f);  // detune is in cents
    
    // Sub oscillator (one octave down)
    freqSub = baseFreq * 0.5f;

    // Reset envelopes
    filterEnvPhase = 0;
    filterEnvValue = 0.0f;
    ampEnvPhase = 0;
    ampEnvValue = 0.0f;

    // Reset LFOs
    lfoPhase = 0.0f;
    lfo2Phase = 0.0f;

    // Reset PWM
    pwmPhaseA = 0.0f;
    pwmPhaseB = 0.0f;
}

void InfectorEngine::noteOff(int note)
{
    if (currentNote == note)
    {
        noteActive = false;
        // Trigger release stage
        ampEnvPhase = 0;  // Will be handled in process
    }
}

void InfectorEngine::allNotesOff()
{
    noteActive = false;
    ampEnvPhase = 0;
    filterEnvPhase = 0;
}

void InfectorEngine::setParam(int index, float value)
{
    if (index >= 0 && index < kNumParams)
    {
        values[index] = value;
        updateParameters();
    }
}

void InfectorEngine::updateParameters()
{
    // Volume
    volume = values[kVolume];

    // OSC1
    waveformA = static_cast<int>(values[kWaveformA]);
    waveformA = std::max(0, std::min(22, waveformA));

    // OSC2
    waveformB = static_cast<int>(values[kWaveformB]);
    waveformB = std::max(0, std::min(22, waveformB));

    transpose = values[kTranspose];
    detune = values[kDetune];
    oscMix = values[kOscMix];

    // Sub oscillator
    subOscWave = static_cast<int>(values[kSubOscWave]);
    subOscWave = std::max(0, std::min(20, subOscWave));
    subOscVolume = values[kSubOscVolume];
    glide = values[kGlide];

    // Filter
    filterCutoff = 20.0f + (values[kFilterCutoff] / 240.0f) * (20000.0f - 20.0f);
    filterResonance = values[kFilterResonance];
    filterModulation = values[kFilterModulation];
    filterAttack = 0.001f + (values[kFilterAttack] / 240.0f) * (4.0f - 0.001f);
    filterDecay = 0.001f + (values[kFilterDecay] / 240.0f) * (4.0f - 0.001f);
    filterSustain = values[kFilterSustain];
    filterRelease = 0.001f + (values[kFilterRelease] / 240.0f) * (4.0f - 0.001f);
    filterShape = values[kFilterShape];
    filterInertia = values[kFilterInertia];
    filterTrack = values[kFilterTrack];

    // LFO1
    lfoRate = 0.01f + (values[kLFORate] / 240.0f) * (20.0f - 0.01f);
    lfoAmount1 = (values[kLFOAmount1] / 120.0f) - 1.0f;  // -1 to 1
    lfoAmount2 = (values[kLFOAmount2] / 120.0f) - 1.0f;

    // LFO2
    lfo2Rate = 0.01f + (values[kLFO2Rate] / 240.0f) * (20.0f - 0.01f);
    lfo2Amount1 = (values[kLFO2Amount1] / 120.0f) - 1.0f;
    lfo2Amount2 = (values[kLFO2Amount2] / 120.0f) - 1.0f;

    // Amp envelope
    ampAttack = 0.001f + (values[kAmpAttack] / 240.0f) * (4.0f - 0.001f);
    ampDecay = 0.001f + (values[kAmpDecay] / 240.0f) * (4.0f - 0.001f);
    ampSustain = values[kAmpSustain];
    ampRelease = 0.001f + (values[kAmpRelease] / 240.0f) * (4.0f - 0.001f);

    // LFO mode
    lfoMode = static_cast<int>(values[kLFOMode]);
    lfoMode = std::max(0, std::min(3, lfoMode));
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

    // Calculate current frequencies (with glide)
    float baseFreq = noteToFreq(currentNote);
    float targetFreqA_new = baseFreq;
    float targetFreqB_new = baseFreq * std::pow(2.0f, transpose / 12.0f);
    targetFreqB_new *= std::pow(2.0f, detune / 1200.0f);

    // Simple glide (linear interpolation)
    float glideAmount = std::exp(-glide * 0.001f * static_cast<float>(sampleRate) / numSamples);
    if (glide > 0.0f)
    {
        targetFreqA = targetFreqA_new;
        targetFreqB = targetFreqB_new;
    }
    else
    {
        freqA = targetFreqA_new;
        freqB = targetFreqB_new;
    }

    // Sub oscillator
    freqSub = baseFreq * 0.5f;

    for (int i = 0; i < numSamples; i++)
    {
        // Update LFO1
        lfoPhase += lfoRate / static_cast<float>(sampleRate);
        if (lfoPhase >= 1.0f)
            lfoPhase -= 1.0f;
        float lfoValue = std::sin(lfoPhase * 2.0f * static_cast<float>(M_PI));

        // Update LFO2
        lfo2Phase += lfo2Rate / static_cast<float>(sampleRate);
        if (lfo2Phase >= 1.0f)
            lfo2Phase -= 1.0f;
        float lfo2Value = std::sin(lfo2Phase * 2.0f * static_cast<float>(M_PI));

        // Update filter envelope (simplified AHDSR)
        if (filterEnvPhase < static_cast<int>(filterAttack * sampleRate))
        {
            filterEnvValue = static_cast<float>(filterEnvPhase) / (filterAttack * sampleRate);
        }
        else if (filterEnvPhase < static_cast<int>((filterAttack + filterDecay) * sampleRate))
        {
            float decayPos = static_cast<float>(filterEnvPhase - filterAttack * sampleRate) / sampleRate;
            filterEnvValue = 1.0f - decayPos / filterDecay;
        }
        else
        {
            filterEnvValue = filterSustain;
        }
        filterEnvPhase++;
        if (filterEnvPhase >= static_cast<int>((filterAttack + filterDecay + filterRelease) * sampleRate))
            filterEnvPhase = 0;

        // Calculate modulated filter cutoff
        float modulatedCutoff = filterCutoff * std::pow(2.0f, filterModulation * filterEnvValue);

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

        // Update PWM phases
        pwmPhaseA += lfoRate / static_cast<float>(sampleRate);
        pwmPhaseB += lfoRate / static_cast<float>(sampleRate);

        // Generate waveforms
        float sampleA = getWaveformValue(waveformA, oscPhaseA);
        float sampleB = getWaveformValue(waveformB, oscPhaseB);
        float sampleSub = getWaveformValue(subOscWave, oscPhaseSub) * subOscVolume;

        // Mix oscillators
        float mixed = (sampleA * (1.0f - oscMix) + sampleB * oscMix) * 0.5f + sampleSub * 0.5f;

        // Simple lowpass filter
        static float filterState = 0.0f;
        float filterCoef = 1.0f - std::exp(-2.0f * static_cast<float>(M_PI) * modulatedCutoff / static_cast<float>(sampleRate));
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
