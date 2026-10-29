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
        kWaveformA,
        kWaveformB,
        kOscMix,
        kSubOscWave,
        kSubOscVolume,
        kFilterType,
        kFilterCutoff,
        kFilterResonance,
        kAmpAttack,
        kAmpDecay,
        kAmpSustain,
        kAmpRelease,
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
    void initParams();
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

    // Amp envelope state
    float ampEnvPhase;
    float ampEnvValue;
    float ampAttack;
    float ampDecay;
    float ampSustain;
    float ampRelease;

    // Volume and mix
    float volume;
    float oscMix;
    int subOscWave;
    float subOscVolume;
    int waveformA;
    int waveformB;
};

} // namespace mt
