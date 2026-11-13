// SPDX-License-Identifier: AGPL-3.0-or-later
#include "TrackerLibraryBrowser.h"
#include "TrackerSongPackage.h"
#include "TrackerPatternText.h"
#include "TrackerPatternPlayback.h"
#include "TrackerShortcutKey.h"
#include "ScreenReaderAnnouncer.h"
#include "PresetBrowserMemory.h"
#include <algorithm>
#include <array>

namespace biscot {
namespace {
const juce::Colour foreground(0xffe5edf6), background(0xff171b26);
constexpr auto browserPreferences="Biscot1 Tracker Library Browser";
const juce::String previewPreference(TrackerLibraryBrowser::Mode m){
 return m==TrackerLibraryBrowser::Mode::patterns?"automaticPreview.patterns":"automaticPreview.songs";
}
bool recalledAutomaticPreview(TrackerLibraryBrowser::Mode m){
 juce::PropertiesFile settings(preset_browser_memory::options(browserPreferences));
 return settings.getBoolValue(previewPreference(m),false);
}
void rememberAutomaticPreview(TrackerLibraryBrowser::Mode m,bool value){
 juce::PropertiesFile settings(preset_browser_memory::options(browserPreferences));
 settings.setValue(previewPreference(m),value);
 settings.saveIfNeeded();
}
bool supported(const juce::File& file,TrackerLibraryBrowser::Mode mode){
 if(mode==TrackerLibraryBrowser::Mode::patterns)return file.hasFileExtension(".biscot1-pattern");
 return file.hasFileExtension(".biscot1-song") || file.hasFileExtension(".biscot1-all-song");
}
}
TrackerLibraryBrowser::TrackerLibraryBrowser(BiscotProcessor& p):list(*this),processor(p){
 setFocusContainerType(juce::Component::FocusContainerType::keyboardFocusContainer);
 setWantsKeyboardFocus(true);
 for(auto* l:{&heading,&help}){
  l->setAccessible(false);l->setColour(juce::Label::textColourId,foreground);addAndMakeVisible(*l);
 }
 list.setName("Biscot1 tracker library files");list.setDescription({});
 list.setWantsKeyboardFocus(true);list.setEditableText(false);
 list.setExplicitFocusOrder(1);
 list.onChange=[this]{
  if(refreshing || list.getSelectedId()<=0)return;
  const int index=list.getSelectedId()-1;
  if(index!=selected){selected=index;selectionChanged(true,automaticPreview);}
 };
 addAndMakeVisible(list);
 yes.setWantsKeyboardFocus(true);no.setWantsKeyboardFocus(true);
 yes.setExplicitFocusOrder(2);no.setExplicitFocusOrder(3);
 yes.onClick=[this]{finishConfirmation(true);};
 no.onClick=[this]{finishConfirmation(false);};
 addAndMakeVisible(yes);addAndMakeVisible(no);
 yes.setVisible(false);no.setVisible(false);
 help.setText("Type filename: incremental search | Up/Down: files | Right: enter folder | Left/Backspace: parent | Space: play/stop full file | +/-: shorten/extend preview | Ctrl+P: automatic preview (remembered) | Ctrl+Left/Right: archive Song | Enter: load | F5: refresh",juce::dontSendNotification);
 setVisible(false);
}
std::unique_ptr<juce::AccessibilityHandler> TrackerLibraryBrowser::createAccessibilityHandler(){
 return createIgnoredAccessibilityHandler(*this);
}
void TrackerLibraryBrowser::resized(){
 auto r=getLocalBounds().reduced(22);
 heading.setBounds(r.removeFromTop(40));help.setBounds(r.removeFromBottom(35));
 r.removeFromTop(10);list.setBounds(r.removeFromTop(35));
 auto buttons=r.removeFromTop(48);
 no.setBounds(buttons.removeFromLeft(110));
 buttons.removeFromLeft(10);
 yes.setBounds(buttons.removeFromLeft(110));
}
void TrackerLibraryBrowser::paint(juce::Graphics& g){
 g.fillAll(background);g.setColour(foreground);g.drawRect(getLocalBounds(),1);
}
void TrackerLibraryBrowser::announce(const juce::String& s){
 announceToActiveScreenReader(confirming?(confirmationYes?static_cast<juce::Component&>(yes):static_cast<juce::Component&>(no)):static_cast<juce::Component&>(list),s);
}
void TrackerLibraryBrowser::open(Mode next){
 mode=next;confirming=false;confirmationYes=false;yes.setVisible(false);no.setVisible(false);list.setVisible(true);automaticPreview=recalledAutomaticPreview(next);selected=0;previewSteps=0;archiveSongSlot=0;search.reset();
 previewCacheFile=juce::File{};previewSongs.reset();previewPatterns.reset();
 root=juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
  .getChildFile("Biscot1-Synth").getChildFile(mode==Mode::patterns?"patterns":"songs");
 if(root.createDirectory().failed()){
  // The non-audio thread is allowed to create the user library directory.
  setVisible(true);list.grabKeyboardFocus();announce("Could not open the Biscot1 library directory");return;
 }
 folder=root;
 setVisible(true);toFront(false);
 scan(false);
 juce::Component::SafePointer<TrackerLibraryBrowser> safe(this);
 juce::MessageManager::callAsync([safe]{
  if(safe!=nullptr && safe->isShowing()){
   safe->list.grabKeyboardFocus();
   safe->announce((safe->mode==Mode::patterns?"Pattern library. ":"Song library. ")+
    juce::String((int)safe->entries.size())+" entries. "+
    (safe->automaticPreview?"Automatic preview On. ":"Automatic preview Off. ")+safe->entryLabel(safe->selected));
  }
 });
}
void TrackerLibraryBrowser::stopPreview(){
 // Never silence a full Song/Pattern that the user explicitly started.
 if(processor.trackerPlaybackStatus.load(std::memory_order_acquire)==3)
  processor.requestTrackerStop();
 if(processor.songPlaybackStatus.load(std::memory_order_acquire)==3)
  processor.requestSongStop();
}
void TrackerLibraryBrowser::close(bool imported,const juce::String& result){
 stopPreview();setVisible(false);confirming=false;yes.setVisible(false);no.setVisible(false);list.setVisible(true);
 if(onClose)onClose(mode,imported,result);
}
void TrackerLibraryBrowser::scan(bool summary){
 search.reset();confirming=false;previewCacheFile=juce::File{};previewSongs.reset();previewPatterns.reset();
 const auto previous=selected>=0 && selected<(int)entries.size()?entries[(size_t)selected].path:juce::File{};
 juce::Array<juce::File> children;
 if(folder.isDirectory())folder.findChildFiles(children,juce::File::findFilesAndDirectories,false,"*");
 entries.clear();
 for(const auto& item:children)
  if(item.isDirectory() || (item.existsAsFile()&&supported(item,mode)))
   entries.push_back({item,item.isDirectory()});
 std::sort(entries.begin(),entries.end(),[](const Entry& a,const Entry& b){
  if(a.directory!=b.directory)return a.directory;
  return a.path.getFileName().compareIgnoreCase(b.path.getFileName())<0;
 });
 selected=0;
 for(int i=0;i<(int)entries.size();++i)if(entries[(size_t)i].path==previous){selected=i;break;}
 refreshing=true;list.clear(juce::dontSendNotification);
 if(entries.empty())list.addItem("No library files in this folder",1);
 else for(int i=0;i<(int)entries.size();++i)list.addItem(entryLabel(i),i+1);
 list.setSelectedId(selected+1,juce::dontSendNotification);
 refreshing=false;
 heading.setText(mode==Mode::patterns?"Biscot1 - Pattern library":"Biscot1 - Song library",juce::dontSendNotification);
 if(summary)announce("Library refreshed. "+juce::String((int)entries.size())+" entries. "+entryLabel(selected));
}
juce::String TrackerLibraryBrowser::entryLabel(int index)const{
 if(entries.empty())return "No library files";
 if(index<0||index>=(int)entries.size())return "No selection";
 const auto& e=entries[(size_t)index];
 if(e.directory)return "Folder, "+e.path.getFileName();
 const auto label=e.path.hasFileExtension(".biscot1-all-song")?"All 100 Songs, ":
                  e.path.hasFileExtension(".biscot1-song")?"Song, ":"Pattern, ";
 return juce::String(label)+e.path.getFileName();
}
void TrackerLibraryBrowser::selectionChanged(bool speak,bool doPreview){
 confirming=false;stopPreview();archiveSongSlot=0;
 previewCacheFile=juce::File{};previewSongs.reset();previewPatterns.reset();
 // The active Song encoded in the archive is the most useful initial preview,
 // but Left/Right can select any of the other 99 static slots.
 if(!entries.empty() && entries[(size_t)selected].path.hasFileExtension(".biscot1-all-song")){
  juce::FileInputStream stream(entries[(size_t)selected].path);
  if(stream.openedOk() && stream.readNextLine()=="BISCOT1_ALL_SONG_BUNDLE_V3"){
   const auto header=stream.readNextLine();
   archiveSongSlot=juce::jlimit(0,99,header.upToFirstOccurrenceOf(" ",false,false).getIntValue());
  }
 }
 if(speak)announce(entryLabel(selected)+". "+juce::String(selected+1)+" of "+juce::String((int)entries.size()));
 if(doPreview)preview();
}
void TrackerLibraryBrowser::parentFolder(){
 if(folder==root)return;
 const auto previousFolder=folder;
 stopPreview();folder=folder.getParentDirectory();selected=0;scan(false);
 for(int i=0;i<(int)entries.size();++i)if(entries[(size_t)i].path==previousFolder){
  selected=i;refreshing=true;list.setSelectedId(i+1,juce::dontSendNotification);refreshing=false;break;
 }
 announce("Parent folder, "+folder.getFileName()+". "+entryLabel(selected));
 if(automaticPreview && !entries.empty() && !entries[(size_t)selected].directory)preview();
}
void TrackerLibraryBrowser::enterFolder(){
 if(entries.empty()){announce("No library files");return;}
 const auto& e=entries[(size_t)selected];
 if(!e.directory){announce("Right arrow enters folders. Press Enter to load a file");return;}
 stopPreview();folder=e.path;selected=0;scan(false);
 announce("Folder, "+folder.getFileName()+". "+juce::String((int)entries.size())+" entries. "+entryLabel(selected));
 if(automaticPreview && !entries.empty() && !entries[(size_t)selected].directory)preview();
}
void TrackerLibraryBrowser::navigate(int step){
 if(entries.empty()){announce("No library files");return;}
 const int next=juce::jlimit(0,(int)entries.size()-1,selected+step);
 if(next!=selected){selected=next;refreshing=true;list.setSelectedId(selected+1,juce::dontSendNotification);
  refreshing=false;selectionChanged(true,automaticPreview);}
 else { /* silent list boundary */ }
}
void TrackerLibraryBrowser::preview(){
 if(entries.empty() || entries[(size_t)selected].directory){announce("Select a library file to preview");return;}
 if(processor.trackerPlaybackStatus.load(std::memory_order_acquire)==1 ||
    processor.songPlaybackStatus.load(std::memory_order_acquire)==1){
  announce("Stop current playback before previewing the library");return;
 }
 const auto file=entries[(size_t)selected].path;
 if(mode==Mode::patterns){
  TrackerPattern pattern(false);int original=-1;
  if(!file.existsAsFile() || file.getSize()>16ll*1024ll*1024ll ||
     !deserializeTrackerPattern(file.loadFileAsString().toStdString(),original,pattern)){
   announce("Invalid Pattern file");return;
  }
  // Preserve ALL leading silence, automation and Repeat: play from step 0.
  if(processor.requestLibraryPatternPreview(pattern,0,previewSteps))
   announce("Preview Pattern "+juce::String(original)+", "+
    (previewSteps==0?juce::String("full Pattern"):
     juce::String(previewSteps)+" actual Pattern steps"));
 }else{
  if(file!=previewCacheFile || !previewSongs || !previewPatterns){
   auto loadedSongs=std::make_unique<TrackerSongBank>();
   auto loadedPatterns=std::make_unique<TrackerPatternBank>();
   loadedPatterns->patterns.clear(); // no false conflict with default Pattern 0
   juce::String status;
   if(!songpkg::importSong(file,*loadedSongs,*loadedPatterns,status)){
    announce("Cannot preview Song. "+status);return;
   }
   previewCacheFile=file;
   previewSongs=std::move(loadedSongs);
   previewPatterns=std::move(loadedPatterns);
  }
  const int slot=file.hasFileExtension(".biscot1-all-song")?archiveSongSlot:previewSongs->activeSong;
  const auto* s=previewSongs->find(slot);
  if(!s){announce("Song slot "+juce::String(slot)+" is empty");return;}
  if(processor.requestLibrarySongPreview(*s,slot,*previewPatterns,*previewSongs,previewSteps))
   announce("Preview Song "+juce::String(slot)+", original three-operator playback, "+
    (previewSteps==0?juce::String("full Song"):
     juce::String(previewSteps)+" sixteenth-note divisions"));
  else announce("Cannot preview Song "+juce::String(slot));
 }
}
bool TrackerLibraryBrowser::importSelected(const juce::File& file,juce::String& result){
 if(processor.trackerPlaybackStatus.load(std::memory_order_acquire)==1 ||
    processor.songPlaybackStatus.load(std::memory_order_acquire)==1){
  result="Stop current playback before importing the library";return false;
 }
 stopPreview();
 if(mode==Mode::songs){
  if(!songpkg::importSong(file,processor.trackerSongs,processor.trackerPatterns,result))return false;
  processor.commitTrackerState();processor.commitSongState();return true;
 }
 TrackerPattern imported(false);int original=-1;
 if(!file.existsAsFile()||file.getSize()>16ll*1024ll*1024ll ||
    !deserializeTrackerPattern(file.loadFileAsString().toStdString(),original,imported)){
  result="Invalid Pattern file";return false;
 }
 auto& bank=processor.trackerPatterns;
 int dest=original;
 if(bank.patterns.count(dest)){
  dest=0;while(dest<1000 && bank.patterns.count(dest))++dest;
 }
 if(dest>=1000){result="Pattern bank is full";return false;}
 bank.patterns.emplace(dest,std::move(imported));bank.activePattern=dest;
 processor.commitTrackerState();
 result="Imported Pattern "+juce::String(dest)+
  (dest!=original?". Original slot "+juce::String(original):juce::String{});
 return true;
}
void TrackerLibraryBrowser::activate(){
 if(entries.empty()){announce("No library files");return;}
 const auto& item=entries[(size_t)selected];
 if(item.directory){enterFolder();return;}
 if(mode==Mode::songs){
  stopPreview();confirming=true;confirmationYes=false;confirmationFile=item.path;
  list.setVisible(false);no.setVisible(true);yes.setVisible(true);
  no.grabKeyboardFocus();
  announce("Replace ALL current Patterns and Songs with the selected " +
   juce::String(item.path.hasFileExtension(".biscot1-all-song")?"100-Song archive":"single Song") +
   "? Yes or No. No selected. Left and Right choose; Enter confirms; Y or N acts immediately; Escape cancels");
  return;
 }
 juce::String status;
 if(importSelected(item.path,status))close(true,status);
 else announce(status);
}
void TrackerLibraryBrowser::finishConfirmation(bool approve){
 if(!confirming)return;
 const auto file=confirmationFile;
 confirming=false;confirmationYes=false;
 no.setVisible(false);yes.setVisible(false);list.setVisible(true);
 if(!approve){list.grabKeyboardFocus();announce("Import cancelled. Patterns and Songs unchanged");return;}
 juce::String status;
 if(importSelected(file,status))close(true,status);
 else{list.grabKeyboardFocus();announce(status);}
}
bool TrackerLibraryBrowser::keyPressed(const juce::KeyPress& k){return handle(k);}
bool TrackerLibraryBrowser::handle(const juce::KeyPress& k){
 const int code=k.getKeyCode();const auto mod=k.getModifiers();
 if(confirming){
  if(code==juce::KeyPress::escapeKey){finishConfirmation(false);return true;}
  if(!mod.isAltDown()&&!mod.isCtrlDown()&&!mod.isCommandDown()){
   const auto ch=juce::CharacterFunctions::toLowerCase(k.getTextCharacter());
   if(ch=='y'||code=='Y'){finishConfirmation(true);return true;}
   if(ch=='n'||code=='N'){finishConfirmation(false);return true;}
   if(code==juce::KeyPress::leftKey || code==juce::KeyPress::rightKey ||
      code==juce::KeyPress::upKey || code==juce::KeyPress::downKey){
    confirmationYes=(code==juce::KeyPress::rightKey||code==juce::KeyPress::upKey);
    (confirmationYes?yes:no).grabKeyboardFocus();announce(confirmationYes?"Yes selected":"No selected");return true;
   }
   if(code==juce::KeyPress::returnKey){finishConfirmation(confirmationYes);return true;}
  }
  return true;
 }
 if(code==juce::KeyPress::escapeKey){
  if(folder!=root){parentFolder();return true;}
  close();return true;
 }
 if(code==juce::KeyPress::backspaceKey){
  parentFolder();return true;
 }
 if(code==juce::KeyPress::leftKey && !mod.isCtrlDown() && !mod.isAltDown()){
  parentFolder();return true;
 }
 if(code==juce::KeyPress::rightKey && !mod.isCtrlDown() && !mod.isAltDown()){
  enterFolder();return true;
 }
 if(code==juce::KeyPress::upKey){search.reset();navigate(-1);return true;}
 if(code==juce::KeyPress::downKey){search.reset();navigate(1);return true;}
 if(code==juce::KeyPress::pageUpKey){search.reset();navigate(-5);return true;}
 if(code==juce::KeyPress::pageDownKey){search.reset();navigate(5);return true;}
 if(code==juce::KeyPress::homeKey){search.reset();navigate(-selected);return true;}
 if(code==juce::KeyPress::endKey){search.reset();navigate((int)entries.size()-1-selected);return true;}
 if(code==juce::KeyPress::F5Key){stopPreview();scan(true);return true;}
 if(code==juce::KeyPress::tabKey){
  announce((mode==Mode::patterns?"Pattern":"Song")+juce::String(" library. ")+
   juce::String((int)entries.size())+" entries. Preview "+
   (previewSteps==0?juce::String("full file"):juce::String(previewSteps)+" divisions")+". "+
   (automaticPreview?"Automatic preview On. ":"Automatic preview Off. ")+entryLabel(selected));return true;
 }
 const auto letter=[&](char target){
  return trackerShortcutLetterMatches(code,(int)k.getTextCharacter(),target,mod.isCtrlDown());
 };
 if(mod.isCtrlDown() && letter('p')){
  automaticPreview=!automaticPreview;rememberAutomaticPreview(mode,automaticPreview);
  announce(automaticPreview?"Automatic preview On":"Automatic preview Off");
  if(automaticPreview)preview();return true;
 }
 if(!mod.isCtrlDown()&&!mod.isAltDown() && code==32){
  if(processor.trackerPlaybackStatus.load(std::memory_order_acquire)==3 ||
     processor.songPlaybackStatus.load(std::memory_order_acquire)==3){
   stopPreview();announce("Preview stopped");
  }else preview();
  return true;
 }
 if(!mod.isCtrlDown()&&!mod.isAltDown() && (code=='+'||code=='-')){
  constexpr std::array<int,7> lengths{1,3,8,16,32,64,0};
  int idx=6;for(int i=0;i<(int)lengths.size();++i)
   if(previewSteps==lengths[(size_t)i]){idx=i;break;}
  idx=juce::jlimit(0,(int)lengths.size()-1,idx+(code=='+'?1:-1));
  previewSteps=lengths[(size_t)idx];
  announce(previewSteps==0?juce::String("Preview full file"):
   juce::String("Preview window ")+juce::String(previewSteps)+" divisions");return true;
 }
 if(mode==Mode::songs && mod.isCtrlDown() && !mod.isAltDown() &&
    !entries.empty() && entries[(size_t)selected].path.hasFileExtension(".biscot1-all-song") &&
    (code==juce::KeyPress::leftKey||code==juce::KeyPress::rightKey)){
  archiveSongSlot=juce::jlimit(0,99,archiveSongSlot+(code==juce::KeyPress::rightKey?1:-1));
  announce("Preview archive Song slot "+juce::String(archiveSongSlot));
  if(automaticPreview)preview();return true;
 }
 if(!mod.isCtrlDown() && !mod.isAltDown() && !mod.isCommandDown() &&
    juce::CharacterFunctions::isLetterOrDigit(k.getTextCharacter())){
  const auto match=search.type(k.getTextCharacter(),selected,(int)entries.size(),
   [this](int index){
    const auto& e=entries[(size_t)index];
    return e.directory?e.path.getFileName():e.path.getFileNameWithoutExtension();
   },mod.isShiftDown());
  if(match<0)announce("No matching library file for "+search.currentPrefix());
  else if(match!=selected){selected=match;refreshing=true;
   list.setSelectedId(selected+1,juce::dontSendNotification);refreshing=false;
   selectionChanged(true,automaticPreview);
  }else announce(entryLabel(selected));
  return true;
 }
 if(code==juce::KeyPress::returnKey){search.reset();activate();return true;}
 return true; // keep typed shortcuts inside the tracker and away from REAPER
}
}
