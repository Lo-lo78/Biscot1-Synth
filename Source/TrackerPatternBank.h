// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include <algorithm>
#include <array>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace biscot {
// Pure-data, JUCE-free port of the Lua tracker Pattern/step/cell memory.
// Step cells stay ordered within OP, Filter and LFO; no allocation in processBlock.
struct TrackerCell { int id=0; double value=0; };
struct TrackerStep { std::array<std::vector<TrackerCell>,3> tabs{}; };
struct TrackerPattern {
 std::string name;
 int length=16, midiNote=-1, repeat=1;
 bool releaseArp=false;
 std::vector<double> repeatSpeeds{1.0};
 std::vector<int> repeatTransposes{0};
 std::map<int,TrackerStep> steps;
 TrackerPattern(bool init=true){
  if(init) for(int s=0;s<16;++s){
   steps[s].tabs[0]={{1,60},{2,0.92},{2004,2},{2002,1},{2008,0},{2009,0.12},{2010,0.7},{2011,0}};
  }
 }
 std::vector<TrackerCell>& cells(int step,int tab){return steps[step].tabs[(size_t)std::clamp(tab,0,2)];}
 const std::vector<TrackerCell>& view(int step,int tab) const {
  static const std::vector<TrackerCell> empty;
  const auto it=steps.find(step);
  return it==steps.end()?empty:it->second.tabs[(size_t)std::clamp(tab,0,2)];
 }
 int totalCells(int step) const {
  const auto it=steps.find(step);if(it==steps.end())return 0;
  int n=0;for(const auto& tab:it->second.tabs)n+=(int)tab.size();return n;
 }
 bool insert(int step,int tab,int position,TrackerCell c){
  if(step<0 || step>=length || tab<0 || tab>2 || totalCells(step)>=128)return false;
  auto& a=cells(step,tab);a.insert(a.begin()+std::clamp(position,0,(int)a.size()),c);return true;
 }
 bool erase(int step,int tab,int position){
  if(step<0||step>=length||tab<0||tab>2)return false;
  auto& a=cells(step,tab);if(position<0||position>=(int)a.size())return false;
  a.erase(a.begin()+position);return true;
 }
};
class TrackerPatternBank {
public:
 static constexpr int maxPatterns=1000,maxSteps=1024,maxCells=128;
 std::map<int,TrackerPattern> patterns;
 int activePattern=0;
 TrackerPatternBank(){patterns.try_emplace(0);}
 TrackerPattern& at(int n){n=std::clamp(n,0,maxPatterns-1);return patterns.try_emplace(n).first->second;}
 const TrackerPattern* find(int n) const {const auto i=patterns.find(n);return i==patterns.end()?nullptr:&i->second;}
 void reset(){patterns.clear();patterns.try_emplace(0);activePattern=0;}
};
}
