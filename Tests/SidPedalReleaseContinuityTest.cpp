#include "../Source/SidSeqMemory.h"
#include "../Source/SidAmpEnvelope.h"
#include "../Source/SidQuantizedArp.h"
#include "../Source/SidArpSustain.h"
#include "../Source/SidArpTransport.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <array>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>

int main(){
 using namespace biscot;
 constexpr double sr=1000., bpm=120., rate=16.;
 // A sounding three-note chord: SID Seq's first/latch root can differ from
 // its current chord root.  At the last physical Note Off under CC64,
 // preserve the LIVE root, not the stale per-voice latch root.
 SidSeqMemory seq;
 seq.clearAll();
 seq.clearSequence(60);seq.record(64);seq.record(67);
 SidHeldStack held;held.add(60);held.add(64);held.add(67);
 seq.snapshot(held);
 assert(seq.latchRoot==60);
 seq.releaseRoot=60;
 const int lastLiveChordRoot=67;
 seq.process(1,lastLiveChordRoot,rate,bpm,sr);
 assert(seq.lastTick);
 const float audiblePitchBefore=(float)lastLiveChordRoot+seq.target;
 const auto initialStep=seq.step;
 const auto initialClock=seq.clock;
 seq.latchCurrentChordRootForRelease(lastLiveChordRoot);
 assert(seq.releaseRoot==lastLiveChordRoot);
 assert(seq.step==initialStep && seq.clock==initialClock);
 // CC64-up itself must not modify the root/clock/target; only the VCA gate.
 const float audiblePitchAtPedalRelease=(float)seq.releaseRoot+seq.target;
 assert(std::fabs(audiblePitchAtPedalRelease-audiblePitchBefore)<1.e-6f);
 seq.process(1,seq.releaseRoot,rate,bpm,sr);
 assert(std::fabs(float(seq.releaseRoot)+seq.target-audiblePitchBefore)<1.e-6f);

 // In classical arp, the live pool becomes the release pool unchanged.
 // Switching to the release pool cannot interrupt or retrigger the clock.
 std::array<int,SidQuantizedArp::capacity> live{},released{};
 live[0]=60;live[1]=64;live[2]=67;
 const int count=3;
 std::copy_n(live.begin(),count,released.begin());
 SidQuantizedArp manual, pedal;
 uint32_t randomManual=145u,randomPedal=145u;
 const auto m0=manual.process(live,count,2,rate,bpm,sr,randomManual);
 const auto p0=pedal.process(live,count,2,rate,bpm,sr,randomPedal);
 assert(m0.note==p0.note && m0.note==60);
 for(int i=0;i<200;++i){
  // Manual release and pedal release now use the same stored note pool.
  const auto a=manual.process(released,count,2,rate,bpm,sr,randomManual);
  const auto b=pedal.process(released,count,2,rate,bpm,sr,randomPedal);
  assert(a.note==b.note && a.edge==b.edge);
  assert(manual.step==pedal.step && manual.phase==pedal.phase);
 }
 // No sharp VCA discontinuity: identical envelope stage transitions for a
 // manually released note and a pedal-latched note at CC64-up.
 SidAmpEnvelope hand,foot;
 hand.noteOn(true);foot.noteOn(true);
 for(int i=0;i<500;++i){hand.tick(.01f,.01f,.7f,sr);foot.tick(.01f,.01f,.7f,sr);}
 hand.noteOff(.8f,sr);foot.noteOff(.8f,sr);
 for(int i=0;i<1000;++i){
  const float a=hand.tick(.01f,.01f,.7f,sr);
  const float b=foot.tick(.01f,.01f,.7f,sr);
  assert(std::fabs(a-b)<1.e-7f);
 }
 std::cout<<"CC64 release matches manual arp memory, pitch root, clock and amplitude tail OK\n";
}
