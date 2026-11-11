#pragma once
// Independent SID Seq dynamic frequency memory, derived from Biscot1 JSFX v2.38
// sid_seq_clear/record/process. No allocations and no locks on the audio thread.
#include "SidMidiNoteState.h"
#include <array>
#include <algorithm>
#include <cmath>

namespace biscot {
struct SidSeqMemory {
 static constexpr int capacity=128;
 static constexpr int empty=-9999;
 std::array<int,capacity*2> slots{}; // original and shadow as in JSFX
 int length=0,step=-1;
 double clock=0.;
 float target=0.f;
 bool lastTick=false, restartPending=false;
 SidHeldStack latch{}; // snapshot on every Note On; persists through release
 int latchRoot=60,releaseRoot=60;
 void clearAll() noexcept {
  slots.fill(empty);length=0;step=-1;clock=0.;target=0.f;lastTick=false;restartPending=false;
  latch.clear();latchRoot=60;releaseRoot=60;
 }
 void resetClock() noexcept {step=-1;clock=0.;target=0.f;lastTick=false;restartPending=false;}
 // Restart the SID Seq address at its NEXT scheduled clock edge, not on Note On.
 void requestRestart() noexcept { if(step>=0)restartPending=true; }
 void snapshot(const SidHeldStack& held) noexcept {
  latch=held;
  if(latch.count>0)latchRoot=latch.notes[0];
 }
 void clearSequence(int root) noexcept {
  slots.fill(empty);length=0;
  if(root>=0){slots[0]=root;slots[capacity]=root;length=1;}
 }
 void record(int note) noexcept {
  if(length<=0){slots[0]=note;slots[capacity]=note;length=1;return;}
  int pos=step<0?0:step+1;
  if(length<capacity){
   pos=std::max(0,std::min(length,pos));
   for(int j=length;j>pos;--j){slots[(size_t)j]=slots[(size_t)(j-1)];slots[(size_t)(capacity+j)]=slots[(size_t)(capacity+j-1)];}
   slots[(size_t)pos]=note;slots[(size_t)(capacity+pos)]=note;++length;
  }else{
   if(pos>=capacity)pos=0;
   pos=std::max(0,std::min(capacity-1,pos));
   slots[(size_t)pos]=note;slots[(size_t)(capacity+pos)]=note;
  }
 }
 // Matches JSFX: 4 beat clock, first step immediately, no Note On retuning.
 // activeCount is non-Chord release latch or current chord note count.
 float process(int activeCount,int root,double rate,double tempo,double sampleRate) noexcept {
  lastTick=false;
  if(activeCount>0 && root>=0 && length>0){
   clock+=std::max(0.125,rate)*std::max(1.,tempo)/(240.*std::max(1.,sampleRate));
   bool newStep=false;
   if(step<0){step=0;clock=0.;newStep=true;}
   else if(clock>=1.-1.e-10){
    clock=std::max(0.,clock-1.);
    if(restartPending){step=0;restartPending=false;}else ++step;
    if(step>=length)step=0;
    newStep=true;
   }
   if(newStep){
    lastTick=true;
    if(step>=length)step=0;
    int selected=slots[(size_t)step];
    if(selected<=empty+1)selected=root;
    target=(float)(selected-root);
   }
  }else{clock=0.;step=-1;target=0.f;}
  return target;
 }
};
}
