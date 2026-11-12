// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include "TrackerPatternBank.h"
#include "TrackerParameterCatalog.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

namespace biscot {
// Immutable, message-thread compiled automation. No scanning backward or
// forward through the editable Pattern is done in processBlock().
struct TrackerAutomatedParameter {
 int synthIndex=0,sourceStep=0,targetStep=0,resolution=0,ratio=0;
 double from=0,to=0,curve=0;
 bool active()const noexcept {
  return targetStep>sourceStep && std::abs(curve)>1e-9 && std::abs(to-from)>1e-9;
 }
 int segments()const noexcept{return resolution>0?resolution:32;}
};
inline double trackerCurveProgress(double t,double amount) noexcept {
 if(std::abs(amount)<1e-9)return 0.;
 const double span=std::clamp(std::abs(amount)/100.,0.0001,1.);
 const double x=amount>0 ? std::clamp(t/span,0.,1.) : std::clamp((t-(1.-span))/span,0.,1.);
 return x*x*(3.-2.*x);
}
inline bool trackerRatioIsCurved(int segment,int mode) noexcept {
 if(mode<=0)return true;
 const int straight=(mode<=4?mode:(mode==8?2:1));
 const int cycle=(mode<=4?mode+1:(mode==5?3:(mode==6?4:(mode==7?5:4))));
 return ((segment-1)%cycle)>=straight;
}
inline double trackerCurveAt(const TrackerAutomatedParameter& a,int step,double fraction)noexcept {
 if(!a.active())return a.from;
 const double t=((double)(step-a.sourceStep)+fraction)/(double)(a.targetStep-a.sourceStep);
 return a.from+(a.to-a.from)*trackerCurveProgress(t,a.curve);
}
// Largest curve segment whose JSFX sample offset is already due. Multiple
// segments falling on the same sample collapse to the same last value.
inline int trackerCurveDueSegment(double stepLength,double elapsed,int segments) noexcept {
 if(segments<1)return 0;
 const double span=std::max(0.,stepLength-1.);
 if(span<1.e-9)return segments;
 return std::clamp((int)std::ceil(((elapsed+1.)*(double)segments/span)-1.e-10)-1,0,segments);
}
inline double trackerCurveEventValue(const TrackerAutomatedParameter& a,int step,int segment)noexcept {
 if(!a.active())return a.from;
 if(a.ratio>0 && !trackerRatioIsCurved(segment,a.ratio))return a.from;
 if(a.ratio==0 && a.resolution>0 && segment%2==1)return a.from;
 return trackerCurveAt(a,step,(double)segment/(double)a.segments());
}
inline std::vector<TrackerCell> trackerFlatten(const TrackerPattern& p,int st){
 std::vector<TrackerCell> r;
 for(int tab=0;tab<3;++tab)for(const auto& c:p.view(st,tab)){
  if(r.size()>=128)break;
  r.push_back(c);
 }
 return r;
}
// Marker semantics: Previous value resolves to the nearest REAL value of the
// same ID, following chains of inherited occurrences backwards.  The marker
// only affects parameters to its right, never Note/Gate/Curve commands.
inline bool trackerIsInherited(const std::vector<TrackerCell>& cells,size_t at){
 bool inherited=false;
 for(size_t i=0;i<at;++i)if(cells[i].id==3)inherited=true;
 return inherited;
}
inline double trackerPreviousValue(const TrackerPattern& p,int st,int wanted,double fallback,
                                   const std::vector<std::vector<TrackerCell>>* cached=nullptr){
 for(int ps=st-1;ps>=0;--ps){
  const auto copy=cached?std::vector<TrackerCell>{}:trackerFlatten(p,ps);
  const auto& cells=cached?(*cached)[(size_t)ps]:copy;
  bool inherited=false,found=false,after=false;
  double value=fallback;
  for(const auto& c:cells){
   if(c.id==3)inherited=true;
   else if(c.id==wanted){found=true;after=inherited;value=c.value;}
  }
  if(found){if(after)continue;return value;}
 }
 return fallback;
}
inline bool trackerNextReal(const TrackerPattern& p,int st,int wanted,int& target,double& value,
                            const std::vector<std::vector<TrackerCell>>* cached=nullptr){
 for(int ns=st+1;ns<p.length;++ns){
  const auto copy=cached?std::vector<TrackerCell>{}:trackerFlatten(p,ns);
  const auto& cells=cached?(*cached)[(size_t)ns]:copy;
  bool inherited=false;
  for(const auto& c:cells){
   if(c.id==3)inherited=true;
   else if(c.id==wanted && !inherited){target=ns;value=c.value;return true;}
  }
 }
 return false;
}
struct TrackerCurveContext {bool inherit=false;double curve=0.;int resolution=0,ratio=0;};
inline void trackerUpdateContext(TrackerCurveContext& ctx,const TrackerCell& c){
 switch(c.id){
  case 3:ctx.inherit=true;break;
  case 4:ctx.curve=std::clamp(c.value,-100.,100.);break;
  case 5:ctx.curve=-std::clamp(std::abs(c.value),0.,100.);break; // legacy
  case 8:ctx.resolution=std::abs(ctx.curve)>1e-9?
    std::clamp(2*(int)std::floor((c.value+1.)/2.),2,1024):0;break;
  case 9:ctx.ratio=std::abs(ctx.curve)>1e-9?
    std::clamp((int)std::lround(c.value),0,8):0;break;
  default:break;
 }
}
inline TrackerAutomatedParameter trackerMakeAutomation(const TrackerPattern& p,int st,const TrackerCell& c,
                                                       const TrackerCurveContext& ctx,
                                                       const std::vector<std::vector<TrackerCell>>* cached=nullptr){
 TrackerAutomatedParameter a;
 const auto* d=findTrackerParam(c.id);
 if(!d)return a;
 a.synthIndex=d->synthIndex;
 a.sourceStep=st;a.targetStep=st;
 a.from=ctx.inherit?trackerPreviousValue(p,st,c.id,c.value,cached):c.value;
 a.to=a.from;a.curve=ctx.curve;a.resolution=ctx.resolution;a.ratio=ctx.ratio;
 if(std::abs(a.curve)>1e-9)trackerNextReal(p,st,c.id,a.targetStep,a.to,cached);
 return a;
}
inline std::vector<TrackerAutomatedParameter> trackerCompileStepAutomation(const TrackerPattern& p,int st,
 const std::vector<std::vector<TrackerCell>>* cached=nullptr){
 std::vector<TrackerAutomatedParameter> out;
 out.reserve(16);
 std::array<bool,234> seen{};
 const auto copy=cached?std::vector<TrackerCell>{}:trackerFlatten(p,st);
 const auto& current=cached?(*cached)[(size_t)st]:copy;
 TrackerCurveContext context;
 for(const auto& c:current){
  if(c.id==3||c.id==4||c.id==5||c.id==8||c.id==9){trackerUpdateContext(context,c);continue;}
  const auto* def=findTrackerParam(c.id);
  if(!def || def->synthIndex<=0 || def->synthIndex>=234 || def->synthIndex==3)continue;
  const auto a=trackerMakeAutomation(p,st,c,context,cached);
  out.push_back(a);seen[(size_t)a.synthIndex]=true;
  if(out.size()>=128)return out;
 }
 // Nearest preceding parameter occurrence owns the curve across empty rows.
 for(int ps=st-1;ps>=0 && out.size()<128;--ps){
  const auto owned=cached?std::vector<TrackerCell>{}:trackerFlatten(p,ps);
  const auto& prev=cached?(*cached)[(size_t)ps]:owned;
  TrackerCurveContext c;
  for(const auto& x:prev){
   if(x.id==3||x.id==4||x.id==5||x.id==8||x.id==9){trackerUpdateContext(c,x);continue;}
   const auto* d=findTrackerParam(x.id);
   if(!d||d->synthIndex<=0||d->synthIndex>=234||d->synthIndex==3)continue;
   if(seen[(size_t)d->synthIndex])continue;
   seen[(size_t)d->synthIndex]=true;
   auto a=trackerMakeAutomation(p,ps,x,c,cached);
   if(a.active() && a.targetStep>st)out.push_back(a);
   if(out.size()>=128)break;
  }
 }
 return out;
}

// Linear-time whole-Pattern compiler. The earlier reference implementation
// above is retained for independent semantic tests; calling it for every step
// would perform quadratic backward scans on 1024-step, 128-cell Patterns and
// stall the editor. This pass keeps the same Lua 0.166 ordering and marker
// behavior, but compiles all rows in one sweep (never on the audio thread).
inline std::vector<std::vector<TrackerAutomatedParameter>>
trackerCompileAllAutomation(const TrackerPattern&,
 const std::vector<std::vector<TrackerCell>>& flat){
 const int n=(int)flat.size();
 std::vector<std::vector<TrackerAutomatedParameter>> explicitRows((size_t)n);
 std::array<double,234> previousReal{},previousAny{};
 std::array<bool,234> hasReal{},hasAny{};
 for(int st=0;st<n;++st){
  TrackerCurveContext context;
  auto& explicitValues=explicitRows[(size_t)st];
  explicitValues.reserve(flat[(size_t)st].size());
  // Snapshot prior rows before this row updates the historical state.
  for(const auto& c:flat[(size_t)st]){
   if(c.id==3||c.id==4||c.id==5||c.id==8||c.id==9){trackerUpdateContext(context,c);continue;}
   const auto* d=findTrackerParam(c.id);
   if(!d||d->synthIndex<=0||d->synthIndex>=234||d->synthIndex==3)continue;
   const int sid=d->synthIndex;
   TrackerAutomatedParameter a;
   a.synthIndex=sid;a.sourceStep=st;a.targetStep=st;
   a.from=context.inherit ? (hasReal[(size_t)sid]?previousReal[(size_t)sid]:
           (hasAny[(size_t)sid]?previousAny[(size_t)sid]:c.value)) :c.value;
   a.to=a.from;a.curve=context.curve;a.resolution=context.resolution;a.ratio=context.ratio;
   explicitValues.push_back(a);
  }
  bool inherited=false;
  for(const auto& c:flat[(size_t)st]){
   if(c.id==3){inherited=true;continue;}
   const auto* d=findTrackerParam(c.id);
   if(!d||d->synthIndex<=0||d->synthIndex>=234||d->synthIndex==3)continue;
   const int sid=d->synthIndex;
   hasAny[(size_t)sid]=true;previousAny[(size_t)sid]=c.value;
   if(!inherited){hasReal[(size_t)sid]=true;previousReal[(size_t)sid]=c.value;}
  }
 }
 std::array<int,234> nextRealStep;nextRealStep.fill(-1);
 std::array<double,234> nextRealValue{};
 for(int st=n-1;st>=0;--st){
  for(auto& a:explicitRows[(size_t)st]){
   const size_t sid=(size_t)a.synthIndex;
   if(std::abs(a.curve)>1e-9 && nextRealStep[sid]>=0){
    a.targetStep=nextRealStep[sid];a.to=nextRealValue[sid];
   }
  }
  std::array<bool,234> first{};
  bool inherited=false;
  for(const auto& c:flat[(size_t)st]){
   if(c.id==3){inherited=true;continue;}
   const auto* d=findTrackerParam(c.id);
   if(!d||d->synthIndex<=0||d->synthIndex>=234||d->synthIndex==3||inherited)continue;
   const size_t sid=(size_t)d->synthIndex;
   if(!first[sid]){first[sid]=true;nextRealStep[sid]=st;nextRealValue[sid]=c.value;}
  }
 }
 std::vector<std::vector<TrackerAutomatedParameter>> result((size_t)n);
 std::array<TrackerAutomatedParameter,234> nearest{};
 std::array<bool,234> known{};
 for(int st=0;st<n;++st){
  auto& dst=result[(size_t)st];
  dst=std::move(explicitRows[(size_t)st]);
  std::array<bool,234> seen{};
  for(const auto& a:dst)seen[(size_t)a.synthIndex]=true;
  for(int sid=1;sid<234 && dst.size()<128;++sid){
   if(seen[(size_t)sid]||!known[(size_t)sid])continue;
   const auto& prev=nearest[(size_t)sid];
   if(prev.active() && prev.targetStep>st)dst.push_back(prev);
  }
  // The previous row's FIRST appearance of a parameter establishes the
  // nearest source (the legacy JSFX walks the row in physical order).
  std::array<bool,234> first{};
  for(const auto& c:flat[(size_t)st]){
   const auto* d=findTrackerParam(c.id);
   if(!d||d->synthIndex<=0||d->synthIndex>=234||d->synthIndex==3)continue;
   const size_t sid=(size_t)d->synthIndex;
   if(first[sid])continue;
   first[sid]=true;
   for(const auto& a:dst)if(a.synthIndex==(int)sid && a.sourceStep==st){
    nearest[sid]=a;known[sid]=true;break;
   }
  }
  if(dst.size()>128)dst.resize(128);
 }
 return result;
}
}
