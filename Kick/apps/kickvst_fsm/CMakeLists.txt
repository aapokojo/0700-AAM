cmake_minimum_required(VERSION 3.15)

project(kickvst_fsm VERSION 1.0.0)

# Generate configuration file
configure_file(${CMAKE_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

# JUCE plugin configuration
juce_add_plugin(${PROJECT_NAME}
    VERSION                     "${PROJECT_VERSION}"
    COMPANY_NAME                "MusicTribe"
    IS_SYNTH                    TRUE
    NEEDS_MIDI_INPUT            TRUE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD     FALSE
    PLUGIN_MANUFACTURER_CODE    LEOM
    PLUGIN_CODE                 FSMK
    FORMATS                     VST3
    PRODUCT_NAME                "${PROJECT_NAME}"
)

juce_generate_juce_header(${PROJECT_NAME})

# Add plugin source files
target_sources(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/PluginEditor.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/PluginProcessor.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/PluginEditor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/PluginProcessor.cpp
)

# Add FSM Kick synthesis implementation
target_sources(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/FSMKickSynth.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/FSMKickSynth.cpp
)

# Add extra JUCE-specific pre-processor definitions
target_compile_definitions(${PROJECT_NAME}
    PUBLIC
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

# Add build directory to include path for config.h
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

# Link to JUCE modules
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
