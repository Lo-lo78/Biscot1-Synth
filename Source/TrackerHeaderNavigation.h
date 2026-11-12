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
inline int trackerReturnToLastStep(int currentRow,int length,int previousStep) noexcept {
 // Only the transition from Repeat -> real steps restores the remembered
 // editor cursor. From Speed/Transpose, Up still visits Repeat first.
 return currentRow==length ? std::clamp(previousStep,0,std::max(0,length-1))
                           : std::clamp(currentRow-1,-1,length+(1000+4)/5);
}
} // namespace biscot
