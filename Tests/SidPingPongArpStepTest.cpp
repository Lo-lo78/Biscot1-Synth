// TEST0.2.50: regression for even numbers of held MIDI notes.
// The synth's MIDI Note On must not advance ping-pong when an internal arp is
// active. Real SID Seq/classical clock edges must alternate on EVERY step.
#include "../Source/SidPingPongPan.h"
#include "../Source/SidQuantizedArp.h"
#include "../Source/SidSeqMemory.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
using namespace biscot;
static constexpr double sr=2000., bpm=120.,rate=16.;

static void runClassical(int notes,float signedDepth,int mode){
 SidQuantizedArp arp;
 SidPingPongPan pan;
 pan.prepare(sr);
 std::array<int,SidQuantizedArp::capacity> pool{};
 for(int j=0;j<notes;++j)pool[(size_t)j]=60+j;
 std::uint32_t randomSeed=321u;
 // The note stack is established before any grid step; its even count
 // must not change the pan side. Note On routing is checked statically too.
 assert(!pan.firstSide);
 int edges=0,afterEdge=-1;
 float expected=0.f;
 for(int sample=0;sample<notes*3*280;++sample){
  const auto event=arp.process(pool,notes,mode,rate,bpm,sr,randomSeed);
  if(event.edge && event.note>=0){
   pan.advance();++edges;afterEdge=0;
   expected=((edges%2)==1?signedDepth:-signedDepth);
  }
  const float x=pan.tick(signedDepth);
  if(afterEdge>=0){
   if(afterEdge==40){
    assert((x>0)==(expected>0));
    assert(std::abs(x)>std::abs(signedDepth)*0.96f);
   }
   ++afterEdge;
  }
 }
 assert(edges>=notes*2);
}
static void runSidSeq(int notes,float signedDepth){
 SidSeqMemory seq;
 seq.clearAll();seq.clearSequence(60);
 for(int j=1;j<notes;++j)seq.record(60+j);
 assert(seq.length==notes);
 SidPingPongPan pan;pan.prepare(sr);
 int edges=0,afterEdge=-1;
 float expected=0.f;
 for(int sample=0;sample<notes*3*280;++sample){
  seq.process(1,60,rate,bpm,sr);
  if(seq.lastTick){
   pan.advance();++edges;afterEdge=0;
   expected=(edges%2==1?signedDepth:-signedDepth);
  }
  const float x=pan.tick(signedDepth);
  if(afterEdge>=0){
   if(afterEdge==40){
    assert((x>0)==(expected>0));
    assert(std::abs(x)>std::abs(signedDepth)*0.96f);
   }
   ++afterEdge;
  }
 }
 assert(edges>=notes*2);
}
int main(){
 for(int notes=2;notes<=8;notes+=2){
  // Up, Down, Up/Down, Press Order and Random use the same clock edges.
  for(int mode=2;mode<=6;++mode){
   runClassical(notes,+1.f,mode);
   runClassical(notes,-1.f,mode);
  }
  runSidSeq(notes,+1.f);
  runSidSeq(notes,-1.f);
 }
 // A cyclic AHDSR and grid tick on the SAME sample represent one stereo edge.
 SidPingPongPan merged;merged.prepare(sr);
 const bool ampReattack=true,arpGridEdge=true;
 if(ampReattack || arpGridEdge)merged.advance();
 for(int j=0;j<100;++j)merged.tick(1.f);
 assert(merged.tick(1.f)>.95f); // a double-advance would point left
 // Classic (Arp Off) must keep the existing per-Note-On alternation.
 merged.advance();for(int j=0;j<100;++j)merged.tick(1.f);
 assert(merged.tick(1.f)<-.95f);
 std::cout<<"OK: classical and SID Seq ping pong for 2,4,6,8 notes, signed directions, merged events\n";
}
