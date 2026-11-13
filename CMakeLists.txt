cmake_minimum_required(VERSION 3.22)
project(Biscot1 VERSION 0.2.37 LANGUAGES C CXX)
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
  DESCRIPTION "Biscot1 synth native VST3 port - accessible editor and native Song editor TEST0.2.37"
  IS_SYNTH TRUE
  NEEDS_MIDI_INPUT TRUE
  NEEDS_MIDI_OUTPUT FALSE
  EDITOR_WANTS_KEYBOARD_FOCUS TRUE
  COPY_PLUGIN_AFTER_BUILD FALSE)
target_sources(Biscot1 PRIVATE Source/PluginProcessor.cpp Source/PluginProcessor.h Source/SidMidiNoteState.h Source/SidSeqMemory.h Source/SidAmpEnvelope.h Source/SidLfoCore.h Source/SidIdleSleep.h Source/SidLfoUsage.h Source/SidArpPoolCache.h Source/SidFilterLfoTrigger.h Source/SidArpSustain.h Source/SidArpReleasePool.h Source/SidQuantizedArp.h Source/SidArpTransport.h Source/PluginEditor.cpp Source/PluginEditor.h Source/ParameterCatalog.h Source/PitchBendRouting.h Source/SidOutputStage.h Source/SidFilterCore.h Source/SidWaveCore.h Source/IncrementalParameterSearch.h Source/ParameterSearchKey.h Source/PresetBrowser.cpp Source/PresetBrowser.h Source/PresetManager.cpp Source/PresetManager.h Source/PresetBrowserMemory.h Source/ScreenReaderAnnouncer.cpp Source/ScreenReaderAnnouncer.h Source/TrackerPatternBank.h Source/TrackerPatternText.h Source/TrackerPatternPlayback.h Source/TrackerShortcutKey.h Source/TrackerHeaderNavigation.h Source/TrackerCellPresentation.h Source/TrackerCellNavigation.h Source/TrackerParameterCatalog.h Source/TrackerPatternAutomation.h Source/TrackerPatternInsert.h Source/TrackerManualNumeric.h Source/TrackerPatternPanel.h Source/TrackerPatternPanel.cpp Source/TrackerSongBank.h Source/TrackerSongPackage.h Source/TrackerLibraryFileNames.h Source/TrackerSongPlayback.h Source/TrackerSongFxCore.h Source/TrackerSongParameters.h Source/TrackerSongPanel.h Source/TrackerSongPanel.cpp Source/BrowserIncrementalSearch.h Source/TrackerLibraryBrowser.h Source/TrackerLibraryBrowser.cpp Source/TrackerMidiRouting.h Source/TrackerStepRecording.h)
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

# Pitch Bend routing is isolated so all 16 MIDI channels can be tested without JUCE.
add_executable(PitchBendRoutingTest Tests/PitchBendRoutingTest.cpp)
target_compile_features(PitchBendRoutingTest PRIVATE cxx_std_20)

# Note-stack / legato / Chord Allocate routing parity tests require no JUCE.
add_executable(SidMidiNoteStateTest Tests/SidMidiNoteStateTest.cpp)
target_compile_features(SidMidiNoteStateTest PRIVATE cxx_std_20)

# JUCE-free regression of JSFX SID Seq's 128-position clocked memory.
add_executable(SidSeqMemoryTest Tests/SidSeqMemoryTest.cpp)
target_compile_features(SidSeqMemoryTest PRIVATE cxx_std_20)

# Independent regression test of all three ADSRs, CC64 latch/release semantics.
add_executable(SidSustainPedalTest Tests/SidSustainPedalTest.cpp)
target_compile_features(SidSustainPedalTest PRIVATE cxx_std_20)

# Sustain-held ARP pools and scoped Chord master must not regress.
add_executable(SidArpSustainTest Tests/SidArpSustainTest.cpp)
target_compile_features(SidArpSustainTest PRIVATE cxx_std_20)

# Clock-quantized classical ARP, musical Trigger / Legato and Release regression.
add_executable(SidQuantizedArpTest Tests/SidQuantizedArpTest.cpp)
target_compile_features(SidQuantizedArpTest PRIVATE cxx_std_20)

# Standalone natural-Release regression: classical arp and SID Seq must keep
# ticking after final Note Off and CC64-up, then stop at ADSR idle.
add_executable(SidArpReleaseTest Tests/SidArpReleaseTest.cpp)
target_compile_features(SidArpReleaseTest PRIVATE cxx_std_20)

# CC64-up must sound like releasing the same chord by hand: no arp root
# discontinuity, no clock reset, no alteration to the Release envelope.
add_executable(SidPedalReleaseContinuityTest Tests/SidPedalReleaseContinuityTest.cpp)
target_compile_features(SidPedalReleaseContinuityTest PRIVATE cxx_std_20)

# Point 3: JUCE-independent numerical and event-regression tests for JSFX 2.38
# filter ADSR, Trigger/Legato and all three LFOs (including One Shot recovery).
add_executable(SidFilterLfoParityTest Tests/SidFilterLfoParityTest.cpp)
target_compile_features(SidFilterLfoParityTest PRIVATE cxx_std_20)

# Point 4: true idle DSP suspension and independent LFO Square PWM regression.
add_executable(SidIdleAndSquarePwmTest Tests/SidIdleAndSquarePwmTest.cpp)
target_compile_features(SidIdleAndSquarePwmTest PRIVATE cxx_std_20)

# Regressions for zero-work idle and event-driven classical arp pool refresh.
add_executable(SidDeepIdleCpuTest Tests/SidDeepIdleCpuTest.cpp)
target_compile_features(SidDeepIdleCpuTest PRIVATE cxx_std_20)

# Initial native Pattern bank: completely independent of JUCE and the audio DSP.
add_executable(TrackerPatternBankTest Tests/TrackerPatternBankTest.cpp)
target_compile_features(TrackerPatternBankTest PRIVATE cxx_std_20)

# Regressions of F2 multi-character/word search, reverse cycling, and F1
# step clipboard that copies all three sections without shared references.
add_executable(TrackerSearchStepTest Tests/TrackerSearchStepTest.cpp)
target_compile_features(TrackerSearchStepTest PRIVATE cxx_std_20)
if(WIN32)
  target_link_libraries(TrackerSearchStepTest PRIVATE User32)
endif()

# Regression: Windows Ctrl+letter control characters must reach F1 clipboard commands.
add_executable(TrackerShortcutTest Tests/TrackerShortcutTest.cpp)
target_compile_features(TrackerShortcutTest PRIVATE cxx_std_20)

# TEST0.2.19 sample-clock Pattern preview/transport logic without JUCE.
add_executable(TrackerPatternPlaybackTest Tests/TrackerPatternPlaybackTest.cpp)
target_compile_features(TrackerPatternPlaybackTest PRIVATE cxx_std_20)

# TEST0.2.19.2: enum-label parity and persistent logical F1 column across steps.
add_executable(TrackerCellPresentationTest Tests/TrackerCellPresentationTest.cpp)
target_compile_features(TrackerCellPresentationTest PRIVATE cxx_std_20)

# TEST0.2.19.2: actual F1 cursor selection semantics and same-param step searches.
add_executable(TrackerCellNavigationTest Tests/TrackerCellNavigationTest.cpp)
target_compile_features(TrackerCellNavigationTest PRIVATE cxx_std_20)

# TEST0.2.19.5: live value-edit preview follows Lua and preserves full playback.
add_executable(TrackerPatternEditAuditionTest Tests/TrackerPatternEditAuditionTest.cpp)
target_compile_features(TrackerPatternEditAuditionTest PRIVATE cxx_std_20)

# TEST0.2.19.5: locale-independent manual values are parsed identically in Release.
add_executable(TrackerManualNumericTest Tests/TrackerManualNumericTest.cpp)
target_compile_features(TrackerManualNumericTest PRIVATE cxx_std_20)

# TEST0.2.20: Lua 0.166 Previous Value / Curve / Resolution / Ratio and parameter insertion.
# TEST0.2.21: Enter opens parameter picker; Alt+E edits; Alt+Home max / Alt+End min.
add_executable(TrackerCurvePreviousTest Tests/TrackerCurvePreviousTest.cpp)
target_compile_features(TrackerCurvePreviousTest PRIVATE cxx_std_20)

# TEST0.2.21: Pattern MIDI triggers, SPSC step recording and Sub Notes/Release Arp.
add_executable(TrackerMidiFeaturesTest Tests/TrackerMidiFeaturesTest.cpp)
target_compile_features(TrackerMidiFeaturesTest PRIVATE cxx_std_20)

# TEST0.2.21.4: real manual Note-Off release chord (no pedal) and
# direct F1 header navigation including forward/reverse repeat cycling.
add_executable(SidArpReleaseLatchTest Tests/SidArpReleaseLatchTest.cpp)
target_compile_features(SidArpReleaseLatchTest PRIVATE cxx_std_20)
add_executable(TrackerHeaderNavigationTest Tests/TrackerHeaderNavigationTest.cpp)
target_compile_features(TrackerHeaderNavigationTest PRIVATE cxx_std_20)

# TEST0.2.21.4: insert/paste at current F1 cell; ordered Curve helpers.
add_executable(TrackerInsertionPositionTest Tests/TrackerInsertionPositionTest.cpp)
target_compile_features(TrackerInsertionPositionTest PRIVATE cxx_std_20)

# Native, portable Pattern-only library (.biscot1-pattern) format.
add_executable(TrackerPatternTextTest Tests/TrackerPatternTextTest.cpp)
target_compile_features(TrackerPatternTextTest PRIVATE cxx_std_20)

# TEST0.2.26: sparse 100-Song / three independent lane memory and F4 parameter groups.
# TEST0.2.26: sample-accurate three-lane Song transport and immutable Pattern clips.
add_executable(TrackerSongPlaybackTest Tests/TrackerSongPlaybackTest.cpp)
target_compile_features(TrackerSongPlaybackTest PRIVATE cxx_std_20)
add_executable(TrackerSongAdvancedTest Tests/TrackerSongAdvancedTest.cpp)
target_compile_features(TrackerSongAdvancedTest PRIVATE cxx_std_20)
add_executable(TrackerSongBankTest Tests/TrackerSongBankTest.cpp)
target_compile_features(TrackerSongBankTest PRIVATE cxx_std_20)

# TEST0.2.26: Song F2 editing history, parameter groups and clipboard operations.
add_executable(TrackerSongEditingTest Tests/TrackerSongEditingTest.cpp)
target_compile_features(TrackerSongEditingTest PRIVATE cxx_std_20)

# TEST0.2.26: complete Song 0 -> Song 1 transition and independent short previews.
add_executable(TrackerSongEndPreviewTest Tests/TrackerSongEndPreviewTest.cpp)
target_compile_features(TrackerSongEndPreviewTest PRIVATE cxx_std_20)

# TEST0.2.37: library preview must reproduce the unmodified three-lane Song timeline.
add_executable(TrackerSongLibraryPreviewTest Tests/TrackerSongLibraryPreviewTest.cpp)
target_compile_features(TrackerSongLibraryPreviewTest PRIVATE cxx_std_20)
