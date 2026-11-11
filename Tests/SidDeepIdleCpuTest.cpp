#include "../Source/SidArpPoolCache.h"
#include "../Source/SidIdleSleep.h"
#include "../Source/SidLfoCore.h"
#include <cassert>
#include <cmath>
#include <iostream>
using namespace biscot;
int main(){
 SidArpPoolCache cache;
 SidArpPoolSettings controls;
 assert(cache.needsRefresh(controls));
 cache.didRefresh(controls);
 for(int block=0;block<30000;++block)
   assert(!cache.needsRefresh(controls)); // 30k unchanged blocks: 0 rescans
 controls.arpMode[0]=3;
 assert(cache.needsRefresh(controls));cache.didRefresh(controls);
 controls.octaveUp[2]=2;
 assert(cache.needsRefresh(controls));cache.didRefresh(controls);
 controls.octaveDown[1]=-2;
 assert(cache.needsRefresh(controls));cache.didRefresh(controls);
 controls.pedalEnabled[1]=true;
 assert(cache.needsRefresh(controls));cache.didRefresh(controls);
 controls.midiMode=2;
 assert(cache.needsRefresh(controls));cache.didRefresh(controls);
 cache.invalidate();assert(cache.needsRefresh(controls));
 // Free clock runs blockwise during cold idle; transport restart may reset
 // phase at the documented play edge without waking expensive audio DSP.
 SidIdleSleep idle;assert(idle.asleep);
 SidLfoCore l;l.reset(0);l.phase=.125;
 SidIdleSleep::advanceFreeClock(l,12000,120.,2.,48000.,true,false);
 assert(std::abs(l.phase-.375)<1e-9);
 SidIdleSleep::advanceFreeClock(l,12000,120.,2.,48000.,false,false);
 assert(std::abs(l.phase-.375)<1e-9);
 idle.wake();assert(!idle.asleep);
 for(int i=0;i<64;++i)idle.observe(true,true,0.f);
 assert(idle.asleep);
 std::cout << "OK: cold idle, no redundant arp rebuild, Free clock\n";
}
