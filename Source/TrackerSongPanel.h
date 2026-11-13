// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "TrackerSongParameters.h"
#include "TrackerSongEditing.h"
#include "IncrementalParameterSearch.h"
#include "ScreenReaderAnnouncer.h"
#include <array>
#include <functional>
#include <vector>

namespace biscot {
// F2 Song: native three-lane editing and first audible playback milestone.
// Standalone Song exports and advanced Song effects remain separate phases.
class TrackerSongPanel final:public juce::Component,private juce::Timer {
public:
 explicit TrackerSongPanel(BiscotProcessor&);
 void resized() override;
 void paint(juce::Graphics&) override;
 std::unique_ptr<juce::AccessibilityHandler> createAccessibilityHandler() override;
 bool keyPressed(const juce::KeyPress&) override;
 void open();
 void restoreAfterPreset();
 void suspendForLibrary();
 void announceLibraryResult(const juce::String&);
 void close();
 std::function<void()> onPattern, onExit;
 std::function<void()> onLibraryBrowser;
 std::function<void(bool)> onPresetBrowser;
private:
 struct Navigator:juce::ComboBox {
  explicit Navigator(TrackerSongPanel& p):owner(p){}
  bool keyPressed(const juce::KeyPress& k) override{return owner.handle(k,false);}
  TrackerSongPanel& owner;
 } navigator;
 struct Numeric:juce::TextEditor {
  explicit Numeric(TrackerSongPanel& p):juce::TextEditor("Song Numeric"),owner(p){}
  bool keyPressed(const juce::KeyPress& k) override{
   if(owner.handle(k,true))return true;
   const bool used=juce::TextEditor::keyPressed(k);
   if((owner.editPurpose==EditPurpose::saveSongName ||
       owner.editPurpose==EditPurpose::saveAllSongsName ||
       owner.editPurpose==EditPurpose::songName) &&
      k.getKeyCode()==32 && !k.getModifiers().isCtrlDown() && !k.getModifiers().isAltDown())
    announceToActiveScreenReader(*this,"spazio");
   return used;
  }
  TrackerSongPanel& owner;
 } numeric;
 BiscotProcessor& processor;
 juce::Label title,help;
 // lane 0..2 => operators, lane 3 => Song global.
 int lane=0,row=-1,cell=0,logicalCell=0,rememberedRow=0,lastOperatorLane=0;
 int pickerIndex=0;bool picker=false,refreshing=false;
 int lastObservedPlayingSong=-1;
 void timerCallback()override;
 void previewCurrentStep();
 TrackerSongHistory history;
 TrackerSongClipboard clipboard;
 int selectionAnchor=-1,selectionEnd=-1;
 int preferredSongParameter=-1;
 bool selectionActive()const{return selectionAnchor>=0&&selectionEnd>=0;}
 void clearSelection(){selectionAnchor=selectionEnd=-1;}
 std::pair<int,int> selectionBounds()const{return {std::min(selectionAnchor,selectionEnd),std::max(selectionAnchor,selectionEnd)};}
 void extendSelection(int delta);
 void copySelected(bool cut);
 void pasteClipboard();
 void clearSelected();
 void reorder(int direction);
 void undoRedo(bool redo);
 void mixVolume(int delta);
 void toggleMuteSolo(bool solo);
 void commitEdit(const TrackerSong& before);
 void renameSong();
 IncrementalParameterSearch search;
 std::array<int,3> lastStep{0,0,0};
 enum class EditPurpose { value, songNumber, stepNumber, songName, saveSongName, saveAllSongsName };
 EditPurpose editPurpose=EditPurpose::value;
 double valueNow() const;
 TrackerSong& song(){return processor.trackerSongs.at(processor.trackerSongs.activeSong);}
 const TrackerSong& song()const{return *processor.trackerSongs.find(processor.trackerSongs.activeSong);}
 int length()const{return song().laneLengths[(size_t)juce::jlimit(0,2,lane)];}
 bool realStep()const{return lane<3&&row>=0&&row<length();}
 bool speedRow()const{return lane<3&&row>length();}
 int visibleCount()const;
 int selectedParameter()const;
 void announce(const juce::String&);
 juce::String positionText(bool full=false)const;
 juce::String paramLabel(int id,double value)const;
 void refresh(bool speak=false);
 void changeLane(int next);
 void changeSong(int next);
 void vertical(int delta);
 void navigateArea(int direction);
 void currentValue(double value);
 void adjustValue(int direction,bool large=false);
 void extreme(bool high);
 void openNumeric(EditPurpose,const juce::String&,const juce::String&);
 void openCurrentNumeric();
 void insertSelectedParameter(bool remain);
 void eraseCurrent();
 bool handle(const juce::KeyPress&,bool editing);
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrackerSongPanel)
};
}
