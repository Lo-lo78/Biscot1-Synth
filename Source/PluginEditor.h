#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
namespace biscot {
class BiscotEditor final : public juce::AudioProcessorEditor,private juce::ListBoxModel,private juce::Slider::Listener {
public:
 explicit BiscotEditor(BiscotProcessor&);
 ~BiscotEditor() override;
 void resized() override;
 bool keyPressed(const juce::KeyPress&) override;
private:
 int getNumRows() override {return (int)catalog.size();}
 void paintListBoxItem(int,juce::Graphics&,int,int,bool) override;
 void selectedRowsChanged(int) override;
 void sliderValueChanged(juce::Slider*) override;
 void select(int);
 BiscotProcessor& proc;
 juce::Label title,message,valueLabel;
 juce::ListBox parametersList;
 juce::Slider slider;
 bool rebuilding=false;
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BiscotEditor)
};
}
