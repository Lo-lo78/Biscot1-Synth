#include "../Source/SidQuantizedArp.h"
#include "../Source/SidSeqMemory.h"
#include "../Source/SidAmpEnvelope.h"
#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>

int main(){
 using namespace biscot;
 std::array<int,640> pool{};
 std::uint32_t randomSeed=12345;
 SidQuantizedArp arp;
 pool[0]=60;
 auto start=arp.process(pool,1,2,16.,120.,48000.,randomSeed);
 assert(start.edge && start.first && start.note==60);
 // At 120 BPM with Rate 16, the period is 6000 audio samples.
 // MIDI Note On in the middle of the beat must not change the pitch.
 pool[1]=64;
 for(int i=0;i<5999;++i){
  const auto e=arp.process(pool,2,2,16.,120.,48000.,randomSeed);
  assert(!e.edge && e.note==60);
 }
 auto beat=arp.process(pool,2,2,16.,120.,48000.,randomSeed);
 assert(beat.edge && !beat.first && beat.note==64);
 assert(sidArpStepRetriggers(beat.edge,beat.first,false,true,3));
 assert(!sidArpStepRetriggers(beat.edge,beat.first,true,true,3));
 assert(!sidArpStepRetriggers(beat.edge,beat.first,false,false,4));
 assert(!sidArpStepRetriggers(start.edge,start.first,false,true,3));
 // Releasing a key or adding one while pedal remains down changes the
 // candidate pool but never interrupts the current output note.
 pool[0]=60;pool[1]=67;pool[2]=72;
 for(int i=0;i<5999;++i){
  const auto e=arp.process(pool,3,2,16.,120.,48000.,randomSeed);
  assert(e.note==64 && !e.edge);
 }
 beat=arp.process(pool,3,2,16.,120.,48000.,randomSeed);
 assert(beat.edge && beat.note==72);
 // Empty live pool (CC64 up) keeps the last sequence during VCA release.
 for(int i=0;i<5999;++i){
  const auto e=arp.process(pool,0,2,16.,120.,48000.,randomSeed);
  assert(!e.edge && e.note==72);
 }
 beat=arp.process(pool,0,2,16.,120.,48000.,randomSeed);
 assert(beat.edge && beat.note==60);
 // SID Seq remains clocked; its lastTick signal marks real steps only.
 SidSeqMemory sid;
 sid.clearAll();sid.clearSequence(60);
 auto target=sid.process(1,60,16.,120.,48000.);
 assert(sid.lastTick && target==0);
 sid.record(64);
 for(int i=0;i<5999;++i){
  target=sid.process(1,60,16.,120.,48000.);
  assert(!sid.lastTick && target==0);
 }
 target=sid.process(1,60,16.,120.,48000.);
 assert(sid.lastTick && target==4);
 // Trigger reattacks with an actual reset; Legato keeps its envelope.
 SidAmpEnvelope env;
 env.noteOn(true);
 for(int i=0;i<50;++i)env.tick(0.f,0.f,1.f,48000.);
 assert(env.level>0.99f);
 if(sidArpStepRetriggers(sid.lastTick,false,false,true,env.stage))env.noteOn(true);
 assert(env.level==0.f && env.stage==1);
 std::cout<<"Clock-quantized classical ARP, SID Seq step trigger, sustain and legato: OK\n";
}
