// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <map>
#include <string>
#include <vector>

namespace biscot {
// Native, sparse port of the Lua 0.166 Song memory. This module never runs
// from processBlock(): the audio engine will later use immutable snapshots.
struct TrackerSongStep {
 // _order from Lua: only explicitly inserted parameters are present.
 // Pattern (1) is one indivisible ordered group; its auxiliary controls
 // (5/6 and read-only 7/8/21) are exposed by the editor, not separately ordered.
 std::vector<int> order;
 std::array<double,24> values{};
 TrackerSongStep() {
  values[1]=-1;values[2]=1;values[4]=1;values[6]=1;
  values[10]=1;values[11]=7;values[12]=values[13]=values[14]=1;
  values[19]=100;values[23]=3; // preview duration, independently saved per Song step
 }
 bool contains(int id) const {
  if(id==5||id==6||id==7||id==8||id==21)return contains(1);
  return std::find(order.begin(),order.end(),id)!=order.end();
 }
 bool insert(int id,int pos=-1){
  if(id<1||id>23||id==5||id==6||id==7||id==8||id==21||contains(id)||order.size()>=18)return false;
  order.insert(order.begin()+(pos<0?(int)order.size():std::clamp(pos,0,(int)order.size())),id);
  return true;
 }
 bool erase(int id){
  if(id==5||id==6||id==7||id==8||id==21)id=1;
  const auto it=std::find(order.begin(),order.end(),id);
  if(it==order.end())return false;
  order.erase(it);return true;
 }
 std::vector<int> visible() const {
  std::vector<int> result;
  for(int id:order){
   if(id==1)result.insert(result.end(),{1,5,6,21,7,8});
   else result.push_back(id);
  }
  return result;
 }
 bool empty() const noexcept {return order.empty();}
};
struct TrackerSong {
 std::string name;
 std::array<int,3> laneLengths{16,16,16};
 int midiNote=-1,songEnd=-1,loopTarget=-1;
 bool chainLoop=false;
 std::array<int,3> laneRepeats{1,1,1};
 std::array<std::vector<double>,3> laneSpeeds{{{1.0},{1.0},{1.0}}};
 int filterOwner=1,filterMask=7;
 std::array<int,3> lfoOwners{1,1,1};
 std::array<int,3> mixerVolumes{100,100,100};
 int mixerMuteMask=0,mixerSoloLane=0;
 std::array<std::map<int,TrackerSongStep>,3> lanes;
 int length() const noexcept {return std::max({laneLengths[0],laneLengths[1],laneLengths[2]});}
 const TrackerSongStep* findStep(int lane,int step) const {
  if(lane<0||lane>2||step<0||step>=4096)return nullptr;
  const auto& a=lanes[(size_t)lane];const auto it=a.find(step);
  return it==a.end()?nullptr:&it->second;
 }
 TrackerSongStep& atStep(int lane,int step){return lanes[(size_t)std::clamp(lane,0,2)][std::clamp(step,0,4095)];}
};
class TrackerSongBank {
public:
 static constexpr int maxSongs=100,maxSteps=4096,maxLanes=3,maxLaneRepeats=1000;
 std::map<int,TrackerSong> songs;
 int activeSong=0;
 TrackerSongBank(){songs.try_emplace(0);}
 TrackerSong& at(int n){return songs.try_emplace(std::clamp(n,0,99)).first->second;}
 const TrackerSong* find(int n)const {const auto it=songs.find(n);return it==songs.end()?nullptr:&it->second;}
 void reset(){songs.clear();songs.try_emplace(0);activeSong=0;}
};
}
