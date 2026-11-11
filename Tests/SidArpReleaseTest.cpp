#include "../Source/SidArpTransport.h"
#include "../Source/SidAmpEnvelope.h"
#include "../Source/SidQuantizedArp.h"
#include "../Source/SidSeqMemory.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <array>
#include <cstdint>
#include <iostream>

int main() {
 using namespace biscot;
 constexpr double sampleRate=1000., bpm=120., rate=16.;
 SidAmpEnvelope envelope;
 envelope.noteOn(true);
 for(int i=0;i<8;++i)envelope.tick(0.f,0.f,1.f,sampleRate);
 assert(envelope.stage==3 && sidArpClockActive(true,envelope.stage));

 std::array<int,SidQuantizedArp::capacity> lastNotes{};
 lastNotes[0]=60;lastNotes[1]=64;lastNotes[2]=67;
 SidQuantizedArp classical;
 std::uint32_t seed=456u;
 auto first=classical.process(lastNotes,3,2,rate,bpm,sampleRate,seed);
 assert(first.edge && first.note==60);

 SidSeqMemory sid;
 sid.clearAll(); sid.clearSequence(60); sid.releaseRoot=60;
 sid.process(1,60,rate,bpm,sampleRate);
 sid.record(64); sid.record(67);
 assert(sid.length==3);

 // Case A: hands released, no pedal. Both memories remain alive and both
 // pitches move on grid edges until the natural envelope is inaudible.
 envelope.noteOff(0.1f,sampleRate);
 assert(envelope.stage==4 && sidArpClockActive(false,envelope.stage));
 bool classicalMoved=false,sidMoved=false;
 int classicalEdges=0,sidEdges=0;
 for(int i=0;i<3000 && envelope.stage!=0;++i) {
  if(sidArpClockActive(false,envelope.stage)) {
   const auto e=classical.process(lastNotes,3,2,rate,bpm,sampleRate,seed);
   if(e.edge){++classicalEdges;if(e.note!=60)classicalMoved=true;}
   const float pitch=sid.process(1,60,rate,bpm,sampleRate);
   if(sid.lastTick){++sidEdges;if(pitch!=0.f)sidMoved=true;}
  }
  envelope.tick(0.f,0.f,1.f,sampleRate);
 }
 assert(classicalMoved && sidMoved && classicalEdges>=2 && sidEdges>=2);
 assert(envelope.stage==0 && !sidArpClockActive(false,envelope.stage));
 // At idle the caller clears release-note memory; no stuck notes/pitches.
 classical.reset();sid.clearAll();
 assert(classical.count==0 && sid.length==0);

 // Case B: CC64 holds the note, then CC64-up transitions to Release.
 SidAmpEnvelope pedalEnvelope;
 pedalEnvelope.noteOn(true);
 for(int i=0;i<8;++i)pedalEnvelope.tick(0.f,0.f,1.f,sampleRate);
 assert(sidArpClockActive(true,pedalEnvelope.stage));
 pedalEnvelope.noteOff(0.1f,sampleRate); // CC64 released, gate=false
 int stepsAfterPedalUp=0;
 SidQuantizedArp pedalArp;
 pedalArp.process(lastNotes,3,2,rate,bpm,sampleRate,seed);
 for(int i=0;i<3000 && pedalEnvelope.stage!=0;++i){
  if(sidArpClockActive(false,pedalEnvelope.stage)) {
   auto e=pedalArp.process(lastNotes,3,2,rate,bpm,sampleRate,seed);
   if(e.edge)++stepsAfterPedalUp;
  }
  pedalEnvelope.tick(0.f,0.f,1.f,sampleRate);
 }
 assert(stepsAfterPedalUp>=2);
 assert(pedalEnvelope.stage==0 && !sidArpClockActive(true,pedalEnvelope.stage));
 std::cout<<"SID Seq and classical arpeggios continue through natural Release with/without CC64, then stop at idle OK\n";
}
