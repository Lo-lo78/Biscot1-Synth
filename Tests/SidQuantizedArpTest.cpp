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
 // Key off with no sustain: do not advance arp during VCA Release.
 assert(sidArpClockActive(true,env.stage));
 env.noteOff(0.03f,48000);
 assert(env.stage==4);
 assert(!sidArpClockActive(false,env.stage));
 const int lastNote=arp.selected;
 const int lastStep=arp.step;
 for(int i=0;i<200000 && env.stage!=0;++i)env.tick(0.f,0.f,1.f,48000);
 assert(arp.selected==lastNote && arp.step==lastStep);
 assert(env.stage==0 && env.level==0.f);
 // CC64 held: gate=true, arpeggio remains clocked until pedal-up.
 SidAmpEnvelope pedalEnv;pedalEnv.noteOn(true);pedalEnv.tick(0,0,1,48000);
 assert(sidArpClockActive(true,pedalEnv.stage));
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
 std::cout<<"ARP Trigger/Legato clock, natural release and SID Seq transport OK\n";
}
