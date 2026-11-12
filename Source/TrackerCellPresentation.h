// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
// JUCE-free text/value formatting for the native tracker. Reuse the synth's
// authoritative option labels instead of presenting bare stored enum indices.
#include "TrackerPatternBank.h"
#include "TrackerParameterCatalog.h"
#include "ParameterCatalog.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>
#include <string_view>

namespace biscot {
inline std::string trackerNumber(double value, int precision=4, bool fixedPrecision=false) {
 if (!std::isfinite(value)) return "0";
 std::ostringstream out;
 out << std::fixed << std::setprecision(precision) << value;
 std::string text=out.str();
 if (!fixedPrecision && text.find('.')!=std::string::npos) {
  while (!text.empty() && text.back()=='0') text.pop_back();
  if (!text.empty() && text.back()=='.') text.pop_back();
 }
 if (text=="-0" || text=="-0.0000") text=fixedPrecision?"0.0000":"0";
 return text;
}
inline std::string trackerNoteName(double value) {
 static constexpr std::array<const char*,12> names={"C","C sharp","D","D sharp","E","F",
  "F sharp","G","G sharp","A","A sharp","B"};
 const int note=std::clamp((int)std::lround(value),0,127);
 return std::to_string(note)+" "+names[(size_t)note%12]+" "+std::to_string(note/12-1);
}
inline std::string trackerChoiceAt(std::string_view options,int at){
 if(at<0||options.empty())return {};
 size_t start=0;
 for(int i=0;i<at;++i){
  const auto sep=options.find('|',start);
  if(sep==std::string_view::npos)return {};
  start=sep+1;
 }
 const auto end=options.find('|',start);
 if(start>=options.size())return {};
 return std::string(options.substr(start,end==std::string_view::npos?std::string_view::npos:end-start));
}
inline std::string trackerCellValue(int id,double value){
 if(id==1||id==7)return trackerNoteName(value);
 if(id==3)return "Previous value";
 if(id==6)return "Pause";
 if(id==4)return trackerNumber(value,2,true);
 if(id==9){
  // The Curve Ratio is a tracker-specific enum, not a synth parameter.
  static constexpr std::array<const char*,9> ratios={"Off","1 to 1","2 to 1",
   "3 to 1","4 to 1","1 to 2","1 to 3","1 to 4","2 to 2"};
  const int idx=(int)std::lround(value);
  if(idx>=0 && idx<(int)ratios.size())return ratios[(size_t)idx];
 }
 if(id>=1231 && id<=1233)return trackerNumber(value,4,true);
 if(const auto* tracker=findTrackerParam(id)){
  if(tracker->synthIndex>0){
   for(const auto& synth : catalog)if(synth.index==tracker->synthIndex && synth.choices && *synth.choices){
    const int choice=(int)std::lround(value-(double)synth.low);
    const auto label=trackerChoiceAt(synth.choices,choice);
    if(!label.empty())return label;
    break;
   }
  }
 }
 return trackerNumber(value);
}
inline std::string trackerCellLabel(const TrackerCell& cell){
 if(cell.id==3)return "Previous value";
 if(cell.id==6)return "Pause";
 const auto* p=findTrackerParam(cell.id);
 return std::string(p?p->name:"Unknown parameter")+", "+trackerCellValue(cell.id,cell.value);
}

// Preserve the preferred horizontal position across steps with different
// numbers of cells. A short/empty step only clamps the visible cursor.
inline int trackerVisibleCell(int preferred,int visibleCount){
 return std::clamp(preferred,0,std::max(0,visibleCount-1));
}
}
