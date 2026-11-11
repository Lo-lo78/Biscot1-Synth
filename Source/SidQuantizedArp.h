#pragma once
// Clock-quantized classical arpeggiator for Biscot1.
// MIDI changes update the candidate pool; only a clock edge can commit them
// or select a new pitch. JUCE-free, fixed storage, audio-thread safe.
#include <array>
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace biscot {
struct SidQuantizedArp {
 static constexpr int capacity=640;
 std::array<int,capacity> committed{};
 int count=0,step=0,selected=-1;
 double phase=0.;
 bool started=false, restartPending=false;
 struct Event {bool edge=false;bool first=false;int note=-1;};
 void reset() noexcept { count=0;step=0;selected=-1;phase=0.;started=false;restartPending=false; }
 // Trigger requests a musical restart on the NEXT beat boundary; it must not
 // break the BPM clock or play the newly pressed note on the Note On itself.
 void requestRestart() noexcept { if(started)restartPending=true; }
 Event process(const std::array<int,capacity>& candidate,int candidateCount,
               int mode,double rate,double bpm,double sampleRate,
               std::uint32_t& randomSeed) noexcept {
  const bool first=!started;
  if(first && candidateCount<=0)return {};
  bool edge=first;
  if(first){started=true;step=0;phase=0.;}
  else {
   phase+=std::max(0.125,rate)*std::max(1.,bpm)/(240.*std::max(1.,sampleRate));
   if(phase>=1.-1.e-10) {
    phase=std::max(0.,phase-1.);
    if(restartPending){step=0;restartPending=false;}else ++step;
    edge=true;
   }
  }
  if(edge){
   // The pool is frozen between clock edges; incoming MIDI cannot retune an
   // oscillator on the sample at which the musician strikes the new key.
   // With no live notes, keep the last pool throughout the envelope Release.
   if(candidateCount>0){
    count=std::max(0,std::min(candidateCount,capacity));
    std::copy_n(candidate.begin(),count,committed.begin());
   }
   if(count>0){
    int pos=step%count;
    if(mode==3)pos=count-1-pos;
    else if(mode==4){
     const int cycle=std::max(1,2*count-2);
     pos=step%cycle;if(pos>=count)pos=cycle-pos;
    }
    else if(mode==6){
     randomSeed^=randomSeed<<13;randomSeed^=randomSeed>>17;randomSeed^=randomSeed<<5;
     pos=(int)(randomSeed%(std::uint32_t)count);
    }
    selected=committed[(std::size_t)pos];
   }
  }
  return {edge,first,selected};
 }
};
} // namespace biscot
