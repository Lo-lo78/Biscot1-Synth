#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "ParameterCatalog.h"
#include "TrackerPatternBank.h"
#include "TrackerPatternPlayback.h"
#include <memory>
#include "SidOutputStage.h"
#include "SidFilterCore.h"
#include "SidWaveCore.h"
#include "SidMidiNoteState.h"
#include "SidSeqMemory.h"
#include "SidAmpEnvelope.h"
#include "SidLfoCore.h"
#include "SidIdleSleep.h"
#include "SidLfoUsage.h"
#include "SidArpPoolCache.h"
#include "SidFilterLfoTrigger.h"
#include "SidArpSustain.h"
#include "SidQuantizedArp.h"
#include "TrackerMidiRouting.h"
#include "SidArpTransport.h"
#include <array>
#include <atomic>
#include <cstdint>

namespace biscot {
class BiscotProcessor final : public juce::AudioProcessor {
public:
 BiscotProcessor();
 void prepareToPlay(double,int) override;
 void releaseResources() override {}
 void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
 bool isBusesLayoutSupported(const BusesLayout&) const override;
 juce::AudioProcessorEditor* createEditor() override;
 bool hasEditor() const override {return true;}
 const juce::String getName() const override {return "Biscot1 Synth";}
 bool acceptsMidi() const override {return true;}
 bool producesMidi() const override {return false;}
 bool isMidiEffect() const override {return false;}
 // JSFX Release=5s is an exponential time constant (~46 s to -80 dB);
 // do not let the host cut the audio tail after only five seconds.
 double getTailLengthSeconds() const override {return 50.0;}
 int getNumPrograms() override {return 1;}
 int getCurrentProgram() override {return 0;}
 void setCurrentProgram(int) override {}
 const juce::String getProgramName(int) override {return {};}
 void changeProgramName(int,const juce::String&) override {}
 void getStateInformation(juce::MemoryBlock&) override;
 void setStateInformation(const void*,int) override;
 juce::AudioProcessorValueTreeState state;
 TrackerPatternBank trackerPatterns;
 using TrackerRecordedMidiEvent=biscot::TrackerRecordedMidiEvent;
 std::atomic<bool> trackerStepRecording{false};
 bool popTrackerRecordedMidi(TrackerRecordedMidiEvent& e) noexcept {return trackerMidiQueue.pop(e);}
 void commitTrackerState(); // UI thread: serialize and publish an immutable Pattern snapshot
 void requestTrackerStart(int step,bool audition=false) noexcept;
 void requestTrackerPause() noexcept;
 void requestTrackerStop() noexcept;
 std::atomic<int> trackerAudibleStep{-1}; // audio-clock position for accessible pause
 std::atomic<int> trackerPlaybackStatus{0}; // 0 stopped, 1 playing, 2 paused, 3 audition
 float param(int i) const noexcept {
  if(i<0 || i>=(int)parameters.size())return 0.f;
  if(trackerOverridesEnabled.load(std::memory_order_relaxed) &&
     trackerOverrideMask[(size_t)i].load(std::memory_order_relaxed))
   return trackerOverrideValues[(size_t)i].load(std::memory_order_relaxed);
  return parameters[(size_t)i] ? parameters[(size_t)i]->load(std::memory_order_relaxed) : 0.f;
 }
 static juce::String id(int slider){return "s"+juce::String(slider).paddedLeft('0',3);}
private:
 static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
 struct Voice {
  int note=-1,channel=1;bool gate=false;
  double phase=0,oldPhase=0,noteSmooth=-1,arpPhase=0;
  float vel=1.f,bend=0.f,drift=0.f,oscPrev=0.f,arpPitch=0.f;
  uint32_t noiseSeed=0x12345678u;
  uint64_t cleanNoiseSeed=1234567, sidNoiseSeed=5930003;
  double noiseClock=0;float noise=0.f;
  SidAmpEnvelope amp,mod;
  bool pedalLatched=false;int pedalChannel=1;
  int arpStep=0;bool arpGoingUp=true;
  SidQuantizedArp quantizedArp{};
 };
 using Lfo = SidLfoCore;
 std::array<Voice,3> voices{};
 std::atomic<std::shared_ptr<const TrackerPlayablePattern>> trackerPlayable{};
 std::atomic<std::shared_ptr<const TrackerMidiMap>> trackerMidiMap{};
 TrackerMidiInputQueue trackerMidiQueue{};
 int trackerMidiTriggerNote=-1;
 bool trackerReleaseArpActive=false;
 double trackerReleaseSamplesRemaining=0,trackerReleaseStepRemain=0,trackerReleaseStepSamples=0;
 int trackerReleaseStep=0,trackerReleaseLastNote=-1,trackerReleaseTranspose=0;
 void trackerReleaseArpTick() noexcept;
 bool handleTrackerExternalMidi(const juce::MidiMessage&) noexcept;
 std::shared_ptr<const TrackerPlayablePattern> trackerPlaybackSnapshot; // pinned while audio transport is active
 std::atomic<int> trackerRequested{0}; // 1..1024 start; 2049..3072 audition; -1 stop; -2 pause
 TrackerPatternTransport trackerTransport{}; // audio thread only
 std::array<std::atomic<float>,234> trackerOverrideValues{};
 std::array<std::atomic<bool>,234> trackerOverrideMask{};
 std::atomic<bool> trackerOverridesEnabled{false};
 void trackerSetOverride(int index,float value) noexcept;
 void trackerClearOverrides() noexcept;
 void trackerApplyStep(const TrackerPlayableRow& row,int transpose,double stepSamples);
 void trackerRunCurveEvents(const TrackerPlayableRow& row) noexcept;
 std::array<int,128> trackerCurveNextSegments{};
 void trackerReleaseNote() noexcept;
 SidMidiNoteState heldNotes;
 std::array<SidSeqMemory,3> sidSeq{};
 SidArpSustain sustainedArp;
 int filterKeyNote=60;
 std::array<Lfo,3> lfos{};
 SidAmpEnvelope filterEnvelope;
 std::array<bool,16> pedalDown{};
 std::array<SidFilterCore,2> filters{};
 SidWaveCore waveCore;
 SidIdleSleep idleSleep{};
 std::array<bool,3> lfoUsedCached{};
 // Mix L/R and the four optional stereo stems use independent JSFX output circuits.
 std::array<SidOutputStage,10> outputStages{};
 std::array<std::atomic<float>*,234> parameters{};
 std::array<std::array<std::uint64_t,128>,16> pressed{};
 std::array<std::array<unsigned int,128>,16> pressedCounts{};
 std::uint64_t nextPress=0;
 std::array<std::array<int,640>,3> arpPools{};
 std::array<int,3> arpPoolCounts{};
 SidArpPoolCache arpPoolCache{};
 // Last live pool continues through the natural envelope release tail.
 std::array<std::array<int,640>,3> arpReleasePools{};
 std::array<int,3> arpReleaseCounts{};
 // Chord Op1 Controls shares only ADSR, Portamento, Sustain Pedal,
 // Level and PWM. Other sources (waves, pitch, pan, LFO, etc.) stay independent.
 bool chordMaster() const noexcept {return chordMasterEnabled((int)param(3),param(230)>0.5f);}
 float opSharedParam(int base,int op) const noexcept {return param(base+(chordMaster()?0:op));}
 bool opPedalEnabled(int op) const noexcept {return opSharedParam(227,op)>0.5f;}
 void rebuildArpPools();
 void refreshArpPoolsIfNeeded();
 SidArpPoolSettings currentArpPoolSettings() const noexcept;
 double sampleRateHz=44100.,tempoBpm=120.;bool lastPlaying=false;
 void noteOn(int,int,int,bool,bool);
 void noteOff(int,int);
 void triggerFilterAndLfos(int,bool);
 void onMidi(const juce::MidiMessage&);
 void releasePedalLatches();
 void renderOne(float&,float&,std::array<float,8>&);
 void advanceIdleLfos(int samples) noexcept;
 void enterColdIdle() noexcept;
 float processFilter(float,float,int,float);
 float processLfo(int,const std::array<float,3>&);
 float pulse(double,float,double) const;
 static float polyBlep(double,double);
 static float saturate(float);
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BiscotProcessor)
};
}
