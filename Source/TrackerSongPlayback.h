// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include "TrackerSongBank.h"
#include "TrackerPatternPlayback.h"
#include <array>
#include <algorithm>
#include <cmath>
#include <map>
#include <memory>
#include <vector>
namespace biscot {
// Compiled on the message thread; audio thread touches only this immutable graph.
struct TrackerSongClip {
 int songRow=0, patternId=-1, patternStart=0, playLength=0;
 int repeat=1, transpose=0, startOffset=0;
 double speed=1.; bool legato=false;
 int filterOwner=0, filterMask=-1;
 std::array<int,3> lfoOwners{};
 std::shared_ptr<const TrackerPlayablePattern> pattern;
};
struct TrackerPlayableSong {
 int songNumber=0;
 std::array<int,3> lengths{16,16,16}, repeats{1,1,1};
 std::array<std::vector<double>,3> laneSpeeds;
 std::array<std::vector<TrackerSongClip>,3> clips;
 std::array<float,3> mixer{{1,1,1}};
 int filterOwner=1,filterMask=7;
 std::array<int,3> lfoOwners{1,1,1};
 int midiNote=-1, songEnd=-1, loopTarget=-1;
 bool chainLoop=false;
};
inline TrackerPlayableSong compileTrackerSong(const TrackerSong& song,int songNumber,const TrackerPatternBank& patterns){
 TrackerPlayableSong result;result.songNumber=songNumber;
 result.lengths=song.laneLengths;result.repeats=song.laneRepeats;
 result.laneSpeeds=song.laneSpeeds;result.filterOwner=song.filterOwner;
 result.filterMask=song.filterMask;result.lfoOwners=song.lfoOwners;
 result.midiNote=song.midiNote;result.songEnd=song.songEnd;
 result.loopTarget=song.loopTarget;result.chainLoop=song.chainLoop;
 std::map<int,std::shared_ptr<const TrackerPlayablePattern>> cache;
 for(int lane=0;lane<3;++lane){
  const bool solo=song.mixerSoloLane!=0;
  const bool silenced=(song.mixerMuteMask&(1<<lane))!=0 || (solo && song.mixerSoloLane!=lane+1);
  result.mixer[(size_t)lane]=silenced?0.f:(float)std::clamp(song.mixerVolumes[(size_t)lane]/100.,0.,1.);
  result.lengths[(size_t)lane]=std::clamp(result.lengths[(size_t)lane],1,4096);
  result.repeats[(size_t)lane]=std::clamp(result.repeats[(size_t)lane],1,1000);
  for(const auto& [position,step]:song.lanes[(size_t)lane]){
   if(position>=result.lengths[(size_t)lane]||position<0||!step.contains(1))continue;
   const int id=(int)std::lround(step.values[1]);
   if(id<0||id>=TrackerPatternBank::maxPatterns)continue;
   const auto* source=patterns.find(id);if(!source)continue;
   auto& compiled=cache[id];
   if(!compiled)compiled=std::make_shared<const TrackerPlayablePattern>(compileTrackerPattern(*source));
   TrackerSongClip clip;
   clip.songRow=position;clip.patternId=id;clip.pattern=compiled;
   clip.patternStart=std::clamp((int)std::lround(step.values[5]),0,compiled->length-1);
   clip.playLength=std::clamp((int)std::lround(step.values[6]),1,compiled->length-clip.patternStart);
   clip.repeat=std::clamp((int)std::lround(step.values[4]),1,16);
   clip.transpose=std::clamp((int)std::lround(step.values[3]),-96,96);
   clip.speed=std::clamp(step.values[2],0.25,16.);
   clip.startOffset=std::clamp((int)std::lround(step.values[22]),0,31);
   clip.legato=step.values[9]>=0.5;
   if(step.contains(10))clip.filterOwner=std::clamp((int)step.values[10],1,3);
   if(step.contains(11))clip.filterMask=std::clamp((int)step.values[11],0,7);
   for(int i=0;i<3;++i)if(step.contains(12+i))clip.lfoOwners[(size_t)i]=std::clamp((int)step.values[(size_t)(12+i)],1,3);
   result.clips[(size_t)lane].push_back(std::move(clip));
  }
 }
 return result;
}
struct TrackerSongMidiMap {
 std::array<std::shared_ptr<const TrackerPlayableSong>,128> atNote{};
 std::array<int,128> songIndex{};
 TrackerSongMidiMap(){songIndex.fill(-1);}
};
inline TrackerSongMidiMap compileTrackerSongMidiMap(const TrackerSongBank& songs,const TrackerPatternBank& patterns){
 TrackerSongMidiMap result;
 for(const auto& [slot,song]:songs.songs){
  if(song.midiNote>=0&&song.midiNote<128){
   const int note=song.midiNote;
   if(result.songIndex[(size_t)note]<0||slot==songs.activeSong){
    result.songIndex[(size_t)note]=slot;
    result.atNote[(size_t)note]=std::make_shared<const TrackerPlayableSong>(compileTrackerSong(song,slot,patterns));
   }
  }
 }
 return result;
}
struct TrackerSongLaneTick {
 const TrackerSongClip* clip=nullptr;
 int patternStep=-1, songRow=-1, transpose=0;
 double duration=0.;
 bool release=false, finished=false;
};
struct TrackerSongTick {
 std::array<TrackerSongLaneTick,3> lanes{};
 bool finished=false;
};
// All scheduling happens at sample boundaries, no allocations or mutable bank access.
class TrackerSongTransport {
public:
 struct Lane {
  int row=0,cycle=0,clipIndex=0,patternStep=0,patternRepeat=0,entryRepeat=0;
  double remaining=0.,gateRemaining=-1.,stepDuration=0.;
  bool playingClip=false,done=false;
  int currentSongRow=-1,currentPatternStep=-1,audibleClipIndex=-1;
 };
 std::array<Lane,3> lanes{};
 bool running=false;
 void stop()noexcept{running=false;for(auto& l:lanes)l=Lane{};}
 void start(int row,const TrackerPlayableSong& song)noexcept{
  running=true;
  for(int i=0;i<3;++i){
   auto& l=lanes[(size_t)i];l=Lane{};
   l.row=std::clamp(row,0,song.lengths[(size_t)i]-1);
   const auto& v=song.clips[(size_t)i];
   while(l.clipIndex<(int)v.size()&&v[(size_t)l.clipIndex].songRow<l.row)++l.clipIndex;
  }
 }
 static double gridSamples(double sr,double bpm,double speed)noexcept{
  return std::max(1.,sr/(std::clamp(bpm,1.,999.)/60.*4.*std::clamp(speed,0.25,4096.)));
 }
 TrackerSongTick tick(const TrackerPlayableSong& song,double sr,double bpm)noexcept{
  TrackerSongTick tick; if(!running)return tick;
  bool allFinished=true;
  for(int i=0;i<3;++i){
   auto& l=lanes[(size_t)i];auto& e=tick.lanes[(size_t)i];
   if(l.done)continue;
   allFinished=false;
   if(l.gateRemaining>0){l.gateRemaining-=1.;if(l.gateRemaining<=0){l.gateRemaining=-1;e.release=true;}}
   if(l.remaining>0){l.remaining-=1.;continue;}
   const auto& clips=song.clips[(size_t)i];
   const auto laneSpeed=[&](){const auto& speeds=song.laneSpeeds[(size_t)i];return l.cycle<(int)speeds.size()?std::clamp(speeds[(size_t)l.cycle],0.25,16.):1.;};
   if(l.playingClip){
    const auto& c=clips[(size_t)l.clipIndex];const auto& p=*c.pattern;
    const int step=c.patternStart+l.patternStep;
    e.clip=&c;e.patternStep=step;e.songRow=l.row;
    const double repeatSpeed=l.patternRepeat<(int)p.repeatSpeeds.size()?p.repeatSpeeds[(size_t)l.patternRepeat]:1.;
    e.transpose=c.transpose+(l.patternRepeat<(int)p.repeatTransposes.size()?p.repeatTransposes[(size_t)l.patternRepeat]:0);
    e.duration=gridSamples(sr,bpm,laneSpeed()*c.speed*repeatSpeed);
    l.remaining=e.duration-1.;l.stepDuration=e.duration;
    l.currentSongRow=l.row;l.currentPatternStep=step;l.audibleClipIndex=l.clipIndex;
    ++l.patternStep;
    if(l.patternStep>=c.playLength){
     l.patternStep=0;
     if(++l.patternRepeat>=p.repeat){l.patternRepeat=0;
      if(++l.entryRepeat>=c.repeat){l.entryRepeat=0;l.playingClip=false;++l.row;++l.clipIndex;}
     }
    }
    continue;
   }
   if(l.row>=song.lengths[(size_t)i]){
    ++l.cycle;
    if(l.cycle>=song.repeats[(size_t)i]){l.done=true;e.finished=true;continue;}
    l.row=0;l.clipIndex=0;
   }
   if(l.clipIndex<(int)clips.size() && clips[(size_t)l.clipIndex].songRow==l.row){
    const auto& c=clips[(size_t)l.clipIndex];
    l.playingClip=true;l.patternStep=l.patternRepeat=l.entryRepeat=0;
    // Song Start Offset: 32 subdivisions of one Song grid cell.
    if(c.startOffset>0){l.remaining=gridSamples(sr,bpm,laneSpeed())*(double)c.startOffset/32.;l.remaining=std::max(0.,l.remaining-1.);}
    else {
     const auto& p=*c.pattern;
     e.clip=&c;e.patternStep=c.patternStart;e.songRow=l.row;
     e.transpose=c.transpose+(!p.repeatTransposes.empty()?p.repeatTransposes[0]:0);
     const double ps=!p.repeatSpeeds.empty()?p.repeatSpeeds[0]:1.;
     e.duration=gridSamples(sr,bpm,laneSpeed()*c.speed*ps);
     l.remaining=e.duration-1.;l.stepDuration=e.duration;
     l.currentSongRow=l.row;l.currentPatternStep=e.patternStep;l.audibleClipIndex=l.clipIndex;
     ++l.patternStep;
     if(l.patternStep>=c.playLength){l.patternStep=0;
      if(++l.patternRepeat>=p.repeat){l.patternRepeat=0;
       if(++l.entryRepeat>=c.repeat){l.entryRepeat=0;l.playingClip=false;++l.row;++l.clipIndex;}
      }
     }
    }
   }else{
    // Blank Song rows occupy exactly one grid cell, not a synthetic Note.
    l.currentSongRow=l.row;l.currentPatternStep=-1;l.audibleClipIndex=-1;
    l.remaining=gridSamples(sr,bpm,laneSpeed())-1.;++l.row;
    // Preserve an already sounding note until its gate or the next entry.
   }
  }
  if(allFinished){running=false;tick.finished=true;}
  return tick;
 }
};
}
