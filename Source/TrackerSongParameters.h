// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include "TrackerSongBank.h"
namespace biscot {
struct TrackerSongParameter {
 int id;const char* name;double min,max,step;
};
// Lua F4_PARAMETER_IDS, in original order. Pattern's detail cells are generated
// by its group and never separately inserted into _order.
inline constexpr std::array<TrackerSongParameter,17> songParameters{{
 {1,"Pattern",-1,999,1},
 {22,"Start Offset",0,31,1},
 {2,"Speed",0.25,16,0.25},
 {3,"Transpose",-96,96,1},
 {4,"Repeat",1,16,1},
 {9,"Legato",0,1,1},
 {10,"Filter Owner",1,3,1},
 {11,"Filter Operators",0,7,1},
 {12,"LFO1 Owner",1,3,1},
 {13,"LFO2 Owner",1,3,1},
 {14,"LFO3 Owner",1,3,1},
 {15,"Sidechain Source",0,3,1},
 {16,"Sidechain Amount",0,100,1},
 {17,"Delay Speed",0,8,0.125},
 {18,"Delay Feedback",0,100,1},
 {19,"Delay Volume",0,100,1},
 {20,"Delay Ping-Pong Depth",0,100,1}
}};
inline const TrackerSongParameter* songParam(int id){
 const auto it=std::find_if(songParameters.begin(),songParameters.end(),[id](const auto& p){return p.id==id;});
 return it==songParameters.end()?nullptr:&*it;
}
inline double songDefaultValue(int id){
 switch(id){case 1:return -1;case 2:case 4:case 6:case 10:case 12:case 13:case 14:return 1;
 case 11:return 7;case 19:return 100;default:return 0;}
}
inline double songClampValue(int id,double value){
 if(!std::isfinite(value))return songDefaultValue(id);
 if(id==5)return std::clamp(std::round(value),0.,1023.);
 if(id==6)return std::clamp(std::round(value),1.,1024.);
 if(const auto* p=songParam(id)){
  value=std::clamp(value,p->min,p->max);
  return p->step<1?std::round(value/p->step)*p->step:std::round(value);
 }
 return value;
}
}
