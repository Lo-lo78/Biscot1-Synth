// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include "TrackerSongBank.h"
#include <algorithm>
#include <map>
#include <optional>
#include <utility>
#include <vector>
namespace biscot {
// Song edit operations are UI-thread only. All snapshots are sparse; nothing in
// this module is called by the audio thread.
class TrackerSongHistory {
public:
 static constexpr size_t limit=10;
 struct Entries {std::vector<TrackerSong> undo,redo;};
 std::map<int,Entries> bySong;
 void record(int slot,const TrackerSong& before){
  auto& h=bySong[slot];h.undo.push_back(before);
  if(h.undo.size()>limit)h.undo.erase(h.undo.begin());
  h.redo.clear();
 }
 bool restore(int slot,TrackerSong& current,bool redo){
  auto& h=bySong[slot];auto& source=redo?h.redo:h.undo;
  auto& dest=redo?h.undo:h.redo;
  if(source.empty())return false;
  dest.push_back(current);if(dest.size()>limit)dest.erase(dest.begin());
  current=std::move(source.back());source.pop_back();return true;
 }
 void clear(){bySong.clear();}
};
struct TrackerSongClipboard {
 enum class Mode{none,parameter,steps,wholeSong};
 Mode mode=Mode::none;
 int sourceSong=-1,sourceLane=-1,sourceStep=-1;
 int parameterId=-1;
 TrackerSongStep parameterData;
 std::vector<std::optional<TrackerSongStep>> steps;
 TrackerSong whole;
 void copyParameter(int song,int lane,int step,int id,const TrackerSongStep& data){
  mode=Mode::parameter;sourceSong=song;sourceLane=lane;sourceStep=step;
  parameterId=(id==5||id==6||id==7||id==8||id==21)?1:id;parameterData=data;
 }
 void copySteps(int song,int lane,int start,int end,const TrackerSong& data){
  mode=Mode::steps;sourceSong=song;sourceLane=lane;sourceStep=start;steps.clear();
  for(int st=start;st<=end;++st){
   const auto* src=data.findStep(lane,st);
   if(src)steps.emplace_back(*src);else steps.emplace_back(std::nullopt);
  }
 }
 void copySong(int song,const TrackerSong& data){mode=Mode::wholeSong;sourceSong=song;whole=data;}
 bool pasteParameter(TrackerSong& dest,int lane,int step,int insertAt)const {
  if(mode!=Mode::parameter||parameterId<1||lane<0||lane>=3||step<0||step>=4096)return false;
  auto& entry=dest.atStep(lane,step);const auto existing=entry.visible();
  insertAt=std::clamp(insertAt,0,(int)existing.size());
  int pos=0;
  for(int i=0;i<insertAt;++i){
   const int id=existing[(size_t)i];
   if(id!=5&&id!=6&&id!=7&&id!=8&&id!=21)++pos;
  }
  if(!entry.insert(parameterId,pos))return false;
  entry.values[(size_t)parameterId]=parameterData.values[(size_t)parameterId];
  if(parameterId==1){entry.values[5]=parameterData.values[5];entry.values[6]=parameterData.values[6];}
  return true;
 }
 int pasteSteps(TrackerSong& dest,int lane,int start)const {
  if(mode!=Mode::steps||lane<0||lane>=3||start<0||start>=dest.laneLengths[(size_t)lane])return 0;
  int count=0;auto& storage=dest.lanes[(size_t)lane];
  for(const auto& item:steps){
   const int target=start+count;
   if(target>=dest.laneLengths[(size_t)lane])break;
   if(item&& !item->empty())storage[target]=*item;else storage.erase(target);
   ++count;
  }
  return count;
 }
 bool pasteSong(TrackerSong& destination)const {
  if(mode!=Mode::wholeSong)return false;
  const auto name=destination.name;destination=whole;destination.name=name;
  return true;
 }
};
inline bool moveSongParameter(TrackerSongStep& entry,int id,int direction){
 if(id==5||id==6||id==7||id==8||id==21)id=1;
 auto it=std::find(entry.order.begin(),entry.order.end(),id);
 if(it==entry.order.end()||direction==0)return false;
 const int index=(int)std::distance(entry.order.begin(),it),dst=index+(direction<0?-1:1);
 if(dst<0||dst>=(int)entry.order.size())return false;
 std::swap(entry.order[(size_t)index],entry.order[(size_t)dst]);return true;
}
inline int eraseSongSteps(TrackerSong& song,int lane,int start,int end){
 if(lane<0||lane>2)return 0;
 start=std::clamp(start,0,song.laneLengths[(size_t)lane]-1);
 end=std::clamp(end,start,song.laneLengths[(size_t)lane]-1);
 for(int i=start;i<=end;++i)song.lanes[(size_t)lane].erase(i);
 return end-start+1;
}
}
