// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
// F1 direct header/repeat shortcuts. JUCE-free so the complete cursor
// semantics can be regression-tested in Release builds.
#include <algorithm>

namespace biscot {
struct TrackerHeaderLocation { int row=0, cell=0; bool valid=false; };
inline TrackerHeaderLocation trackerHeaderShortcut(char key, bool backwards,
                                                   int length, int repeats,
                                                   int row, int cell) noexcept {
 key = (key>='A'&&key<='Z') ? char(key-'A'+'a') : key;
 switch(key){
  case 'l': return {-1,0,true};
  case 'm': return {-1,1,true};
  case 's': return {-1,2,true};
  case 'r': return {-1,3,true};
  case 'p': return {length,0,true};
  default: break;
 }
 const bool speed=key=='d', transpose=key=='t';
 if(!speed&&!transpose)return {};
 repeats=std::clamp(repeats,1,1000);
 int index=backwards?repeats-1:0;
 if(row>length){
  const int current=(row-length-1)*5+cell/2;
  const bool same=(speed && cell%2==0) || (transpose && cell%2==1);
  if(same && current>=0 && current<repeats)
   index=(current+(backwards?repeats-1:1))%repeats;
 }
 return {length+1+index/5, (index%5)*2+(transpose?1:0),true};
}
// F1 has three independent vertical navigation regions: Pattern header,
// actual steps, and Repeat/Speed/Transpose. Plain Up/Down and PageUp/Down
// must NEVER silently cross a boundary; only explicit Ctrl+Home/End can.
inline int trackerMoveWithinF1Region(int row,int delta,int length,int repeats) noexcept {
 length=std::clamp(length,1,1024);
 repeats=std::clamp(repeats,1,1000);
 const int lastFooterRow=length+(repeats+4)/5;
 if(row<0)return -1;  // The four header fields are one horizontal row.
 if(row<length)return std::clamp(row+delta,0,length-1);
 return std::clamp(row+delta,length,lastFooterRow);
}
inline int trackerF1HomeTarget(int row,int length,int savedStep) noexcept {
 if(row<0)return -1;
 if(row<length)return -1; // Steps -> Length/header.
 return std::clamp(savedStep,0,std::max(0,length-1)); // Repeat/Speed -> remembered step.
}
inline int trackerF1EndTarget(int row,int length,int repeats,int savedStep) noexcept {
 if(row<0)return std::clamp(savedStep,0,std::max(0,length-1)); // Header -> steps.
 if(row<length)return length; // Steps -> Repeat.
 return row; // Footer (Repeat/Speed/Transpose) is ONE area.
}
// Plain Home/End visit the first/last position WITHIN the current area.
// They never cross between Header, Steps or Repeat/Speed/Transpose.
// In the footer, End selects the last available Transpose, not an
// invalid item in the final partially populated row.
inline TrackerHeaderLocation trackerF1AreaEdge(int row, int cell, int length, int repeats, bool toEnd) noexcept {
 length=std::clamp(length,1,1024);
 repeats=std::clamp(repeats,1,1000);
 if(row<0)return {-1,cell,true}; // Header: no vertical movement.
 if(row<length)return {toEnd?length-1:0,cell,true};
 if(!toEnd)return {length,0,true}; // First footer item = Repeat.
 const int idx=repeats-1;
 return {length+1+idx/5, (idx%5)*2+1, true};
}
inline int trackerReturnToLastStep(int currentRow,int length,int previousStep) noexcept {
 // Only the transition from Repeat -> real steps restores the remembered
 // editor cursor. From Speed/Transpose, Up still visits Repeat first.
 return currentRow==length ? std::clamp(previousStep,0,std::max(0,length-1))
                           : std::clamp(currentRow-1,-1,length+(1000+4)/5);
}
} // namespace biscot
