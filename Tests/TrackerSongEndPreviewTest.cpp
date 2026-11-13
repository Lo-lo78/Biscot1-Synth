// TEST0.2.26: actual sample-clock Song 0 -> Song 1 chain and one-lane preview.
#include "../Source/TrackerSongPlayback.h"
#include <cstdlib>
#include <cstdio>
#include <vector>
using namespace biscot;
static void check(bool b,const char* why){if(!b){std::fprintf(stderr,"FAILED Song preview/chain: %s\n",why);std::abort();}}
int main(){
 TrackerPatternBank patterns;
 for(int i=0;i<2;++i){
  auto& p=patterns.at(i);p.length=8;p.repeat=i==0?2:1;
  for(int step=0;step<8;++step)p.insert(step,0,0,{1,(double)(60+i*12+step)});
 }
 TrackerSongBank bank;
 auto& intro=bank.at(0);auto& verse=bank.at(1);
 intro.laneLengths={1,1,1};verse.laneLengths={1,1,1};
 intro.songEnd=1;verse.songEnd=-1;
 auto& first=intro.atStep(0,0);check(first.insert(1),"first Pattern insert");first.values[1]=0;first.values[6]=8;
 auto& second=verse.atStep(1,0);check(second.insert(1),"second Pattern insert");second.values[1]=1;second.values[6]=8;
 auto chains=compileTrackerSongChainMap(bank,patterns);
 check(chains.byNumber[0]&&chains.byNumber[1],"both songs compiled");
 check(chains.byNumber[0]->songEnd==1,"Song End saved for first song");
 auto song=chains.byNumber[0];
 TrackerSongTransport tr;tr.start(0,*song);
 int transitions=0,secondNotes=0;
 for(int sample=0;sample<1500;++sample){
  const auto event=tr.tick(*song,60.,60.);
  if(transitions==1&&event.lanes[1].clip)++secondNotes;
  if(event.finished){
   if(song->songEnd>=0){
    ++transitions;
    song=chains.byNumber[(size_t)song->songEnd];
    check(song&&song->songNumber==1,"chain target is Song 1");
    tr.start(0,*song);
   }else break;
  }
 }
 check(transitions==1,"Song 0 must continue automatically as Song 1");
 check(secondNotes==8,"Song 1 operator 2 plays its eight Pattern notes");
 const auto preview=compileTrackerSongRowPreview(*chains.byNumber[0],0,0,3);
 check(preview!=nullptr,"audition snapshot");
 check(preview->songEnd==-1,"audition never follows Song End");
 check(preview->mixer[0]==1.f&&preview->mixer[1]==0.f&&preview->mixer[2]==0.f,"solo operator 1");
 check(preview->clips[0].size()==1&&preview->clips[1].empty()&&preview->clips[2].empty(),"one lane");
 check(preview->clips[0][0].playLength==3,"three-step default");
 check(preview->clips[0][0].pattern->repeat==1,"preview ignores whole Pattern repeats");
 check(!compileTrackerSongRowPreview(*chains.byNumber[0],0,1,3),"no preview of empty Song rows");
 check(compileTrackerSongRowPreview(*chains.byNumber[0],0,0,500)->clips[0][0].playLength==8,
       "preview cannot exceed real Pattern window");
 auto& step=bank.at(0).atStep(0,0);
 check(step.insert(23),"Preview Steps must be selectable");
 check(step.values[23]==3.,"default preview length from JSFX is three");
 step.values[23]=32;
 check(step.contains(23)&&step.values[23]==32,"preview steps persisted in Song step");
 return 0;
}
