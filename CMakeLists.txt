cmake_minimum_required(VERSION 3.22)
project(PTZRCloud VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS version")
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Universal binary")
endif()

# JUCE: use -DJUCE_PATH=/caminho/local/JUCE or let CMake download it
if(JUCE_PATH)
    add_subdirectory(${JUCE_PATH} JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.6
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(JUCE)
endif()

if(APPLE)
    set(PTZR_FORMATS AU VST3 Standalone)
else()
    set(PTZR_FORMATS VST3 Standalone)
endif()
if(PTZR_ONLY_VST3)
    set(PTZR_FORMATS VST3)
endif()

juce_add_binary_data(PTZRAssets SOURCES Resources/ptzr_logo.png)
set_target_properties(PTZRAssets PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

juce_add_plugin(PTZRCloud
    COMPANY_NAME "PTZR"
    BUNDLE_ID com.ptzr.cloud
    PLUGIN_MANUFACTURER_CODE Ptzr
    PLUGIN_CODE Pcld
    FORMATS ${PTZR_FORMATS}
    PRODUCT_NAME "PTZR CLOUD"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    AU_MAIN_TYPE kAudioUnitType_MusicDevice
    VST3_CATEGORIES Instrument Synth)

target_sources(PTZRCloud PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp)

target_compile_definitions(PTZRCloud PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_REPORT_APP_USAGE=0)

if(PTZR_NO_AUDIO_DEVICES)
    target_compile_definitions(PTZRCloud PUBLIC JUCE_ALSA=0 JUCE_JACK=0)
endif()

target_link_libraries(PTZRCloud
    PRIVATE
        PTZRAssets
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
