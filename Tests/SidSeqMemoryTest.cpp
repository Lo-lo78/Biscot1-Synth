#include "../Source/SidSeqMemory.h"
#include <cassert>
#include <cmath>
#include <iostream>
using namespace biscot;
int main(){
 SidSeqMemory s;s.clearAll();
 s.clearSequence(60);assert(s.length==1&&s.slots[0]==60&&s.slots[128]==60);
 // Initial note sounds immediately at step zero; a second note is inserted
 // AFTER the currently playing step, NOT appended to a live-note pool.
 assert(s.process(1,60,16,120,48000)==0.f);assert(s.step==0);
 s.record(64);assert(s.length==2&&s.slots[0]==60&&s.slots[1]==64);
 // At 120 bpm / rate 16, one step lasts exactly 6000 samples.
 for(int i=0;i<6002;++i)s.process(1,60,16,120,48000);
 assert(s.step==1&&s.target==4.f);
 s.record(67);assert(s.length==3&&s.slots[2]==67);
 for(int i=0;i<6002;++i)s.process(1,60,16,120,48000);
 assert(s.step==2&&s.target==7.f);
 // Insert during step two: at the end of the sequence this goes to end.
 s.record(69);assert(s.length==4&&s.slots[3]==69);
 for(int i=0;i<6002;++i)s.process(1,60,16,120,48000);
 assert(s.step==3&&s.target==9.f);
 // A Note Off does NOT truncate the recording while the release latch is alive.
 SidHeldStack held;held.add(60);held.add(64);s.snapshot(held);
 held.removeNewest(64);assert(s.latch.count==2&&s.latchRoot==60);
 const auto tail=s.process(s.latch.count,s.latchRoot,16,120,48000);
 assert(std::isfinite(tail));
 // All gates ended: Chord mode stops its clock; non-Chord latch can continue.
 assert(s.process(0,60,16,120,48000)==0.f&&s.step==-1);
 // Interleaved clock insertion, independent instances and dynamic length limit.
 SidSeqMemory a,b;a.clearAll();b.clearAll();a.clearSequence(48);b.clearSequence(60);
 a.process(1,48,16,120,48000);b.process(1,60,32,120,48000);
 a.record(52);b.record(72);assert(a.slots[1]==52&&b.slots[1]==72);
 for(int n=0;n<150;++n)a.record(n%128);
 assert(a.length==128);assert(a.slots[0]==48);
 // At capacity the next incoming pitch replaces the next clock position.
 a.step=127;a.record(71);assert(a.slots[0]==71&&a.slots[128]==71);
 SidSeqMemory c;c.clearAll();c.clearSequence(60);c.record(61);c.record(62);c.step=0;
 c.record(63);assert(c.length==4&&c.slots[0]==62&&c.slots[1]==63&&c.slots[2]==61&&c.slots[3]==60);
 std::cout<<"JSFX SID Seq recorded-memory tests passed\n";
}
