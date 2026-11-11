#pragma once
#include <algorithm>
#include <cmath>
#include "SidLfoCore.h"

namespace biscot {
// JSFX v2.38 style cold-idle transition. MIDI messages are always read by
// the processor; silent samples are skipped after the physical gates close,
// the VCA envelopes reach idle, and the output circuit becomes inaudible.
struct SidIdleSleep {
 bool asleep=true;
 int quietSamples=0;
 static constexpr int requiredQuietSamples=64;
 static constexpr float audibleFloor=0.00001f;
 void reset() noexcept {asleep=true;quietSamples=0;}
 void wake() noexcept {asleep=false;quietSamples=0;}
 bool observe(bool noPhysicalGate,bool allVcaIdle,float peak) noexcept {
  if(asleep)return false;
  if(!noPhysicalGate || !allVcaIdle || peak>audibleFloor)
   quietSamples=0;
  else if(++quietSamples>=requiredQuietSamples){asleep=true;quietSamples=0;return true;}
  return false;
 }
 // Advance only the FREE clock, without simulating any oscillator, envelope,
 // filter, smoothing or output processing for silent samples. Unlike a reset,
 // this preserves the deterministic free-LFO phase through a long idle period.
 static void advanceFreeClock(SidLfoCore& l,int frames, double bpm,double rate,
                              double sampleRate,bool freeMode,bool oneShot) noexcept {
  if(frames<=0 || !freeMode || oneShot)return;
  const double frequency=std::max(0.001,bpm)/240.0*std::max(0.125,rate);
  const double period=frequency*frames/std::max(1.,sampleRate);
  l.phase+=period;
  l.phase-=std::floor(l.phase);
 }
};
}
