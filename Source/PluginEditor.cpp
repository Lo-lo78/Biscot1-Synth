#include "PluginEditor.h"
#include <algorithm>
#include <cmath>
#if JUCE_WINDOWS
 #ifndef NOMINMAX
  #define NOMINMAX
 #endif
 #include <Windows.h>
 #include <UIAutomation.h>
 #include <oleauto.h>
#endif

namespace biscot {
namespace {
constexpr int rowsPerColumn=8;
const juce::Colour foreground=juce::Colour(0xffe5edf6);
const juce::Colour background=juce::Colour(0xff171b26);
}
// Native focus events already communicate roles and labels to NVDA.
// Do not generate a second UIA grabFocus, and do not mutate title/description
// on focus: that caused duplicate "parameter ... Grid ..." announcements.
void BiscotEditor::ParameterGrid::focusGained(FocusChangeType why) {
 juce::ComboBox::focusGained(why);
}
void BiscotEditor::ParameterValue::focusGained(FocusChangeType why) {
 juce::Slider::focusGained(why);
}
void BiscotEditor::NumericEdit::focusGained(FocusChangeType why) {
 juce::TextEditor::focusGained(why);
 selectAll();
}
BiscotEditor::BiscotEditor(BiscotProcessor& p):juce::AudioProcessorEditor(p),grid(*this),value(*this),edit(*this),proc(p),presetManager(p){
 setFocusContainerType(juce::Component::FocusContainerType::keyboardFocusContainer);
 setWantsKeyboardFocus(true);
 createGroups();
 title.setText("Biscot1 Synth - VST3 TEST0.2.41",juce::dontSendNotification);
 sectionLabel.setText({},juce::dontSendNotification);
 instructions.setText("Grid / Value / Edit | Alt+Shift+Left/Right: section | Alt+Q: tracker | Alt+B: presets | Alt+S: save | type: search",juce::dontSendNotification);
 for(auto* l : {&title,&sectionLabel,&instructions}){
  l->setColour(juce::Label::textColourId,foreground);
  l->setAccessible(false); // purely visual: no long startup announcement
  addAndMakeVisible(*l);
 }
 grid.setName("Grid");grid.setTitle({});grid.setDescription({});grid.setWantsKeyboardFocus(true);grid.setExplicitFocusOrder(1);
 grid.setEditableText(false);grid.setJustificationType(juce::Justification::centredLeft);
 grid.onChange=[this]{selectedFromGrid();};addAndMakeVisible(grid);
 value.setSliderStyle(juce::Slider::LinearHorizontal);
 value.setTextBoxStyle(juce::Slider::TextBoxRight,false,130,25);
 value.setWantsKeyboardFocus(true);value.setTitle({});value.setDescription({});value.setExplicitFocusOrder(2);addAndMakeVisible(value);
 edit.setMultiLine(false);edit.setName("Edit");edit.setTitle({});edit.setDescription({});edit.setWantsKeyboardFocus(true);edit.setExplicitFocusOrder(3);addAndMakeVisible(edit);
 presetBrowser=std::make_unique<PresetBrowser>(presetManager);
 addAndMakeVisible(*presetBrowser);
 presetBrowser->setVisible(false);
 presetBrowser->onClose=[this]{focusBackFromPreset();};
 presetBrowser->onPresetLoaded=[this]{
  lastTrackerWasSong=(int)proc.state.state.getProperty(juce::Identifier("trackerLastMode"),0)==1;
  refreshLabels(true);if(trackerPanel)trackerPanel->restoreAfterPreset();if(songPanel)songPanel->restoreAfterPreset();};
 trackerPanel=std::make_unique<TrackerPatternPanel>(proc);
 addAndMakeVisible(*trackerPanel);trackerPanel->setVisible(false);
 songPanel=std::make_unique<TrackerSongPanel>(proc);
 addAndMakeVisible(*songPanel);songPanel->setVisible(false);
 libraryBrowser=std::make_unique<TrackerLibraryBrowser>(proc);
 addAndMakeVisible(*libraryBrowser);libraryBrowser->setVisible(false);
 libraryBrowser->onClose=[this](TrackerLibraryBrowser::Mode mode,bool imported,const juce::String& result){
  if(imported){
   if(mode==TrackerLibraryBrowser::Mode::patterns)trackerPanel->restoreAfterPreset();
   else songPanel->restoreAfterPreset();
  }
  showSynthWidgets(false);
  if(mode==TrackerLibraryBrowser::Mode::patterns){
   trackerPanel->open();
   if(result.isNotEmpty()){
    juce::Component::SafePointer<TrackerPatternPanel> safe(trackerPanel.get());
    juce::MessageManager::callAsync([safe,result]{if(safe!=nullptr&&safe->isShowing())safe->announceLibraryResult(result);});
   }
  }else{
   songPanel->open();
   if(result.isNotEmpty()){
    juce::Component::SafePointer<TrackerSongPanel> safe(songPanel.get());
    juce::MessageManager::callAsync([safe,result]{if(safe!=nullptr&&safe->isShowing())safe->announceLibraryResult(result);});
   }
  }
 };
 trackerPanel->onLibraryBrowser=[this]{
  trackerPanel->suspendForLibrary();libraryBrowser->open(TrackerLibraryBrowser::Mode::patterns);
 };
 songPanel->onLibraryBrowser=[this]{
  songPanel->suspendForLibrary();libraryBrowser->open(TrackerLibraryBrowser::Mode::songs);
 };
 songPanel->onPattern=[this]{
  lastTrackerWasSong=false;proc.state.state.setProperty(juce::Identifier("trackerLastMode"),0,nullptr);
  showSynthWidgets(false);if(trackerPanel){trackerPanel->setCursorRegion(songPanel->cursorRegion());trackerPanel->open();}};
 songPanel->onExit=[this]{songPanel->setVisible(false);showSynthWidgets(true);focus(Control::grid,true);};
 songPanel->onPresetBrowser=[this](bool save){
  songBrowserReturn=true;songPanel->close();
  showSynthWidgets(true);openPresetBrowser(save);
 };
 trackerPanel->onOpenSong=[this]{
  if(!songPanel)return;
  trackerPanel->suspendForSong();
  lastTrackerWasSong=true;
  songPanel->setCursorRegion(trackerPanel->cursorRegion());
  proc.state.state.setProperty(juce::Identifier("trackerLastMode"),1,nullptr);
  songPanel->open();
 };
 trackerPanel->onClose=[this]{showSynthWidgets(true);focus(Control::grid,true);};
 trackerPanel->onPresetBrowser=[this](bool save){
  trackerBrowserReturn=true;trackerPanel->setVisible(false);
  showSynthWidgets(true);openPresetBrowser(save);
 };
 setSize(940,610);
 lastTrackerWasSong=(int)proc.state.state.getProperty(juce::Identifier("trackerLastMode"),0)==1;
 restoreLocation();
 populateGroup();
 startTimerHz(8);
 // No eager focus handoff: REAPER announces its own FX window once.
 // As soon as the editor receives keyboard focus, initialFocus() selects Grid.
}
BiscotEditor::~BiscotEditor(){
 stopTimer();
 if(libraryBrowser){libraryBrowser->onClose={};libraryBrowser.reset();}
 if(songPanel){songPanel->onLibraryBrowser={};songPanel->onPattern={};songPanel->onExit={};songPanel->onPresetBrowser={};songPanel.reset();}
 if(trackerPanel){trackerPanel->onLibraryBrowser={};trackerPanel->onOpenSong={};trackerPanel->onClose={};trackerPanel->onPresetBrowser={};trackerPanel.reset();}
 if(presetBrowser){presetBrowser->onClose={};presetBrowser->onPresetLoaded={};presetBrowser->close();presetBrowser.reset();}
 attachment.reset();grid.onChange=nullptr;
}
// Match LB-303: the editor root is a keyboard focus container, not a
// screen-reader element. REAPER can focus it temporarily while opening FX;
// exposing this JUCE root produces NVDA's spurious "unknown" announcement.
// Keep the native ComboBox, Slider and TextEditor accessible as before.
std::unique_ptr<juce::AccessibilityHandler> BiscotEditor::createAccessibilityHandler(){
 return createIgnoredAccessibilityHandler(*this);
}
void BiscotEditor::createGroups(){
 for(const auto& d:catalog){
  int i=d.index,g=0;
  if(i>=122 && i<=190) g=5+(i-122)/23;
  else if(i>=76 && i<=93)g=8;
  else if((i>=94&&i<=103)||(i>=191&&i<=202))g=9;
  else if((i>=104&&i<=107)||(i>=203&&i<=210))g=10;
  else if((i>=58&&i<=63)||(i>=211&&i<=225))g=11;
  else if(i>=108&&i<=121)g=4;
  else if(juce::String(d.name).startsWith("Op1 "))g=1;
  else if(juce::String(d.name).startsWith("Op2 "))g=2;
  else if(juce::String(d.name).startsWith("Op3 "))g=3;
  groups[(size_t)g].push_back(i);
 }
 // Append-only LFO Square PWM parameters belong beside each LFO Wave,
 // not at the end of the Global page. Existing IDs and presets stay stable.
 for(int i=0;i<3;++i){
  auto& lfoPage=groups[(size_t)(5+i)];
  const int pwm=231+i, wave=124+i*23;
  const auto existing=std::find(lfoPage.begin(),lfoPage.end(),pwm);
  if(existing!=lfoPage.end())lfoPage.erase(existing);
  const auto at=std::find(lfoPage.begin(),lfoPage.end(),wave);
  if(at!=lfoPage.end())lfoPage.insert(at+1,pwm);
  else lfoPage.push_back(pwm);
 }
 // Sustain Pedal is an independent CC64 enable switch per operator,
 // placed beside that operator's ADSR controls rather than at the end.
 for(int i=0;i<3;++i){
  auto& operatorPage=groups[(size_t)(i+1)];
  const int sustainId=227+i,adsrSustain=40+i;
  const auto pedal=std::find(operatorPage.begin(),operatorPage.end(),sustainId);
  if(pedal!=operatorPage.end())operatorPage.erase(pedal);
  const auto afterSustain=std::find(operatorPage.begin(),operatorPage.end(),adsrSustain);
  if(afterSustain!=operatorPage.end())operatorPage.insert(afterSustain+1,sustainId);
  else operatorPage.push_back(sustainId);
 }
 // New stable s230 selector stays near s226 Pitch Bend Mode. Do not
 // renumber the JSFX parameters, existing presets, or REAPER automation.
 auto& global=groups[0];
 for(const int n : {226,230}){
  const auto found=std::find(global.begin(),global.end(),n);
  if(found!=global.end())global.erase(found);
 }
 const auto midiParam=std::find(global.begin(),global.end(),3);
 if(midiParam!=global.end()){
  const auto pos=std::find(global.begin(),global.end(),3)+1;
  global.insert(pos,{226,230});
 }
}
int BiscotEditor::currentIndex() const {return groups[(size_t)group][(size_t)selected];}
const ParameterDef& BiscotEditor::currentDef() const {return catalog[(size_t)(currentIndex()-2)];}
juce::String BiscotEditor::formatValue(int i) const{
 const auto& d=catalog[(size_t)(i-2)];const auto v=proc.param(i);
 if(d.choices && d.choices[0]){
  juce::StringArray a;a.addTokens(juce::String(d.choices),"|","");
  const int index=juce::jlimit(0,a.size()-1,juce::roundToInt(v));
  if(index<a.size())return a[index];
 }
 if(d.index>=231 && d.index<=233)return juce::String(v,4);
 if(d.step>=1.f)return juce::String(juce::roundToInt(v));
 return juce::String(v,d.step>=0.1f?1:(d.step>=0.01f?2:3));
}
juce::String BiscotEditor::labelFor(int row) const{
 const int index=groups[(size_t)group][(size_t)row];
 return juce::String(catalog[(size_t)(index-2)].name)+", "+formatValue(index);
}
void BiscotEditor::populateGroup(){
 updating=true;grid.clear(juce::dontSendNotification);
 const auto& entries=groups[(size_t)group];
 for(int row=0;row<(int)entries.size();++row)grid.addItem(labelFor(row),row+1);
 selected=juce::jlimit(0,(int)entries.size()-1,lastSelected[(size_t)group]);
 grid.setSelectedId(selected+1,juce::dontSendNotification);
 sectionLabel.setText(groupNames[(size_t)group]+" ("+juce::String(group+1)+"/12)",juce::dontSendNotification);
 updating=false;selectedFromGrid();repaint();
}
void BiscotEditor::selectedFromGrid(){
 if(updating)return;
 const int n=grid.getSelectedId()-1;
 if(!juce::isPositiveAndBelow(n,(int)groups[(size_t)group].size()))return;
 selected=n;lastSelected[(size_t)group]=n;
 rememberLocation();
 attachment.reset();
 const auto& d=currentDef();
 value.setRange(d.low,d.high,d.step>0?d.step:0.001);
 // Native Value announces the same four-decimal duty cycle as Grid.
 if(d.index>=231 && d.index<=233)
  value.textFromValueFunction=[](double v){return juce::String(v,4);};
 else value.textFromValueFunction={};
 value.setValue(proc.param(d.index),juce::dontSendNotification);
 value.setName("Value. "+juce::String(d.name));
 edit.setName("Edit. "+juce::String(d.name));
 edit.setText(formatValue(d.index),juce::dontSendNotification);
 attachment=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(proc.state,BiscotProcessor::id(d.index),value);
 repaint(drawingArea);
}
void BiscotEditor::refreshLabels(bool force){
 if(updating || (presetBrowser && presetBrowser->active()))return;
 bool changed=false;
 for(int i=0;i<(int)groups[(size_t)group].size();++i){
  const auto label=labelFor(i);
  if(force || label!=grid.getItemText(i)){
   grid.changeItemText(i+1,label);changed=true;
  }
 }
 if(changed){grid.setSelectedId(selected+1,juce::dontSendNotification);repaint(drawingArea);}
 if(!edit.hasKeyboardFocus(true))edit.setText(formatValue(currentIndex()),juce::dontSendNotification);
}
void BiscotEditor::timerCallback(){if(!presetBrowser || !presetBrowser->active())refreshLabels();}
void BiscotEditor::select(int i){
 i=juce::jlimit(0,(int)groups[(size_t)group].size()-1,i);
 if(i==selected)return; // silent boundaries: no selection change, no speech
 // Native ComboBox selection remains silent to avoid duplicate screen reader
 // announcements. We explicitly announce only the NEW parameter and value.
 grid.setSelectedId(i+1,juce::dontSendNotification);
 selectedFromGrid();
 announce(grid,labelFor(selected));
}
void BiscotEditor::move(int rows,int cols){
 const int total=(int)groups[(size_t)group].size();
 const int col=selected/rowsPerColumn;
 const int newCol=col+cols;
 if(newCol<0 || newCol*rowsPerColumn>=total)return;
 int target=juce::jlimit(newCol*rowsPerColumn,juce::jmin(total-1,(newCol+1)*rowsPerColumn-1),selected+rows+cols*rowsPerColumn);
 select(target);
}
void BiscotEditor::changeGroup(int dir){
 const int next=group+dir;
 if(next<0||next>=12)return;
 group=next;parameterSearch.reset();populateGroup();
 // One concise page announcement: no redundant value/title changes.
 announce(grid,groupNames[(size_t)group]+". "+labelFor(selected));
}
void BiscotEditor::setValue(double x){
 const auto& d=currentDef();
 const float val=juce::jlimit(d.low,d.high,(float)x);
 if(auto* p=proc.state.getParameter(BiscotProcessor::id(d.index))){
  p->beginChangeGesture();p->setValueNotifyingHost(p->convertTo0to1(val));p->endChangeGesture();
 }
 value.setValue(proc.param(d.index),juce::dontSendNotification);
 edit.setText(formatValue(d.index),juce::dontSendNotification);refreshLabels(false);
 announce(grid,labelFor(selected));
}
void BiscotEditor::adjustValue(int direction,double multiplier){
 const auto& d=currentDef();
 const double step=(d.choices[0] || d.step>=1.f)?1.0:juce::jmax((double)d.step,stepScales[(size_t)stepScaleIndex]);
 const double next=juce::jlimit((double)d.low,(double)d.high,(double)proc.param(d.index)+step*multiplier*direction);
 if(std::abs(next-proc.param(d.index))<0.0000001)return;
 setValue(next);
}
void BiscotEditor::boundary(bool maximum){setValue(maximum?currentDef().high:currentDef().low);}
void BiscotEditor::changeStep(int direction){
 const int n=juce::jlimit(0,(int)stepScales.size()-1,stepScaleIndex+direction);
 if(n==stepScaleIndex)return;
 stepScaleIndex=n;announce(grid,"Step "+juce::String(stepScales[(size_t)stepScaleIndex],3));
}
void BiscotEditor::resetValue(){setValue(currentDef().initial);}
void BiscotEditor::enterEdit(){edit.setText(formatValue(currentIndex()),juce::dontSendNotification);focus(Control::edit);edit.selectAll();}
bool BiscotEditor::commitEdit(){
 const auto& d=currentDef();
 if(d.choices[0]){
  juce::StringArray labels;labels.addTokens(juce::String(d.choices),"|","");
  // StringArray has no indexOfIgnoreCase() in JUCE 8.
  // Preserve case-insensitive entry of named choices in the accessible editor.
  const auto entered=edit.getText().trim();
  int n=-1;
  for(int i=0;i<labels.size();++i){
   if(labels[i].equalsIgnoreCase(entered)){n=i;break;}
  }
  if(n<0)n=entered.getIntValue();
  setValue(n);return true;
 }
 const juce::String text=edit.getText().trim();
 if(text.isEmpty())return false;
 bool valid=true;
 for(int i=0;i<text.length();++i){const auto c=text[i];if(!juce::CharacterFunctions::isDigit(c) && c!='-' && c!='+' && c!='.' && c!=','){valid=false;break;}}
 if(!valid){announce(edit,"Invalid numeric value");return false;}
 setValue(text.replaceCharacter(',','.').getDoubleValue());return true;
}
void BiscotEditor::cancelEdit(){edit.setText(formatValue(currentIndex()),juce::dontSendNotification);}
void BiscotEditor::focus(Control c,bool deferred){
 auto* destination=c==Control::grid?static_cast<juce::Component*>(&grid):c==Control::value?static_cast<juce::Component*>(&value):static_cast<juce::Component*>(&edit);
 auto action=[safe=juce::Component::SafePointer<BiscotEditor>(this),destination]{
  if(safe==nullptr)return;
  destination->grabKeyboardFocus();
 };
 if(deferred)juce::MessageManager::callAsync(action);else action();
}
void BiscotEditor::initialFocus(){
 if((libraryBrowser && libraryBrowser->isVisible()) || (trackerPanel && trackerPanel->isVisible()) || (songPanel && songPanel->isVisible()))return;
 if(initialFocusPending)return;
 initialFocusPending=true;
 // LB-303 pattern: do not hand off focus until the native UIA child exists.
 // A host often focuses the unnamed editor root first (NVDA: "unknown").
 juce::Timer::callAfterDelay(110,[safe=juce::Component::SafePointer<BiscotEditor>(this)]{
  if(safe==nullptr)return;safe->initialFocusPending=false;
  if(!safe->isShowing() || (safe->presetBrowser && safe->presetBrowser->active()))return;
  auto* focused=juce::Component::getCurrentlyFocusedComponent();
  if(focused!=nullptr && focused!=safe.getComponent())return; // LB-303: never steal Value/Edit or REAPER focus
  auto* peer=safe->getPeer();
  auto* handler=safe->grid.getAccessibilityHandler();
  if(peer==nullptr || !peer->isFocused() || handler==nullptr ||
     handler->getNativeImplementation()==nullptr){
   if(++safe->focusRetries<8)safe->initialFocus();
   return;
  }
  safe->focusRetries=0;
  safe->grid.hidePopup();
  juce::AccessibilityHandler::clearCurrentlyFocusedHandler();
  safe->grid.grabKeyboardFocus();
  if(safe->grid.hasKeyboardFocus(false))handler->grabFocus();
  else if(++safe->focusRetries<8)safe->initialFocus();
 });
}
void BiscotEditor::focusGained(FocusChangeType){initialFocus();}
bool BiscotEditor::keyPressed(const juce::KeyPress& key){
 if(libraryBrowser && libraryBrowser->isVisible())return libraryBrowser->keyPressed(key);
 if(trackerPanel && trackerPanel->isVisible())return trackerPanel->keyPressed(key);
 if(songPanel && songPanel->isVisible())return songPanel->keyPressed(key);
 if(presetBrowser && presetBrowser->active())return presetBrowser->shortcut(key);
 const Control c=edit.hasKeyboardFocus(true)?Control::edit:value.hasKeyboardFocus(true)?Control::value:Control::grid;
 return handleKey(key,c);
}
bool BiscotEditor::handleKey(const juce::KeyPress& k,Control c){
 if(presetBrowser && presetBrowser->active())return presetBrowser->shortcut(k);
 const auto m=k.getModifiers();const int code=k.getKeyCode();
 // Reuse the proven LB-303 incremental search: leading words, Shift-backward,
 // Shift+digits, repeated-key cycling, 1000ms prefix window, no value matching.
 if(biscotParameterSearchCharacter(k)==0 || c==Control::edit)parameterSearch.reset();
 auto letter=[&](char ch){return juce::CharacterFunctions::toLowerCase(k.getTextCharacter())==ch;};
 if(m.isAltDown() && m.isShiftDown() && !m.isCtrlDown() && !m.isCommandDown()
    && (code==juce::KeyPress::rightKey || code==juce::KeyPress::leftKey)){
  if(c==Control::edit && !commitEdit())return true;
  const int oldGroup=group;
  changeGroup(code==juce::KeyPress::rightKey?1:-1);
  if(c==Control::edit && group!=oldGroup)focus(Control::grid,true);
  else if(c==Control::value && group!=oldGroup){}
  return true; // never let REAPER consume page-navigation arrows
 }
 if(m.isCtrlDown() && code==juce::KeyPress::tabKey){changeGroup(m.isShiftDown()?-1:1);return true;}
 if(!m.isAltDown() && m.isCtrlDown() && !m.isShiftDown() && letter('o')){
  openPresetBrowser(false);return true;
 }
 if(m.isAltDown() && !m.isCtrlDown()){
  if(letter('b')){openPresetBrowser(false);return true;}
  if(letter('s')){openPresetBrowser(true);return true;}
  if(letter('l')){if(c!=Control::grid)focus(Control::grid,c==Control::edit);return true;}
  if(letter('e')){enterEdit();return true;}
  if(letter('q')){openTracker();return true;}
  if(code==juce::KeyPress::upKey){adjustValue(1,m.isShiftDown()?10:1);return true;}
  if(code==juce::KeyPress::downKey){adjustValue(-1,m.isShiftDown()?10:1);return true;}
  if(code==juce::KeyPress::leftKey){changeStep(-1);return true;}
  if(code==juce::KeyPress::rightKey){changeStep(1);return true;}
  if(code==juce::KeyPress::homeKey){boundary(true);return true;}
  if(code==juce::KeyPress::endKey){boundary(false);return true;}
  if(code==juce::KeyPress::pageUpKey){adjustValue(1,40);return true;}
  if(code==juce::KeyPress::pageDownKey){adjustValue(-1,40);return true;}
  return c==Control::edit;
 }
 if(c==Control::edit){
  if(code==juce::KeyPress::escapeKey){cancelEdit();focus(Control::grid,true);return true;}
  if(code==juce::KeyPress::returnKey || code==juce::KeyPress::tabKey){
   if(!commitEdit())return true;
   focus(code==juce::KeyPress::tabKey && m.isShiftDown()?Control::value:Control::grid,true);return true;
  }
  return false;
 }
 if(code==juce::KeyPress::tabKey){
  if(c==Control::grid)focus(m.isShiftDown()?Control::edit:Control::value);
  else focus(m.isShiftDown()?Control::grid:Control::edit);
  return true;
 }
 if(code==juce::KeyPress::returnKey){focus(c==Control::grid?Control::value:Control::grid);return true;}
 if(code==juce::KeyPress::backspaceKey){resetValue();return true;}
 if(m.isCtrlDown() && code==juce::KeyPress::homeKey){select(0);return true;}
 if(m.isCtrlDown() && code==juce::KeyPress::endKey){select((int)groups[(size_t)group].size()-1);return true;}
 if(c==Control::grid || c==Control::value){
  if(const auto ch=biscotParameterSearchCharacter(k);ch!=0){
   const auto& entries=groups[(size_t)group];
   const int found=parameterSearch.next(
      (wchar_t)ch,selected,(int)entries.size(),m.isShiftDown()?-1:1,
      juce::Time::getMillisecondCounter(),[this,&entries](int n){
       return std::wstring(juce::String(catalog[(size_t)(entries[(size_t)n]-2)].name).trimStart().toWideCharPointer());
      });
   if(found>=0){
    if(found!=selected){
      select(found); // select() makes the ONE concise navigation announcement
    }else{
      // Repeated search on the current parameter remains audible.
      announce(c==Control::value?static_cast<juce::Component&>(value):static_cast<juce::Component&>(grid),labelFor(selected));
    }
   }
   return true; // prevent native ComboBox one-letter cycling
  }
 }
 if(c==Control::value){
  if(code==juce::KeyPress::upKey){adjustValue(1,1);return true;}
  if(code==juce::KeyPress::downKey){adjustValue(-1,1);return true;}
  if(code==juce::KeyPress::leftKey){changeStep(-1);return true;}
  if(code==juce::KeyPress::rightKey){changeStep(1);return true;}
  if(code==juce::KeyPress::pageUpKey){adjustValue(1,40);return true;}
  if(code==juce::KeyPress::pageDownKey){adjustValue(-1,40);return true;}
  if(code==juce::KeyPress::homeKey){boundary(true);return true;}
  if(code==juce::KeyPress::endKey){boundary(false);return true;}
  return false;
 }
 if(code==juce::KeyPress::upKey){move(-1,0);return true;}
 if(code==juce::KeyPress::downKey){move(1,0);return true;}
 if(code==juce::KeyPress::leftKey){move(0,-1);return true;}
 if(code==juce::KeyPress::rightKey){move(0,1);return true;}
 const int start=selected/rowsPerColumn*rowsPerColumn;
 if(code==juce::KeyPress::homeKey){select(start);return true;}
 if(code==juce::KeyPress::endKey){select(juce::jmin(start+rowsPerColumn-1,(int)groups[(size_t)group].size()-1));return true;}
 if(code==juce::KeyPress::pageUpKey){select(juce::jmax(start,selected-5));return true;}
 if(code==juce::KeyPress::pageDownKey){select(juce::jmin(juce::jmin(start+rowsPerColumn-1,(int)groups[(size_t)group].size()-1),selected+5));return true;}
 // Never return unhandled arrow keys to the host: REAPER could move focus out of VST.
 return false;
}
// The selection is part of the host-restored state, not an extra audio parameter.
void BiscotEditor::rememberLocation(){
 const int index=currentIndex();
 proc.state.state.setProperty(juce::Identifier("editorLastParameter"),index,nullptr);
}
void BiscotEditor::restoreLocation(){
 const int previous=(int)proc.state.state.getProperty(juce::Identifier("editorLastParameter"),2);
 for(int g=0;g<(int)groups.size();++g){
  const auto& entries=groups[(size_t)g];
  const auto found=std::find(entries.begin(),entries.end(),previous);
  if(found!=entries.end()){
   group=g;
   lastSelected[(size_t)g]=(int)std::distance(entries.begin(),found);
   selected=lastSelected[(size_t)g];
   return;
  }
 }
}
void BiscotEditor::showSynthWidgets(bool showing){
 for(auto* c:{static_cast<juce::Component*>(&title),static_cast<juce::Component*>(&sectionLabel),
              static_cast<juce::Component*>(&instructions),static_cast<juce::Component*>(&grid),
              static_cast<juce::Component*>(&value),static_cast<juce::Component*>(&edit)})
  c->setVisible(showing);
 repaint();
}
void BiscotEditor::openTracker(){
 if(!trackerPanel)return;
 if(presetBrowser && presetBrowser->active())return;
 showSynthWidgets(false);
 if(lastTrackerWasSong && songPanel)songPanel->open();
 else trackerPanel->open();
}
void BiscotEditor::openPresetBrowser(bool save){
 if(!presetBrowser)return;
 if(presetBrowser->active()){presetBrowser->close();return;}
 presetReturnControl=edit.hasKeyboardFocus(true)?Control::edit:(value.hasKeyboardFocus(true)?Control::value:Control::grid);
 presetReturnGroup=group;presetReturnSelection=selected;
 if(save)presetBrowser->saveAs();else presetBrowser->browse();
}
void BiscotEditor::focusBackFromPreset(){
 if(presetBrowser && presetBrowser->active())return;
 if(songBrowserReturn){
  songBrowserReturn=false;
  if(songPanel){showSynthWidgets(false);songPanel->open();}
  return;
 }
 if(trackerBrowserReturn){
  trackerBrowserReturn=false;
  if(trackerPanel){showSynthWidgets(false);trackerPanel->open();}
  return;
 }
 if(group!=presetReturnGroup){
  group=presetReturnGroup;lastSelected[(size_t)group]=presetReturnSelection;populateGroup();
 }else if(selected!=presetReturnSelection)select(presetReturnSelection);
 refreshLabels(true);rememberLocation();
 // Returning focus on the same UI message can leave ListBox as key recipient.
 focus(presetReturnControl,true);
 if(presetReturnControl==Control::grid){
  juce::Timer::callAfterDelay(70,[safe=juce::Component::SafePointer<BiscotEditor>(this)]{
   if(safe==nullptr || (safe->presetBrowser && safe->presetBrowser->active()))return;
   if(safe->value.hasKeyboardFocus(true) || safe->edit.hasKeyboardFocus(true))return;
   safe->grid.hidePopup();
   safe->grid.grabKeyboardFocus();
  });
 }
}
void BiscotEditor::announce(juce::Component& source,const juce::String& text)const{
 // Do not setDescription(text): NVDA reads it AGAIN during later Tab / Alt+L.
 #if JUCE_WINDOWS
 if(!UiaClientsAreListening())return;
 if(auto* handler=source.getAccessibilityHandler()){
  auto* unk=reinterpret_cast<IUnknown*>(handler->getNativeImplementation());
  if(!unk)return;
  IRawElementProviderSimple* provider=nullptr;
  if(SUCCEEDED(unk->QueryInterface(__uuidof(IRawElementProviderSimple),reinterpret_cast<void**>(&provider)))){
   auto* msg=SysAllocString(text.toWideCharPointer());auto* activity=SysAllocString(L"Biscot1");
   UiaRaiseNotificationEvent(provider,NotificationKind_Other,NotificationProcessing_ImportantMostRecent,msg,activity);
   SysFreeString(msg);SysFreeString(activity);provider->Release();
  }
 }
 #else
 if(auto* a=source.getAccessibilityHandler())a->notifyAccessibilityEvent(juce::AccessibilityEvent::titleChanged);
 #endif
}
void BiscotEditor::resized(){
 auto area=getLocalBounds().reduced(17);
 title.setBounds(area.removeFromTop(35));
 sectionLabel.setBounds(area.removeFromTop(30));
 instructions.setBounds(area.removeFromBottom(30));
 area.removeFromBottom(8);
 auto controls=area.removeFromBottom(48);
 edit.setBounds(controls.removeFromRight(160));controls.removeFromRight(10);
 value.setBounds(controls);
 area.removeFromBottom(14);
 grid.setBounds(area.removeFromTop(32));
 area.removeFromTop(10);drawingArea=area;
 if(presetBrowser)presetBrowser->setBounds(getLocalBounds().reduced(12));
 if(trackerPanel)trackerPanel->setBounds(getLocalBounds().reduced(9));
 if(songPanel)songPanel->setBounds(getLocalBounds().reduced(9));
 if(libraryBrowser)libraryBrowser->setBounds(getLocalBounds().reduced(9));
}
void BiscotEditor::paint(juce::Graphics& g){
 g.fillAll(background);
 if((libraryBrowser && libraryBrowser->isVisible()) || (trackerPanel && trackerPanel->isVisible()) || (songPanel && songPanel->isVisible()))return;
 const auto& entries=groups[(size_t)group];
 const int columns=((int)entries.size()+rowsPerColumn-1)/rowsPerColumn;
 if(columns<=0 || drawingArea.isEmpty())return;
 const int gap=8;
 const int w=(drawingArea.getWidth()-(columns-1)*gap)/columns;
 const int h=juce::jmax(24,juce::jmin(40,drawingArea.getHeight()/rowsPerColumn-4));
 for(int i=0;i<(int)entries.size();++i){
  const int col=i/rowsPerColumn, row=i%rowsPerColumn;
  auto r=juce::Rectangle<int>(drawingArea.getX()+col*(w+gap),drawingArea.getY()+row*(h+4),w,h);
  g.setColour(i==selected?juce::Colour(0xff365d82):juce::Colour(0xff222b38));g.fillRoundedRectangle(r.toFloat(),4.f);
  g.setColour(foreground);g.setFont(juce::Font(13.f));
  g.drawFittedText(labelFor(i),r.reduced(6,2),juce::Justification::centredLeft,1);
 }
}
}
