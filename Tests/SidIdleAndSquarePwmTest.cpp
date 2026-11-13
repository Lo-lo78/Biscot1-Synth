#include "../Source/SidIdleSleep.h"
#include "../Source/SidLfoUsage.h"
#include <cassert>
#include <cmath>
#include <iostream>
using namespace biscot;
static bool closeTo(double x,double y,double delta=1e-9){return std::abs(x-y)<=delta;}
int main(){
 // The new parameter changes only LFO Square duty, never oscillator pulse width.
 SidLfoCore l;
 l.reset(0);l.phase=0.40;l.delayAge=1.;l.age=0;
 assert(l.tick(0.,4,0.,0.,false,1.,0.,0.,48000.,.50)>0.9f);
 l.reset(0);l.phase=.40;l.delayAge=1.;l.age=0;
 assert(l.tick(0.,4,0.,0.,false,1.,0.,0.,48000.,.30)<-0.9f);
 l.reset(0);l.phase=.40;l.delayAge=1.;l.age=0;
 assert(l.tick(0.,4,0.,0.,false,1.,0.,0.,48000.,.70)>0.9f);
 l.reset(0);l.phase=.96;l.delayAge=1.;l.age=0;
 assert(l.tick(0.,4,0.,0.,false,1.,0.,0.,48000.,.99)<-0.9f); // max 0.95
 // Non-square LFO shapes are unchanged regardless of squarePWM setting.
 SidLfoCore a,b;a.reset(0);b.reset(0);a.phase=b.phase=.25;
 assert(closeTo(a.tick(0.,0,0.,0.,false,1.,0.,0.,48000.,.05),
                b.tick(0.,0,0.,0.,false,1.,0.,0.,48000.,.95)));
 // Dormant LFO strips remain suspended; ring/sync and cross-rate routes wake
 // their source even when all conventional LFO destination depths are zero.
 std::array<float,249> params{};
 auto get=[&](int n){return params[(size_t)n];};
 auto used=determineUsedLfos(get);
 assert(!used[0]&&!used[1]&&!used[2]);
 params[132]=.4f; used=determineUsedLfos(get);
 assert(used[0]&&!used[1]&&!used[2]);
 params[132]=0;params[192]=3;used=determineUsedLfos(get);
 assert(!used[0]&&!used[1]&&used[2]);
 params[192]=0;params[220]=2;params[221]=.2f;
 used=determineUsedLfos(get);
 assert(used[0]&&used[1]&&!used[2]);
 // An idle block can skip its complete sample span; the FREE clock advances.
 SidIdleSleep idle;
 assert(idle.asleep);
 l.reset(0);l.phase=.125;
 SidIdleSleep::advanceFreeClock(l,48000,120.,2.,48000.,true,false);
 assert(closeTo(l.phase,.125)); // 1 Hz -> 1 full cycle
 SidIdleSleep::advanceFreeClock(l,12000,120.,2.,48000.,true,false);
 assert(closeTo(l.phase,.375));
 SidIdleSleep::advanceFreeClock(l,12000,120.,2.,48000.,false,false);
 assert(closeTo(l.phase,.375)); // triggered mode freezes
 SidIdleSleep::advanceFreeClock(l,12000,120.,2.,48000.,true,true);
 assert(closeTo(l.phase,.375)); // One Shot freezes
 // Gate, envelope and audible tail ALL prevent cold sleep.
 idle.wake();
 for(int n=0;n<1000;++n)assert(!idle.observe(true,true,.01f));
 for(int n=0;n<1000;++n)assert(!idle.observe(false,true,0.f));
 for(int n=0;n<1000;++n)assert(!idle.observe(true,false,0.f));
 for(int n=0;n<SidIdleSleep::requiredQuietSamples-1;++n)
  assert(!idle.observe(true,true,0.f));
 assert(idle.observe(true,true,0.f));assert(idle.asleep);
 idle.wake();assert(!idle.asleep);assert(!idle.observe(true,true,0.f));
 std::cout << "OK: JSFX cold idle, free LFO phase, LFO Square PWM\n";
}
