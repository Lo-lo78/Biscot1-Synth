#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "ParameterCatalog.h"
#include "SidOutputStage.h"
#include "SidFilterCore.h"
#include "SidWaveCore.h"
#include "SidMidiNoteState.h"
#include "SidSeqMemory.h"
#include "SidAmpEnvelope.h"
#include "SidArpSustain.h"
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
 float param(int i) const noexcept {return i>=0 && i<(int)parameters.size() && parameters[(size_t)i] ? parameters[(size_t)i]->load(std::memory_order_relaxed) : 0.f;}
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
 };
 struct Lfo {
  double phase=0,age=0,delayAge=0;
  float output=0,previous=0,hold=0,smoothed=0;
  uint32_t seed=0xabcdef01u;
  bool active=true;
 };
 
 std::array<Voice,3> voices{};
 SidMidiNoteState heldNotes;
 std::array<SidSeqMemory,3> sidSeq{};
 SidArpSustain sustainedArp;
 int filterKeyNote=60;
 std::array<Lfo,3> lfos{};
 SidAmpEnvelope filterEnvelope;
 std::array<bool,16> pedalDown{};
 std::array<SidFilterCore,2> filters{};
 SidWaveCore waveCore;
 // Mix L/R and the four optional stereo stems use independent JSFX output circuits.
 std::array<SidOutputStage,10> outputStages{};
 std::array<std::atomic<float>*,231> parameters{};
 std::array<std::array<std::uint64_t,128>,16> pressed{};
 std::array<std::array<unsigned int,128>,16> pressedCounts{};
 std::uint64_t nextPress=0;
 std::array<std::array<int,640>,3> arpPools{};
 std::array<int,3> arpPoolCounts{};
 // Last live pool continues through the natural envelope release tail.
 std::array<std::array<int,640>,3> arpReleasePools{};
 std::array<int,3> arpReleaseCounts{};
 // Chord Op1 Controls shares only ADSR, Portamento, Sustain Pedal,
 // Level and PWM. Other sources (waves, pitch, pan, LFO, etc.) stay independent.
 bool chordMaster() const noexcept {return chordMasterEnabled((int)param(3),param(230)>0.5f);}
 float opSharedParam(int base,int op) const noexcept {return param(base+(chordMaster()?0:op));}
 bool opPedalEnabled(int op) const noexcept {return opSharedParam(227,op)>0.5f;}
 void rebuildArpPools();
 double sampleRateHz=44100.,tempoBpm=120.;bool lastPlaying=false;
 void noteOn(int,int,int,bool);
 void noteOff(int,int);
 void triggerFilterAndLfos(int,bool);
 void onMidi(const juce::MidiMessage&);
 void releasePedalLatches();
 void renderOne(float&,float&,std::array<float,8>&);
 float processFilter(float,float,int,float);
 float processLfo(int,const std::array<float,3>&);
 float pulse(double,float,double) const;
 static float polyBlep(double,double);
 static float saturate(float);
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BiscotProcessor)
};
}
