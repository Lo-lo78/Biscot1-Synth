// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include "TrackerPatternBank.h"
#include <algorithm>
namespace biscot {
// Lua 0.166: MIDI Note On replaces Pause, CC64 pedal-down replaces Note.
// Keeps all unrelated OP, Filter and LFO cells untouched.
inline bool trackerRecordStep(TrackerPattern& pat,int step,int note,bool pause){
 if(step<0||step>=pat.length)return false;
 auto& cells=pat.cells(step,0);
 cells.erase(std::remove_if(cells.begin(),cells.end(),[pause](const TrackerCell& c){
  return c.id==(pause?1:6);}),cells.end());
 const int id=pause?6:1;
 const double value=pause?1.0:std::clamp((double)note,0.0,127.0);
 auto it=std::find_if(cells.begin(),cells.end(),[id](const TrackerCell& c){return c.id==id;});
 if(it==cells.end())cells.insert(cells.begin(),TrackerCell{id,value});
 else it->value=value;
 return true;
}
}
