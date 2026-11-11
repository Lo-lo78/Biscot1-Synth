#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "ParameterCatalog.h"
#include "SidOutputStage.h"
#include "SidFilterCore.h"
#include "SidWaveCore.h"
#include "SidMidiNoteState.h"
#include "SidSeqMemory.h"
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
 double getTailLengthSeconds() const override {return 5.0;}
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
 struct Envelope {
  float level=0.f,releaseCoefficient=0.f;int stage=0;
  void noteOn(bool reset){if(reset)level=0;stage=1;}
  void noteOff(float sec,double sr){releaseCoefficient=1.f/(float)std::max(1.,(double)sec*sr);stage=4;}
  float tick(float attack,float decay,float sustain,double sr);
 };
 struct Voice {
  int note=-1,channel=1;bool gate=false;
  double phase=0,oldPhase=0,noteSmooth=-1,arpPhase=0;
  float vel=1.f,bend=0.f,drift=0.f,oscPrev=0.f,arpPitch=0.f;
  uint32_t noiseSeed=0x12345678u;
  uint64_t cleanNoiseSeed=1234567, sidNoiseSeed=5930003;
  double noiseClock=0;float noise=0.f;
  Envelope amp,mod;
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
 int filterKeyNote=60;
 std::array<Lfo,3> lfos{};
 Envelope filterEnvelope;
 std::array<SidFilterCore,2> filters{};
 SidWaveCore waveCore;
 // Mix L/R and the four optional stereo stems use independent JSFX output circuits.
 std::array<SidOutputStage,10> outputStages{};
 std::array<std::atomic<float>*,227> parameters{};
 std::array<std::array<std::uint64_t,128>,16> pressed{};
 std::array<std::array<unsigned int,128>,16> pressedCounts{};
 std::uint64_t nextPress=0;
 std::array<std::array<int,640>,3> arpPools{};
 std::array<int,3> arpPoolCounts{};
 void rebuildArpPools();
 double sampleRateHz=44100.,tempoBpm=120.;bool lastPlaying=false;
 void noteOn(int,int,int,bool);
 void noteOff(int,int);
 void triggerFilterAndLfos(int,bool);
 void onMidi(const juce::MidiMessage&);
 void renderOne(float&,float&,std::array<float,8>&);
 float processFilter(float,float,int,float);
 float processLfo(int,const std::array<float,3>&);
 float pulse(double,float,double) const;
 static float polyBlep(double,double);
 static float saturate(float);
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BiscotProcessor)
};
}
