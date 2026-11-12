// SPDX-License-Identifier: AGPL-3.0-or-later
#include "TrackerPatternPanel.h"
#include "ScreenReaderAnnouncer.h"
#include <algorithm>
#include <cmath>
#include <utility>
#include <string>

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
 help.setText("F1 Pattern | F2 Parameters | 1/2/3 OP/Filter/LFO | Esc: synth",juce::dontSendNotification);
 for(auto* label:{&header,&help}){label->setAccessible(false);label->setColour(juce::Label::textColourId,fg);addAndMakeVisible(*label);}
 navigator.setName("Tracker Grid");navigator.setDescription({});navigator.setWantsKeyboardFocus(true);
 navigator.setEditableText(false);navigator.setExplicitFocusOrder(1);
 navigator.onChange=[this]{if(!refreshing && navigator.getSelectedId()>0){
  if(page==1){cell=navigator.getSelectedId()-1;announce(positionText());}
  else {parameterIndex=navigator.getSelectedId()-1;announce(positionText());}
 }};
 addAndMakeVisible(navigator);
 numeric.setName("Tracker numeric edit");numeric.setDescription({});numeric.setMultiLine(false);
 numeric.setWantsKeyboardFocus(true);numeric.setExplicitFocusOrder(2);
 addAndMakeVisible(numeric);numeric.setVisible(false);
 refreshParameterList();refresh();
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
 parameterSearch.reset();clearSelection();page=1;refresh();setVisible(true);toFront(false);
 juce::Component::SafePointer<TrackerPatternPanel> safe(this);
 juce::MessageManager::callAsync([safe]{if(safe!=nullptr && safe->isShowing())safe->navigator.grabKeyboardFocus();});
}
void TrackerPatternPanel::restoreAfterPreset(){undo.clear();redo.clear();clearSelection();parameterSearch.reset();row=-1;cell=0;page=1;refresh();}
void TrackerPatternPanel::close(){setVisible(false);if(onClose)onClose();}
void TrackerPatternPanel::refreshParameterList(){
 parameterList.clear();for(const auto& p:trackerCatalog)if(p.tab==tab+1)parameterList.push_back(&p);
 parameterIndex=juce::jlimit(0,juce::jmax(0,(int)parameterList.size()-1),lastParam[(size_t)tab]);
}
const TrackerParam* TrackerPatternPanel::currentParameter() const{
 if(page!=2||parameterList.empty())return nullptr;
 return parameterList[(size_t)juce::jlimit(0,(int)parameterList.size()-1,parameterIndex)];
}
TrackerCell* TrackerPatternPanel::selectedCell(){
 if(page!=1||row<0||row>=currentPattern().length)return nullptr;
 auto& values=currentPattern().cells(row,tab);
 if(cell<0||cell>=(int)values.size())return nullptr;
 return &values[(size_t)cell];
}
juce::String TrackerPatternPanel::cellText(const TrackerCell& c) const{
 const auto* p=findTrackerParam(c.id);
 juce::String value=juce::String(c.value,4).trimCharactersAtEnd("0").trimCharactersAtEnd(".");
 if(c.id==1||c.id==7)value=noteName((int)std::round(c.value));
 if(c.id==3)value="Previous value";
 if(c.id==6)value="Pause";
 return juce::String(p?p->name:"Unknown parameter")+", "+value;
}
juce::String TrackerPatternPanel::positionText() const{
 const int n=processor.trackerPatterns.activePattern;
 const auto& p=*processor.trackerPatterns.find(n);
 if(page==2){const auto* def=currentParameter();return "Parameters "+juce::String(sectionNames[tab])+". "+juce::String(def?def->name:"empty");}
 if(row==-1){return "Pattern "+juce::String(n)+". "+
  (cell==0?("Length, "+juce::String(p.length)):
   ("MIDI Note, "+(p.midiNote<0?juce::String("Off"):noteName(p.midiNote))));}
 if(row==p.length)return "Pattern "+juce::String(n)+". Repeat, "+juce::String(p.repeat);
 if(row>p.length){
  const int idx=(row-p.length-1)*5+cell/2;
  if(idx<0||idx>=p.repeat)return "Pattern "+juce::String(n)+". Speed row "+juce::String(row-p.length);
  return "Pattern "+juce::String(n)+". "+(cell%2==0?"Speed ":"Transpose ")+juce::String(idx+1)+", "+
   (cell%2==0?juce::String(p.repeatSpeeds[(size_t)idx],2):juce::String(p.repeatTransposes[(size_t)idx]));
 }
 const auto& values=p.view(row,tab);
 return "Pattern "+juce::String(n)+". "+sectionNames[tab]+". step "+juce::String(row)+". "+
  (values.empty()?juce::String("empty"):cellText(values[(size_t)juce::jlimit(0,(int)values.size()-1,cell)]));
}
void TrackerPatternPanel::refresh(bool speakPosition){
 auto& pat=currentPattern();
 const int maxRow=pat.length+(pat.repeat+4)/5;
 row=juce::jlimit(-1,maxRow,row);
 refreshing=true;navigator.clear(juce::dontSendNotification);
 if(page==2){
  refreshParameterList();
  for(int i=0;i<(int)parameterList.size();++i)navigator.addItem(juce::String(parameterList[(size_t)i]->name),i+1);
  navigator.setSelectedId(parameterIndex+1,juce::dontSendNotification);
  header.setText("Biscot1 Tracker - F2 "+juce::String(sectionNames[tab])+" Parameters",juce::dontSendNotification);
 }else{
  header.setText("Biscot1 Tracker - F1 Pattern "+juce::String(processor.trackerPatterns.activePattern)+" / "+sectionNames[tab],juce::dontSendNotification);
  if(row==-1){
   navigator.addItem("Length, "+juce::String(pat.length),1);
   navigator.addItem("MIDI Note, "+(pat.midiNote<0?juce::String("Off"):noteName(pat.midiNote)),2);
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
  cell=juce::jlimit(0,juce::jmax(0,navigator.getNumItems()-1),cell);
  navigator.setSelectedId(cell+1,juce::dontSendNotification);
 }
 refreshing=false;
 repaint();if(speakPosition)announce(positionText());
}
void TrackerPatternPanel::recordUndo(){
 const int n=processor.trackerPatterns.activePattern;
 auto& stack=undo[n];if(stack.size()>=128)stack.erase(stack.begin());stack.push_back(currentPattern());redo[n].clear();
}
void TrackerPatternPanel::finishEdit(){processor.commitTrackerState();refresh(true);}
void TrackerPatternPanel::changeValue(double v){
 auto& p=currentPattern();
 if(page!=1)return;
 if(row==-1){recordUndo();if(cell==0){p.length=juce::jlimit(1,1024,(int)std::round(v));row=juce::jmin(row,p.length);}else p.midiNote=juce::jlimit(-1,127,(int)std::round(v));finishEdit();return;}
 if(row==p.length){recordUndo();p.repeat=juce::jlimit(1,1000,(int)std::round(v));p.repeatSpeeds.resize((size_t)p.repeat,1.0);p.repeatTransposes.resize((size_t)p.repeat,0);finishEdit();return;}
 if(row>p.length){const int idx=(row-p.length-1)*5+cell/2;if(idx>=0&&idx<p.repeat){recordUndo();if(cell%2==0)p.repeatSpeeds[(size_t)idx]=juce::jlimit(0.25,16.0,v);else p.repeatTransposes[(size_t)idx]=juce::jlimit(-96,96,(int)std::round(v));finishEdit();}return;}
 if(auto* c=selectedCell()){
  if(const auto* d=findTrackerParam(c->id)){
   recordUndo();c->value=juce::jlimit(d->min,d->max,v);finishEdit();
  }
 }
}
void TrackerPatternPanel::adjustValue(int direction,double multiplier){
 if(page!=1)return;
 const auto& p=currentPattern();double old=0,step=1;
 if(row==-1){old=cell==0?p.length:p.midiNote;}
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
 tab=next;cell=0;refresh(true);
}
void TrackerPatternPanel::changeRow(int delta){clearSelection();const int to=juce::jlimit(-1,currentPattern().length+(currentPattern().repeat+4)/5,row+delta);
 if(to==row)return;row=to;cell=0;refresh(true);
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
 cell=0;refresh();
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
 clearSelection();cell=0;processor.commitTrackerState();refresh();
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
 clearSelection();cell=0;processor.commitTrackerState();refresh();
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
 clearSelection();cell=0;finishEdit();announce("Step "+juce::String(row)+" "+sectionNames[tab]+" initialized");
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
 clearSelection();cell=0;finishEdit();announce(juce::String(sectionNames[tab])+" table cleared");
}
bool TrackerPatternPanel::keyPressed(const juce::KeyPress& k){return handle(k,false);}
bool TrackerPatternPanel::handle(const juce::KeyPress& k,bool editing){
 const auto m=k.getModifiers();const int code=k.getKeyCode();
 const auto letter=[&](char c){return juce::CharacterFunctions::toLowerCase(k.getTextCharacter())==c;};
 if(editing){
  if(code==juce::KeyPress::returnKey){
   const auto txt=numeric.getText().trim().replaceCharacter(',','.');
   if(txt.isNotEmpty()){changeValue(txt.getDoubleValue());}
   numeric.setVisible(false);navigator.grabKeyboardFocus();return true;
  }
  if(code==juce::KeyPress::escapeKey){numeric.setVisible(false);navigator.grabKeyboardFocus();return true;}
  return false;
 }
 if(code==juce::KeyPress::escapeKey || (m.isAltDown() && letter('q'))){parameterSearch.reset();clearSelection();close();return true;}
 if(m.isAltDown() && !m.isCtrlDown() && letter('s')){if(onPresetBrowser)onPresetBrowser(true);return true;}
 if(m.isAltDown() && !m.isCtrlDown() && letter('b')){if(onPresetBrowser)onPresetBrowser(false);return true;}
 if(code==juce::KeyPress::F1Key){page=1;parameterSearch.reset();refresh(true);return true;}
 if(code==juce::KeyPress::F2Key){if(row>=0&&row<currentPattern().length){clearSelection();page=2;parameterSearch.reset();refresh(true);}return true;}
 if(code>=juce::KeyPress::F3Key&&code<=juce::KeyPress::F6Key){announce("Song, browser and diagnostics arrive in later tracker versions");return true;}
 if(m.isCtrlDown()&&code==juce::KeyPress::tabKey&&page==2){setTab((tab+(m.isShiftDown()?2:1))%3);return true;}
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
    auto& pat=currentPattern();const int insertAt=juce::jlimit(0,(int)pat.view(row,tab).size(),cell+1);
    if(pat.totalCells(row)>=128){announce("Step is full, 128 cells");return true;}
    recordUndo();if(pat.insert(row,tab,insertAt,{def->id,def->initial})){
     cell=insertAt;processor.commitTrackerState();announce(juce::String(def->name)+" inserted");
    }
   }
   if(!m.isCtrlDown()){page=1;refresh(true);}return true;
  }
  // Multi-character F2 search, with the same whole-label/word-prefix,
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
 if(m.isShiftDown() && !m.isCtrlDown() && !m.isAltDown()){
  if(code==juce::KeyPress::upKey){extendStepSelection(-1);return true;}
  if(code==juce::KeyPress::downKey){extendStepSelection(1);return true;}
  if(code==juce::KeyPress::pageUpKey){extendStepSelection(-5);return true;}
  if(code==juce::KeyPress::pageDownKey){extendStepSelection(5);return true;}
 }
 if(!m.isAltDown() && !m.isCtrlDown() && !m.isShiftDown()){
  if(letter('1')||letter('2')||letter('3')){setTab(k.getTextCharacter()-'1');return true;}
 }
 if(m.isCtrlDown() && (code==juce::KeyPress::upKey||code==juce::KeyPress::downKey)){
  auto& bank=processor.trackerPatterns;
  const int next=juce::jlimit(0,999,bank.activePattern+(code==juce::KeyPress::upKey?-1:1));
  if(next!=bank.activePattern){clearSelection();bank.activePattern=next;row=-1;cell=0;processor.commitTrackerState();refresh(true);}return true;
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
  }return true;
 }
 if(m.isCtrlDown() && letter('x')){
  if(hasStepSelection()){eraseSelectedSteps(true);return true;}
  if(const auto* c=selectedCell()){
   clipboard=*c;hasClipboard=true;clipboardMode=ClipboardMode::parameter;
   recordUndo();currentPattern().erase(row,tab,cell);finishEdit();announce("Parameter cut");
  }return true;
 }
 if(m.isCtrlDown() && letter('v')){
  if(clipboardMode==ClipboardMode::steps){pasteSelectedSteps();return true;}
  if(clipboardMode==ClipboardMode::parameter && hasClipboard && row>=0&&row<currentPattern().length){
   auto& pat=currentPattern();if(pat.totalCells(row)<128){recordUndo();const int at=cell+1;
    pat.insert(row,tab,at,clipboard);cell=at;finishEdit();}
  }return true;
 }
 if(m.isCtrlDown() && code==juce::KeyPress::leftKey && row>=0 && row<currentPattern().length){
  auto& a=currentPattern().cells(row,tab);if(cell>0&&cell<(int)a.size()){
   recordUndo();std::swap(a[(size_t)cell],a[(size_t)(cell-1)]);--cell;finishEdit();}return true;
 }
 if(m.isCtrlDown() && code==juce::KeyPress::rightKey && row>=0 && row<currentPattern().length){
  auto& a=currentPattern().cells(row,tab);if(cell>=0&&cell+1<(int)a.size()){
   recordUndo();std::swap(a[(size_t)cell],a[(size_t)(cell+1)]);++cell;finishEdit();}return true;
 }
 if(m.isAltDown()){
  if(letter('e')){double v=0;
   if(row==currentPattern().length)v=currentPattern().repeat;
   else if(row==-1)v=cell==0?currentPattern().length:currentPattern().midiNote;
   else if(row>currentPattern().length){const int idx=(row-currentPattern().length-1)*5+cell/2;
    if(idx<currentPattern().repeat)v=cell%2==0?currentPattern().repeatSpeeds[(size_t)idx]:currentPattern().repeatTransposes[(size_t)idx];
   }else if(auto* c=selectedCell())v=c->value;
   numeric.setText(juce::String(v,4),juce::dontSendNotification);numeric.setVisible(true);numeric.grabKeyboardFocus();numeric.selectAll();
  return true;}
  if(code==juce::KeyPress::upKey){adjustValue(1);return true;}
  if(code==juce::KeyPress::downKey){adjustValue(-1);return true;}
  if(code==juce::KeyPress::pageUpKey){adjustValue(1,10);return true;}
  if(code==juce::KeyPress::pageDownKey){adjustValue(-1,10);return true;}
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
 if(code==juce::KeyPress::pageUpKey){changeRow(-5);return true;}
 if(code==juce::KeyPress::pageDownKey){changeRow(5);return true;}
 if(code==juce::KeyPress::leftKey||code==juce::KeyPress::rightKey){
  clearSelection();const int next=juce::jlimit(0,juce::jmax(0,navigator.getNumItems()-1),cell+(code==juce::KeyPress::leftKey?-1:1));
  if(next!=cell){cell=next;refresh(true);}return true;
 }
 // User-preferred F1 direction: Home travels backward toward Length;
 // End travels forward toward Repeat and Speed. F2 is deliberately unchanged.
 if(code==juce::KeyPress::homeKey){
  clearSelection();
  if(row>currentPattern().length){row=currentPattern().length;cell=0;refresh(true);}
  else if(row==currentPattern().length){row=currentPattern().length-1;cell=0;refresh(true);}
  else if(row>=0){row=-1;cell=0;refresh(true);}
  return true;
 }
 if(code==juce::KeyPress::endKey){
  clearSelection();
  if(row==-1)changeRow(1);
  else if(row<currentPattern().length){row=currentPattern().length;cell=0;refresh(true);}
  else if(row==currentPattern().length)changeRow(1);
  return true;
 }
 return true; // All keys are owned by Alt+Q; never leak into REAPER shortcuts.
}
}
