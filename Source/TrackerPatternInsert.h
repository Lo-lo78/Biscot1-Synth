// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include "TrackerPatternBank.h"
#include "TrackerParameterCatalog.h"
#include <algorithm>
#include <cmath>

namespace biscot {
enum class TrackerInsertStatus { inserted, alreadyPresent, needsArpeggiator, full, invalid };
struct TrackerInsertResult {TrackerInsertStatus status; int index=-1;};
// An explicit position is zero-based and means INSERT AT that position (before
// the currently selected cell), never after it. -1 retains append semantics
// for old callers/tests. The step-wide 128-cell capacity is still enforced.
inline int trackerInsertionPosition(int requested,int count) noexcept {
 return requested<0?count:std::clamp(requested,0,count);
}
inline int trackerFindCell(const std::vector<TrackerCell>& cells,int id) noexcept {
 for(int i=0;i<(int)cells.size();++i)if(cells[(size_t)i].id==id)return i;
 return -1;
}
inline bool trackerArpActiveBefore(const std::vector<TrackerCell>& cells,int position) noexcept {
 bool active=false;
 for(int i=0;i<position && i<(int)cells.size();++i)
  if(cells[(size_t)i].id==2022)active=cells[(size_t)i].value>0.5;
 return active;
}
// F1 Ctrl+V: the copied parameter replaces the current *insertion position*
// rather than being appended after it. Lua 0.166 does not duplicate ordinary
// parameter IDs on paste; Sub Note is the sole repeatable parameter.
inline TrackerInsertResult pasteTrackerParameter(TrackerPattern& p,int step,int tab,
                                                 int requestedPosition,TrackerCell copied){
 if(step<0||step>=p.length||tab<0||tab>2)return {TrackerInsertStatus::invalid,-1};
 auto& cells=p.cells(step,tab);
 const int at=trackerInsertionPosition(requestedPosition,(int)cells.size());
 if(copied.id!=7){
  const int existing=trackerFindCell(cells,copied.id);
  if(existing>=0)return {TrackerInsertStatus::alreadyPresent,existing};
 }else if(!trackerArpActiveBefore(cells,at))
  return {TrackerInsertStatus::needsArpeggiator,-1};
 if(p.totalCells(step)>=128)return {TrackerInsertStatus::full,-1};
 if(!p.insert(step,tab,at,copied))return {TrackerInsertStatus::invalid,-1};
 return {TrackerInsertStatus::inserted,at};
}
// Insertion from the Enter-opened parameter picker. Keep positional insertion
// while preserving the musical order Curve -> Resolution -> Ratio and the
// chronological Previous Value marker. Existing IDs are focused, not duplicated.
inline TrackerInsertResult insertTrackerParameter(TrackerPattern& p,int step,int tab,
                                                   const TrackerParam& d,int requestedPosition=-1) {
 if(step<0||step>=p.length||tab<0||tab>2)return {TrackerInsertStatus::invalid,-1};
 auto& cells=p.cells(step,tab);
 if(d.id!=7){
  const int existing=trackerFindCell(cells,d.id);
  if(existing>=0)return {TrackerInsertStatus::alreadyPresent,existing};
 }
 int at=trackerInsertionPosition(requestedPosition,(int)cells.size());
 if(d.id==7 && !trackerArpActiveBefore(cells,at))
  return {TrackerInsertStatus::needsArpeggiator,-1};
 const bool autoCurve=(d.id==8||d.id==9) && trackerFindCell(cells,4)<0;
 if(p.totalCells(step)+(autoCurve?2:1)>128)return {TrackerInsertStatus::full,-1};
 if(autoCurve){
  cells.insert(cells.begin()+at,{4,100.0});
  ++at;
 }
 if(d.id==8 || d.id==9){
  const int curveIndex=trackerFindCell(cells,4);
  if(curveIndex>=0 && at<=curveIndex)at=curveIndex+1;
  if(d.id==8){
   // Curve Resolution precedes Curve Ratio, even when the user asks for an
   // insertion position beyond an existing Ratio.
   const int ratioIndex=trackerFindCell(cells,9);
   if(ratioIndex>=0 && ratioIndex>=curveIndex+1)at=std::min(at,ratioIndex);
  }else{
   // Curve Ratio follows an existing Resolution (otherwise it cannot act
   // on that Resolution in the left-to-right tracker stream).
   const int resIndex=trackerFindCell(cells,8);
   if(resIndex>=0 && at<=resIndex)at=resIndex+1;
  }
 }
 double initial=d.initial;
 if(step>0 && d.synthIndex>0){
  const int previousMarker=trackerFindCell(cells,3);
  // Previous Value only affects parameters inserted to its RIGHT.
  if(previousMarker>=0 && previousMarker<at){
   const auto& prior=p.view(step-1,tab);
   for(const auto& c:prior)if(c.id==d.id)initial=c.value;
  }
 }
 at=std::clamp(at,0,(int)cells.size());
 cells.insert(cells.begin()+at,{d.id,initial});
 return {TrackerInsertStatus::inserted,at};
}
}
