#include "PluginEditor.h"
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
void BiscotEditor::ParameterGrid::focusGained(FocusChangeType why) {
 setTitle(owner.groupNames[(size_t)owner.group]+". Grid");setDescription("Alt+L");
 juce::ComboBox::focusGained(why);
 juce::Timer::callAfterDelay(25,[safe=juce::Component::SafePointer<ParameterGrid>(this)]{
  if(safe!=nullptr && safe->hasKeyboardFocus(true)) if(auto* a=safe->getAccessibilityHandler())a->grabFocus();
 });
 juce::Timer::callAfterDelay(240,[safe=juce::Component::SafePointer<ParameterGrid>(this)]{
  if(safe!=nullptr && safe->hasKeyboardFocus(true)){safe->setTitle({});safe->setDescription({});}
 });
}
void BiscotEditor::ParameterValue::focusGained(FocusChangeType why) {
 setName("Value. "+juce::String(owner.currentDef().name));setTitle(getName());setDescription("Tab");
 juce::Slider::focusGained(why);
 juce::Timer::callAfterDelay(200,[safe=juce::Component::SafePointer<ParameterValue>(this)]{
  if(safe!=nullptr && safe->hasKeyboardFocus(true)){safe->setName(safe->owner.currentDef().name);safe->setTitle(safe->getName());safe->setDescription({});}
 });
}
void BiscotEditor::NumericEdit::focusGained(FocusChangeType why) {
 setTitle("Edit. "+juce::String(owner.currentDef().name));setDescription("Alt+E");
 juce::TextEditor::focusGained(why);selectAll();
 juce::Timer::callAfterDelay(25,[safe=juce::Component::SafePointer<NumericEdit>(this)]{
  if(safe!=nullptr && safe->hasKeyboardFocus(true)) if(auto* a=safe->getAccessibilityHandler())a->grabFocus();
 });
}
BiscotEditor::BiscotEditor(BiscotProcessor& p):juce::AudioProcessorEditor(p),grid(*this),value(*this),edit(*this),proc(p){
 setFocusContainerType(juce::Component::FocusContainerType::keyboardFocusContainer);
 setWantsKeyboardFocus(true);
 createGroups();
 title.setText("Biscot1 Synth — VST3 TEST0.2.1",juce::dontSendNotification);
 sectionLabel.setText({},juce::dontSendNotification);
 instructions.setText("Grid / Value / Edit | Ctrl+Tab: section | Alt+arrows: value / step | Alt+E: edit | Alt+L: grid",juce::dontSendNotification);
 for(auto* l : {&title,&sectionLabel,&instructions}){l->setColour(juce::Label::textColourId,foreground);addAndMakeVisible(*l);}
 grid.setName("Parameters");grid.setWantsKeyboardFocus(true);grid.setExplicitFocusOrder(1);
 grid.setEditableText(false);grid.setJustificationType(juce::Justification::centredLeft);
 grid.onChange=[this]{selectedFromGrid();};addAndMakeVisible(grid);
 value.setSliderStyle(juce::Slider::LinearHorizontal);
 value.setTextBoxStyle(juce::Slider::TextBoxRight,false,130,25);
 value.setWantsKeyboardFocus(true);value.setExplicitFocusOrder(2);addAndMakeVisible(value);
 edit.setMultiLine(false);edit.setWantsKeyboardFocus(true);edit.setExplicitFocusOrder(3);addAndMakeVisible(edit);
 setSize(940,610);
 populateGroup();
 startTimerHz(8);
 initialFocus();
}
BiscotEditor::~BiscotEditor(){stopTimer();attachment.reset();grid.onChange=nullptr;}
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
 attachment.reset();
 const auto& d=currentDef();
 value.setRange(d.low,d.high,d.step>0?d.step:0.001);
 value.setValue(proc.param(d.index),juce::dontSendNotification);
 value.setName(d.name);value.setTitle(d.name);
 edit.setTitle("Edit. "+juce::String(d.name));
 edit.setText(formatValue(d.index),juce::dontSendNotification);
 attachment=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(proc.state,BiscotProcessor::id(d.index),value);
 repaint(drawingArea);
}
void BiscotEditor::refreshLabels(bool force){
 if(updating)return;
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
void BiscotEditor::timerCallback(){refreshLabels();}
void BiscotEditor::select(int i){
 i=juce::jlimit(0,(int)groups[(size_t)group].size()-1,i);
 if(i==selected)return;
 grid.setTitle({});grid.setDescription({});
 grid.setSelectedId(i+1,juce::sendNotificationSync);
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
 group=next;searchText.clear();populateGroup();
 announce(grid,groupNames[(size_t)group]+". "+labelFor(selected));
}
void BiscotEditor::setValue(double x){
 const auto& d=currentDef();
 const float val=juce::jlimit(d.low,d.high,(float)x);
 if(auto* p=proc.state.getParameter(BiscotProcessor::id(d.index))){
  p->beginChangeGesture();p->setValueNotifyingHost(p->convertTo0to1(val));p->endChangeGesture();
 }
 value.setValue(proc.param(d.index),juce::dontSendNotification);
 edit.setText(formatValue(d.index),juce::dontSendNotification);refreshLabels(true);
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
  juce::AccessibilityHandler::clearCurrentlyFocusedHandler();
  destination->grabKeyboardFocus();
  if(auto* handler=destination->getAccessibilityHandler())handler->grabFocus();
 };
 if(deferred)juce::MessageManager::callAsync(action);else action();
}
void BiscotEditor::initialFocus(){
 if(initialFocusPending)return;
 initialFocusPending=true;
 juce::Timer::callAfterDelay(70,[safe=juce::Component::SafePointer<BiscotEditor>(this)]{
  if(safe==nullptr)return;safe->initialFocusPending=false;
  if(auto* peer=safe->getPeer();peer!=nullptr && peer->isFocused()){
   safe->focus(Control::grid);safe->focusRetries=0;
  }else if(++safe->focusRetries<8)safe->initialFocus();
 });
}
void BiscotEditor::focusGained(FocusChangeType){initialFocus();}
bool BiscotEditor::keyPressed(const juce::KeyPress& key){
 const Control c=edit.hasKeyboardFocus(true)?Control::edit:value.hasKeyboardFocus(true)?Control::value:Control::grid;
 return handleKey(key,c);
}
void BiscotEditor::typeSearch(juce::juce_wchar c,bool backwards){
 const uint32_t now=juce::Time::getMillisecondCounter();
 if(now-lastSearchMs>950)searchText.clear();lastSearchMs=now;
 const juce::String charText=juce::String::charToString(juce::CharacterFunctions::toLowerCase(c));
 if(searchText==charText)searchText.clear();
 searchText+=charText;
 auto find=[this,backwards](const juce::String& text)->int{
  const auto& entries=groups[(size_t)group];
  for(int offset=0;offset<(int)entries.size();++offset){
   const int i=(selected+(backwards?-offset-1:offset+1)+(int)entries.size()*2)%(int)entries.size();
   juce::String name=catalog[(size_t)(entries[(size_t)i]-2)].name;
   if(name.startsWithIgnoreCase(text)||name.containsIgnoreCase(" "+text))return i;
  }
  return -1;
 };
 int found=find(searchText);
 if(found<0 && searchText.length()>1){searchText=charText;found=find(searchText);}
 if(found>=0)select(found);
}
bool BiscotEditor::handleKey(const juce::KeyPress& k,Control c){
 const auto m=k.getModifiers();const int code=k.getKeyCode();
 auto letter=[&](char ch){return juce::CharacterFunctions::toLowerCase(k.getTextCharacter())==ch;};
 if(m.isCtrlDown() && code==juce::KeyPress::tabKey){changeGroup(m.isShiftDown()?-1:1);return true;}
 if(m.isAltDown() && !m.isCtrlDown()){
  if(letter('l')){focus(Control::grid,c==Control::edit);return true;}
  if(letter('e')){enterEdit();return true;}
  if(letter('q')){announce(grid,"Tracker Pattern and Song will arrive in a later version. Synth stays active.");return true;}
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
 if(!m.isCtrlDown() && !m.isAltDown() && !m.isCommandDown()){
  const auto ch=k.getTextCharacter();
  if(juce::CharacterFunctions::isLetterOrDigit(ch)){
   typeSearch(ch,m.isShiftDown());return true;
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
void BiscotEditor::announce(juce::Component& source,const juce::String& text)const{
 source.setDescription(text);
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
}
void BiscotEditor::paint(juce::Graphics& g){
 g.fillAll(background);
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
