#pragma once
#include <JuceHeader.h>
#include "ParameterCatalog.h"
#include <array>
namespace biscot {
class BiscotProcessor final : public juce::AudioProcessor {
public:
 BiscotProcessor();
 void prepareToPlay(double, int) override;
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
 float param(int slider) const noexcept {auto* p=slider>=0 && slider<(int)parameters.size()?parameters[(size_t)slider]:nullptr; return p?p->load(std::memory_order_relaxed):0.f;}
 static juce::String id(int slider) {return "s"+juce::String(slider).paddedLeft('0',3);}
private:
 static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
 struct Voice {int note=-1,channel=0; double phase=0,env=0; int stage=0; float bend=0.f; bool gate=false;};
 std::array<Voice,3> voices{};
 std::array<std::atomic<float>*,226> parameters{};
 double sampleRateHz=44100;
 float filterLpL=0,filterLpR=0,dcL=0,dcR=0,prevInL=0,prevInR=0;
 void onMidi(const juce::MidiMessage&);
 void renderOne(float& left,float& right,std::array<float,6>& stems);
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BiscotProcessor)
};
} // namespace biscot
