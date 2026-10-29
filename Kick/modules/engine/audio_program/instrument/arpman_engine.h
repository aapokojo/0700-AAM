/*
 * FSM ArpMan Instrument Engine
 * 
 * Based on FSM ArpMan Buzz machine by Krzysztof Foltman <kfoltman@users.sourceforge.net>
 * Original code: buzzmachines/FSM/ArpMan/ArpMan.cpp
 * Original license: LGPL 2.1 (see buzzmachines/FSM/COPYING)
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#pragma once

#include "mt_instrument_engine.h"

namespace mt
{
    /**
     * FSM ArpMan synthesis engine - Arpeggiator with dual oscillators and filter
     * based on the FSM ArpMan Buzz machine.
     * 
     * This implements an arpeggiator with:
     * - Dual oscillators with selectable waveforms
     * - Transpose and detune for OSC2
     * - Glide (portamento)
     * - Multi-mode filter with cutoff, resonance, and envelope
     * - LFO for filter modulation
     * - Amplitude envelope
     * - Arpeggiator with multiple modes and timings
     */
    class ArpManEngine : public IInstrumentEngine
    {
    public:
        enum Param
        {
            kVolume,          // Volume (added - not in original)
            kWaveformA,       // OSC1 Waveform (0-13)
            kWaveformB,       // OSC2 Waveform (0-13)
            kTranspose,       // OSC2 Transpose (0-72)
            kDetune,          // OSC Detune (0-240)
            kGlide,           // Glide (0-240)
            kFilterType,      // Filter Type (0-6)
            kFilterCutoff,    // Filter Cutoff (0-240)
            kFilterResonance, // Filter Resonance (0-240)
            kFilterModulation,// Filter Modulation (0-240)
            kFilterAttack,    // Filter Attack (0-240)
            kFilterDecay,     // Filter Decay (0-240)
            kFilterShape,     // Filter Modulation Shape (0-240)
            kLFORate,         // LFO Rate (0-240)
            kLFOAmount1,      // LFO Amount 1 (0-240)
            kLFOAmount2,      // LFO Amount 2 (0-240)
            kLFOShape,        // LFO Shape (0-240)
            kAmpAttack,       // Amp Attack (0-240)
            kAmpDecay,        // Amp Decay (0-240)
            kArpType,         // Arpeggiator Type (0-?)
            kArpTiming,       // Arpeggiator Timing (0-?)
            kPolyphony,       // Polyphony (0-?)
            kFilterInertia,   // Filter Inertia (0-240)
            kLFOPhase,        // LFO Phase (0-240)
            kArpPhase,        // Arpeggiator Phase (0-240)
            kNote,            // Note (0-?)
            kAccent,         // Accent (0-?)
            kLength,         // Length (0-?)
            kNumParams
        };

        ArpManEngine();

        void prepare(double sampleRate, int maxBlockSize) override;
        void reset() override;
        void noteOn(int note, float velocity) override;
        void noteOff(int note) override;
        void allNotesOff() override;
        void process(float* const* out, int numChannels, int numSamples) override;

        const std::vector<InstrumentParam>& getParams() const override { return params; }
        void setParam(int index, float value) override;

    private:
        std::vector<InstrumentParam> params;
        float values[kNumParams];

        // Synthesis state
        double sampleRate;
        float volume;             // Volume control
        
        // Oscillator state
        double oscPhaseA, oscPhaseB;
        double freqA, freqB;
        float transpose;  // in semitones
        float detune;     // in cents
        float glide;      // portamento time
        
        // Filter state
        int filterType;
        float filterCutoff;
        float filterResonance;
        float filterModulation;
        float filterAttack;
        float filterDecay;
        float filterShape;
        float filterInertia;
        
        // Filter envelope state
        float filterEnvPhase;
        float filterEnvValue;
        
        // LFO state
        float lfoPhase;
        float lfoRate;
        float lfoAmount1, lfoAmount2;
        float lfoShape;
        
        // Amp envelope state
        float ampEnvPhase;
        float ampEnvValue;
        float ampAttack;
        float ampDecay;
        
        // Arpeggiator state
        int arpType;
        int arpTiming;
        int arpPhase;
        int arpNote;
        int arpAccent;
        int arpLength;
        int arpPatternPos;
        
        // Polyphony
        int polyphony;
        
        // Note state
        bool noteActive;
        int currentNote;
        float noteVelocity;
        int noteLength;
        int noteAccent;
        
        // Wavetable data (will be generated on init)
        float waveTables[14][4096];  // 14 waveforms, 4096 samples each
        
        // Parameter smoothing
        void updateParameters();
        void generateWavetables();
        
        // Helper functions
        float getWaveformValue(int waveform, double phase);
        float noteToFreq(int note);
        void triggerNote(int note, float velocity);
        void updateArpeggiator();
    };

} // namespace mt
