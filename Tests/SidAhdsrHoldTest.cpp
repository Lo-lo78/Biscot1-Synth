// SPDX-License-Identifier: AGPL-3.0-or-later
#include "../Source/SidAmpEnvelope.h"
#include "../Source/ParameterCatalog.h"
#include "../Source/TrackerParameterCatalog.h"
#include "../Source/TrackerPatternAutomation.h"
#include "../Source/TrackerSongPlayback.h"
#include <algorithm>
#ifdef NDEBUG
 #undef NDEBUG // Release CI must execute all test assertions
#endif
#include <cassert>
#include <cmath>
#include <iostream>
using namespace biscot;
int main(){
 constexpr double sr=1000.;
 static_assert(catalog.size()==260);
 static_assert(trackerCatalog.size()==126);
 for (int i=0;i<7;++i){
  const auto& def=catalog[(size_t)(247+i)];
  assert(def.index==249+i && def.low==0.f && def.high==5.f && def.step==0.001f && def.initial==0.f);
 }
 for (int op=0;op<3;++op){
  SidAmpEnvelope amp,mod;
  amp.noteOn(true);mod.noteOn(true);
  // Attack=0 is the first sample; Hold remains at peak for *exactly*
  // 100 and 200 subsequent samples, then starts Decay.
  assert(amp.tick(0.f,.100f,.500f,.2f,sr)==1.f && amp.stage==5);
  assert(mod.tick(0.f,.200f,.500f,.2f,sr)==1.f && mod.stage==5);
  for(int n=0;n<99;++n){assert(amp.tick(0.f,.100f,.500f,.2f,sr)==1.f && amp.stage==5);}
  assert(amp.tick(0.f,.100f,.500f,.2f,sr)==1.f && amp.stage==2);
  assert(amp.tick(0.f,.100f,.500f,.2f,sr)<1.f);
  for(int n=0;n<200;++n)mod.tick(0.f,.200f,.500f,.2f,sr);
  assert(mod.stage==2);
  assert(mod.tick(0.f,.200f,.500f,.2f,sr)<1.f);
 }
 // Hold 0 is exactly equivalent to pre-AHDSR ADSR for both rate curves.
 SidAmpEnvelope oldStyle,newStyle;
 oldStyle.noteOn(true);newStyle.noteOn(true);
 for (int i=0;i<2000;++i){
  if(i==1300){oldStyle.noteOff(.4f,sr);newStyle.noteOff(.4f,sr);}
  const float a=oldStyle.tick(.1f,.3f,.45f,sr);
  const float b=newStyle.tick(.1f,0.f,.3f,.45f,sr);
  assert(a==b && oldStyle.stage==newStyle.stage);
 }
 // Note Off, including very first Hold sample, always starts normal release.
 SidAmpEnvelope e;e.noteOn(true);e.tick(0.f,5.f,.2f,.1f,sr);
 assert(e.stage==5 && e.level==1.f);
 e.noteOff(.5f,sr);assert(e.stage==4 && e.holdSamplesRemaining==0);
 const float before=e.level;
 assert(e.tick(0.f,5.f,.2f,.1f,sr)<before && e.stage==4);
 e.noteOff(0.f,sr);assert(e.stage==4);
 // Retrigger during Hold starts new attack and removes the old hold timer.
 e.noteOn(false);assert(e.stage==1 && e.holdSamplesRemaining==0);
 e.tick(0.f,.020f,.2f,.1f,sr);assert(e.stage==5 && e.holdSamplesRemaining==20);
 for(int i=0;i<20;++i)e.tick(0.f,.020f,.2f,.1f,sr);
 assert(e.stage==2);
 // Filter uses exactly the same core and preserves its own independent timer.
 SidAmpEnvelope filter;filter.noteOn(true);filter.tick(0.f,.030f,0.f,.5f,sr);
 for(int i=0;i<30;++i)filter.tick(0.f,.030f,0.f,.5f,sr);
 assert(filter.stage==2);filter.tick(0.f,.030f,0.f,.5f,sr);
 assert(filter.stage==3 && filter.level==.5f);
 // Pattern F1 exposes each shared operator parameter and the filter.
 assert(findTrackerParam(3015)->synthIndex==249);
 assert(findTrackerParam(3016)->synthIndex==252);
 assert(findTrackerParam(3017)->synthIndex==255);
 std::cout << "OK: 7 AHDSR Hold parameters, sample-accurate stages, zero parity, release, retrigger" << std::endl;
}
