# Porting BuzzMachines Plugins to Kick

This guide outlines the process for porting BuzzMachines synthesis algorithms to the Behringer KICK (0700-AAM) desktop build as either standalone VST3 plugins or internal instrument engines.

## Prerequisites

### Source Repository
- [Buzztrax/buzzmachines](https://github.com/Buzztrax/buzzmachines)
- Most plugins are under **LGPL-2.1** license (compatible with AGPL-3.0)

### Verify License
```bash
# Check the COPYING file in the plugin directory
cat buzzmachines/FSM/COPYING
# Should show: LGPL-2.1 or similar compatible license
```

## Option A: Create Standalone VST3 Plugin

For creating a VST3 plugin that works in DAWs.

### Step 1: Directory Structure
```
Kick/apps/kickvst_<pluginname>/
├── CMakeLists.txt          # JUCE plugin configuration
├── src/
│   ├── PluginProcessor.h/.cpp   # JUCE AudioProcessor
│   ├── PluginEditor.h/.cpp     # Simple UI (optional)
│   └── <PluginName>Synth.h/.cpp # Ported algorithm
```

### Step 2: Port the Algorithm

**Original Buzz Machine Code:**
```cpp
// buzzmachines/FSM/Kick/kick.cpp
float phase;
float freq;

void Process(float* buffer, int samples) {
    // Direct buffer access
    for (int i = 0; i < samples; i++) {
        buffer[i] = sin(phase) * 0.5;
        phase += 2 * M_PI * freq / sampleRate;
    }
}
```

**Ported to JUCE:**
```cpp
// Kick/apps/kickvst_fsm/src/FSMKickSynth.h
#pragma once
#include <JuceHeader.h>

class FSMKickSynth {
public:
    void setSampleRate(float rate);
    void trigger();
    void processBlock(juce::AudioBuffer<float>& buffer, int startSample, int numSamples);
    
private:
    double sampleRate;
    float phase;
    float freq;
    // Parameters
    float startFreq;
    float endFreq;
    float toneDecay;
    // ...
};
```

**Implementation:**
```cpp
// Kick/apps/kickvst_fsm/src/FSMKickSynth.cpp
#include "FSMKickSynth.h"

void FSMKickSynth::processBlock(juce::AudioBuffer<float>& buffer, int startSample, int numSamples) {
    float* left = buffer.getWritePointer(0);
    float* right = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;
    
    for (int i = startSample; i < startSample + numSamples; i++) {
        float sample = sin(phase) * 0.5;
        left[i] += sample;
        if (right) right[i] += sample;
        phase += 2.0f * juce::MathConstants<float>::pi * freq / sampleRate;
    }
}
```

### Step 3: JUCE Plugin Processor

**PluginProcessor.h:**
```cpp
#pragma once
#include <JuceHeader.h>
#include "FSMKickSynth.h"

class FSMKickAudioProcessor : public juce::AudioProcessor
{
public:
    FSMKickAudioProcessor();
    ~FSMKickAudioProcessor() override;
    
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;
    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    double getTailLengthSeconds() const override;
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;
    
private:
    FSMKickSynth kickSynth;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FSMKickAudioProcessor)
};
```

**PluginProcessor.cpp:**
```cpp
#include "PluginProcessor.h"
#include "PluginEditor.h"

FSMKickAudioProcessor::FSMKickAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo()))
{
    kickSynth.setSampleRate(44100.0);
    kickSynth.setStartFrequency(150.0f);
    kickSynth.setEndFrequency(50.0f);
}

void FSMKickAudioProcessor::prepareToPlay(int samplesPerBlockExpected, double sampleRate) {
    kickSynth.setSampleRate(sampleRate);
}

void FSMKickAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    juce::ScopedNoDenormals noDenormals;
    
    // Process MIDI
    for (const auto& metadata : midiMessages) {
        const auto& msg = metadata.getMessage();
        if (msg.isNoteOn())
            kickSynth.trigger();
    }
    
    // Process audio
    kickSynth.processBlock(buffer, 0, buffer.getNumSamples());
}

// Factory function required by JUCE
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new FSMKickAudioProcessor();
}
```

### Step 4: Plugin Editor (Simple)

**PluginEditor.h:**
```cpp
#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class FSMKickAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    FSMKickAudioProcessorEditor(FSMKickAudioProcessor&);
    ~FSMKickAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    
private:
    FSMKickAudioProcessor& audioProcessor;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FSMKickAudioProcessorEditor)
};
```

**PluginEditor.cpp:**
```cpp
#include "PluginEditor.h"

FSMKickAudioProcessorEditor::FSMKickAudioProcessorEditor(FSMKickAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setSize(400, 200);
}

void FSMKickAudioProcessorEditor::paint(juce::Graphics& g) {
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::white);
    g.setFont(15.0f);
    g.drawFittedText("FSM Kick", getLocalBounds(), juce::Justification::centred, 1);
}

void FSMKickAudioProcessorEditor::resized() {}
```

### Step 5: CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.15)
project(kickvst_fsm VERSION 1.0.0)

# Generate configuration file
configure_file(${CMAKE_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

# JUCE plugin configuration
juce_add_plugin(${PROJECT_NAME}
    VERSION "${PROJECT_VERSION}"
    COMPANY_NAME "MusicTribe"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE LEOM
    PLUGIN_CODE FSMK  # Unique 4-character code
    FORMATS VST3      # Build as VST3
    PRODUCT_NAME "${PROJECT_NAME}"
)

juce_generate_juce_header(${PROJECT_NAME})

# Source files
target_sources(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/PluginProcessor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/PluginEditor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/FSMKickSynth.cpp
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

# JUCE modules
target_link_libraries(${PROJECT_NAME} PRIVATE
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_gui_basics
    juce::juce_data_structures
    juce::juce_events
)
```

### Step 6: Update Main CMakeLists.txt

**Kick/apps/CMakeLists.txt:**
```cmake
# FSM Kick VST3 plugin
if( KICK_VST_FSM )
    add_subdirectory(kickvst_fsm)
endif()
```

**Kick/CMakeLists.txt:**
```cmake
option( KICK_VST_FSM "FSM Kick VST3 Plugin" OFF )
```

### Step 7: Update Build Script

**Kick/build-mac.sh:**
```bash
"$CMAKE" -S "$ROOT/Kick" -B "$BUILD" -G Ninja \
    -DKICK_SIMULATOR=ON -DKICK_VST_PLUGIN=OFF -DKICK_VST_FSM=ON \
    ...

"$NINJA" -C "$BUILD" Kick KT_KickEM kickvst_fsm kickvst_fsm_VST3

# Create symlink for expected location
if [ -d "$BUILD/apps/kickvst_fsm/kickvst_fsm_artefacts/Release/VST3/kickvst_fsm.vst3" ]; then
    ln -sf "$BUILD/apps/kickvst_fsm/kickvst_fsm_artefacts/Release/VST3/kickvst_fsm.vst3" "$BUILD/kickvst_fsm.vst3"
fi
```

### Step 8: Install and Test

```bash
# Copy to DAW VST3 folder
cp -r build-mac/kickvst_fsm.vst3 ~/Library/Audio/Plug-Ins/VST3/

# Test in DAW
# Load "kickvst_fsm" as instrument plugin
```

---

## Option B: Create Internal Instrument Engine (for Emulator)

For using the plugin **inside** the KT_KickEM emulator as a Plugin program type.

### Step 1: Create Engine Class

**Kick/modules/engine/audio_program/instrument/fsm_kick_engine.h:**
```cpp
#pragma once
#include "mt_instrument_engine.h"

namespace mt
{
    class FSMKickEngine : public IInstrumentEngine
    {
    public:
        enum Param
        {
            kStartFreq,
            kEndFreq,
            kToneDecay,
            kToneShape,
            kAmpDecay,
            kVolume,
            kNumParams
        };

        FSMKickEngine();
        
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
        double sampleRate;
        bool noteActive;
        // ... audio processing state
    };
}
```

### Step 2: Register Engine

**Kick/modules/engine/audio_program/instrument/mt_basic_synth_engine.cpp:**
```cpp
namespace
{
    std::map<juce::String, InstrumentEngineRegistry::Factory>& registry()
    {
        static std::map<juce::String, InstrumentEngineRegistry::Factory> engines = {
            { "Basic Synth", []() { return std::make_unique<BasicSynthEngine>(); } },
            { "Modular", []() { return std::make_unique<ModularEngine>(); } },
            { "FSM Kick", []() { return std::make_unique<FSMKickEngine>(); } },  // NEW
        };
        return engines;
    }
}
```

### Step 3: Include in Module

**Kick/modules/engine/audio_program/audio_program.cpp:**
```cpp
// Add to the list of included source files
#include "instrument/fsm_kick_engine.cpp"
```

**Kick/modules/engine/audio_program/audio_program.h:**
```cpp
// Add to module includes
#include "instrument/fsm_kick_engine.h"
```

### Step 4: Rebuild and Test

```bash
# Rebuild emulator
cd Kick && bash build-mac.sh

# Run emulator
./build-mac/KT_KickEM.app/Contents/MacOS/KT_KickEM

# In emulator:
# 1. Go to Program tab
# 2. Click + (Add)
# 3. Select PLUGIN
# 4. Click Apply
# 5. Select FSM Kick as engine
# 6. Parameters will appear automatically (4 per page)
```

---

## Documentation

After porting, add to **Kick/CONTRIBUTORS.md**:

```markdown
## <Plugin Name> VST3 Plugin

### <Original Author> <email>
- **Source**: [Buzztrax/buzzmachines](https://github.com/Buzztrax/buzzmachines/tree/master/<PATH>)
- **License**: LGPL 2.1 (see buzzmachines/<PATH>/COPYING)
- **Contributions**: Original <plugin name> synthesis algorithm

## Internal Engine (Emulator)

The <Plugin Name> synth is also available as an internal instrument engine
in the emulator's Plugin program type, with the following parameters:
- Parameter 1: Description (range)
- Parameter 2: Description (range)
```

---

## Common Porting Challenges

### 1. Global State
**Problem:** Buzz machines often use global variables.
**Solution:** Convert to member variables in a class.

### 2. Direct Buffer Access
**Problem:** Buzz writes directly to float buffers.
**Solution:** Use `juce::AudioBuffer<float>` and its `getWritePointer()` method.

### 3. Sample Rate
**Problem:** Buzz machines often hardcode or assume sample rate.
**Solution:** Pass sample rate via `prepareToPlay()` or `setSampleRate()`.

### 4. MIDI Handling
**Problem:** Buzz has its own MIDI system.
**Solution:** Use JUCE's `MidiBuffer` and check for noteOn/noteOff messages.

### 5. Parameter Automation
**Problem:** Buzz parameters are simple floats.
**Solution:** Use JUCE's `AudioProcessorValueTreeState` or implement `setParam()`.

### 6. Missing JUCE Modules
**Problem:** Plugin doesn't link due to missing audio modules.
**Solution:** Add required modules in CMakeLists.txt:
```cmake
juce::juce_audio_basics
juce::juce_audio_devices  
juce::juce_audio_formats
juce::juce_audio_processors
juce::juce_audio_utils
```

---

## References

- [JUCE Framework](https://juce.com/)
- [Buzztrax BuzzMachines](https://github.com/Buzztrax/buzzmachines)
- [VST3 SDK](https://www.steinberg.net/en/company/developers.html)
- [CMake JUCE Integration](https://github.com/juce-framework/JUCE/blob/master/docs/CMake.md)
