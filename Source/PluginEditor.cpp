#include "PluginEditor.h"
namespace biscot {
BiscotEditor::BiscotEditor(BiscotProcessor& p):juce::AudioProcessorEditor(p),proc(p){
 title.setText("Biscot1 Synth - VST3 native TEST0.1",juce::dontSendNotification);
 message.setText("DSP partial port; Alt+Q tracker not implemented",juce::dontSendNotification);
 valueLabel.setText("Value",juce::dontSendNotification);
 addAndMakeVisible(title);addAndMakeVisible(message);addAndMakeVisible(valueLabel);
 parametersList.setModel(this);parametersList.setRowHeight(22);parametersList.setWantsKeyboardFocus(true);
 parametersList.setTitle("Biscot1 parameters");addAndMakeVisible(parametersList);
 slider.setSliderStyle(juce::Slider::LinearHorizontal);slider.setTextBoxStyle(juce::Slider::TextBoxRight,false,110,24);
 slider.addListener(this);addAndMakeVisible(slider);
 setSize(650,520);select(0);
}
BiscotEditor::~BiscotEditor(){slider.removeListener(this);parametersList.setModel(nullptr);}
void BiscotEditor::resized(){auto b=getLocalBounds().reduced(12);title.setBounds(b.removeFromTop(28));message.setBounds(b.removeFromTop(27));auto bottom=b.removeFromBottom(70);valueLabel.setBounds(bottom.removeFromTop(20));slider.setBounds(bottom);parametersList.setBounds(b);}
void BiscotEditor::paintListBoxItem(int row,juce::Graphics& g,int w,int h,bool selected){
 if(row<0 || row>=(int)catalog.size())return;
 if(selected)g.fillAll(juce::Colours::darkslategrey);
 g.setColour(juce::Colours::white);
 const auto& d=catalog[(size_t)row];
 g.drawText(juce::String(d.index-1)+". "+d.name+"   "+juce::String(proc.param(d.index),3),6,0,w-12,h,juce::Justification::centredLeft);
}
void BiscotEditor::selectedRowsChanged(int row){select(row);}
void BiscotEditor::select(int row){
 if(row<0||row>=(int)catalog.size())return;
 const auto& d=catalog[(size_t)row];
 rebuilding=true;slider.setRange(d.low,d.high,d.step>0?d.step:0.001);slider.setValue(proc.param(d.index),juce::dontSendNotification);
 slider.setName(d.name);valueLabel.setText(d.name,juce::dontSendNotification);rebuilding=false;
 parametersList.selectRow(row);
}
void BiscotEditor::sliderValueChanged(juce::Slider*){
 if(rebuilding)return;
 int row=parametersList.getSelectedRow();if(row<0||row>=(int)catalog.size())return;
 auto& d=catalog[(size_t)row];
 if(auto* p=proc.state.getParameter(BiscotProcessor::id(d.index))){
  p->beginChangeGesture();p->setValueNotifyingHost(p->convertTo0to1((float)slider.getValue()));p->endChangeGesture();
 }
 parametersList.repaintRow(row);
}
bool BiscotEditor::keyPressed(const juce::KeyPress& key){
 if(key.getModifiers().isAltDown() && (key.getTextCharacter()=='q'||key.getTextCharacter()=='Q')){
  message.setText("Tracker Alt+Q: not yet migrated",juce::dontSendNotification);return true;
 }
 return false;
}
}
