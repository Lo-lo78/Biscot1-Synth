// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "TrackerParameterCatalog.h"
#include <functional>
#include <map>
#include <vector>

namespace biscot {
// Initial native F1/F2 tracker editor. The JUCE controls, not a Lua/GFX window,
// receive keyboard focus and expose their selected text through Windows UIA.
class TrackerPatternPanel final : public juce::Component {
public:
 explicit TrackerPatternPanel(BiscotProcessor&);
 void resized() override;
 void paint(juce::Graphics&) override;
 std::unique_ptr<juce::AccessibilityHandler> createAccessibilityHandler() override;
 bool keyPressed(const juce::KeyPress&) override;
 void open();
 void restoreAfterPreset();
 std::function<void()> onClose;
 std::function<void(bool)> onPresetBrowser;
private:
 struct Navigator : juce::ComboBox {
  explicit Navigator(TrackerPatternPanel& parent):owner(parent){}
  bool keyPressed(const juce::KeyPress& k) override {return owner.handle(k,false);}
  TrackerPatternPanel& owner;
 } navigator;
 struct Numeric : juce::TextEditor {
  explicit Numeric(TrackerPatternPanel& parent):juce::TextEditor("Tracker Edit"),owner(parent){}
  bool keyPressed(const juce::KeyPress& k) override {return owner.handle(k,true) || juce::TextEditor::keyPressed(k);}
  TrackerPatternPanel& owner;
 } numeric;
 BiscotProcessor& processor;
 juce::Label header,help;
 int page=1,tab=0,row=-1,cell=0,parameterIndex=0,paramHeight=8;
 std::array<int,3> lastParam{0,0,0};
 bool refreshing=false;
 std::vector<const TrackerParam*> parameterList;
 std::map<int,std::vector<TrackerPattern>> undo,redo;
 TrackerCell clipboard{};
 bool hasClipboard=false;
 TrackerPattern& currentPattern(){return processor.trackerPatterns.at(processor.trackerPatterns.activePattern);}
 const TrackerParam* currentParameter() const;
 TrackerCell* selectedCell();
 void refresh(bool speakPosition=false);
 void announce(const juce::String&);
 juce::String positionText() const;
 juce::String cellText(const TrackerCell&) const;
 void refreshParameterList();
 void recordUndo();
 void finishEdit();
 void changeValue(double value);
 void adjustValue(int direction,double multiplier=1);
 bool handle(const juce::KeyPress&,bool editing);
 void setTab(int);
 void changeRow(int);
 void close();
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrackerPatternPanel)
};
}
