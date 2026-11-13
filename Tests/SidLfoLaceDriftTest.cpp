#include "../Source/SidLfoCore.h"
#include "../Source/SidLfoUsage.h"
#include "../Source/SidLfoLace.h"
#include "../Source/SidIdleSleep.h"
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>
using namespace biscot;
void check(bool c,const char* msg){if(!c){std::fprintf(stderr,"FAIL: %s\n",msg);std::exit(1);}}
int main(){
 // Each successive complete cycle has its own phase shift; the fifth repeats.
 SidLfoCore a;a.reset(0);a.phase=.24;a.delayAge=10.;
 std::array<float,6> values{};
 for(int j=0;j<6;++j){
  a.phase=.24;a.cycles=(unsigned long long)j;
  values[(size_t)j]=a.tick(0.,0,0.,0.,false,1.,0.,0.,48000.,.5,.9,5);
 }
 check(std::abs(values[0]-values[1])>.2f,"drift changes per cycle");
 check(std::abs(values[0]-values[5])<1e-6f,"drift repeats after N cycles");
 // Changing cycle count changes phrase, not the independent underlying rate.
 SidLfoCore b;b.reset(0);b.phase=.0;b.cycles=3;b.delayAge=10.;
 const float five=b.tick(0.,4,0.,0.,false,1.,0.,0.,48000.,.5,1.,5);
 b.phase=.0;b.cycles=3;
 const float seven=b.tick(0.,4,0.,0.,false,1.,0.,0.,48000.,.5,1.,7);
 check(five!=seven,"different cycle counts give different square pattern");
 for(int wave=0;wave<=6;++wave){
  SidLfoCore c;c.reset(0);c.phase=.22;c.cycles=4;c.delayAge=10.;
  const float v=c.tick(0.,wave,0.,0.,false,1.,0.,0.,48000.,.5,.8,7);
  check(std::isfinite(v),"all waveforms finite with drift");
  c.cycles=11;c.smoothed=0.;
  const float v2=c.tick(0.,wave,0.,0.,false,1.,0.,0.,48000.,.5,.8,7);
  check(std::abs(v-v2)<1e-5f,"all waveforms repeat with drift");
 }
 // No drift means the original behavior is exactly unchanged.
 for(int wave=0;wave<=6;++wave){
  SidLfoCore x,y;x.reset(1);y.reset(1);x.phase=y.phase=.43;x.delayAge=y.delayAge=4.;
  check(x.tick(0.,wave,0.,0.,false,1.,0.,0.,48000.,.37)==
        y.tick(0.,wave,0.,0.,false,1.,0.,0.,48000.,.37,0.,5),"zero drift parity");
 }
 // Four slots at Lace Rate 2: own, source, own, source.
 check(sidLfoLace(.5f,-.5f,.10,2,1.f)==.5f,"Lace keeps own even slot");
 check(sidLfoLace(.5f,-.5f,.30,2,1.f)==-.5f,"Lace selects source on odd slot");
 check(sidLfoLace(.5f,-.5f,.30,2,.5f)==0.f,"Lace half mix is midpoint");
 check(sidLfoLace(.5f,-.5f,.30,2,0.f)==.5f,"Lace off is transparent");
 std::array<float,249> params{};
 auto value=[&](int i){return params[(size_t)i];};
 params[132]=.5f; // LFO1 routed
 params[236]=2.f;params[238]=1.f; // LFO1 Lace Source=LFO2, Mix=100%
 auto used=determineUsedLfos(value);
 check(used[0]&&used[1]&&!used[2],"Lace source wakes independent LFO");
 // In deep idle a free-running oscillator must keep both phase and cycle count.
 SidLfoCore idle;idle.reset(0);idle.phase=.25;
 SidIdleSleep::advanceFreeClock(idle,96000,120.,2.,48000.,true,false);
 check(std::abs(idle.phase-.25)<1e-9 && idle.cycles==2ULL,"idle clock preserves cycles");
 idle.noteTrigger(true,false,.0);
 check(idle.cycles==0ULL,"trigger resets drift phrase");
 std::puts("OK: LFO Phase Drift repeats, Lace source usage, legacy parity and idle phase");
}
