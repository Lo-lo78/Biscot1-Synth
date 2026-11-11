#pragma once
// Biscot1 v2.38 JSFX held-note logic, kept independent of JUCE for regression testing.
// MIDI channels affect voice routing only in MIDI Channels 1-3 mode; the JSFX
// deliberately stores pitches rather than channel/note pairs in each mono stack.
#include <array>
#include <algorithm>
#include <cstddef>

namespace biscot {
struct SidHeldStack {
 static constexpr int capacity = 16;
 std::array<int,capacity> notes{};
 int count=0;
 void clear() noexcept { count=0; }
 bool empty() const noexcept { return count==0; }
 int top() const noexcept { return count>0 ? notes[(size_t)(count-1)] : -1; }
 void add(int note) noexcept {
  if(count==capacity){
   for(int i=1;i<capacity;++i)notes[(size_t)(i-1)]=notes[(size_t)i];
   --count;
  }
  notes[(size_t)count++]=note;
 }
 // Remove the newest matching Note On; duplicates are never collapsed.
 bool removeNewest(int note) noexcept {
  for(int i=count-1;i>=0;--i){
   if(notes[(size_t)i]==note){
    for(int j=i+1;j<count;++j)notes[(size_t)(j-1)]=notes[(size_t)j];
    --count;return true;
   }
  }
  return false;
 }
};

// ARP: only the first note of the phrase retriggers an operator.
// Non-ARP: Trigger mode retriggers on every note; Legato only the first.
inline bool sidShouldRetrigger(bool arpOn,bool wasHeld,bool legato) noexcept {
 return arpOn ? !wasHeld : (!legato || !wasHeld);
}

struct SidMidiNoteState {
 std::array<SidHeldStack,3> voices{};
 SidHeldStack filter{},chord{};
 struct On {
  bool filterRetrigger=false;
  std::array<bool,3> trigger{};
  std::array<bool,3> wasHeld{};
 };
 struct Off {
  std::array<bool,3> affected{};
  std::array<int,3> voiceTop{{-1,-1,-1}};
  bool filterReleased=false;
  int filterTop=-1;
 };
 void clear() noexcept {
  for(auto& voice:voices)voice.clear();
  filter.clear();chord.clear();
 }
 // mode 0: All Operators Same Note; 1: Chord Allocate; 2: MIDI Channels 1-3.
 On noteOn(int mode,int channel,int note,bool op1Legato) noexcept {
  On result{};
  if(note<0||note>127||channel<1||channel>16)return result;
  const bool filterWasHeld=!filter.empty();
  filter.add(note);
  result.filterRetrigger=!op1Legato||!filterWasHeld;
  if(mode==1)chord.add(note);
  int selected=-1;
  if(mode==1){
   selected=0;
   for(int i=0;i<3;++i)if(voices[(size_t)i].empty()){selected=i;break;}
  }else if(mode==2){
   if(channel<=3)selected=channel-1;
  }
  for(int i=0;i<3;++i){
   if(mode==0||i==selected){
    result.trigger[(size_t)i]=true;
    result.wasHeld[(size_t)i]=!voices[(size_t)i].empty();
    voices[(size_t)i].add(note);
   }
  }
  return result;
 }
 Off noteOff(int mode,int channel,int note) noexcept {
  Off result{};
  if(note<0||note>127||channel<1||channel>16)return result;
  filter.removeNewest(note);
  result.filterReleased=filter.empty();
  result.filterTop=filter.top();
  if(mode==1)chord.removeNewest(note);
  for(int i=0;i<3;++i){
   const bool addressesVoice=mode==0||mode==1||(mode==2&&channel==i+1);
   if(addressesVoice){
    voices[(size_t)i].removeNewest(note);
    result.affected[(size_t)i]=true;
    result.voiceTop[(size_t)i]=voices[(size_t)i].top();
   }
  }
  return result;
 }
};
}
