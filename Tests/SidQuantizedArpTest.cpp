#include "../Source/SidQuantizedArp.h"
#include "../Source/SidSeqMemory.h"
#include "../Source/SidAmpEnvelope.h"
#include "../Source/SidArpTransport.h"
#include <array>
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <cstdint>
#include <iostream>

int main(){
 using namespace biscot;
 std::array<int,640> pool{};
 std::uint32_t seed=12345;
 SidQuantizedArp arp;
 pool[0]=60;
 auto start=arp.process(pool,1,2,16.,120.,48000.,seed);
 assert(start.edge && start.first && start.note==60);
 pool[1]=64;
 // MIDI notes are queued and heard only on the arp grid.
 for(int i=0;i<5999;++i){
  auto e=arp.process(pool,2,2,16.,120.,48000.,seed);
  assert(!e.edge && e.note==60);
 }
 auto beat=arp.process(pool,2,2,16.,120.,48000.,seed);
 assert(beat.edge && !beat.first && beat.note==64);
 // On Trigger, a new key requests restart of the PATTERN, not the ADSR.
 // Do not restart the BPM phase. A MIDI note pressed midstep is deferred.
 assert(sidArpRestartRequested(true,true,true));
 assert(!sidArpRestartRequested(false,true,true));
 assert(!sidArpRestartRequested(true,false,true));
 assert(!sidArpRestartRequested(true,true,false));
 SidAmpEnvelope env;env.noteOn(true);
 for(int i=0;i<100;++i)env.tick(0.f,0.f,1.f,48000);
 assert(env.stage==3 && env.level==1.f);
 for(int i=0;i<3000;++i)arp.process(pool,2,2,16.,120.,48000.,seed);
 arp.requestRestart();
 pool[2]=67;
 for(int i=0;i<2999;++i){
  auto e=arp.process(pool,3,2,16.,120.,48000.,seed);
  assert(!e.edge && e.note==64);
 }
 beat=arp.process(pool,3,2,16.,120.,48000.,seed);
 assert(beat.edge && beat.note==60 && arp.step==0);
 assert(env.stage==3 && env.level==1.f); // step did NOT reattack envelope
 // Legato uses the same time source and advances to the NEXT pattern step.
 for(int i=0;i<5999;++i)arp.process(pool,3,2,16.,120.,48000.,seed);
 beat=arp.process(pool,3,2,16.,120.,48000.,seed);
 assert(beat.edge && beat.note==64 && arp.step==1);
 // Final Note Off must NOT freeze the clock: JSFX continues frequency
 // arpeggiation through the audible VCA Release, with the last note pool.
 assert(sidArpClockActive(true,env.stage));
 env.noteOff(0.2f,48000);
 assert(env.stage==4);
 assert(sidArpClockActive(false,env.stage));
 const int previousNote=arp.selected;
 bool advancedDuringRelease=false;
 for(int i=0;i<20000 && env.stage!=0;++i){
  const auto e=arp.process(pool,3,2,16.,120.,48000.,seed);
  if(e.edge && e.note!=previousNote)advancedDuringRelease=true;
  env.tick(0.f,0.f,1.f,48000);
 }
 assert(advancedDuringRelease);
 // Sustain pedal holds the envelope AND the clock, then the same running
 // clock continues after CC64-up while the amplitude envelope releases.
 SidAmpEnvelope pedalEnv;pedalEnv.noteOn(true);pedalEnv.tick(0,0,1,48000);
 assert(sidArpClockActive(true,pedalEnv.stage));
 pedalEnv.noteOff(0.01f,48000);
 assert(sidArpClockActive(false,pedalEnv.stage));
 for(int i=0;i<10000 && pedalEnv.stage!=0;++i)pedalEnv.tick(0,0,1,48000);
 assert(pedalEnv.stage==0 && !sidArpClockActive(false,pedalEnv.stage));
 // SID Seq receives the same quantized restart without erasing recorded notes.
 SidSeqMemory sid;sid.clearAll();sid.clearSequence(60);
 assert(sid.process(1,60,16.,120.,48000.)==0.f);
 sid.record(64);sid.record(67);
 const auto count=sid.length;
 for(int i=0;i<3000;++i)sid.process(1,60,16.,120.,48000.);
 sid.requestRestart();
 for(int i=0;i<2999;++i){sid.process(1,60,16.,120.,48000.);assert(!sid.lastTick);}
 sid.process(1,60,16.,120.,48000.);
 assert(sid.lastTick && sid.step==0 && sid.length==count);
 std::cout<<"ARP Trigger/Legato clock, running release and SID Seq transport OK\n";
}
