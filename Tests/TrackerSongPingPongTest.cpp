// SPDX-License-Identifier: AGPL-3.0-or-later
#include "../Source/TrackerSongFxCore.h"
#include <cassert>
#include <cmath>
int main(){
 biscot::TrackerSongFxCore effect;
 effect.prepare(1000.);
 biscot::TrackerSongRowFx step{};
 step.present[17]=true;step.value[17]=8.;
 step.present[18]=true;step.value[18]=80.;
 step.present[19]=true;step.value[19]=100.;
 step.present[20]=true;step.value[20]=100.;
 effect.setRow(step);
 float echoL1=0,echoR1=0,echoL2=0,echoR2=0;
 for(int n=0;n<205;++n){
  float l=0,r=0;
  effect.process(n==0?1.f:0.f,n==0?1.f:0.f,l,r,120.);
  if(n==63){echoL1=l;echoR1=r;}
  if(n==126){echoL2=l;echoR2=r;}
 }
 // At 100% a center-panned impulse must NOT echo identically in L/R.
 assert(echoL1>0.9f && std::fabs(echoR1)<0.001f);
 assert(std::fabs(echoL2)<0.001f && echoR2>0.7f);
 // At depth zero the same centered impulse remains centered.
 effect.prepare(1000.);step.value[20]=0;effect.setRow(step);
 for(int n=0;n<64;++n){float l=0,r=0;effect.process(n==0?1.f:0.f,n==0?1.f:0.f,l,r,120.);
  if(n==63)assert(std::fabs(l-r)<0.0001f);}
}
