// TEST0.2.25: native Song End graph, Pattern-first Filter/LFO ownership,
// parameter-only rows, and per-operator Sidechain / Delay engine.
#include "../Source/TrackerSongPlayback.h"
#include "../Source/TrackerSongFxCore.h"
#include <cstdio>
#include <cstdlib>
#include <cmath>
using namespace biscot;
static int counter=0;
static void check(bool ok){if(!ok){std::fprintf(stderr,"Song advanced failure %d\n",++counter);std::abort();}++counter;}
int main(){
 TrackerPatternBank patterns;
 for(int id=0;id<3;++id){
  auto& p=patterns.at(id);p.length=2;p.repeat=1;
  // A genuine Filter or LFO use must appear in the compiled Pattern automation.
  p.insert(0,1,0,{1108,1200}); // Filter Cutoff
  if(id==0)p.insert(0,2,0,{1133,1}); // LFO1 Op1 Pitch Depth
 }
 TrackerSongBank bank;
 auto& a=bank.at(0);a.name="Intro";a.songEnd=1;a.laneLengths={2,2,2};
 auto& b=bank.at(1);b.name="Verse";b.songEnd=-1;b.laneLengths={2,2,2};
 a.atStep(0,0).insert(1);a.atStep(0,0).values[1]=0;
 a.atStep(0,0).values[5]=1; // starting after the original Pattern state
 b.atStep(1,0).insert(1);b.atStep(1,0).values[1]=1;
 auto& empty=a.atStep(2,0);
 check(empty.insert(15));empty.values[15]=1;
 check(empty.insert(16));empty.values[16]=80;
 check(empty.insert(17));empty.values[17]=2.;
 check(empty.insert(19));empty.values[19]=40;
 check(!empty.contains(1));
 auto intro=compileTrackerSong(a,0,patterns);
 check(intro.clips[2].empty());
 check((intro.clips[0][0].resourceFlags&3)==3);
 check(intro.clips[0][0].patternStart==1);
 bool primedFilter=false,primedLfo=false;
 for(const auto& [idx,value]:intro.clips[0][0].initialParams){
  if(idx==108)primedFilter=true;
  if(idx==133)primedLfo=true;
 }
 check(primedFilter&&primedLfo);
 check(intro.fxRows[2].size()==1);
 check(intro.fxRows[2].at(0).value[15]==1);
 TrackerSongTransport tr;tr.start(0,intro);
 auto at0=tr.tick(intro,48000.,120.);
 check(at0.lanes[0].clipStarted);
 check(at0.lanes[2].rowFx!=nullptr);
 check(at0.lanes[2].rowFx->present[19]);
 auto chains=compileTrackerSongChainMap(bank,patterns);
 check(chains.byNumber[0]&&chains.byNumber[1]);
 check(chains.byNumber[0]->songEnd==1);
 check(chains.byNumber[1]->songEnd==-1);
 check(trackerSongResolveOwner(0,3)==3);
 check(trackerSongResolveOwner(1,3)==1);
 check(trackerSongResolveOwner(2,1)==2);
 check(trackerSongResolveOwner(4,1)==3);
 check(trackerSongResolveOwner(3,2)==2);
 check(trackerSongResolveOwner(5,2)==1);
 check(trackerSongResolveOwner(6,1)==2);
 TrackerSongFxCore fx;fx.prepare(100.);
 TrackerSongRowFx set;set.present[15]=true;set.value[15]=2;
 set.present[16]=true;set.value[16]=100;
 set.present[17]=true;set.value[17]=1;
 set.present[18]=true;set.value[18]=0;
 set.present[19]=true;set.value[19]=100;
 fx.setRow(set);
 check(fx.sidechainSource==2 && fx.sidechainAmount==1.f);
 check(fx.delaySpeed==1.f);
 float l=0,r=0;float delayed=0;
 for(int t=0;t<110;++t){
  float in=(t==0?1.f:0.f);
  fx.process(in,0.f,l,r,60.);
  if(t==100)delayed=l;
 }
 check(std::abs(delayed-1.f)<.00001f);
 check(fx.hasTail());
 TrackerSongRowFx off;off.present[15]=true;off.value[15]=0;
 off.present[17]=true;off.value[17]=0;
 fx.setRow(off);
 check(fx.sidechainSource==0 && fx.sidechainAmount==0.f);
 check(fx.delaySpeed==0.f && !fx.hasTail());
 fx.clearDelay();
 return 0;
}
