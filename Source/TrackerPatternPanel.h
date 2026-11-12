// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "TrackerParameterCatalog.h"
#include "IncrementalParameterSearch.h"
#include "ParameterSearchKey.h"
#include <functional>
#include <map>
#include <array>
#include <vector>

namespace biscot {
// Native F1 Pattern grid and Enter-opened parameter picker. The JUCE controls, not a Lua/GFX window,
// receive keyboard focus and expose their selected text through Windows UIA.
class TrackerPatternPanel final : public juce::Component, private juce::Timer {
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
 int page=1,tab=0,row=-1,cell=0,logicalCell=0,parameterIndex=0,paramHeight=8;
 std::array<int,3> lastLogicalCell{0,0,0};
 enum class NumericPurpose { value, patternNumber, stepNumber };
 NumericPurpose numericPurpose=NumericPurpose::value;
 std::array<int,5> jumpSizes{1,5,10,15,20};
 int jumpIndex=0;
 std::array<int,3> lastParam{0,0,0};
 bool refreshing=false;
 bool stepRecording=false,recordSustainDown=false;
 int recordingNextStep=0,lastRealStep=0;
 void timerCallback() override;
 void recordMidiStep(int midiNote,bool pause);
 void setStepRecording(bool enable);
 std::vector<const TrackerParam*> parameterList;
 std::map<int,std::vector<TrackerPattern>> undo,redo;
 TrackerCell clipboard{};
 bool hasClipboard=false;
 enum class ClipboardMode { none, parameter, steps, pattern };
 ClipboardMode clipboardMode=ClipboardMode::none;
 std::vector<TrackerStep> stepClipboard;
 TrackerPattern wholePatternClipboard{false};
 int selectionAnchor=-1,selectionEnd=-1;
 IncrementalParameterSearch parameterSearch;
 void clearSelection() noexcept { selectionAnchor=-1;selectionEnd=-1; }
 bool hasStepSelection() const noexcept {return selectionAnchor>=0 && selectionEnd>=0;}
 void extendStepSelection(int delta);
 void copySelectedSteps();
 void pasteSelectedSteps();
 void eraseSelectedSteps(bool cut);
 void resetCurrentStep();
 void clearCurrentSection();
 void openSelectedValueEditor();
 void openNumericEditor(NumericPurpose,const juce::String&,const juce::String&);
 void jumpToPattern(int);
 void jumpToStep(int);
 void jumpToNonemptyStep(int);
 void jumpToSameParameter(int);
 void resetCurrentCell();
 void setCurrentValueExtreme(bool);
 void resetWholePattern();
 void playFromCursor();
 void pauseAtAudibleStep();
 void auditionCurrentStep();
 TrackerPattern& currentPattern(){return processor.trackerPatterns.at(processor.trackerPatterns.activePattern);}
 const TrackerParam* currentParameter() const;
 TrackerCell* selectedCell();
 void refresh(bool speakPosition=false);
 void announce(const juce::String&);
 juce::String positionText(bool includePattern=false) const;
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
