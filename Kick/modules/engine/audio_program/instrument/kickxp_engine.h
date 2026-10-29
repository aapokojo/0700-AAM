/*
 * FSM KickXP Instrument Engine
 * 
 * Based on FSM KickXP Buzz machine by Krzysztof Foltman <kfoltman@users.sourceforge.net>
 * Original code: buzzmachines/FSM/KickXP/KickXP.cpp
 * Original license: LGPL 2.1 (see buzzmachines/FSM/COPYING)
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#pragma once

#include "mt_instrument_engine.h"

namespace mt
{
    /**
     * FSM KickXP synthesis engine - Extended kick drum synthesizer
     * based on the FSM KickXP Buzz machine.
     * 
     * This implements a kick drum with:
     * - Start and end frequency with pitch sweep
     * - Buzz, click, and punch components
     * - Tone, buzz, click+punch decay with shape controls
     * - Amplitude decay with slope and time
     * - Release slope
     * - Note delay/retrigger
     */
    class KickXPEngine : public IInstrumentEngine
    {
    public:
        enum Param
        {
            kVolume,          // Volume (added - not in original)
            kPitchLimit,      // Lower pitch limit (note)
            kStartFreq,       // Start frequency
            kEndFreq,         // End frequency
            kBuzzAmt,         // Amount of buzz
            kClickAmt,        // Amount of click
            kPunchAmt,        // Amount of punch
            kToneDecay,       // Tone decay rate
            kToneShape,       // Tone decay shape
            kBDecay,          // Buzz decay rate
            kCDecay,          // Click+punch decay rate
            kDecSlope,        // Amplitude decay slope
            kDecTime,         // Amplitude decay time
            kRelSlope,        // Amplitude release slope
            kNoteDelay,       // Note delay/retrigger
            kNumParams
        };

        KickXPEngine();

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
        
        // Per-voice state (we support one voice for simplicity)
        double oscPhase;           // Oscillator phase
        double xSin, xCos;         // Sine/cosine state
        double dxSin, dxCos;       // Phase increments
        
        // Envelope and amplitude state
        float amp;                // Current amplitude
        float decAmp;             // Amplitude decay rate
        float bAmp;               // Buzz amplitude
        float mulBAmp;            // Buzz amplitude multiplier
        float cAmp;               // Click amplitude
        float mulCAmp;            // Click amplitude multiplier
        float frequency;          // Current frequency
        
        // Parameters (smoothed)
        float volume;            // Volume control
        float pitchLimit;         // Lower pitch limit
        float startFreq;          // Start frequency
        float endFreq;            // End frequency
        float buzzAmt;            // Buzz amount
        float clickAmt;           // Click amount
        float punchAmt;           // Punch amount
        float toneDecay;          // Tone decay rate
        float toneShape;          // Tone decay shape
        float bDecay;             // Buzz decay rate
        float cDecay;             // Click+punch decay rate
        float decSlope;           // Amplitude decay slope
        float decTime;            // Amplitude decay time
        float relSlope;           // Amplitude release slope
        float noteDelay;          // Note delay/retrigger
        
        // Envelope tracking
        int envPhase;             // Envelope phase counter
        int samplesToGo;          // Samples until retrigger
        int retrig;               // Retrigger count
        int retrigCount;          // Retrigger counter
        int age;                  // Sample age for click
        int leftOver;             // Samples left in current block
        
        // Anti-click and last value
        float antiClick;
        float lastValue;
        
        // Thump data (click sample)
        float thumpData[1024];
        
        // Voice state
        bool noteActive;
        int currentNote;
        float noteVelocity;
        
        // Parameter smoothing
        void updateParameters();
        void trigger();
        
        // Helper function to generate thump data
        void generateThumpData();
        
        // Note to frequency conversion
        float noteToFreq(int note);
    };

} // namespace mt
