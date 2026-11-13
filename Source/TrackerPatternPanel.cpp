// SPDX-License-Identifier: AGPL-3.0-or-later
#include "TrackerPatternPanel.h"
#include "ScreenReaderAnnouncer.h"
#include <algorithm>
#include <cmath>
#include "TrackerManualNumeric.h"
#include <utility>
#include <string>
#include "TrackerShortcutKey.h"
#include "TrackerCellPresentation.h"
#include "TrackerCellNavigation.h"
#include "TrackerPatternInsert.h"
#include "TrackerStepRecording.h"
#include "TrackerHeaderNavigation.h"
#include "TrackerPatternText.h"
#include "TrackerLibraryFileNames.h"

namespace biscot {
namespace {
const juce::Colour fg(0xffe5edf6),bg(0xff171b26);
const char* const sectionNames[]{"OP","Filter","LFO"};
juce::String noteName(int n){
 static const char* names[]{"C","C sharp","D","D sharp","E","F","F sharp","G","G sharp","A","A sharp","B"};
 n=juce::jlimit(0,127,n);return juce::String(n)+" "+names[n%12]+" "+juce::String(n/12-1);
}
}
TrackerPatternPanel::TrackerPatternPanel(BiscotProcessor& p):navigator(*this),numeric(*this),processor(p){
 setFocusContainerType(juce::Component::FocusContainerType::keyboardFocusContainer);
 setWantsKeyboardFocus(true);
 header.setText("Biscot1 Tracker - F1 Patterns",juce::dontSendNotification);
 help.setText("F1 Pattern | Enter: parameters | Alt+E: edit value | Alt+Enter: Pattern | F2: Song | Space: play | Esc: back",juce::dontSendNotification);
 for(auto* label:{&header,&help}){label->setAccessible(false);label->setColour(juce::Label::textColourId,fg);addAndMakeVisible(*label);}
 navigator.setName("Tracker Grid");navigator.setDescription({});navigator.setWantsKeyboardFocus(true);
 navigator.setEditableText(false);navigator.setExplicitFocusOrder(1);
 navigator.onChange=[this]{if(!refreshing && navigator.getSelectedId()>0){
  if(page==1){
   const int requested=navigator.getSelectedId()-1;
   // A synthetic notification for the selected item of a newly populated
   // step is not a horizontal cursor move. Keep the Lua logical column.
   if(!trackerCellSelectionWasChanged(requested,cell))return;
   cell=requested;
   if(row>=0 && row<currentPattern().length){logicalCell=cell;lastLogicalCell[(size_t)tab]=logicalCell;}
   announce(positionText());}
  else {parameterIndex=navigator.getSelectedId()-1;announce(positionText());}
 }};
 addAndMakeVisible(navigator);
 numeric.setName("Tracker numeric edit");numeric.setDescription({});numeric.setMultiLine(false);
 numeric.setWantsKeyboardFocus(true);numeric.setExplicitFocusOrder(2);
 addAndMakeVisible(numeric);numeric.setVisible(false);
 readCursor();refreshParameterList();refresh();
}
std::unique_ptr<juce::AccessibilityHandler> TrackerPatternPanel::createAccessibilityHandler(){
 return createIgnoredAccessibilityHandler(*this);
}
void TrackerPatternPanel::paint(juce::Graphics& g){g.fillAll(bg);g.setColour(fg);g.drawRect(getLocalBounds(),1);}
void TrackerPatternPanel::resized(){
 auto bounds=getLocalBounds().reduced(22);
 header.setBounds(bounds.removeFromTop(35));
 help.setBounds(bounds.removeFromBottom(32));
 bounds.removeFromTop(10);
 navigator.setBounds(bounds.removeFromTop(34));
 bounds.removeFromTop(12);
 numeric.setBounds(bounds.removeFromTop(32));
}
void TrackerPatternPanel::announce(const juce::String& s){announceToActiveScreenReader(navigator,s);}
void TrackerPatternPanel::open(){
 startTimerHz(40);parameterSearch.reset();clearSelection();refresh();setVisible(true);toFront(false);
 juce::Component::SafePointer<TrackerPatternPanel> safe(this);
 juce::MessageManager::callAsync([safe]{if(safe!=nullptr && safe->isShowing()){
  safe->navigator.grabKeyboardFocus();safe->announce(safe->positionText(true));}});
}
void TrackerPatternPanel::suspendForLibrary(){
 stopTimer();setStepRecording(false);setVisible(false);
}
void TrackerPatternPanel::announceLibraryResult(const juce::String& s){announce(s);}
void TrackerPatternPanel::suspendForSong(){
 stopTimer();setStepRecording(false);processor.requestTrackerStop();setVisible(false);
}
void TrackerPatternPanel::restoreAfterPreset(){
 setStepRecording(false);patternExportFiles.clear();undo.clear();redo.clear();clearSelection();parameterSearch.reset();
 readCursor();refresh();
}
void TrackerPatternPanel::close(){
 stopTimer();setStepRecording(false);processor.requestTrackerStop();setVisible(false);if(onClose)onClose();
}
void TrackerPatternPanel::readCursor(){
 const auto& state=processor.state.state;
 page=juce::jlimit(1,2,(int)state.getProperty(juce::Identifier("trackerPatternPage"),1));
 tab=juce::jlimit(0,2,(int)state.getProperty(juce::Identifier("trackerPatternTab"),0));
 row=(int)state.getProperty(juce::Identifier("trackerPatternRow"),-1);
 cell=juce::jmax(0,(int)state.getProperty(juce::Identifier("trackerPatternCell"),0));
 logicalCell=juce::jmax(0,(int)state.getProperty(juce::Identifier("trackerPatternLogicalCell"),cell));
 lastLogicalCell[(size_t)tab]=logicalCell;
 lastRealStep=juce::jlimit(0,1023,(int)state.getProperty(juce::Identifier("trackerPatternLastStep"),0));
}
void TrackerPatternPanel::persistCursor(){
 auto& state=processor.state.state;
 state.setProperty(juce::Identifier("trackerPatternPage"),page,nullptr);
 state.setProperty(juce::Identifier("trackerPatternTab"),tab,nullptr);
 state.setProperty(juce::Identifier("trackerPatternRow"),row,nullptr);
 state.setProperty(juce::Identifier("trackerPatternCell"),cell,nullptr);
 state.setProperty(juce::Identifier("trackerPatternLogicalCell"),logicalCell,nullptr);
 state.setProperty(juce::Identifier("trackerPatternLastStep"),lastRealStep,nullptr);
}
void TrackerPatternPanel::refreshParameterList(){
 parameterList.clear();
 if(tab>0)for(const int shared:{3,4,6})if(const auto* d=findTrackerParam(shared))parameterList.push_back(d);
 for(const auto& p:trackerCatalog)if(p.tab==tab+1)parameterList.push_back(&p);
 parameterIndex=juce::jlimit(0,juce::jmax(0,(int)parameterList.size()-1),lastParam[(size_t)tab]);
}
const TrackerParam* TrackerPatternPanel::currentParameter() const{
 if(page!=2||parameterList.empty())return nullptr;
 return parameterList[(size_t)juce::jlimit(0,(int)parameterList.size()-1,parameterIndex)];
}
bool TrackerPatternPanel::parameterAlreadyInStep(int id) const{
 if(row<0||row>=currentPattern().length)return false;
 const auto& values=currentPattern().view(row,tab);
 return std::any_of(values.begin(),values.end(),[id](const TrackerCell& c){return c.id==id;});
}
TrackerCell* TrackerPatternPanel::selectedCell(){
 if(page!=1||row<0||row>=currentPattern().length)return nullptr;
 auto& values=currentPattern().cells(row,tab);
 if(cell<0||cell>=(int)values.size())return nullptr;
 return &values[(size_t)cell];
}
juce::String TrackerPatternPanel::cellText(const TrackerCell& c) const{
 // Enumerations must speak their actual choice names, as the main synth does.
 return juce::String(trackerCellLabel(c).c_str());
}
juce::String TrackerPatternPanel::positionText(bool includePattern) const{
 const juce::String prefix=includePattern?"Pattern "+juce::String(processor.trackerPatterns.activePattern)+". ":juce::String{};
 const int n=processor.trackerPatterns.activePattern;
 const auto& p=*processor.trackerPatterns.find(n);
 if(page==2){const auto* def=currentParameter();return "Parameters "+juce::String(sectionNames[tab])+". "+
  juce::String(def?(parameterAlreadyInStep(def->id)?"Set, ":""):"")+
  juce::String(def?def->name:"empty");}
 if(row==-1){return prefix+
  (cell==0?("Length, "+juce::String(p.length)):
   cell==1?("MIDI Note, "+(p.midiNote<0?juce::String("Off"):noteName(p.midiNote))):
   cell==2?("Step Recording, "+juce::String(stepRecording?"On":"Off")):
   ("Release Arpeggio, "+juce::String(p.releaseArp?"On":"Off")));}
 if(row==p.length)return prefix+"Repeat, "+juce::String(p.repeat);
 if(row>p.length){
  const int idx=(row-p.length-1)*5+cell/2;
  if(idx<0||idx>=p.repeat)return prefix+"Speed row "+juce::String(row-p.length);
  return prefix+(cell%2==0?"Speed ":"Transpose ")+juce::String(idx+1)+", "+
   (cell%2==0?juce::String(p.repeatSpeeds[(size_t)idx],2):juce::String(p.repeatTransposes[(size_t)idx]));
 }
 const auto& values=p.view(row,tab);
 return prefix+sectionNames[tab]+". step "+juce::String(row)+". "+
  (values.empty()?juce::String("empty"):cellText(values[(size_t)juce::jlimit(0,(int)values.size()-1,cell)]));
}
void TrackerPatternPanel::refresh(bool speakPosition){
 auto& pat=currentPattern();
 const int maxRow=pat.length+(pat.repeat+4)/5;
 row=juce::jlimit(-1,maxRow,row);
 refreshing=true;navigator.clear(juce::dontSendNotification);
 if(page==2){
  refreshParameterList();
  for(int i=0;i<(int)parameterList.size();++i){
   const auto* def=parameterList[(size_t)i];
   navigator.addItem(juce::String(parameterAlreadyInStep(def->id)?"Set, ":"")+def->name,i+1);
  }
  navigator.setSelectedId(parameterIndex+1,juce::dontSendNotification);
  header.setText("Biscot1 Tracker - "+juce::String(sectionNames[tab])+" parameter picker",juce::dontSendNotification);
 }else{
  header.setText("Biscot1 Tracker - F1 Pattern "+juce::String(processor.trackerPatterns.activePattern)+" / "+sectionNames[tab],juce::dontSendNotification);
  if(row==-1){
   navigator.addItem("Length, "+juce::String(pat.length),1);
   navigator.addItem("MIDI Note, "+(pat.midiNote<0?juce::String("Off"):noteName(pat.midiNote)),2);
   navigator.addItem("Step Recording, "+juce::String(stepRecording?"On":"Off"),3);
   navigator.addItem("Release Arpeggio, "+juce::String(pat.releaseArp?"On":"Off"),4);
  }else if(row==pat.length){
   navigator.addItem("Repeat, "+juce::String(pat.repeat),1);
  }else if(row>pat.length){
   const int start=(row-pat.length-1)*5;
   for(int i=start;i<juce::jmin(start+5,pat.repeat);++i){
    const double speed=i<(int)pat.repeatSpeeds.size()?pat.repeatSpeeds[(size_t)i]:1.0;
    const int trans=i<(int)pat.repeatTransposes.size()?pat.repeatTransposes[(size_t)i]:0;
    navigator.addItem("Speed "+juce::String(i+1)+", "+juce::String(speed,2),2*(i-start)+1);
    navigator.addItem("Transpose "+juce::String(i+1)+", "+juce::String(trans),2*(i-start)+2);
   }
  }else{
   const auto& a=pat.view(row,tab);
   if(a.empty())navigator.addItem("step "+juce::String(row)+". empty",1);
   else for(int i=0;i<(int)a.size();++i)navigator.addItem("step "+juce::String(row)+". "+cellText(a[(size_t)i]),i+1);
  }
  if(row>=0 && row<pat.length)
   cell=trackerVisibleCell(logicalCell,navigator.getNumItems());
  else
   cell=juce::jlimit(0,juce::jmax(0,navigator.getNumItems()-1),cell);
  navigator.setSelectedId(cell+1,juce::dontSendNotification);
 }
 refreshing=false;
 persistCursor();
 repaint();if(speakPosition)announce(positionText());
}
// A named, independent library export. This never modifies the .biscot preset.
void TrackerPatternPanel::saveCurrentPattern(const juce::String& requestedName){
 const int slot=processor.trackerPatterns.activePattern;
 auto baseName=trackerLibraryFileStem(requestedName);
 if(baseName.endsWithIgnoreCase(".biscot1-pattern"))
  baseName=trackerLibraryFileStem(baseName.dropLastCharacters(16));
 if(baseName.isEmpty()){announce("Enter a Pattern name");return;}
 const auto root=juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                  .getChildFile("Biscot1-Synth");
 const auto singles=root.getChildFile("patterns");
 const auto songs=root.getChildFile("songs");
 const auto patternDir=singles.createDirectory();
 const auto songDir=songs.createDirectory();
 if(patternDir.failed() || songDir.failed()){
  announce("Cannot create Biscot1 Pattern/Song library folders");return;
 }
 const auto payload=serializeTrackerPattern(currentPattern(),slot);
 // Each save creates a new file. In particular, resaving the same Pattern
 // name must never silently replace an earlier library snapshot on disk.
 // The Song and full preset browsers use explicit Yes/No for destructive loads;
 // standalone Pattern exports simply choose a duplicate-safe suffix.
 juce::File file;
 for(int suffix=1;suffix<=9999;++suffix){
  const auto name=baseName+(suffix==1?juce::String{}:"_"+juce::String(suffix))+".biscot1-pattern";
  const auto candidate=singles.getChildFile(name);
  if(!candidate.exists()){file=candidate;break;}
 }
 if(file==juce::File{}){announce("Pattern library has too many duplicate names");return;}
 if(!file.existsAsFile() || file.loadFileAsString().toStdString()!=payload){
  juce::TemporaryFile temporary(file);
  if(!temporary.getFile().replaceWithText(juce::String::fromUTF8(payload.c_str())) ||
     !temporary.overwriteTargetFileWithTemporary()){
   announce("Could not save Pattern "+juce::String(slot));return;
  }
 }
 patternExportFiles[slot]=file;
 announce("Pattern "+juce::String(slot)+" saved to patterns, "+file.getFileName());
}
void TrackerPatternPanel::recordUndo(){
 const int n=processor.trackerPatterns.activePattern;
 auto& stack=undo[n];if(stack.size()>=128)stack.erase(stack.begin());stack.push_back(currentPattern());redo[n].clear();
}
void TrackerPatternPanel::finishEdit(){processor.commitTrackerState();refresh(true);}
void TrackerPatternPanel::changeValue(double v){
 auto& p=currentPattern();
 if(page!=1)return;
 if(row==-1){
  if(cell==2){setStepRecording(v>0.5);refresh(true);return;}
  recordUndo();
  if(cell==0)p.length=juce::jlimit(1,1024,(int)std::round(v));
  else if(cell==1)p.midiNote=juce::jlimit(-1,127,(int)std::round(v));
  else p.releaseArp=v>0.5;
  finishEdit();return;
 }
 if(row==p.length){recordUndo();p.repeat=juce::jlimit(1,1000,(int)std::round(v));p.repeatSpeeds.resize((size_t)p.repeat,1.0);p.repeatTransposes.resize((size_t)p.repeat,0);finishEdit();return;}
 if(row>p.length){const int idx=(row-p.length-1)*5+cell/2;if(idx>=0&&idx<p.repeat){recordUndo();if(cell%2==0)p.repeatSpeeds[(size_t)idx]=juce::jlimit(0.25,16.0,v);else p.repeatTransposes[(size_t)idx]=juce::jlimit(-96,96,(int)std::round(v));finishEdit();}return;}
 if(auto* c=selectedCell()){
  if(const auto* d=findTrackerParam(c->id)){
   const double adjusted=juce::jlimit(d->min,d->max,v);
   // Exactly like Lua change_value(): no sound or undo entry at a value boundary.
   if(std::abs(c->value-adjusted)<1.0e-12)return;
   recordUndo();c->value=adjusted;finishEdit();
   // Publish the fresh immutable snapshot first, then audition the edited
   // Note/Gate/OP/Filter/LFO value through the existing audio-thread clock.
   auditionCurrentStep();
  }
 }
}
void TrackerPatternPanel::adjustValue(int direction,double multiplier){
 if(page!=1)return;
 const auto& p=currentPattern();double old=0,step=1;
 if(row==-1){old=cell==0?p.length:cell==1?p.midiNote:cell==2?double(stepRecording):double(p.releaseArp);}
 else if(row==p.length){old=p.repeat;}
 else if(row>p.length){const int idx=(row-p.length-1)*5+cell/2;if(idx<0||idx>=p.repeat)return;
  if(cell%2==0){old=p.repeatSpeeds[(size_t)idx];step=0.25;}else old=p.repeatTransposes[(size_t)idx];}
 else if(auto* c=selectedCell()){old=c->value;if(auto* def=findTrackerParam(c->id))step=def->step;}
 else return;
 changeValue(old+direction*multiplier*step);
}
void TrackerPatternPanel::setTab(int next){if(next<0||next>=3||next==tab)return;
 parameterSearch.reset();clearSelection();
 if(page==2)lastParam[(size_t)tab]=parameterIndex;
 if(page==1 && row>=0 && row<currentPattern().length)
  lastLogicalCell[(size_t)tab]=logicalCell;
 tab=next;
 if(row>=0 && row<currentPattern().length)
  logicalCell=lastLogicalCell[(size_t)tab];
 else cell=0;
 refresh(true);
}
void TrackerPatternPanel::changeRow(int delta){
 const auto& pat=currentPattern();
 // Respect the three independent F1 regions. Never leave Steps through
 // an arrow or a Page key, and never enter Steps from Repeat/Speed.
 const int to=trackerMoveWithinF1Region(row,delta,pat.length,pat.repeat);
 if(to==row)return; // Silent border, as in other accessible grids.
 clearSelection();row=to;
 if(row>=0&&row<pat.length)lastRealStep=row;
 refresh(true);auditionCurrentStep();
}
// Pattern step selection follows TrackerControl 0.166: the first Shift+arrow
// selects the current step, subsequent Shift+arrows extend the inclusive range.
void TrackerPatternPanel::extendStepSelection(int delta){
 auto& pat=currentPattern();
 if(row<0)row=0;
 else if(row>=pat.length)row=pat.length-1;
 if(!hasStepSelection()){selectionAnchor=row;selectionEnd=row;}
 else{
  const int next=juce::jlimit(0,pat.length-1,selectionEnd+delta);
  if(next==selectionEnd)return;
  selectionEnd=next;row=next;
 }
 refresh();
 const int lo=juce::jmin(selectionAnchor,selectionEnd);
 const int hi=juce::jmax(selectionAnchor,selectionEnd);
 announce(lo==hi?"Step "+juce::String(lo)+" selected":
  "Steps "+juce::String(lo)+" to "+juce::String(hi)+" selected");
}
void TrackerPatternPanel::copySelectedSteps(){
 if(!hasStepSelection())return;
 stepClipboard.clear();
 const int lo=juce::jmin(selectionAnchor,selectionEnd);
 const int hi=juce::jmax(selectionAnchor,selectionEnd);
 const auto& pat=currentPattern();
 for(int st=lo;st<=hi;++st){
  const auto i=pat.steps.find(st);
  stepClipboard.push_back(i==pat.steps.end()?TrackerStep{}:i->second);
 }
 clipboardMode=ClipboardMode::steps;
 announce(stepClipboard.size()==1?"One step copied":juce::String((int)stepClipboard.size())+" steps copied");
}
void TrackerPatternPanel::pasteSelectedSteps(){
 if(stepClipboard.empty() || row<0 || row>=currentPattern().length)return;
 auto& pat=currentPattern();
 const int n=juce::jmin((int)stepClipboard.size(),pat.length-row);
 if(n<=0)return;
 recordUndo();
 // Safe even when the source and destination belong to the same Pattern:
 // the clipboard owns complete, independent snapshots of all three sections.
 for(int i=0;i<n;++i)pat.steps[row+i]=stepClipboard[(size_t)i];
 clearSelection();cell=0;logicalCell=0;lastLogicalCell[(size_t)tab]=0;processor.commitTrackerState();refresh();
 announce(n==1?"One step pasted":juce::String(n)+" steps pasted");
}
void TrackerPatternPanel::eraseSelectedSteps(bool cut){
 if(!hasStepSelection())return;
 const int lo=juce::jmin(selectionAnchor,selectionEnd);
 const int hi=juce::jmax(selectionAnchor,selectionEnd);
 if(cut)copySelectedSteps();
 recordUndo();
 auto& pat=currentPattern();
 for(int st=lo;st<=hi;++st)pat.steps.erase(st);
 clearSelection();cell=0;logicalCell=0;lastLogicalCell[(size_t)tab]=0;processor.commitTrackerState();refresh();
 const int n=hi-lo+1;
 announce(juce::String(n==1?"One step ":"Steps ")+juce::String(cut?"cut":"deleted")+
  (n==1?juce::String():" ("+juce::String(n)+")"));
}
void TrackerPatternPanel::resetCurrentStep(){
 if(row<0 || row>=currentPattern().length)return;
 recordUndo();
 auto& pat=currentPattern();
 // Lua initializes the OP row with the same eight starting cells.
 pat.steps[row].tabs[(size_t)tab].clear();
 if(tab==0){
  const TrackerPattern defaults;
  pat.steps[row].tabs[0]=defaults.view(0,0);
 }
 clearSelection();cell=0;logicalCell=0;lastLogicalCell[(size_t)tab]=0;finishEdit();announce("Step "+juce::String(row)+" "+sectionNames[tab]+" initialized");
}
void TrackerPatternPanel::clearCurrentSection(){
 recordUndo();
 auto& pat=currentPattern();
 for(auto it=pat.steps.begin();it!=pat.steps.end();){
  it->second.tabs[(size_t)tab].clear();
  if(it->second.tabs[0].empty() && it->second.tabs[1].empty() && it->second.tabs[2].empty())
   it=pat.steps.erase(it);
  else ++it;
 }
 clearSelection();cell=0;logicalCell=0;lastLogicalCell[(size_t)tab]=0;finishEdit();announce(juce::String(sectionNames[tab])+" table cleared");
}
void TrackerPatternPanel::openSelectedValueEditor(){
 if(page!=1)return; // Numeric editing is Alt+E in the Pattern grid.
 const auto& pat=currentPattern();
 double value=0;
 juce::String name;
 if(row==-1){
  if(cell==0){value=pat.length;name="Length";}
  else if(cell==1){value=pat.midiNote;name="MIDI Note";}
  else if(cell==2){value=stepRecording?1:0;name="Step Recording (0 Off, 1 On)";}
  else {value=pat.releaseArp?1:0;name="Release Arpeggio (0 Off, 1 On)";}
 }else if(row==pat.length){value=pat.repeat;name="Repeat";}
 else if(row>pat.length){
  const int idx=(row-pat.length-1)*5+cell/2;
  if(idx<0 || idx>=pat.repeat){announce("No parameter to edit");return;}
  if(cell%2==0){value=pat.repeatSpeeds[(size_t)idx];name="Speed "+juce::String(idx+1);}
  else {value=pat.repeatTransposes[(size_t)idx];name="Transpose "+juce::String(idx+1);}
 }else{
  const auto* selected=selectedCell();
  if(selected==nullptr){announce("Empty step. Press Enter to add a parameter");return;}
  value=selected->value;
  const auto* definition=findTrackerParam(selected->id);
  name=definition!=nullptr?juce::String(definition->name):juce::String("Parameter");
 }
 openNumericEditor(NumericPurpose::value,"Edit "+name,juce::String(value,4));
}
void TrackerPatternPanel::openNumericEditor(NumericPurpose purpose,const juce::String& label,
                                             const juce::String& initial){
 numericPurpose=purpose;
 numeric.setName(label);
 numeric.setText(initial,juce::dontSendNotification);
 numeric.setVisible(true);
 numeric.grabKeyboardFocus();numeric.selectAll();
}
void TrackerPatternPanel::jumpToPattern(int target){
 if(target<0||target>=TrackerPatternBank::maxPatterns){announce("Invalid pattern number");return;}
 auto& bank=processor.trackerPatterns;
 if(target==bank.activePattern){announce("Pattern "+juce::String(target));return;}
 const int oldLength=currentPattern().length;
 const int oldRow=row,oldCell=cell;
 clearSelection();bank.activePattern=target;
 const auto& replacement=currentPattern();
 if(oldRow<0)row=-1;
 else if(oldRow<oldLength)row=juce::jlimit(0,replacement.length-1,oldRow);
 else if(oldRow==oldLength)row=replacement.length;
 else row=juce::jlimit(replacement.length+1,replacement.length+(replacement.repeat+4)/5,replacement.length+oldRow-oldLength);
 cell=oldCell;logicalCell=cell;lastRealStep=juce::jlimit(0,replacement.length-1,lastRealStep);
 processor.commitTrackerState();
 if(page==2){refresh(false);announce("Pattern "+juce::String(target)+". "+positionText());}
 else {refresh(false);announce(positionText(true));}
}
void TrackerPatternPanel::jumpToStep(int target){
 if(target<0||target>=currentPattern().length){announce("Invalid step number");return;}
 clearSelection();row=target;lastRealStep=row;refresh(true);auditionCurrentStep();
}
void TrackerPatternPanel::jumpToSameParameter(int direction){
 if(row<0 || row>=currentPattern().length){announce("Select a step parameter first");return;}
 const auto* selected=selectedCell();
 if(selected==nullptr){announce("No parameter in this step");return;}
 const int parameterId=selected->id;
 const auto match=trackerFindMatchingParameter(currentPattern(),row,tab,parameterId,direction);
 if(match.first<0){
  const auto* d=findTrackerParam(parameterId);
  announce(juce::String("No ")+(direction<0?"previous":"next")+
           " occurrence of "+juce::String(d?d->name:"parameter"));
  return;
 }
 clearSelection();row=match.first;lastRealStep=row;cell=match.second;
 logicalCell=cell;lastLogicalCell[(size_t)tab]=logicalCell;
 refresh(true);auditionCurrentStep();
}
void TrackerPatternPanel::jumpToNonemptyStep(int direction){
 if(row<0||row>=currentPattern().length)return;
 const auto& p=currentPattern();
 for(int next=row+direction;next>=0&&next<p.length;next+=direction){
  const auto it=p.steps.find(next);
  if(it!=p.steps.end() && (!it->second.tabs[0].empty() || !it->second.tabs[1].empty()
                           || !it->second.tabs[2].empty())){
   clearSelection();row=next;refresh(true);return;
  }
 }
}
void TrackerPatternPanel::resetCurrentCell(){
 if(page!=1)return;
 const auto& pattern=currentPattern();
 // Backspace is the factory/default-value shortcut for every F1 cell,
 // including header and repeat fields, not a destructive Delete.
 if(row<0){changeValue(cell==0?16.:cell==1?-1.:0.);return;}
 if(row==pattern.length){changeValue(1.);return;}
 if(row>pattern.length){changeValue(cell%2==0?1.:0.);return;}
 auto* c=selectedCell();if(!c)return;
 const auto* def=findTrackerParam(c->id);if(!def)return;
 if(c->value==def->initial)return;
 recordUndo();c->value=def->initial;finishEdit();
 // Resetting a parameter with Backspace is an audible value edit too.
 auditionCurrentStep();
}
void TrackerPatternPanel::setCurrentValueExtreme(bool maximum){
 if(row<0){changeValue(cell==0?(maximum?1024.0:1.0):cell==1?(maximum?127.0:-1.0):(maximum?1.0:0.0));return;}
 const auto& p=currentPattern();
 if(row==p.length){changeValue(maximum?1000:1);return;}
 if(row>p.length){changeValue(cell%2==0?(maximum?16.0:0.25):(maximum?96.0:-96.0));return;}
 if(const auto* c=selectedCell())if(const auto* def=findTrackerParam(c->id))
  changeValue(maximum?def->max:def->min);
}
void TrackerPatternPanel::auditionCurrentStep(){
 if(page!=1 || row<0 || row>=currentPattern().length)return;
 // As in the Lua tracker, visiting an actual step auditions just that step;
 // do not interrupt an intentionally running full Pattern.
 const int status=processor.trackerPlaybackStatus.load(std::memory_order_relaxed);
 if(trackerCanAuditionStep(currentPattern(),row,status))
  processor.requestTrackerStart(row,true);
}
void TrackerPatternPanel::playFromCursor(){
 const int status=processor.trackerPlaybackStatus.load(std::memory_order_relaxed);
 if(status==1){
  processor.requestTrackerStop();announce("Pattern stopped");return;
 }
 const int step=(row>=0 && row<currentPattern().length)?row:0;
 processor.requestTrackerStart(step,false);
 announce("Pattern "+juce::String(processor.trackerPatterns.activePattern)+
          " playing from step "+juce::String(step));
}
void TrackerPatternPanel::pauseAtAudibleStep(){
 const int status=processor.trackerPlaybackStatus.load(std::memory_order_relaxed);
 if(status!=1 && status!=3){announce("Pattern not playing");return;}
 processor.requestTrackerPause();
 // The audio callback consumes the pause request at the next block boundary.
 // Update the edit cursor from the ACTUAL audio step, not a guessed next step.
 juce::Component::SafePointer<TrackerPatternPanel> safe(this);
 juce::Timer::callAfterDelay(65,[safe]{
  if(safe==nullptr || !safe->isShowing() ||
     safe->processor.trackerPlaybackStatus.load(std::memory_order_relaxed)!=2)return;
  const int step=safe->processor.trackerAudibleStep.load(std::memory_order_relaxed);
  if(step>=0 && step<safe->currentPattern().length){
   safe->row=step;safe->clearSelection();safe->page=1;
   safe->refresh(false);
   safe->announce("Paused at step "+juce::String(step));
  }else safe->announce("Pattern paused");
 });
}
void TrackerPatternPanel::resetWholePattern(){
 recordUndo();currentPattern()=TrackerPattern{};
 clearSelection();row=-1;cell=0;logicalCell=0;lastLogicalCell={0,0,0};finishEdit();announce("Pattern initialized");
}
bool TrackerPatternPanel::keyPressed(const juce::KeyPress& k){return handle(k,false);}
bool TrackerPatternPanel::handle(const juce::KeyPress& k,bool editing){
 const auto m=k.getModifiers();const int code=k.getKeyCode();
 const auto letter=[&](char c){return trackerShortcutLetterMatches(
  code,(int)k.getTextCharacter(),c,m.isCtrlDown());};
 if(m.isAltDown()&&!m.isCtrlDown()&&!m.isShiftDown()&&letter('a')){if(onAbout)onAbout();return true;}
 if(editing){
  if(code==juce::KeyPress::returnKey){
   const auto txt=numeric.getText().trim();
   const auto purpose=numericPurpose;
   if(purpose==NumericPurpose::savePatternName && txt.isEmpty()){
    announceToActiveScreenReader(numeric,"Enter a Pattern file name");return true;
   }
   numeric.setVisible(false);navigator.grabKeyboardFocus();
   if(purpose==NumericPurpose::savePatternName){saveCurrentPattern(txt);return true;}
   if(purpose==NumericPurpose::value){
    if(txt.isNotEmpty()){
     double newValue=0;
     if(parseTrackerManualNumeric(txt.toStdString(),newValue))changeValue(newValue);
     else announce("Invalid numeric value");
    }
   }else if(txt.isNotEmpty() && [&]{
    const auto digits=txt.toStdString();
    return std::all_of(digits.begin(),digits.end(),
                       [](unsigned char d){return d>='0'&&d<='9';});
   }()){
    const int n=txt.getIntValue();
    if(purpose==NumericPurpose::patternNumber)jumpToPattern(n);
    else jumpToStep(n);
   }else if(txt.isNotEmpty())announce(purpose==NumericPurpose::patternNumber?
     "Invalid pattern number":"Invalid step number");
   if(shortcutEdit){shortcutEdit=false;row=shortcutRow;cell=shortcutCell;logicalCell=cell;refresh(false);}
   return true;
  }
  if(code==juce::KeyPress::escapeKey){numeric.setVisible(false);navigator.grabKeyboardFocus();
   if(shortcutEdit){shortcutEdit=false;row=shortcutRow;cell=shortcutCell;logicalCell=cell;refresh(false);}return true;}
  return false;
 }
 // Inside the parameter picker, Esc returns to the Pattern editor; a second
 // Esc leaves Alt+Q. Alt+Q always returns directly to the synthesizer.
 if(code==juce::KeyPress::escapeKey && page==2){page=1;parameterSearch.reset();refresh(true);return true;}
 if(code==juce::KeyPress::escapeKey || (m.isAltDown() && letter('q'))){parameterSearch.reset();clearSelection();close();return true;}
 if(m.isCtrlDown() && !m.isAltDown() && !m.isShiftDown() && letter('s')){
  openNumericEditor(NumericPurpose::savePatternName,
   "Save Pattern file name. Enter to save; Escape to cancel",
   "Pattern_"+juce::String(processor.trackerPatterns.activePattern).paddedLeft('0',3));
  return true;
 }
 if(m.isCtrlDown() && !m.isAltDown() && !m.isShiftDown() && letter('o')){
  if(onLibraryBrowser)onLibraryBrowser();else announce("Library browser unavailable");
  return true;
 }
 if(m.isAltDown() && !m.isCtrlDown() && letter('s')){if(onPresetBrowser)onPresetBrowser(true);return true;}
 if(m.isAltDown() && !m.isCtrlDown() && letter('b'))return true; // disabled and silent inside Alt+Q
 if(code==juce::KeyPress::F1Key){page=1;parameterSearch.reset();refresh(true);return true;}
 if(code==32){
  if(m.isCtrlDown() && !m.isAltDown()){pauseAtAudibleStep();return true;}
  if(!m.isCtrlDown() && !m.isAltDown() && !m.isShiftDown() && page==1){playFromCursor();return true;}
  return true;
 }
 // F2 is reserved for the forthcoming Song page. The old F2 parameter
 // picker is now opened with Enter from an actual Pattern step.
 if(code==juce::KeyPress::F2Key){if(onOpenSong)onOpenSong();return true;}
 if(code>=juce::KeyPress::F3Key&&code<=juce::KeyPress::F6Key){announce("Song, browser and diagnostics arrive in later tracker versions");return true;}
 if(m.isCtrlDown()&&code==juce::KeyPress::tabKey&&page==2){setTab((tab+(m.isShiftDown()?2:1))%3);return true;}
 if(!m.isCtrlDown() && code==juce::KeyPress::tabKey){
  // Lua F1: Tab speaks the full row/section/parameter position and never
  // transfers keyboard focus out of the tracker editor.
  announce(positionText(true));return true;
 }
 if(page==1 && m.isCtrlDown() && code==juce::KeyPress::tabKey)return true;
 // Alt+Enter preserves the old Enter shortcut for choosing Pattern 0..999.
 // It also works in the parameter picker without replacing its Enter-to-insert action.
 if(m.isAltDown() && !m.isCtrlDown() && !m.isShiftDown() && code==juce::KeyPress::returnKey){
  openNumericEditor(NumericPurpose::patternNumber,"Go to Pattern 0 to 999",
                    juce::String(processor.trackerPatterns.activePattern));return true;
 }
 if(page==2){
  if(m.isCtrlDown() && code==juce::KeyPress::homeKey){
   parameterSearch.reset();parameterIndex=0;lastParam[(size_t)tab]=parameterIndex;refresh(true);return true;
  }
  if(m.isCtrlDown() && code==juce::KeyPress::endKey){
   parameterSearch.reset();parameterIndex=juce::jmax(0,(int)parameterList.size()-1);
   lastParam[(size_t)tab]=parameterIndex;refresh(true);return true;
  }
  const int count=(int)parameterList.size();if(count==0)return true;
  if(biscotParameterSearchCharacter(k)==0)parameterSearch.reset();
  int move=0;
  if(code==juce::KeyPress::upKey)move=-1;
  if(code==juce::KeyPress::downKey)move=1;
  if(code==juce::KeyPress::leftKey)move=-paramHeight;
  if(code==juce::KeyPress::rightKey)move=paramHeight;
  if(code==juce::KeyPress::pageUpKey)move=-5;
  if(code==juce::KeyPress::pageDownKey)move=5;
  if(m.isCtrlDown() && code==juce::KeyPress::leftKey){paramHeight=juce::jmax(1,paramHeight-1);announce("Grid "+juce::String(paramHeight));return true;}
  if(m.isCtrlDown() && code==juce::KeyPress::rightKey){paramHeight=juce::jmin(100,paramHeight+1);announce("Grid "+juce::String(paramHeight));return true;}
  if(code==juce::KeyPress::homeKey){move=-parameterIndex%paramHeight;}
  if(code==juce::KeyPress::endKey){move=juce::jmin(count-1-parameterIndex,paramHeight-1-parameterIndex%paramHeight);}
  if(move!=0){const int next=juce::jlimit(0,count-1,parameterIndex+move);if(next!=parameterIndex){parameterIndex=next;lastParam[(size_t)tab]=next;refresh(true);}return true;}
  if(code==juce::KeyPress::returnKey){
   if(const auto* def=currentParameter()){
    if(row<0 || row>=currentPattern().length){announce("Select a pattern step first");return true;}
    auto& pat=currentPattern();
    // A cheap scratch copy avoids adding an Undo entry if the command is
    // already present, invalid, or the row has reached its cell limit.
    TrackerPattern candidate=pat;
    const auto test=insertTrackerParameter(candidate,row,tab,*def,cell);
    if(test.status==TrackerInsertStatus::needsArpeggiator){announce("Sub Note requires OP Arp Mode");return true;}
    if(test.status==TrackerInsertStatus::full){announce("Step is full, 128 cells");return true;}
    if(test.status==TrackerInsertStatus::invalid){announce("Select a pattern step first");return true;}
    if(test.status==TrackerInsertStatus::inserted){
     recordUndo();pat=std::move(candidate);processor.commitTrackerState();
    }
    cell=test.index;logicalCell=cell;lastLogicalCell[(size_t)tab]=logicalCell;
    if(!m.isCtrlDown()){page=1;refresh(false);}else refresh(false);
    announce(juce::String(def->name)+(test.status==TrackerInsertStatus::inserted?" inserted":" already present"));
   }
   return true;
  }
  // Multi-character parameter picker search, with the same whole-label/word-prefix,
  // Shift-reverse and repeated-letter cycling used by Biscot1's main grid.
  // Words such as "Square", "Rate", "Cutoff" match even after LFO1/Filter.
  if(const auto ch=biscotParameterSearchCharacter(k);ch!=0){
   const int found=parameterSearch.next(
     (wchar_t)ch,parameterIndex,count,m.isShiftDown()?-1:1,
     juce::Time::getMillisecondCounter(),[this](int idx){
      return std::wstring(juce::String(parameterList[(size_t)idx]->name).toWideCharPointer());
     });
   if(found>=0){
    parameterIndex=found;lastParam[(size_t)tab]=found;refresh(true);
   }
   return true;
  }
  parameterSearch.reset();
  return true;
 }
 // Enter on a Pattern step opens the parameter list; Alt+E is the sole
 // numeric editor shortcut, also on Length / MIDI Note / Repeat / Speed.
 if(!m.isCtrlDown() && !m.isAltDown() && !m.isShiftDown() && code==juce::KeyPress::returnKey){
  if(row>=0 && row<currentPattern().length){
   clearSelection();page=2;parameterSearch.reset();refresh(true);
  }else announce("Select a Pattern step to add a parameter. Alt+E edits this value");
  return true;
 }
 if(m.isCtrlDown() && !m.isAltDown() && !m.isShiftDown() && letter('j')){
  openNumericEditor(NumericPurpose::stepNumber,
   "Go to step 0 to "+juce::String(currentPattern().length-1),
   juce::String(juce::jlimit(0,currentPattern().length-1,row)));
  return true;
 }
 // Native extension to Lua F1: jump to the next/previous step containing
 // exactly the same parameter (same ID, current OP/Filter/LFO section).
 // This is distinct from Ctrl+PageUp/PageDown, which visits ANY nonempty step.
 if(m.isCtrlDown() && m.isAltDown() && !m.isShiftDown() &&
    (code==juce::KeyPress::upKey || code==juce::KeyPress::downKey)){
  jumpToSameParameter(code==juce::KeyPress::upKey?-1:1);return true;
 }
 if(m.isCtrlDown() && !m.isAltDown() && !m.isShiftDown() && code==juce::KeyPress::pageUpKey){
  jumpToNonemptyStep(-1);return true;
 }
 if(m.isCtrlDown() && !m.isAltDown() && !m.isShiftDown() && code==juce::KeyPress::pageDownKey){
  jumpToNonemptyStep(1);return true;
 }
 if(m.isAltDown() && !m.isCtrlDown() && !m.isShiftDown() && code==juce::KeyPress::leftKey){
  if(jumpIndex>0){--jumpIndex;announce("Jump "+juce::String(jumpSizes[(size_t)jumpIndex]));}return true;
 }
 if(m.isAltDown() && !m.isCtrlDown() && !m.isShiftDown() && code==juce::KeyPress::rightKey){
  if(jumpIndex<(int)jumpSizes.size()-1){++jumpIndex;announce("Jump "+juce::String(jumpSizes[(size_t)jumpIndex]));}return true;
 }
 if(m.isCtrlDown() && m.isShiftDown() && m.isAltDown() && letter('c')){
  wholePatternClipboard=currentPattern();clipboardMode=ClipboardMode::pattern;
  announce("Entire Pattern copied");return true;
 }
 if(m.isCtrlDown() && m.isShiftDown() && m.isAltDown() && code==juce::KeyPress::deleteKey){
  resetWholePattern();return true;
 }
 if(!m.isCtrlDown() && !m.isAltDown() && !m.isShiftDown() &&
    code==juce::KeyPress::backspaceKey){resetCurrentCell();return true;}
 if(m.isShiftDown() && !m.isCtrlDown() && !m.isAltDown()){
  if(code==juce::KeyPress::upKey){extendStepSelection(-1);return true;}
  if(code==juce::KeyPress::downKey){extendStepSelection(1);return true;}
  if(code==juce::KeyPress::pageUpKey){extendStepSelection(-5);return true;}
  if(code==juce::KeyPress::pageDownKey){extendStepSelection(5);return true;}
 }
 // Lua F1: unmodified 1/2/3 switches OP / Filter / LFO sections.
 // These are NUMBER keys, so do not call the Ctrl+letter helper here.
 // The parameter picker intentionally retains 1/2/3 for incremental search.
 if(!m.isAltDown() && !m.isCtrlDown() && !m.isShiftDown()){
  const int section=trackerShortcutSectionIndex(code,(int)k.getTextCharacter());
  if(section>=0){
   // The three operator/filter/LFO sections are selected only from Steps.
   // Length, Repeat and Speed/Transpose are shared across all three.
   if(page==1 && row>=0 && row<currentPattern().length)setTab(section);
   return true;
  }
 }
 // F1 quick navigation to Pattern header/repeat cells.
 // D/T cycle forward; Shift+D/T cycle backward across every repeat row.
 // These are plain letter shortcuts only: Alt+E and Ctrl combinations remain untouched.
 if(!m.isAltDown() && !m.isCtrlDown()){
  for(const char command : {'l','m','s','r','p','d','t'}){
   if(letter(command)){
    const auto target=trackerHeaderShortcut(command,m.isShiftDown(),
      currentPattern().length,currentPattern().repeat,row,cell);
    if(target.valid){
     shortcutRow=row;shortcutCell=cell;shortcutEdit=true;
     row=target.row;cell=target.cell;
     refresh(false);openSelectedValueEditor();
     if(!numeric.isVisible()){shortcutEdit=false;row=shortcutRow;cell=shortcutCell;refresh(false);}
    }
    return true;
   }
  }
 }
 // Unified F1/F2 tracker navigation:
 // Ctrl+Up/Down moves among Header, remembered Steps and Repeat/Speed;
 // Ctrl+Shift+Up/Down changes the active Pattern without moving the cursor.
 if(m.isCtrlDown() && !m.isAltDown() &&
    (code==juce::KeyPress::upKey||code==juce::KeyPress::downKey)){
  if(m.isShiftDown()){
   auto& bank=processor.trackerPatterns;
   const int next=juce::jlimit(0,999,bank.activePattern+(code==juce::KeyPress::upKey?-1:1));
   if(next!=bank.activePattern)jumpToPattern(next);
  }else{
   const auto& pat=currentPattern();
   const int next=code==juce::KeyPress::upKey
    ?trackerF1HomeTarget(row,pat.length,lastRealStep)
    :trackerF1EndTarget(row,pat.length,pat.repeat,lastRealStep);
   if(next!=row){
    clearSelection();
    if(row>=0 && row<pat.length)lastRealStep=row;
    row=next;
    if(row<0 || row>=pat.length)cell=0;
    refresh(true);auditionCurrentStep();
   }
  }
  return true;
 }
 if(m.isCtrlDown() && letter('z')){
  const int n=processor.trackerPatterns.activePattern;
  auto& from=m.isShiftDown()?redo[n]:undo[n];auto& to=m.isShiftDown()?undo[n]:redo[n];
  if(!from.empty()){
   to.push_back(currentPattern());currentPattern()=std::move(from.back());from.pop_back();finishEdit();
  }return true;
 }
 if(m.isCtrlDown() && letter('c')){
  if(hasStepSelection()){copySelectedSteps();return true;}
  if(const auto* c=selectedCell()){
   clipboard=*c;hasClipboard=true;clipboardMode=ClipboardMode::parameter;announce("Parameter copied");
  }else announce("No parameter to copy");return true;
 }
 if(m.isCtrlDown() && letter('x')){
  if(hasStepSelection()){eraseSelectedSteps(true);return true;}
  if(const auto* c=selectedCell()){
   clipboard=*c;hasClipboard=true;clipboardMode=ClipboardMode::parameter;
   recordUndo();currentPattern().erase(row,tab,cell);finishEdit();announce("Parameter cut");
  }else announce("No parameter to cut");return true;
 }
 if(m.isCtrlDown() && letter('v')){
  if(clipboardMode==ClipboardMode::pattern){
   recordUndo();currentPattern()=wholePatternClipboard;
   row=-1;cell=0;logicalCell=0;lastLogicalCell={0,0,0};clearSelection();finishEdit();announce("Entire Pattern pasted");return true;
  }
  if(clipboardMode==ClipboardMode::steps){
   if(row<0||row>=currentPattern().length)announce("Select a step before pasting");
   else pasteSelectedSteps();return true;
  }
  if(clipboardMode==ClipboardMode::parameter && hasClipboard && row>=0&&row<currentPattern().length){
   auto& pat=currentPattern();
   // Insert at the selected position, not after it. Use the same validated
   // message-thread operation as the parameter picker; no partial undo state.
   TrackerPattern candidate=pat;
   const auto result=pasteTrackerParameter(candidate,row,tab,cell,clipboard);
   if(result.status==TrackerInsertStatus::inserted){
    recordUndo();pat=std::move(candidate);
    cell=result.index;logicalCell=cell;lastLogicalCell[(size_t)tab]=logicalCell;
    finishEdit();announce("Parameter pasted");
   }else if(result.status==TrackerInsertStatus::alreadyPresent){
    cell=result.index;logicalCell=cell;lastLogicalCell[(size_t)tab]=logicalCell;
    refresh(true);announce("Parameter already present");
   }else if(result.status==TrackerInsertStatus::needsArpeggiator){
    announce("Sub Note requires OP Arp Mode before this cell");
   }else if(result.status==TrackerInsertStatus::full){
    announce("Step is full, 128 cells");
   }else announce("Select a step before pasting");
  }else announce(clipboardMode==ClipboardMode::none?
                "Clipboard empty":"Select a step before pasting");return true;
 }
 if(m.isCtrlDown() && code==juce::KeyPress::leftKey && row>=0 && row<currentPattern().length){
  auto& a=currentPattern().cells(row,tab);if(cell>0&&cell<(int)a.size()){
   recordUndo();std::swap(a[(size_t)cell],a[(size_t)(cell-1)]);--cell;logicalCell=cell;lastLogicalCell[(size_t)tab]=logicalCell;finishEdit();}return true;
 }
 if(m.isCtrlDown() && code==juce::KeyPress::rightKey && row>=0 && row<currentPattern().length){
  auto& a=currentPattern().cells(row,tab);if(cell>=0&&cell+1<(int)a.size()){
   recordUndo();std::swap(a[(size_t)cell],a[(size_t)(cell+1)]);++cell;logicalCell=cell;lastLogicalCell[(size_t)tab]=logicalCell;finishEdit();}return true;
 }
 if(m.isAltDown()){
  // Alt+E performs numeric editing (including Pattern header and Speed rows).
  if(letter('e')){openSelectedValueEditor();return true;}
  if(code==juce::KeyPress::upKey){adjustValue(1);return true;}
  if(code==juce::KeyPress::downKey){adjustValue(-1);return true;}
  if(code==juce::KeyPress::pageUpKey){adjustValue(1,10);return true;}
  if(code==juce::KeyPress::pageDownKey){adjustValue(-1,10);return true;}
  // Requested value boundaries: Alt+Home = maximum, Alt+End = minimum.
  if(code==juce::KeyPress::homeKey){setCurrentValueExtreme(true);return true;}
  if(code==juce::KeyPress::endKey){setCurrentValueExtreme(false);return true;}
 }
 if(code==juce::KeyPress::deleteKey && m.isCtrlDown() && m.isShiftDown() && !m.isAltDown()){
  clearCurrentSection();return true;
 }
 if(code==juce::KeyPress::deleteKey && m.isCtrlDown() && !m.isAltDown()){
  resetCurrentStep();return true;
 }
 if(code==juce::KeyPress::deleteKey && hasStepSelection() && !m.isCtrlDown() && !m.isAltDown()){
  eraseSelectedSteps(false);return true;
 }
 if(code==juce::KeyPress::deleteKey && row>=0 && row<currentPattern().length){
  if(currentPattern().view(row,tab).empty())return true;
  recordUndo();currentPattern().erase(row,tab,cell);finishEdit();return true;
 }
 if(code==juce::KeyPress::upKey){changeRow(-1);return true;}
 if(code==juce::KeyPress::downKey){changeRow(1);return true;}
 if(code==juce::KeyPress::pageUpKey){changeRow(-juce::jmax(5,jumpSizes[(size_t)jumpIndex]));return true;}
 if(code==juce::KeyPress::pageDownKey){changeRow(juce::jmax(5,jumpSizes[(size_t)jumpIndex]));return true;}
 if(code==juce::KeyPress::leftKey||code==juce::KeyPress::rightKey){
  clearSelection();const int next=juce::jlimit(0,juce::jmax(0,navigator.getNumItems()-1),cell+(code==juce::KeyPress::leftKey?-1:1));
  if(next!=cell || (row>=0 && row<currentPattern().length && logicalCell!=next)){
   cell=next;if(row>=0 && row<currentPattern().length){logicalCell=cell;lastLogicalCell[(size_t)tab]=logicalCell;}refresh(true);}
  return true;
 }
 // Ctrl+Home/End no longer change tracker regions; Ctrl+Up/Down do.
 if(m.isCtrlDown() && !m.isAltDown() &&
    (code==juce::KeyPress::homeKey || code==juce::KeyPress::endKey))return true;
 // Plain Home/End move only to the beginning/end of the CURRENT region.
 // Header stays put, Step range is 0..Length-1, footer is Repeat..last
 // Transpose. Preserve each OP/Filter/LFO logical column between steps.
 if(!m.isCtrlDown() && !m.isAltDown() && !m.isShiftDown() &&
    (code==juce::KeyPress::homeKey || code==juce::KeyPress::endKey)){
  const auto& pat=currentPattern();
  const auto target=trackerF1AreaEdge(row,cell,pat.length,pat.repeat,
                                    code==juce::KeyPress::endKey);
  if(target.row!=row || target.cell!=cell){
   clearSelection();row=target.row;cell=target.cell;
   if(row>=0 && row<pat.length)lastRealStep=row;
   refresh(true);auditionCurrentStep();
  }
  return true;
 }
 return true; // All keys are owned by Alt+Q; never leak into REAPER shortcuts.
}

void TrackerPatternPanel::setStepRecording(bool enable){
 if(enable && !stepRecording){
  BiscotProcessor::TrackerRecordedMidiEvent ignored;
  while(processor.popTrackerRecordedMidi(ignored)){}
  recordingNextStep=juce::jlimit(0,currentPattern().length-1,lastRealStep);
 }
 stepRecording=enable;recordSustainDown=false;
 processor.trackerStepRecording.store(enable,std::memory_order_release);
 if(enable)processor.requestTrackerStop();
}
void TrackerPatternPanel::recordMidiStep(int midiNote,bool pause){
 if(!stepRecording || page!=1 || row<0 || row>=currentPattern().length)return;
 auto& pat=currentPattern();recordUndo();
 // Lua fills skipped recording positions with Pause if the user moves the cursor ahead.
 for(int st=recordingNextStep;st<row;++st)trackerRecordStep(pat,st,0,true);
 trackerRecordStep(pat,row,midiNote,pause);
 auto& cells=pat.cells(row,0);
 const int id=pause?6:1;
 tab=0;logicalCell=0;lastLogicalCell[0]=0;
 for(int i=0;i<(int)cells.size();++i)if(cells[(size_t)i].id==id){cell=i;logicalCell=i;lastLogicalCell[0]=i;break;}
 const int recorded=row;recordingNextStep=row+1;
 if(row+1<pat.length){++row;lastRealStep=row;}
 else setStepRecording(false);
 processor.commitTrackerState();refresh(true);
 if(!stepRecording)announce("Step Recording completed at step "+juce::String(recorded));
}
void TrackerPatternPanel::timerCallback(){
 BiscotProcessor::TrackerRecordedMidiEvent event;
 int count=0;
 while(count++<128 && processor.popTrackerRecordedMidi(event)){
  if(!stepRecording)continue;
  if(event.type==2){recordSustainDown=false;continue;}
  if(event.type==1){if(!recordSustainDown){recordSustainDown=true;recordMidiStep(0,true);}continue;}
  if(event.type==0)recordMidiStep(event.note,false);
 }
}
}

namespace biscot {
int TrackerPatternPanel::cursorRegion() const {
 if(row<0)return 0;
 if(row<currentPattern().length)return 1;
 return row==currentPattern().length?2:3;
}
void TrackerPatternPanel::setCursorRegion(int region){
 if(page!=1)page=1;
 if(region==0)row=-1;
 else if(region==1)row=juce::jlimit(0,currentPattern().length-1,lastRealStep);
 else if(region==2)row=currentPattern().length;
 else row=currentPattern().length+1;
 cell=0;logicalCell=0;refresh(false);
}
}
