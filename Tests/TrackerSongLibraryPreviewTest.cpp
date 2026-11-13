// TEST0.2.35: the file browser must audition the ACTUAL Song timeline, not
// realign one clip from each operator into a fabricated arrangement.
#include "../Source/TrackerSongPlayback.h"
#include <cstdlib>
#include <array>
#include <vector>
#include <tuple>
using namespace biscot;
static void check(bool okay){if(!okay)std::abort();}
struct Entry {int time, lane, step, row, transpose;};
static std::vector<Entry> timeline(const TrackerPlayableSong& song){
 TrackerSongTransport transport;transport.start(0,song);
 std::vector<Entry> events;
 for(int sample=0;sample<250000;++sample){
  const auto tick=transport.tick(song,60.,60.);
  for(int lane=0;lane<3;++lane){
   const auto& e=tick.lanes[(size_t)lane];
   if(e.clip)events.push_back({sample,lane,e.patternStep,e.songRow,e.transpose});
  }
  if(tick.finished)break;
 }
 return events;
}
int main(){
 TrackerPatternBank patterns;
 for(int n=0;n<3;++n){
  auto& p=patterns.at(n);p.length=5+n;p.repeat=2;
  p.repeatSpeeds={1.,2.};p.repeatTransposes={0,12};
  for(int step=0;step<p.length;++step)
   check(p.insert(step,0,0,{1,(double)(60+n*12+step)}));
 }
 TrackerSong song;
 song.laneLengths={6,8,4};song.laneRepeats={2,3,1};
 song.laneSpeeds[0]={1.,1.5};song.laneSpeeds[1]={2.,1.,3.};song.laneSpeeds[2]={.5};
 song.songEnd=13;song.loopTarget=20;song.chainLoop=true;
 song.mixerVolumes={100,50,75};song.mixerMuteMask=2;
 for(int lane=0;lane<3;++lane){
  const int position=lane+1; // leading empty Song rows must remain audible silence
  auto& row=song.atStep(lane,position);
  check(row.insert(1));row.values[1]=lane;
  row.values[5]=lane;row.values[6]=3;
  row.values[4]=2;row.values[22]=lane*4;
  row.values[2]=lane==1?2.:1.;row.values[3]=lane*7;
 }
 auto& fx=song.atStep(2,0);
 check(fx.insert(18));fx.values[18]=71;
 auto compiled=compileTrackerSong(song,7,patterns);
 auto browser=compileTrackerSongLibraryPreview(compiled,3);
 check(browser && browser->songNumber==7);
 check(browser->songEnd==13 && browser->loopTarget==20 && browser->chainLoop);
 check((browser->lengths==std::array<int,3>{6,8,4}));
 check((browser->repeats==std::array<int,3>{2,3,1}));
 check((browser->mixer==std::array<float,3>{1.f,0.f,.75f}));
 check(browser->fxRows[2].count(0));
 check(browser->fxRows[2].at(0).present[18]);
 check(browser->fxRows[2].at(0).value[18]==71);
 for(int lane=0;lane<3;++lane){
  check(browser->clips[(size_t)lane].size()==1);
  const auto& clip=browser->clips[(size_t)lane][0];
  check(clip.songRow==lane+1 && clip.playLength==3 && clip.repeat==2);
  check(clip.patternStart==lane && clip.transpose==lane*7);
  check(clip.startOffset==lane*4);
  check(clip.pattern && clip.pattern->repeat==2);
 }
 // Time and event identity: even a 3-division audition plays exactly the
 // original events until its independent wall-clock stop limit.
 const auto actual=timeline(compiled), audition=timeline(*browser);
 check(actual.size()==audition.size() && !actual.empty());
 for(size_t i=0;i<actual.size();++i){
  check(actual[i].time==audition[i].time);
  check(actual[i].lane==audition[i].lane);
  check(actual[i].row==audition[i].row);
  check(actual[i].step==audition[i].step);
  check(actual[i].transpose==audition[i].transpose);
 }
 check(actual.front().time>0); // retains initial blank Song row
 // F2 cursor navigation preview remains short and one-operator only.
 auto cursor=compileTrackerSongRowPreview(compiled,0,1,3);
 check(cursor && cursor->clips[0].size()==1);
 check(cursor->clips[1].empty() && cursor->clips[2].empty());
 // Real Song End edges are preserved inside the separate packaged bank.
 TrackerSongBank songs;
 songs.songs.clear();songs.songs[7]=song;
 songs.songs[13]=song;songs.songs[13].songEnd=-1;
 const auto chain=compileTrackerSongChainMap(songs,patterns);
 check(chain.byNumber[7] && chain.byNumber[13] && !chain.byNumber[12]);
 check(chain.byNumber[7]->songEnd==13 && chain.byNumber[13]->songEnd==-1);
 return 0;
}
