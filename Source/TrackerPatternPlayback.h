// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include "TrackerPatternBank.h"
#include "TrackerParameterCatalog.h"
#include "TrackerPatternAutomation.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

namespace biscot {
// Message-thread compiled, immutable Pattern snapshot. The audio thread never
// visits the editable TrackerPatternBank/map or allocates Pattern rows.
struct TrackerPlayableRow {
 std::vector<TrackerCell> ordered;
 int note=-1;
 float gate=0.92f;
 bool pause=false;
 bool gateExplicit=false;
 bool holdIntoNext=false;
 std::vector<TrackerAutomatedParameter> automation;
 std::vector<int> curveIndices;
};
struct TrackerPlayablePattern {
 int length=16, repeat=1;
 bool releaseArp=false;
 std::vector<double> repeatSpeeds;
 std::vector<int> repeatTransposes;
 std::vector<TrackerPlayableRow> rows;
};
inline TrackerPlayablePattern compileTrackerPattern(const TrackerPattern& pattern) {
 TrackerPlayablePattern result;
 result.length=std::clamp(pattern.length,1,1024);
 result.repeat=std::clamp(pattern.repeat,1,1000);
 result.repeatSpeeds=pattern.repeatSpeeds;
 result.repeatTransposes=pattern.repeatTransposes;
 result.releaseArp=pattern.releaseArp;
 result.rows.resize((size_t)result.length);
 std::vector<std::vector<TrackerCell>> flattened;flattened.reserve((size_t)result.length);
 for(int i=0;i<result.length;++i)flattened.push_back(trackerFlatten(pattern,i));
 auto automation=trackerCompileAllAutomation(pattern,flattened);
 for(int i=0;i<result.length;++i){
  auto& out=result.rows[(size_t)i];
  out.automation=std::move(automation[(size_t)i]);
  for(int a=0;a<(int)out.automation.size();++a)
   if(out.automation[(size_t)a].active())out.curveIndices.push_back(a);
  for(int tab=0;tab<3;++tab)
   for(const auto& c:pattern.view(i,tab)){
    if(out.ordered.size()<128)out.ordered.push_back(c);
    if(c.id==1)out.note=std::clamp((int)std::lround(c.value),0,127);
    if(c.id==2){out.gate=(float)std::clamp(c.value,0.01,1.);out.gateExplicit=true;}
    if(c.id==6)out.pause=true;
   }
  // The Lua's Previous value at the next step preserves the preceding gate.
  if(i+1<result.length)for(int tab=0;tab<3;++tab)
   for(const auto& c:pattern.view(i+1,tab))
    if(c.id==3 && c.value<1.5)out.holdIntoNext=true;
 }
 return result;
}
// Lua 0.166 previews a step only when that step has an explicit Note.
// Previewing a parameter-only row must not restart an unrelated held note.
inline bool trackerStepHasAuditionNote(const TrackerPattern& pattern,int step){
 if(step<0 || step>=pattern.length)return false;
 for(int tab=0;tab<3;++tab)
  for(const auto& c:pattern.view(step,tab))
   if(c.id==1)return true;
 return false;
}
// Status 1 is a user-started complete Pattern: editing must not interrupt it.
// Status 3 is a one-step audition, which an edit is allowed to restart.
inline bool trackerCanAuditionStep(const TrackerPattern& pattern,int step,int status){
 return status!=1 && trackerStepHasAuditionNote(pattern,step);
}
struct TrackerTransportTick {int startStep=-1; bool gateOff=false, finished=false;};
class TrackerPatternTransport {
public:
 bool running=false, audition=false;
 int currentStep=-1, nextStep=0, repeatIndex=0, activeNote=-1;
 double stepRemaining=0,stepLength=0,gateRemaining=-1;
 void start(int step,const TrackerPlayablePattern& p,bool onlyOne=false){
  running=true;audition=onlyOne;currentStep=-1;nextStep=std::clamp(step,0,p.length-1);
  repeatIndex=0;activeNote=-1;stepRemaining=0;stepLength=0;gateRemaining=-1;
 }
 void stop(){running=false;audition=false;currentStep=-1;activeNote=-1;stepRemaining=0;stepLength=0;gateRemaining=-1;}
 double stepSamples(const TrackerPlayablePattern& p,double rate,double bpm)const{
  const double speed=repeatIndex<(int)p.repeatSpeeds.size()?p.repeatSpeeds[(size_t)repeatIndex]:1.;
  // Original JSFX division_value(slider3==4) = 16, i.e. 16th notes.
  return std::max(1.,rate/((std::clamp(bpm,1.,999.)/60.)*4.*std::clamp(speed,0.25,16.)));
 }
 void gateFor(double samples,bool hold){gateRemaining=hold?-1:std::max(1.,std::floor(samples));}
 TrackerTransportTick tick(const TrackerPlayablePattern& p,double rate,double bpm){
  TrackerTransportTick ev;
  if(!running)return ev;
  if(gateRemaining>0){gateRemaining-=1.;if(gateRemaining<=0){ev.gateOff=true;gateRemaining=-1;}}
  if(stepRemaining<=0){
   if(nextStep<0){ev.finished=true;running=false;return ev;}
   currentStep=nextStep;ev.startStep=currentStep;
   stepLength=stepSamples(p,rate,bpm);stepRemaining=stepLength;
   ++nextStep;
   if(audition)nextStep=-1;
   else if(nextStep>=p.length){
    ++repeatIndex;
    nextStep=repeatIndex<p.repeat?0:-1;
   }
  }
  stepRemaining-=1.;
  return ev;
 }
};
}
