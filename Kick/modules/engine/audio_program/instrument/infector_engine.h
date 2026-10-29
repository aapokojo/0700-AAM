/*
 * FSM Infector Instrument Engine
 * 
 * Based on FSM Infector Buzz machine by Krzysztof Foltman <kfoltman@users.sourceforge.net>
 * Original code: buzzmachines/FSM/Infector/Infector.cpp
 * Original license: LGPL 2.1 (see buzzmachines/FSM/COPYING)
 * 
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#pragma once

#include "mt_instrument_engine.h"

namespace mt
{

class InfectorEngine : public IInstrumentEngine
{
public:
    enum Param
    {
        kVolume,
        // OSC1
        kWaveformA,
        kPWMRateA,
        kPWMRangeA,
        kPWOffsetA,
        // OSC2
        kWaveformB,
        kPWMRateB,
        kPWMRangeB,
        kPWOffsetB,
        // Mix
        kTranspose,
        kDetune,
        kOscMix,
        kSubOscWave,
        kSubOscVolume,
        kGlide,
        // Filter
        kFilterType,
        kFilterCutoff,
        kFilterResonance,
        kFilterModulation,
        kFilterAttack,
        kFilterDecay,
        kFilterSustain,
        kFilterRelease,
        kFilterShape,
        kFilterInertia,
        kFilterTrack,
        // LFO1
        kLFORate,
        kLFOAmount1,
        kLFOAmount2,
        kLFOShape,
        // LFO2
        kLFO2Rate,
        kLFO2Amount1,
        kLFO2Amount2,
        kLFO2Shape,
        // Amp Envelope
        kAmpAttack,
        kAmpDecay,
        kAmpSustain,
        kAmpRelease,
        // LFO Mode
        kLFOMode,
        kLFOPhase,
        // Note parameters
        kNote,
        kVelocity,
        kLength,
        kCommand1,
        kArgument1,
        kCommand2,
        kArgument2,
        kVibrato2,
        kNumParams
    };

    InfectorEngine();

    void prepare(double sampleRate, int maxBlockSize) override;
    void reset() override;
    void noteOn(int note, float velocity) override;
    void noteOff(int note) override;
    void allNotesOff() override;
    void process(float* const* out, int numChannels, int numSamples) override;

    const std::vector<InstrumentParam>& getParams() const override { return params; }
    void setParam(int index, float value) override;

private:
    void updateParameters();
    float noteToFreq(int note);
    float getWaveformValue(int waveform, float phase);

    std::vector<InstrumentParam> params;
    float values[kNumParams];
    double sampleRate;
    bool noteActive;
    int currentNote;
    float noteVelocity;

    // Oscillator state
    float oscPhaseA;
    float oscPhaseB;
    float oscPhaseSub;
    float freqA;
    float freqB;
    float freqSub;

    // Filter state
    float filterCutoff;
    float filterResonance;
    float filterModulation;
    float filterAttack;
    float filterDecay;
    float filterSustain;
    float filterRelease;
    float filterShape;
    float filterInertia;
    float filterTrack;
    float filterEnvPhase;
    float filterEnvValue;

    // LFO state
    float lfoPhase;
    float lfoRate;
    float lfoAmount1;
    float lfoAmount2;
    float lfoShape;
    float lfo2Phase;
    float lfo2Rate;
    float lfo2Amount1;
    float lfo2Amount2;
    float lfo2Shape;
    int lfoMode;

    // Amp envelope state
    float ampEnvPhase;
    float ampEnvValue;
    float ampAttack;
    float ampDecay;
    float ampSustain;
    float ampRelease;

    // Glide
    float glide;
    float targetFreqA;
    float targetFreqB;

    // PWM state
    float pwmPhaseA;
    float pwmPhaseB;

    // Volume
    float volume;
    float transpose;
    float detune;
    float oscMix;
    int subOscWave;
    float subOscVolume;
    int waveformA;
    int waveformB;
};

} // namespace mt
