cmake_minimum_required(VERSION 3.22)
project(Biscot1 VERSION 0.2.3 LANGUAGES C CXX)
if(MSVC)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()
include(FetchContent)
set(JUCE_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(JUCE_BUILD_EXTRAS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.15 GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)
juce_add_plugin(Biscot1
  COMPANY_NAME "Lo-lo"
  BUNDLE_ID "org.lolo.biscot1"
  PLUGIN_MANUFACTURER_CODE LoLo
  PLUGIN_CODE Bs01
  FORMATS VST3
  PRODUCT_NAME "Biscot1 Synth"
  DESCRIPTION "Biscot1 synth native VST3 port - accessible editor and extended native DSP TEST0.2.3"
  IS_SYNTH TRUE
  NEEDS_MIDI_INPUT TRUE
  NEEDS_MIDI_OUTPUT FALSE
  EDITOR_WANTS_KEYBOARD_FOCUS TRUE
  COPY_PLUGIN_AFTER_BUILD FALSE)
target_sources(Biscot1 PRIVATE Source/PluginProcessor.cpp Source/PluginProcessor.h Source/PluginEditor.cpp Source/PluginEditor.h Source/ParameterCatalog.h Source/SidOutputStage.h Source/SidFilterCore.h Source/SidWaveCore.h Source/IncrementalParameterSearch.h Source/ParameterSearchKey.h)
target_compile_features(Biscot1 PRIVATE cxx_std_20)
target_compile_definitions(Biscot1 PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(Biscot1 PRIVATE juce::juce_audio_utils juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)

# NVDA/UI Automation notifications, as in the user-provided LB-303 reference.
if(WIN32)
  target_link_libraries(Biscot1 PRIVATE Uiautomationcore OleAut32)
endif()

# Standalone DSP parity regression against numerical JSFX v2.38 reference.
add_executable(SidCoreStandaloneTest Tests/SidCoreStandaloneTest.cpp)
target_compile_features(SidCoreStandaloneTest PRIVATE cxx_std_20)
