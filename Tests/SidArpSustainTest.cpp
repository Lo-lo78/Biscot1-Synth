#include "../Source/SidArpSustain.h"
#include "../Source/SidMidiNoteState.h"
#include "../Source/SidSeqMemory.h"
#include "../Source/SidAmpEnvelope.h"
#include <array>
#include <cassert>
#include <iostream>

using namespace biscot;
int main(){
 SidArpSustain pedal;
 // Three distinct operators; no global omni pool or stuck notes.
 pedal.hold(0,1,60,1);pedal.hold(1,1,60,1);pedal.hold(2,1,60,1);
 for(int i=0;i<3;++i){assert(pedal.any(i));assert(pedal.timestamp(i,1,60)==1);}
 // Add another key while CC64 is down: the old note remains in the pool.
 pedal.hold(0,1,64,2);pedal.hold(1,1,64,2);pedal.hold(2,1,64,2);
 for(int i=0;i<3;++i){assert(pedal.timestamp(i,1,60));assert(pedal.timestamp(i,1,64));}
 // Other channels / voices must remain independent on pedal release.
 pedal.hold(0,2,67,3);pedal.clearChannel(1);
 assert(pedal.timestamp(0,2,67)==3 && pedal.any(0));
 assert(!pedal.any(1)&&!pedal.any(2));
 pedal.clearOperator(0);assert(!pedal.any(0));
 // Default: independent, even in Chord mode; other modes ignore master.
 assert(!chordMasterEnabled(1,false));
 assert(chordVoiceIndex(1,false,2)==2);
 assert(chordVoiceIndex(1,true,2)==0);
 assert(chordVoiceIndex(0,true,2)==2);
 assert(chordVoiceIndex(2,true,2)==2);
 assert(midiAddressesOperator(1,2,9));
 assert(!midiAddressesOperator(2,2,2));
 assert(midiAddressesOperator(2,2,3));
 // Trigger retriggers held notes. Legato does not; arp new notes keep the clock.
 assert(sidShouldRetrigger(false,true,false));
 assert(!sidShouldRetrigger(false,true,true));
 assert(!sidShouldRetrigger(true,true,false));
 assert(sidShouldRetrigger(true,false,false));
 // A second note should extend SID Seq; this must never clear its memory.
 SidSeqMemory seq;
 seq.clearAll();seq.clearSequence(60);seq.releaseRoot=60;
 assert(seq.process(1,60,16,120,1000)==0.f);
 seq.record(64);assert(seq.length==2);
 seq.record(67);assert(seq.length==3);
 assert(seq.slots[0]!=SidSeqMemory::empty);
 seq.process(1,60,16,120,1000);
 assert(seq.length==3);
 // Sustained attack holds envelope; CC64 up can initiate its actual release.
 SidAmpEnvelope env;env.noteOn(true);
 for(int n=0;n<1000;++n)env.tick(0,0,0.8f,1000.);
 assert(env.stage==3 && env.level>0.f);
 env.noteOff(2,1000);
 assert(env.stage==4);
 env.tick(0,0,0.8f,1000.);
 assert(env.stage==4 && env.level>0.f);
 std::cout<<"CC64 arp pool, SID Seq continuation and Chord Op1 master tests OK\n";
}
