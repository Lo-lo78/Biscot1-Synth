#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "IncrementalParameterSearch.h"
#include "ParameterSearchKey.h"
#include "PresetManager.h"
#include "PresetBrowser.h"
#include "TrackerPatternPanel.h"
#include "TrackerSongPanel.h"
#include "TrackerLibraryBrowser.h"
#include <array>
#include <vector>
#include <memory>

namespace biscot {
class BiscotEditor final : public juce::AudioProcessorEditor, private juce::Timer {
public:
 explicit BiscotEditor(BiscotProcessor&);
 ~BiscotEditor() override;
 void paint(juce::Graphics&) override;
 void resized() override;
 void focusGained(FocusChangeType) override;
 std::unique_ptr<juce::AccessibilityHandler> createAccessibilityHandler() override;
 bool keyPressed(const juce::KeyPress&) override;
private:
 enum class Control { grid, value, edit };
 struct ParameterGrid final : juce::ComboBox {
   explicit ParameterGrid(BiscotEditor& e) : owner(e) {}
   bool keyPressed(const juce::KeyPress& k) override {return owner.handleKey(k,Control::grid);}
   void focusGained(FocusChangeType cause) override;
   BiscotEditor& owner;
 } grid;
 struct ParameterValue final : juce::Slider {
   explicit ParameterValue(BiscotEditor& e) : owner(e) {}
   bool keyPressed(const juce::KeyPress& k) override {return owner.handleKey(k,Control::value);}
   void focusGained(FocusChangeType cause) override;
   BiscotEditor& owner;
 } value;
 struct NumericEdit final : juce::TextEditor {
   explicit NumericEdit(BiscotEditor& e) : juce::TextEditor("Edit"), owner(e) {}
   bool keyPressed(const juce::KeyPress& k) override {return owner.handleKey(k,Control::edit) || juce::TextEditor::keyPressed(k);}
   void focusGained(FocusChangeType cause) override;
   BiscotEditor& owner;
 } edit;
 BiscotProcessor& proc;
 PresetManager presetManager;
 std::unique_ptr<PresetBrowser> presetBrowser;
 std::unique_ptr<TrackerPatternPanel> trackerPanel;
 std::unique_ptr<TrackerSongPanel> songPanel;
 std::unique_ptr<TrackerLibraryBrowser> libraryBrowser;
 bool songBrowserReturn=false;
 bool lastTrackerWasSong=false;
 bool trackerBrowserReturn=false;
 Control presetReturnControl=Control::grid;
 int presetReturnGroup=0, presetReturnSelection=0;
 juce::Label title, sectionLabel, instructions;
 std::array<juce::String,12> groupNames {{"Global / MIDI", "Operator 1", "Operator 2", "Operator 3", "Filter", "LFO 1", "LFO 2", "LFO 3", "Arpeggiators", "Ring / Sync", "OSC / ENV Cross", "LFO Cross / Rate"}};
 std::array<std::vector<int>,12> groups;
 int group=0, selected=0;
 std::array<int,12> lastSelected{};
 std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
 bool updating=false, initialFocusPending=false;
 int focusRetries=0, stepScaleIndex=0;
 std::array<double,9> stepScales {{0.001,0.01,0.1,0.5,1,2,5,10,20}};
 IncrementalParameterSearch parameterSearch;
 juce::Rectangle<int> drawingArea;
 void timerCallback() override;
 void createGroups();
 int currentIndex() const;
 const ParameterDef& currentDef() const;
 juce::String formatValue(int) const;
 juce::String labelFor(int) const;
 void refreshLabels(bool force=false);
 void populateGroup();
 void selectedFromGrid();
 void select(int);
 void move(int rowStep,int colStep);
 void changeGroup(int);
 void setValue(double);
 void adjustValue(int,double);
 void boundary(bool);
 void changeStep(int);
 void resetValue();
 void enterEdit();
 bool commitEdit();
 void cancelEdit();
 void focus(Control,bool deferred=false);
 void initialFocus();
 void rememberLocation();
 void restoreLocation();
 void openPresetBrowser(bool save);
 void openTracker();
 void showSynthWidgets(bool showing);
 void focusBackFromPreset();
 bool handleKey(const juce::KeyPress&,Control);
 void announce(juce::Component&, const juce::String&) const;
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BiscotEditor)
};
}
