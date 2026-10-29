/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "fsm_kick_engine.h"
#include <cmath>

namespace mt
{
    FSMKickEngine::FSMKickEngine()
    {
        params = {
            { "startFreq", "Start Freq", "Hz", 20.f, 500.f, 150.f },
            { "endFreq", "End Freq", "Hz", 20.f, 500.f, 50.f },
            { "toneDecay", "Tone Decay", "s", 0.001f, 2.f, 0.5f },
            { "toneShape", "Tone Shape", "", 0.f, 1.f, 0.5f },
            { "ampDecay", "Amp Decay", "s", 0.001f, 4.f, 0.5f },
            { "volume", "Volume", "", 0.f, 1.f, 0.8f },
        };
        jassert( params.size() == kNumParams );
        for( int i = 0; i < kNumParams; ++i )
            values[i] = params[(size_t)i].defaultValue;
            
        sampleRate = 44100.0;
        currentPhase = 0.f;
        currentFreq = 0.f;
        targetFreq = 0.f;
        freqDecay = 0.f;
        ampDecay = 0.f;
        envLevel = 0.f;
        envDecayCoef = 0.f;
        volume = 0.f;
        noteActive = false;
        currentNote = 60;
        noteVelocity = 1.f;
        
        // Initialize smoothed values
        smoothedStartFreq = 150.f;
        smoothedEndFreq = 50.f;
        smoothedToneDecay = 0.5f;
        smoothedToneShape = 0.5f;
        smoothedAmpDecay = 0.5f;
        smoothedVolume = 0.8f;
    }

    void FSMKickEngine::prepare( double SampleRate, int /*MaxBlockSize*/ )
    {
        sampleRate = SampleRate > 0 ? SampleRate : 44100.0;
        // Update decay coefficients
        envDecayCoef = (float)std::exp( -6.9 / juce::jmax( 1.0, values[kAmpDecay] * sampleRate ) );
    }

    void FSMKickEngine::reset()
    {
        currentPhase = 0.f;
        currentFreq = 0.f;
        targetFreq = 0.f;
        envLevel = 0.f;
        noteActive = false;
    }

    float FSMKickEngine::noteToFreq( int note )
    {
        // Convert MIDI note to frequency
        return 440.0f * std::pow( 2.0f, ( note - 69.0f ) / 12.0f );
    }

    void FSMKickEngine::noteOn( int Note, float Velocity )
    {
        currentNote = Note;
        noteVelocity = juce::jlimit( 0.f, 1.f, Velocity );
        noteActive = true;
        
        // Trigger the kick with the note's frequency
        const float noteFreq = noteToFreq( Note );
        currentFreq = noteFreq * (smoothedStartFreq / 150.f); // Scale by start freq relative to default
        targetFreq = noteFreq * (smoothedEndFreq / 50.f);    // Scale by end freq relative to default
        
        // If start and end are the same, just use the note frequency
        if( std::abs( smoothedStartFreq - smoothedEndFreq ) < 0.01f )
        {
            currentFreq = noteFreq;
            targetFreq = noteFreq;
        }
        
        // Calculate frequency decay rate based on toneDecay
        // Higher toneDecay = faster decay
        const float maxDecaySamples = smoothedToneDecay * sampleRate;
        freqDecay = 1.0f / juce::jmax( 1.0f, maxDecaySamples );
        
        // Set envelope to max
        envLevel = 1.0f;
        
        // Calculate amp decay coefficient
        envDecayCoef = decayCoef( smoothedAmpDecay, sampleRate );
    }

    void FSMKickEngine::noteOff( int /*Note*/ )
    {
        // Kick drums typically don't need note-off handling
        // The envelope will naturally decay
    }

    void FSMKickEngine::allNotesOff()
    {
        noteActive = false;
        envLevel = 0.f;
    }

    void FSMKickEngine::setParam( int Index, float Value )
    {
        if( !juce::isPositiveAndBelow( Index, kNumParams ) )
            return;
            
        const auto& p = params[(size_t)Index];
        values[Index] = juce::jlimit( p.minValue, p.maxValue, Value );
        
        // Update smoothed values immediately (we could add smoothing later)
        switch( Index )
        {
            case kStartFreq:     smoothedStartFreq = values[kStartFreq]; break;
            case kEndFreq:       smoothedEndFreq = values[kEndFreq]; break;
            case kToneDecay:     smoothedToneDecay = values[kToneDecay]; break;
            case kToneShape:     smoothedToneShape = values[kToneShape]; break;
            case kAmpDecay:      smoothedAmpDecay = values[kAmpDecay]; break;
            case kVolume:        smoothedVolume = values[kVolume]; break;
            default: break;
        }
    }

    void FSMKickEngine::process( float* const* Out, int NumChannels, int NumSamples )
    {
        // Clear output buffers
        for( int ch = 0; ch < NumChannels; ++ch )
            juce::FloatVectorOperations::clear( Out[ch], NumSamples );
        
        if( NumChannels <= 0 || !noteActive )
            return;

        // Get current parameter values
        const float startFreq = smoothedStartFreq;
        const float endFreq = smoothedEndFreq;
        const float toneDecay = smoothedToneDecay;
        const float toneShape = smoothedToneShape;
        const float ampDecay = smoothedAmpDecay;
        const float vol = smoothedVolume;
        
        // Calculate frequency decay based on toneDecay parameter
        const float maxDecaySamples = toneDecay * sampleRate;
        const float freqDecayRate = 1.0f / juce::jmax( 1.0f, maxDecaySamples );
        
        // Calculate amp decay coefficient
        const float ampDecayCoef = (float)std::exp( -6.9 / juce::jmax( 1.0, ampDecay * sampleRate ) );
        
        float* left = Out[0];

        for( int i = 0; i < NumSamples; ++i )
        {
            if( !noteActive )
                break;

            // Frequency envelope: exponential decay from startFreq to endFreq
            const float freqRatio = endFreq / startFreq;
            const float progress = 1.0f - std::pow( 1.0f - freqDecayRate, (float)i + 1.0f );
            
            // Apply toneShape to the progress (0 = linear, 1 = exponential)
            const float shapedProgress = juce::jmap( toneShape, 
                progress,  // linear
                progress * progress  // exponential
            );
            
            currentFreq = startFreq + ( endFreq - startFreq ) * shapedProgress;
            
            // Generate sine wave at current frequency
            currentPhase += 2.0f * juce::MathConstants<float>::pi * currentFreq / (float)sampleRate;
            if( currentPhase >= 2.0f * juce::MathConstants<float>::pi )
                currentPhase -= 2.0f * juce::MathConstants<float>::pi;
            
            const float sample = std::sin( currentPhase );
            
            // Apply amplitude envelope
            envLevel *= ampDecayCoef;
            if( envLevel < 0.0001f )
                noteActive = false;
            
            // Write to left channel
            left[i] += sample * envLevel * vol * noteVelocity;
        }

        // Copy to all other channels
        for( int ch = 1; ch < NumChannels; ++ch )
            juce::FloatVectorOperations::copy( Out[ch], left, NumSamples );
    }

} // namespace mt
