// TEST0.2.24: JUCE-free sample-clock Song engine, all three operators.
#include "../Source/TrackerSongPlayback.h"
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <vector>
using namespace biscot;
static void check(bool p){static int n=0;++n;if(!p){std::fprintf(stderr,"FAILED CHECK #%d\n",n);std::abort();}}
int main(){
 TrackerPatternBank patterns;
 auto& a=patterns.at(0);a.length=4;a.repeat=1;
 auto& b=patterns.at(1);b.length=3;b.repeat=2;
 auto& c=patterns.at(2);c.length=2;c.repeat=1;
 TrackerSong song;
 song.laneLengths={3,3,3};song.laneRepeats={1,1,1};
 auto& s0=song.atStep(0,0);check(s0.insert(1));s0.values[1]=0;s0.values[5]=1;s0.values[6]=2;
 auto& s1=song.atStep(1,0);check(s1.insert(1));s1.values[1]=1;s1.values[4]=2;s1.values[6]=3;
 auto& s2=song.atStep(2,1);check(s2.insert(1));s2.values[1]=2;s2.values[3]=5;s2.values[6]=2;
 song.mixerVolumes={100,25,75};song.mixerMuteMask=4;
 auto compiled=compileTrackerSong(song,0,patterns);
 check(compiled.clips[0].size()==1&&compiled.clips[1].size()==1&&compiled.clips[2].size()==1);
 check(compiled.clips[0][0].patternStart==1&&compiled.clips[0][0].playLength==2);
 check(compiled.clips[2][0].transpose==5);
 check(compiled.mixer[0]==1.f&&compiled.mixer[1]==.25f&&compiled.mixer[2]==0.f);
 TrackerSongTransport tr;tr.start(0,compiled);
 std::vector<int> op1,op2,op3;std::vector<int> op1time,op2time,op3time;
 // At 60Hz / 60BPM a 16th-note is 15 samples. No events between boundaries.
 for(int t=0;t<500;++t){
  const auto ev=tr.tick(compiled,60.,60.);
  for(int lane=0;lane<3;++lane){
   const auto& e=ev.lanes[(size_t)lane];
   if(e.clip){
    auto& steps=lane==0?op1:lane==1?op2:op3;
    auto& times=lane==0?op1time:lane==1?op2time:op3time;
    steps.push_back(e.patternStep);times.push_back(t);
    check(e.songRow>=0&&e.songRow<3);
   }
  }
  if(ev.finished)break;
 }
 check((op1==std::vector<int>{1,2}));
 check((op2==std::vector<int>{0,1,2,0,1,2,0,1,2,0,1,2}));
 check((op3==std::vector<int>{0,1}));
 check(op1time[0]==0&&op1time[1]==15);
 check(op2time[0]==0&&op2time[1]==15);
 check(op3time[0]==15); // one blank Song row
 // Song row-level Speed affects only its own lane and is multiplicative.
 song.atStep(0,0).values[2]=2.0;
 compiled=compileTrackerSong(song,0,patterns);tr.start(0,compiled);
 std::vector<int> fast;
 for(int t=0;t<20;++t)if(tr.tick(compiled,60,60).lanes[0].clip)fast.push_back(t);
 check((fast==std::vector<int>{0,8}));
 // Explicit Start Offset consumes 8/32 of a Song grid, not Pattern steps.
 song.atStep(2,1).values[22]=8;
 compiled=compileTrackerSong(song,0,patterns);tr.start(0,compiled);
 int first=-1;
 for(int t=0;t<40;++t)if(tr.tick(compiled,60,60).lanes[2].clip){first=t;break;}
 check(first==19); // 15 + ceil(15*8/32)
 // Song MIDI assignment wins by active Song and snapshots the right Pattern.
 TrackerSongBank bank;
 bank.at(0)=song;bank.at(0).midiNote=69;
 bank.at(1).midiNote=69;bank.at(1).laneLengths={2,2,2};
 bank.activeSong=1;
 const auto midi=compileTrackerSongMidiMap(bank,patterns);
 check(midi.songIndex[69]==1&&midi.atNote[69]!=nullptr);
 check(midi.atNote[68]==nullptr);
 // Invalid reference ignored: silent lane, no phantom notes.
 song.atStep(0,0).values[1]=999;
 compiled=compileTrackerSong(song,0,patterns);
 check(compiled.clips[0].empty());
 return 0;
}
