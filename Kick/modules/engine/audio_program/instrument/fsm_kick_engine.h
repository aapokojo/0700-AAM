/*
 * FSM Kick Instrument Engine
 * 
 * Based on FSM Kick Buzz machine by Krzysztof Foltman.
 * Original code licensed under LGPL 2.1.
 * See: https://github.com/Buzztrax/buzzmachines/tree/master/FSM/Kick
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#pragma once

namespace mt
{
    /**
     * FSM Kick synthesis engine - a simple but powerful kick drum synthesizer
     * based on the classic FSM (Finite State Machine) kick from Buzz machines.
     * 
     * This implements a single-voice kick drum with:
     * - Start frequency (pitch of the initial click)
     * - End frequency (pitch the kick decays to)
     * - Tone decay (how fast the pitch bends down)
     * - Tone shape (curve of the pitch bend)
     * - Amp decay (envelope decay time)
     * - Volume (output level)
     */
    class FSMKickEngine : public IInstrumentEngine
    {
    public:
        enum Param
        {
            kStartFreq,      // Starting frequency of the kick (Hz)
            kEndFreq,        // Frequency the kick decays to (Hz)
            kToneDecay,      // How fast the pitch decays (0-1)
            kToneShape,      // Shape of the pitch decay curve (0-1)
            kAmpDecay,       // How fast the amplitude decays (0-1)
            kVolume,         // Output volume (0-1)
            kNumParams
        };

        FSMKickEngine();

        void prepare( double SampleRate, int MaxBlockSize ) override;
        void reset() override;
        void noteOn( int Note, float Velocity ) override;
        void noteOff( int Note ) override;
        void allNotesOff() override;
        void process( float* const* Out, int NumChannels, int NumSamples ) override;

        const std::vector<InstrumentParam>& getParams() const override { return params; }
        void setParam( int Index, float Value ) override;

    private:
        std::vector<InstrumentParam> params;
        float values[ kNumParams ];
        
        // Audio processing state
        double sampleRate;
        float currentPhase;
        float currentFreq;
        float targetFreq;
        float freqDecay;
        float ampDecay;
        float envLevel;
        float envDecayCoef;
        float volume;
        
        // Voice state
        bool noteActive;
        int currentNote;
        float noteVelocity;
        
        // Parameter smoothing
        float smoothedStartFreq;
        float smoothedEndFreq;
        float smoothedToneDecay;
        float smoothedToneShape;
        float smoothedAmpDecay;
        float smoothedVolume;
        
        // Note-to-frequency conversion
        float noteToFreq( int note );
    };

} // namespace mt
