// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include "TrackerPatternBank.h"
#include "TrackerParameterCatalog.h"
#include <algorithm>
#include <cmath>

namespace biscot {
enum class TrackerInsertStatus { inserted, alreadyPresent, needsArpeggiator, full, invalid };
struct TrackerInsertResult {TrackerInsertStatus status; int index=-1;};
// F2 command semantics from Biscot1 TrackerControl 0.166. Operates only on the
// editable message-thread bank; never called by the audio processor.
inline TrackerInsertResult insertTrackerParameter(TrackerPattern& p,int step,int tab,const TrackerParam& d) {
 if(step<0||step>=p.length||tab<0||tab>2)return {TrackerInsertStatus::invalid,-1};
 auto& cells=p.cells(step,tab);
 const auto findIndex=[&](int id)->int {
  for(int i=0;i<(int)cells.size();++i)if(cells[(size_t)i].id==id)return i;
  return -1;
 };
 if(d.id!=7){const int found=findIndex(d.id);if(found>=0)return {TrackerInsertStatus::alreadyPresent,found};}
 if(d.id==7){
  // Sub Note may be repeated, but requires a non-Off Arp Mode in this row.
  bool active=false;for(const auto& c:cells)if(c.id==2022 && c.value>0.5)active=true;
  if(!active)return {TrackerInsertStatus::needsArpeggiator,-1};
 }
 const bool autoCurve=(d.id==8||d.id==9) && findIndex(4)<0;
 if(p.totalCells(step)+(autoCurve?2:1)>128)return {TrackerInsertStatus::full,-1};
 if(autoCurve)cells.push_back({4,100.0});
 if(d.id==8){
  // Resolution follows Curve, preceding Ratio, where possible.
  int at=findIndex(9);
  if(at<0)at=(int)cells.size();
  cells.insert(cells.begin()+at,{d.id,d.initial});
  return {TrackerInsertStatus::inserted,at};
 }
 double initial=d.initial;
 if(step>0 && d.synthIndex>0 && findIndex(3)>=0){
  const auto& prior=p.view(step-1,tab);
  for(const auto& c:prior)if(c.id==d.id)initial=c.value;
 }
 cells.push_back({d.id,initial});
 return {TrackerInsertStatus::inserted,(int)cells.size()-1};
}
}
