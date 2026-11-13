// SPDX-License-Identifier: AGPL-3.0-or-later
#include "TrackerSongPanel.h"
#include "ScreenReaderAnnouncer.h"
#include "TrackerManualNumeric.h"
#include "TrackerShortcutKey.h"
#include "TrackerSongPackage.h"
#include "ParameterSearchKey.h"
#include <algorithm>
#include <cmath>
#include <string>

namespace biscot {
namespace {
const juce::Colour fg(0xffe5edf6),bg(0xff171b26);
juce::String number(double d){return juce::String(d,3).trimCharactersAtEnd("0").trimCharactersAtEnd(".");}
juce::String midiLabel(int n){
 if(n<0)return "Off";
 static const char* names[]{"C","C sharp","D","D sharp","E","F","F sharp","G","G sharp","A","A sharp","B"};
 return juce::String(n)+" "+names[n%12]+" "+juce::String(n/12-1);
}
}
TrackerSongPanel::TrackerSongPanel(BiscotProcessor& p):navigator(*this),numeric(*this),processor(p){
 setFocusContainerType(juce::Component::FocusContainerType::keyboardFocusContainer);
 setWantsKeyboardFocus(true);
 title.setText("Biscot1 Tracker - F2 Song",juce::dontSendNotification);
 help.setText("F1 Pattern | 1/2/3: Operator | Ctrl+Up/Down: region | Ctrl+Shift+Up/Down: Song | Enter: Song parameters | Alt+E: edit | Tab: position | Esc: exit",juce::dontSendNotification);
 for(auto* l:{&title,&help}){l->setAccessible(false);l->setColour(juce::Label::textColourId,fg);addAndMakeVisible(*l);}
 navigator.setName("Song Grid");navigator.setWantsKeyboardFocus(true);navigator.setEditableText(false);
 navigator.setExplicitFocusOrder(1);
 navigator.onChange=[this]{
  if(refreshing||navigator.getSelectedId()<=0)return;
  if(picker){pickerIndex=navigator.getSelectedId()-1;announce(positionText());}
  else {cell=navigator.getSelectedId()-1;
   if(realStep())logicalCell=cell;
   announce(positionText());}
 };
 addAndMakeVisible(navigator);
 numeric.setName("Song numeric edit");numeric.setMultiLine(false);numeric.setWantsKeyboardFocus(true);
 numeric.setExplicitFocusOrder(2);addAndMakeVisible(numeric);numeric.setVisible(false);
 readCursor();refresh();
}
std::unique_ptr<juce::AccessibilityHandler> TrackerSongPanel::createAccessibilityHandler(){
 return createIgnoredAccessibilityHandler(*this);
}
void TrackerSongPanel::paint(juce::Graphics& g){g.fillAll(bg);g.setColour(fg);g.drawRect(getLocalBounds(),1);}
void TrackerSongPanel::resized(){
 auto b=getLocalBounds().reduced(22);
 title.setBounds(b.removeFromTop(35));help.setBounds(b.removeFromBottom(34));
 b.removeFromTop(10);navigator.setBounds(b.removeFromTop(34));b.removeFromTop(12);
 numeric.setBounds(b.removeFromTop(34));
}
void TrackerSongPanel::announce(const juce::String& t){announceToActiveScreenReader(navigator,t);}
void TrackerSongPanel::timerCallback(){
 if(processor.songPlaybackStatus.load(std::memory_order_relaxed)!=1){
  lastObservedPlayingSong=-1;return;
 }
 const int playing=processor.playingSongNumber();
 if(playing<0||playing>=TrackerSongBank::maxSongs)return;
 if((lastObservedPlayingSong<0 && playing!=processor.trackerSongs.activeSong) ||
    (lastObservedPlayingSong>=0 && playing!=lastObservedPlayingSong)){
  processor.trackerSongs.activeSong=playing;
  processor.trackerSongs.at(playing);
  processor.commitSongState();
  picker=false;search.reset();clearSelection();
  if(lane==3)lane=lastOperatorLane;
  if(lane<3){row=juce::jlimit(0,length()-1,row);lastStep[(size_t)lane]=row;}
  refresh(false);
  announce("Song "+juce::String(playing)+" started. "+positionText());
 }
 lastObservedPlayingSong=playing;
}
void TrackerSongPanel::close(){stopTimer();processor.requestSongStop();setVisible(false);}
void TrackerSongPanel::open(){
 processor.commitSongState();
 picker=false;search.reset();clearSelection();numeric.setVisible(false);refresh(false);setVisible(true);toFront(false);
 lastObservedPlayingSong=processor.songPlaybackStatus.load(std::memory_order_relaxed)==1?processor.playingSongNumber():-1;
 startTimerHz(12);
 juce::Component::SafePointer<TrackerSongPanel> safe(this);
 juce::MessageManager::callAsync([safe]{if(safe!=nullptr&&safe->isShowing()){
  safe->navigator.grabKeyboardFocus();safe->announce(safe->positionText(true));}});
}
void TrackerSongPanel::suspendForLibrary(){stopTimer();setVisible(false);}
void TrackerSongPanel::announceLibraryResult(const juce::String& s){announce(s);}
void TrackerSongPanel::restoreAfterPreset(){
 picker=false;search.reset();history.clear();clipboard={};clearSelection();numeric.setVisible(false);stopTimer();
 readCursor();refresh();
}
int TrackerSongPanel::visibleCount()const{
 if(picker)return (int)songParameters.size();
 if(lane==3)return 1;
 if(row<0)return 2;
 const int len=length();
 if(row==len)return 1;
 if(row>len){
  const int first=(row-len-1)*10;
  return std::max(1,std::min(10,song().laneRepeats[(size_t)lane]-first));
 }
 const auto* t=song().findStep(lane,row);
 return t?std::max(1,(int)t->visible().size()):1;
}
int TrackerSongPanel::selectedParameter()const{
 if(!realStep())return -1;
 const auto* t=song().findStep(lane,row);if(!t)return -1;
 const auto ids=t->visible();
 return cell>=0&&cell<(int)ids.size()?ids[(size_t)cell]:-1;
}
juce::String TrackerSongPanel::paramLabel(int id,double v)const{
 const auto& pat=processor.trackerPatterns;
 if(id==1)return v<0?"Pattern, Off":"Pattern, "+juce::String((int)v);
 if(id==23)return "Preview Steps, "+juce::String((int)v);
 if(id==5)return "Pattern Start, step "+juce::String((int)v);
 if(id==6)return "Play Length, "+juce::String((int)v)+" steps";
 if(id==7||id==8||id==21){
  const auto* t=song().findStep(lane,row);if(!t)return "Pattern info";
  const int start=(int)t->values[5],count=std::max(1,(int)t->values[6]);
  if(id==21)return "Pattern Position, step "+juce::String(start)+" to "+juce::String(start+count-1);
  if(id==7)return "Next Pattern Step, "+juce::String(start+count);
  return "Pattern Remaining, "+juce::String(std::max(0,(int)(t->values[1]>=0 && pat.find((int)t->values[1])?pat.find((int)t->values[1])->length:16)-start-count));
 }
 const auto* p=songParam(id);
 const juce::String label=p?p->name:"Parameter";
 if(id==9)return label+", "+(v>=0.5?"On":"Off");
 if(id==10||id==12||id==13||id==14)return label+", Operator "+juce::String((int)v);
 if(id==15)return label+", "+(v<0.5?juce::String("Off"):"Operator "+juce::String((int)v));
 if(id==11){
  static const char* masks[]{"Off","Operator 1","Operator 2","Operator 1 and 2","Operator 3","Operator 1 and 3","Operator 2 and 3","All Operators"};
  return label+", "+masks[juce::jlimit(0,7,(int)v)];
 }
 if(id==2)return label+", "+number(v)+"x";
 if(id==17)return label+", "+(v<=0?juce::String("Off"):number(v)+" beats");
 if(id==16||id==18||id==19||id==20)return label+", "+number(v)+" percent";
 if(id==22)return label+", "+juce::String((int)v)+" of 32";
 return label+", "+number(v);
}
bool TrackerSongPanel::parameterAlreadyInStep(int id)const{
 if(!realStep())return false;
 const auto* entry=song().findStep(lane,row);
 return entry!=nullptr && entry->contains(id);
}
juce::String TrackerSongPanel::positionText(bool full)const{
 const juce::String prefix=full?("Song "+juce::String(processor.trackerSongs.activeSong)+
  (song().name.empty()?juce::String{}:" "+juce::String::fromUTF8(song().name.c_str()))+". "):juce::String{};
 if(picker){const auto& def=songParameters[(size_t)juce::jlimit(0,(int)songParameters.size()-1,pickerIndex)];
  return "Song parameters. "+juce::String(parameterAlreadyInStep(def.id)?"Set, ":"")+def.name;}
 if(lane==3)return prefix+"Global. Song End, "+(song().songEnd<0?juce::String("Off"):juce::String(song().songEnd));
 const juce::String laneText=prefix+(full?("Operator "+juce::String(lane+1)+". "):juce::String{});
 if(row<0)return laneText+(cell==0?"Length, "+juce::String(length()):"MIDI Note, "+midiLabel(song().midiNote));
 if(row==length())return laneText+"Repeat, "+juce::String(song().laneRepeats[(size_t)lane]);
 if(speedRow()){
  const int index=(row-length()-1)*10+cell;
  const auto& speeds=song().laneSpeeds[(size_t)lane];
  return laneText+"Speed "+juce::String(index+1)+", "+(index<(int)speeds.size()?number(speeds[(size_t)index]):juce::String("1"))+"x";
 }
 const int id=selectedParameter();
 if(id<0)return laneText+"step "+juce::String(row)+". empty";
 const auto* entry=song().findStep(lane,row);
 return laneText+"step "+juce::String(row)+". "+paramLabel(id,entry->values[(size_t)id]);
}
void TrackerSongPanel::readCursor(){
 const auto& state=processor.state.state;
 lane=juce::jlimit(0,3,(int)state.getProperty(juce::Identifier("trackerSongLane"),0));
 lastOperatorLane=juce::jlimit(0,2,(int)state.getProperty(juce::Identifier("trackerSongLastOperator"),0));
 row=(int)state.getProperty(juce::Identifier("trackerSongRow"),-1);
 cell=juce::jmax(0,(int)state.getProperty(juce::Identifier("trackerSongCell"),0));
 logicalCell=juce::jmax(0,(int)state.getProperty(juce::Identifier("trackerSongLogicalCell"),cell));
 for(int i=0;i<3;++i)
  lastStep[(size_t)i]=juce::jlimit(0,4095,(int)state.getProperty(juce::Identifier(juce::String("trackerSongLastStep")+juce::String(i)),0));
 if(lane<3){
  const int len=length();
  row=juce::jlimit(-1,len+(song().laneRepeats[(size_t)lane]+9)/10,row);
 }else row=-1;
}
void TrackerSongPanel::persistCursor(){
 auto& state=processor.state.state;
 state.setProperty(juce::Identifier("trackerSongLane"),lane,nullptr);
 state.setProperty(juce::Identifier("trackerSongLastOperator"),lastOperatorLane,nullptr);
 state.setProperty(juce::Identifier("trackerSongRow"),row,nullptr);
 state.setProperty(juce::Identifier("trackerSongCell"),cell,nullptr);
 state.setProperty(juce::Identifier("trackerSongLogicalCell"),logicalCell,nullptr);
 for(int i=0;i<3;++i)
  state.setProperty(juce::Identifier(juce::String("trackerSongLastStep")+juce::String(i)),lastStep[(size_t)i],nullptr);
}
void TrackerSongPanel::refresh(bool speak){
 refreshing=true;
 navigator.clear(juce::dontSendNotification);
 if(picker){
  for(int i=0;i<(int)songParameters.size();++i){
   const auto& def=songParameters[(size_t)i];
   navigator.addItem(juce::String(parameterAlreadyInStep(def.id)?"Set, ":"")+def.name,i+1);
  }
  pickerIndex=juce::jlimit(0,(int)songParameters.size()-1,pickerIndex);
  navigator.setSelectedId(pickerIndex+1,juce::dontSendNotification);
 }else if(lane==3){
  navigator.addItem("Song End, "+(song().songEnd<0?juce::String("Off"):juce::String(song().songEnd)),1);
  cell=0;navigator.setSelectedId(1,juce::dontSendNotification);
 }else if(row<0){
  navigator.addItem("Length, "+juce::String(length()),1);
  navigator.addItem("MIDI Note, "+midiLabel(song().midiNote),2);
  cell=juce::jlimit(0,1,cell);navigator.setSelectedId(cell+1,juce::dontSendNotification);
 }else if(row==length()){
  navigator.addItem("Repeat, "+juce::String(song().laneRepeats[(size_t)lane]),1);
  cell=0;navigator.setSelectedId(1,juce::dontSendNotification);
 }else if(speedRow()){
  const int start=(row-length()-1)*10;
  const auto& speeds=song().laneSpeeds[(size_t)lane];
  for(int i=start;i<std::min(start+10,song().laneRepeats[(size_t)lane]);++i)
   navigator.addItem("Speed "+juce::String(i+1)+", "+(i<(int)speeds.size()?number(speeds[(size_t)i]):juce::String("1"))+"x",i-start+1);
  cell=juce::jlimit(0,std::max(0,navigator.getNumItems()-1),cell);
  navigator.setSelectedId(cell+1,juce::dontSendNotification);
 }else{
  const auto* entry=song().findStep(lane,row);
  const auto ids=entry?entry->visible():std::vector<int>{};
  if(ids.empty())navigator.addItem("step "+juce::String(row)+". empty",1);
  else for(int i=0;i<(int)ids.size();++i)
   navigator.addItem("step "+juce::String(row)+". "+paramLabel(ids[(size_t)i],entry->values[(size_t)ids[(size_t)i]]),i+1);
  cell=juce::jlimit(0,std::max(0,navigator.getNumItems()-1),logicalCell);
  navigator.setSelectedId(cell+1,juce::dontSendNotification);
 }
 refreshing=false;
 persistCursor();
 repaint();if(speak){announce(positionText());previewCurrentStep();}
}
void TrackerSongPanel::previewCurrentStep(){
 if(picker||!realStep()||processor.songPlaybackStatus.load(std::memory_order_acquire)==1)return;
 const auto* step=song().findStep(lane,row);
 if(!step||!step->contains(1)||step->values[1]<0)return;
 const int length=step->contains(23)?(int)step->values[23]:3;
 processor.requestSongPreview(lane,row,juce::jlimit(1,1024,length));
}
void TrackerSongPanel::changeLane(int next){
 if(next<0||next>3||next==lane)return;
 const int region=(lane==3?0:row<0?0:row<length()?1:row==length()?2:3);
 if(realStep())lastStep[(size_t)lane]=row;
 clearSelection();
 if(next<3)lastOperatorLane=next;
 else if(lane<3)lastOperatorLane=lane;
 lane=next;picker=false;search.reset();cell=0;logicalCell=0;
 if(lane==3)row=-1;
 else if(region==1)row=juce::jlimit(0,length()-1,lastStep[(size_t)lane]);
 else if(region==2)row=length();
 else if(region==3)row=length()+1;
 else row=-1;
 refresh(false);
 announce(lane==3?positionText():("Operator "+juce::String(lane+1)+". "+positionText()));
 if(realStep())previewCurrentStep();
}
void TrackerSongPanel::changeSong(int next){
 next=juce::jlimit(0,TrackerSongBank::maxSongs-1,next);
 if(next==processor.trackerSongs.activeSong)return;
 const int oldLength=length(),oldRow=row,oldCell=cell;
 processor.trackerSongs.activeSong=next;processor.trackerSongs.at(next);
 picker=false;search.reset();clearSelection();
 if(lane<3){
  if(oldRow<0)row=-1;
  else if(oldRow<oldLength)row=juce::jlimit(0,length()-1,oldRow);
  else if(oldRow==oldLength)row=length();
  else row=juce::jlimit(length()+1,length()+(song().laneRepeats[(size_t)lane]+9)/10,length()+oldRow-oldLength);
 }
 cell=oldCell;logicalCell=cell;
 processor.commitSongState();refresh(false);announce(positionText(true));
}
void TrackerSongPanel::vertical(int delta){
 if(lane==3||picker||delta==0)return;
 const int len=length();
 if(row<0)return; // Song length and MIDI Note stay in the header.
 const int lastSpeed=len+(song().laneRepeats[(size_t)lane]+9)/10;
 int dest=row;
 if(row<len)dest=juce::jlimit(0,len-1,row+delta);
 else dest=juce::jlimit(len,lastSpeed,row+delta);
 if(dest==row)return;
 clearSelection();row=dest;if(realStep())lastStep[(size_t)lane]=row;
 if(!realStep())cell=0;
 refresh(true);
}
void TrackerSongPanel::navigateArea(int direction){
 clearSelection();
 if(lane==3){if(direction<0){lane=lastOperatorLane;row=length();cell=0;refresh(true);}return;}
 if(row<0){if(direction>0){row=lastStep[(size_t)lane];cell=0;logicalCell=0;}}
 else if(row<length()){
  if(direction<0){lastStep[(size_t)lane]=row;row=-1;cell=0;}
  else {lastStep[(size_t)lane]=row;row=length();cell=0;}
 }else{
  if(direction<0){row=lastStep[(size_t)lane];cell=0;logicalCell=0;}
  else {lastOperatorLane=lane;lane=3;row=-1;cell=0;}
 }
 refresh(true);
}
double TrackerSongPanel::valueNow()const{
 if(lane==3)return song().songEnd;
 if(row<0)return cell==0?length():song().midiNote;
 if(row==length())return song().laneRepeats[(size_t)lane];
 if(speedRow()){
  const int idx=(row-length()-1)*10+cell;
  const auto& speeds=song().laneSpeeds[(size_t)lane];return idx<(int)speeds.size()?speeds[(size_t)idx]:1;
 }
 const auto* t=song().findStep(lane,row);const int id=selectedParameter();
 return t&&id>0?t->values[(size_t)id]:0.;
}
void TrackerSongPanel::currentValue(double value){
 if(!std::isfinite(value))return;
 auto& s=song();const TrackerSong before=s;double newValue=value;
 if(lane==3){
  const int n=juce::jlimit(-1,99,(int)std::round(value));
  if(n==s.songEnd)return;
  s.songEnd=n;
  if(n>=0)processor.trackerSongs.at(n); // Ensure the Song End target exists.
 }else if(row<0){
  if(cell==0){
   const int n=juce::jlimit(1,4096,(int)std::round(value));if(n==length())return;
   s.laneLengths[(size_t)lane]=n;
   if(row>=0)row=juce::jmin(row,n-1);
  }else{
   const int n=juce::jlimit(-1,127,(int)std::round(value));if(n==s.midiNote)return;
   s.midiNote=n;
  }
 }else if(row==length()){
  const int n=juce::jlimit(1,1000,(int)std::round(value));
  if(n==s.laneRepeats[(size_t)lane])return;
  auto& speeds=s.laneSpeeds[(size_t)lane];
  speeds.resize((size_t)n,speeds.empty()?1.:speeds.back());s.laneRepeats[(size_t)lane]=n;
 }else if(speedRow()){
  const int idx=(row-length()-1)*10+cell;
  auto& speeds=s.laneSpeeds[(size_t)lane];
  if(idx<0||idx>=(int)speeds.size())return;
  newValue=std::clamp(std::round(value*4.)/4.,0.25,16.);
  if(newValue==speeds[(size_t)idx])return;speeds[(size_t)idx]=newValue;
 }else{
  const int id=selectedParameter();if(id<0||id==7||id==8||id==21)return;
  auto& entry=s.atStep(lane,row);
  newValue=songClampValue(id,value);
  if(id==5||id==6){
   const int pn=(int)entry.values[1];
   const auto* pattern=processor.trackerPatterns.find(pn);
   const int maxLength=pattern?pattern->length:16;
   if(id==5){
    newValue=std::clamp(newValue,0.,(double)maxLength-1.);
    entry.values[6]=std::clamp(entry.values[6],1.,(double)maxLength-newValue);
   }else newValue=std::clamp(newValue,1.,(double)maxLength-entry.values[5]);
  }
  if(entry.values[(size_t)id]==newValue)return;
  entry.values[(size_t)id]=newValue;
  if(id==1){
   const int pn=(int)newValue;
   const auto* pattern=processor.trackerPatterns.find(pn);
   entry.values[5]=0;entry.values[6]=pn>=0?(pattern?pattern->length:16):1;
  }
 }
 commitEdit(before);refresh(true);
}
void TrackerSongPanel::adjustValue(int direction,bool large){
 double step=large?5.:1.;
 if(lane<3&&speedRow())step=large?1.25:0.25;
 else if(realStep()){
  const int id=selectedParameter();if(id==7||id==8||id==21||id<0)return;
  if(const auto* p=songParam(id))step=p->step*(large?5:1);
 }
 currentValue(valueNow()+direction*step);
}
void TrackerSongPanel::extreme(bool high){
 if(lane==3){currentValue(high?99:-1);return;}
 if(row<0){currentValue(cell==0?(high?4096:1):(high?127:-1));return;}
 if(row==length()){currentValue(high?1000:1);return;}
 if(speedRow()){currentValue(high?16:0.25);return;}
 const int id=selectedParameter();if(id==5||id==6){currentValue(high?1024:0);return;}
 if(const auto* p=songParam(id))currentValue(high?p->max:p->min);
}
void TrackerSongPanel::openNumeric(EditPurpose purpose,const juce::String& label,const juce::String& initial){
 editPurpose=purpose;numeric.setName(label);numeric.setText(initial,juce::dontSendNotification);
 numeric.setVisible(true);numeric.grabKeyboardFocus();numeric.selectAll();
}
void TrackerSongPanel::openCurrentNumeric(){
 if(realStep()&&selectedParameter()<0){announce("Empty step. Press Enter to add a Song parameter");return;}
 if(realStep()&&(selectedParameter()==7||selectedParameter()==8||selectedParameter()==21)){
  announce("Read-only Pattern information");return;
 }
 openNumeric(EditPurpose::value,"Edit "+positionText(),number(valueNow()));
}
void TrackerSongPanel::insertSelectedParameter(bool remain){
 if(!realStep()){announce("Select a Song step first");return;}
 const TrackerSong before=song();
 auto& entry=song().atStep(lane,row);
 const int id=songParameters[(size_t)juce::jlimit(0,(int)songParameters.size()-1,pickerIndex)].id;
 const auto prior=entry.visible();
 const int insertPos=std::min(cell,(int)prior.size());
 int groupPos=0;
 for(int i=0;i<insertPos;++i)
  if(prior[(size_t)i]!=5&&prior[(size_t)i]!=6&&prior[(size_t)i]!=7&&prior[(size_t)i]!=8&&prior[(size_t)i]!=21)++groupPos;
 const bool added=entry.insert(id,groupPos);
 if(added){
  const auto after=entry.visible();
  const auto it=std::find(after.begin(),after.end(),id);
  cell=(int)std::distance(after.begin(),it);logicalCell=cell;
  commitEdit(before);
 }
 if(!remain){picker=false;clearSelection();refresh(false);}
 else refresh(false); // Do not mask the insertion announcement with the picker label.
 announce(juce::String(songParameters[(size_t)pickerIndex].name)+(added?" inserted":" already present"));
}
void TrackerSongPanel::eraseCurrent(){
 if(lane<3&&row<0&&cell==1&&song().midiNote>=0){
  const TrackerSong before=song();song().midiNote=-1;commitEdit(before);refresh(true);return;
 }
 if(!realStep())return;
 const int id=selectedParameter();if(id<0||id==7||id==8||id==21)return;
 const TrackerSong before=song();
 auto& a=song().lanes[(size_t)lane];const auto it=a.find(row);if(it==a.end())return;
 if(!it->second.erase(id))return;
 if(it->second.empty())a.erase(it);
 commitEdit(before);refresh(true);
}
void TrackerSongPanel::resetCurrentValue(){
 if(picker)return; // Picker contains parameter names, not editable values.
 if(lane==3){currentValue(-1.);return;}
 if(row<0){currentValue(cell==0?16.:-1.);return;}
 if(row==length()){currentValue(1.);return;}
 if(speedRow()){currentValue(1.);return;}
 const int id=selectedParameter();
 if(id<=0||id==7||id==8||id==21)return; // read-only cells
 const auto* step=song().findStep(lane,row);
 if(step==nullptr || !step->contains(id))return;
 currentValue(songDefaultValue(id));
}
void TrackerSongPanel::commitEdit(const TrackerSong& before){
 history.record(processor.trackerSongs.activeSong,before);
 processor.commitSongState();
}
void TrackerSongPanel::extendSelection(int delta){
 if(!realStep())return;
 if(!selectionActive())selectionAnchor=selectionEnd=row;
 else {selectionEnd=juce::jlimit(0,length()-1,selectionEnd+delta);row=selectionEnd;lastStep[(size_t)lane]=row;}
 refresh(false);
 const auto [a,b]=selectionBounds();
 announce((a==b?"Step "+juce::String(a):"Steps "+juce::String(a)+" to "+juce::String(b))+" selected. "+positionText());
}
void TrackerSongPanel::copySelected(bool cut){
 if(!realStep()){announce("Select a Song step");return;}
 if(selectionActive()){
  const auto [a,b]=selectionBounds();
  clipboard.copySteps(processor.trackerSongs.activeSong,lane,a,b,song());
  if(cut){const TrackerSong before=song();eraseSongSteps(song(),lane,a,b);commitEdit(before);clearSelection();refresh(false);}
  announce(juce::String(b-a+1)+(cut?" Song steps cut":" Song steps copied"));return;
 }
 const int id=selectedParameter();const auto* src=song().findStep(lane,row);
 if(!src||id<0){announce("Empty Song step");return;}
 if(id==7||id==8||id==21){announce("Read-only Pattern information");return;}
 clipboard.copyParameter(processor.trackerSongs.activeSong,lane,row,id,*src);
 if(cut){eraseCurrent();announce("Song parameter cut");}else announce("Song parameter copied");
}
void TrackerSongPanel::pasteClipboard(){
 const auto mode=clipboard.mode;const TrackerSong before=song();
 if(mode==TrackerSongClipboard::Mode::wholeSong){
  if(!clipboard.pasteSong(song()))return;
  clearSelection();row=0;cell=logicalCell=0;lane=0;
  commitEdit(before);refresh(false);announce("Song pasted; destination name preserved");return;
 }
 if(!realStep()){announce("Select a Song step");return;}
 if(mode==TrackerSongClipboard::Mode::steps){
  const int n=clipboard.pasteSteps(song(),lane,row);
  if(n<1){announce("No Song steps pasted");return;}
  clearSelection();commitEdit(before);refresh(true);announce(juce::String(n)+" Song steps pasted");return;
 }
 if(mode==TrackerSongClipboard::Mode::parameter){
  if(clipboard.sourceSong==processor.trackerSongs.activeSong&&clipboard.sourceLane==lane&&clipboard.sourceStep==row){
   announce("Cannot paste parameter in its source step");return;
  }
  if(!clipboard.pasteParameter(song(),lane,row,cell)){announce("Song parameter already present");return;}
  auto* entry=song().findStep(lane,row);
  if(entry){const auto visible=entry->visible();const auto it=std::find(visible.begin(),visible.end(),clipboard.parameterId);
   if(it!=visible.end())cell=logicalCell=(int)std::distance(visible.begin(),it);}
  commitEdit(before);refresh(true);return;
 }
 announce("Nothing to paste");
}
void TrackerSongPanel::clearSelected(){
 if(!realStep())return;
 const TrackerSong before=song();
 if(selectionActive()){
  const auto [a,b]=selectionBounds();eraseSongSteps(song(),lane,a,b);clearSelection();
 }else {if(song().lanes[(size_t)lane].erase(row)==0)return;}
 cell=logicalCell=0;commitEdit(before);refresh(true);
}
void TrackerSongPanel::reorder(int direction){
 if(!realStep())return;
 const int id=selectedParameter();if(id<0)return;
 const TrackerSong before=song();auto& step=song().atStep(lane,row);
 if(!moveSongParameter(step,id,direction))return;
 const auto vis=step.visible();const auto it=std::find(vis.begin(),vis.end(),id);
 cell=logicalCell=(int)std::distance(vis.begin(),it);
 commitEdit(before);refresh(true);
}
void TrackerSongPanel::undoRedo(bool redo){
 if(!history.restore(processor.trackerSongs.activeSong,song(),redo)){
  announce(redo?"Nothing to redo":"Nothing to undo");return;
 }
 clearSelection();if(lane<3){row=juce::jlimit(-1,song().laneLengths[(size_t)lane],row);}
 processor.commitSongState();refresh(false);
 announce(redo?"Redo Song":"Undo Song");
}
void TrackerSongPanel::mixVolume(int delta){
 const int op=lane<3?lane:lastOperatorLane;
 const TrackerSong before=song();const int next=juce::jlimit(0,100,song().mixerVolumes[(size_t)op]+delta);
 if(next==song().mixerVolumes[(size_t)op])return;
 song().mixerVolumes[(size_t)op]=next;commitEdit(before);
 announce("Operator "+juce::String(op+1)+" Song Volume, "+juce::String(next)+" percent");
}
void TrackerSongPanel::toggleMuteSolo(bool solo){
 const int op=lane<3?lane:lastOperatorLane;const TrackerSong before=song();
 if(solo){
  song().mixerSoloLane=(song().mixerSoloLane==op+1?0:op+1);
  if(song().mixerSoloLane) song().mixerMuteMask &= ~(1<<op);
 }else{
  song().mixerMuteMask^=(1<<op);
  if(song().mixerMuteMask&(1<<op))if(song().mixerSoloLane==op+1)song().mixerSoloLane=0;
 }
 commitEdit(before);
 announce("Operator "+juce::String(op+1)+(solo?(song().mixerSoloLane==op+1?", solo":", solo off"):
  ((song().mixerMuteMask&(1<<op))?", mute":", mute off")));
}
void TrackerSongPanel::renameSong(){
 openNumeric(EditPurpose::songName,"Rename Song "+juce::String(processor.trackerSongs.activeSong),
  juce::String::fromUTF8(song().name.c_str()));
}
bool TrackerSongPanel::keyPressed(const juce::KeyPress& k){return handle(k,false);}
bool TrackerSongPanel::handle(const juce::KeyPress& k,bool editing){
 const auto m=k.getModifiers();const int code=k.getKeyCode();
 const auto letter=[&](char c){return trackerShortcutLetterMatches(code,(int)k.getTextCharacter(),c,m.isCtrlDown());};
 if(m.isAltDown()&&!m.isCtrlDown()&&!m.isShiftDown()&&letter('a')){if(onAbout)onAbout();return true;}
 if(editing){
  if(code==juce::KeyPress::returnKey){
   const auto input=numeric.getText().trim();
   const bool savingSong=editPurpose==EditPurpose::saveSongName;
   const bool savingAll=editPurpose==EditPurpose::saveAllSongsName;
   if((savingSong||savingAll)&&input.isEmpty()){
    announceToActiveScreenReader(numeric,"Enter a Song file name");return true;
   }
   numeric.setVisible(false);navigator.grabKeyboardFocus();
   if(savingSong||savingAll){
    const auto documents=juce::File::getSpecialLocation(juce::File::userDocumentsDirectory);
    juce::String result;
    if(savingAll){
     songpkg::exportAllSongs(processor.trackerSongs,processor.trackerPatterns,documents,result,input);
    }else{
     // Saving a named single Song updates its internal name only if the write succeeds.
     const TrackerSong before=song();
     auto displayName=input;
     if(displayName.endsWithIgnoreCase(".biscot1-song"))
      displayName=displayName.dropLastCharacters(juce::String(".biscot1-song").length());
     song().name=displayName.toStdString();
     if(songpkg::exportSong(processor.trackerSongs,processor.trackerPatterns,documents,result,input)){
      if(before.name!=song().name)commitEdit(before);
     }else song().name=before.name;
    }
    announce(result);return true;
   }
   if(editPurpose==EditPurpose::songName){
    const TrackerSong before=song();
    auto clean=input.replace("\r"," ").replace("\n"," ").replace("\t"," ").trim();
    song().name=clean.toStdString();
    if(before.name!=song().name)commitEdit(before);
    announce(song().name.empty()?"Song name cleared":"Song renamed, "+clean);
    return true;
   }
   double n=0;
   if(input.isEmpty()||!parseTrackerManualNumeric(input.toStdString(),n)){
    announce("Invalid number");return true;
   }
   if(editPurpose==EditPurpose::value)currentValue(n);
   else if(n==std::round(n)){
    const int value=(int)std::clamp(n,-1.,4096.);
    if(editPurpose==EditPurpose::songNumber)changeSong(value);
    else if(value>=0&&lane<3&&value<length()){
     clearSelection();row=value;lastStep[(size_t)lane]=row;cell=0;logicalCell=0;refresh(true);
    }else announce("Invalid step number");
   }else announce("Enter an integer number");
   if(shortcutEdit){shortcutEdit=false;lane=shortcutLane;row=shortcutRow;cell=shortcutCell;logicalCell=cell;refresh(false);}
   return true;
  }
  if(code==juce::KeyPress::escapeKey){numeric.setVisible(false);navigator.grabKeyboardFocus();
   if(shortcutEdit){shortcutEdit=false;lane=shortcutLane;row=shortcutRow;cell=shortcutCell;logicalCell=cell;refresh(false);}return true;}
  return false;
 }
 if(code==juce::KeyPress::escapeKey){
  if(picker){picker=false;search.reset();refresh(true);return true;}
  close();if(onExit)onExit();return true;
 }
 if(m.isAltDown()&&letter('q')){close();if(onExit)onExit();return true;}
 if(code==juce::KeyPress::F1Key){close();if(onPattern)onPattern();return true;}
 if(code==juce::KeyPress::F2Key){announce(positionText(true));return true;}
 if(code==32){
  if(m.isCtrlDown()){
   if(processor.songPlaybackStatus.load(std::memory_order_relaxed)==1){
    processor.requestSongPause();
    if(lane<3){const int audible=processor.songAudibleRows[(size_t)lane].load(std::memory_order_relaxed);
     if(audible>=0){row=juce::jlimit(0,length()-1,audible);lastStep[(size_t)lane]=row;cell=logicalCell=0;}}
    refresh();announce("Song paused. "+positionText());
   }else announce("Song is not playing");
  }else{
   if(processor.songPlaybackStatus.load(std::memory_order_relaxed)==1){processor.requestSongStop();announce("Song stopped");}
   else{processor.commitSongState();processor.requestSongStart(lane<3&&realStep()?row:0);
    announce("Song playback started from step "+juce::String(lane<3&&realStep()?row:0));}
  }return true;
 }
 if(m.isAltDown()&&!m.isCtrlDown()&&letter('b'))return true; // disabled and silent inside Alt+Q
 if(m.isAltDown()&&!m.isCtrlDown()&&letter('s')){
  openNumeric(EditPurpose::saveAllSongsName,
   "Save all 100 Songs file name. Enter to save; Escape to cancel", "All_Songs");
  return true;
 }
 if(m.isCtrlDown()&&!m.isAltDown()&&letter('s')){
  const auto songName=juce::String::fromUTF8(song().name.c_str());
  openNumeric(EditPurpose::saveSongName,
   "Save current Song file name. Enter to save; Escape to cancel",
   songName.isNotEmpty()?songName:"Song_"+juce::String(processor.trackerSongs.activeSong));
  return true;
 }
 if(m.isCtrlDown()&&!m.isAltDown()&&letter('o')){
  if(onLibraryBrowser)onLibraryBrowser();else announce("Library browser unavailable");
  return true;
 }
 if(!m.isCtrlDown()&&code==juce::KeyPress::tabKey){announce(positionText(true));return true;}
 if(m.isAltDown()&&code==juce::KeyPress::returnKey){
  openNumeric(EditPurpose::songNumber,"Go to Song 0 to 99",juce::String(processor.trackerSongs.activeSong));return true;
 }
 // Direct F1/F2-compatible numeric shortcuts. Keep the original cursor
 // and restore it after Enter or Escape, including when editing from steps.
 if(!picker && !m.isCtrlDown() && !m.isAltDown() && !m.isShiftDown() && lane<3){
  char command=0;
  for(char c:{'l','m','p','r','d','s'})if(letter(c)){command=c;break;}
  if(command){
   shortcutLane=lane;shortcutRow=row;shortcutCell=cell;shortcutEdit=true;
   if(command=='l'||command=='m'){row=-1;cell=command=='l'?0:1;}
   else if(command=='p'||command=='r'){row=length();cell=0;}
   else {row=length()+1;cell=0;}
   refresh(false);openCurrentNumeric();
   if(!numeric.isVisible()){shortcutEdit=false;row=shortcutRow;cell=shortcutCell;refresh(false);}
   return true;
  }
 }
 // Match F1: Ctrl+Up/Down changes region; Ctrl+Shift+Up/Down changes Song.
 // Keep the parameter picker independent of these tracker navigation keys.
 if(!picker&&m.isCtrlDown()&&!m.isAltDown()&&
    (code==juce::KeyPress::upKey||code==juce::KeyPress::downKey)){
  const int direction=code==juce::KeyPress::upKey?-1:1;
  if(m.isShiftDown())changeSong(processor.trackerSongs.activeSong+direction);
  else navigateArea(direction);
  return true;
 }
 if(!picker&&!m.isCtrlDown()&&!m.isAltDown()&&!m.isShiftDown()&&code>='1'&&code<='4'){
  changeLane(code-'1');return true;
 }
 if(!picker&&m.isCtrlDown()&&!m.isAltDown()&&letter('j')){
  if(lane<3)openNumeric(EditPurpose::stepNumber,"Go to Song step 0 to "+juce::String(length()-1),juce::String(juce::jlimit(0,length()-1,row)));
  return true;
 }
 if(!picker){
  if(m.isCtrlDown()&&!m.isAltDown()&&letter('z')){undoRedo(m.isShiftDown());return true;}
  if(m.isCtrlDown()&&!m.isAltDown()&&!m.isShiftDown()&&letter('r')){renameSong();return true;}
  if(m.isCtrlDown()&&m.isShiftDown()&&m.isAltDown()&&letter('c')){
   clipboard.copySong(processor.trackerSongs.activeSong,song());announce("Entire Song copied");return true;
  }
  if(m.isCtrlDown()&&!m.isAltDown()&&!m.isShiftDown()&&letter('c')){copySelected(false);return true;}
  if(m.isCtrlDown()&&!m.isAltDown()&&!m.isShiftDown()&&letter('x')){copySelected(true);return true;}
  if(m.isCtrlDown()&&!m.isAltDown()&&!m.isShiftDown()&&letter('v')){pasteClipboard();return true;}
  if(m.isCtrlDown()&&m.isShiftDown()&&m.isAltDown()&&code==juce::KeyPress::deleteKey){
   const TrackerSong before=song();song()=TrackerSong{};clearSelection();row=-1;cell=logicalCell=0;
   commitEdit(before);refresh(true);return true;
  }
  if(m.isCtrlDown()&&m.isShiftDown()&&!m.isAltDown()&&code==juce::KeyPress::deleteKey){
   if(lane<3){const TrackerSong before=song();song().lanes[(size_t)lane].clear();clearSelection();
    commitEdit(before);refresh(true);}return true;
  }
  if(m.isCtrlDown()&&!m.isShiftDown()&&!m.isAltDown()&&code==juce::KeyPress::deleteKey){clearSelected();return true;}
  if(m.isShiftDown()&&!m.isCtrlDown()&&!m.isAltDown()&&lane<3){
   if(code==juce::KeyPress::upKey){extendSelection(-1);return true;}
   if(code==juce::KeyPress::downKey){extendSelection(1);return true;}
   if(code==juce::KeyPress::pageUpKey){extendSelection(-5);return true;}
   if(code==juce::KeyPress::pageDownKey){extendSelection(5);return true;}
  }
  if(m.isCtrlDown()&&!m.isShiftDown()&&!m.isAltDown()&&code==juce::KeyPress::leftKey){reorder(-1);return true;}
  if(m.isCtrlDown()&&!m.isShiftDown()&&!m.isAltDown()&&code==juce::KeyPress::rightKey){reorder(1);return true;}
  if(!m.isCtrlDown()&&!m.isAltDown()){
   if(letter('m')){toggleMuteSolo(false);return true;}
   if(letter('s')){toggleMuteSolo(true);return true;}
   if(code=='+'){mixVolume(1);return true;}
   if(code=='-'){mixVolume(-1);return true;}
  }
 }
 if(picker){
  if(code==juce::KeyPress::returnKey){insertSelectedParameter(m.isCtrlDown());return true;}
  int move=0;
  if(code==juce::KeyPress::upKey)move=-1;
  if(code==juce::KeyPress::downKey)move=1;
  if(code==juce::KeyPress::leftKey)move=-8;
  if(code==juce::KeyPress::rightKey)move=8;
  if(code==juce::KeyPress::pageUpKey)move=-5;
  if(code==juce::KeyPress::pageDownKey)move=5;
  if(code==juce::KeyPress::homeKey)move=-(pickerIndex%8);
  if(code==juce::KeyPress::endKey)move=std::min(7-pickerIndex%8,(int)songParameters.size()-1-pickerIndex);
  if(m.isCtrlDown()&&code==juce::KeyPress::homeKey)move=-pickerIndex;
  if(m.isCtrlDown()&&code==juce::KeyPress::endKey)move=(int)songParameters.size()-1-pickerIndex;
  if(move){
   // Match the F1 Pattern picker: a clamped movement that does not change
   // the selected parameter must be completely silent for NVDA/OSARA.
   const int next=juce::jlimit(0,(int)songParameters.size()-1,pickerIndex+move);
   if(next!=pickerIndex){pickerIndex=next;search.reset();refresh(true);}
   return true;
  }
  if(const auto ch=biscotParameterSearchCharacter(k);ch!=0){
   const int found=search.next((wchar_t)ch,pickerIndex,(int)songParameters.size(),m.isShiftDown()?-1:1,
    juce::Time::getMillisecondCounter(),[](int i){return std::wstring(juce::String(songParameters[(size_t)i].name).toWideCharPointer());});
   if(found>=0){pickerIndex=found;refresh(true);}return true;
  }
  return true;
 }
 if(m.isAltDown()&&!m.isCtrlDown()&&letter('e')){openCurrentNumeric();return true;}
 if(m.isAltDown()&&!m.isCtrlDown()&&code==juce::KeyPress::upKey){adjustValue(1);return true;}
 if(m.isAltDown()&&!m.isCtrlDown()&&code==juce::KeyPress::downKey){adjustValue(-1);return true;}
 if(m.isAltDown()&&!m.isCtrlDown()&&code==juce::KeyPress::pageUpKey){adjustValue(1,true);return true;}
 if(m.isAltDown()&&!m.isCtrlDown()&&code==juce::KeyPress::pageDownKey){adjustValue(-1,true);return true;}
 if(m.isAltDown()&&!m.isCtrlDown()&&code==juce::KeyPress::homeKey){extreme(true);return true;}
 if(m.isAltDown()&&!m.isCtrlDown()&&code==juce::KeyPress::endKey){extreme(false);return true;}
 if(code==juce::KeyPress::returnKey){
  if(realStep()){picker=true;search.reset();refresh(true);}else announce("Alt+E edits this Song value");
  return true;
 }
 if(!m.isCtrlDown()&&!m.isAltDown()&&!m.isShiftDown()&&
    code==juce::KeyPress::backspaceKey){resetCurrentValue();return true;}
 if(code==juce::KeyPress::deleteKey){if(selectionActive())clearSelected();else eraseCurrent();return true;}
 // Region navigation moved to Ctrl+Up/Down; keep Ctrl+Home/End inert in F2 grid.
 if(m.isCtrlDown()&&!m.isAltDown()&&
    (code==juce::KeyPress::homeKey||code==juce::KeyPress::endKey))return true;
 if(code==juce::KeyPress::homeKey||code==juce::KeyPress::endKey){
  const bool first=code==juce::KeyPress::homeKey;
  if(lane==3)return true;
  if(row<0)return true; // Same as F1: header cells use Left/Right, not Home/End.
  else if(realStep()){row=first?0:length()-1;lastStep[(size_t)lane]=row;}
  else if(row>=length()){row=first?length():length()+(song().laneRepeats[(size_t)lane]+9)/10;cell=0;}
  refresh(true);return true;
 }
 if(code==juce::KeyPress::upKey||code==juce::KeyPress::downKey||
    code==juce::KeyPress::pageUpKey||code==juce::KeyPress::pageDownKey){
  vertical((code==juce::KeyPress::upKey?-1:code==juce::KeyPress::downKey?1:code==juce::KeyPress::pageUpKey?-5:5));return true;
 }
 if(code==juce::KeyPress::leftKey||code==juce::KeyPress::rightKey){
  const int next=juce::jlimit(0,visibleCount()-1,cell+(code==juce::KeyPress::leftKey?-1:1));
  if(next!=cell){clearSelection();cell=next;if(realStep())logicalCell=cell;refresh(true);}return true;
 }
 return true; // keep the Alt+Q keyboard focus inside the native Song grid
}
}

namespace biscot {
int TrackerSongPanel::cursorRegion() const {
 if(lane>=3||row<0)return 0;
 if(row<length())return 1;
 return row==length()?2:3;
}
void TrackerSongPanel::setCursorRegion(int region){
 if(lane>=3)lane=lastOperatorLane;
 if(region==0)row=-1;
 else if(region==1)row=juce::jlimit(0,length()-1,lastStep[(size_t)lane]);
 else if(region==2)row=length();
 else row=length()+1;
 cell=logicalCell=0;refresh(false);
}
}
