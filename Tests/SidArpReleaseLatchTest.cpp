// Release regression: note-off events arrive one after another, often with
// whole audio blocks in between. The complete last Note-On chord is not
// allowed to collapse to the final key as the physical pool shrinks.
#include "../Source/SidArpReleasePool.h"
#include "../Source/SidAmpEnvelope.h"
#include "../Source/SidQuantizedArp.h"
#include <array>
#include <cmath>
#include <iostream>
#define CHECK(x) do {if(!(x)){std::cerr<<"FAILED "<<#x<<" line "<<__LINE__<<"\n";return 1;}}while(0)
int main(){
 using namespace biscot;
 std::array<int,640> live{},release{};
 int releaseCount=0;
 // During Note-On the live pool grows from one to three notes.
 for(int n=0;n<3;++n){live[n]=60+4*n;sidLatchArpReleasePool(live,n+1,release,releaseCount,true);}
 CHECK(releaseCount==3 && release[0]==60 && release[1]==64 && release[2]==68);
 // Three successive manual key releases; each pool rebuild must leave the
 // release latch completely unchanged. This used to fail in Biscot1 0.2.21.
 live[0]=64;live[1]=68;
 sidLatchArpReleasePool(live,2,release,releaseCount,false);
 CHECK(releaseCount==3 && release[0]==60);
 live[0]=68;
 sidLatchArpReleasePool(live,1,release,releaseCount,false);
 CHECK(releaseCount==3 && release[0]==60 && release[1]==64);
 sidLatchArpReleasePool(live,0,release,releaseCount,false);
 CHECK(releaseCount==3 && release[2]==68);
 SidAmpEnvelope envelope; envelope.noteOn(true);
 for(int i=0;i<10;++i)envelope.tick(0.f,0.f,1.f,1000.);
 envelope.noteOff(.6f,1000.);
 SidQuantizedArp arp;
 std::uint32_t seed=456u;
 int edges=0;bool moved=false;int first=-1;
 for(int i=0;i<1000 && envelope.stage!=0;++i){
  const auto tick=arp.process(release,releaseCount,2,16.,120.,1000.,seed);
  if(tick.edge){++edges;if(first<0)first=tick.note;else if(tick.note!=first)moved=true;}
  envelope.tick(0.f,0.f,1.f,1000.);
 }
 CHECK(edges>=3 && moved);
 // A new phrase updates the release latch; previous keys don't leak.
 live.fill(0);live[0]=72;
 sidLatchArpReleasePool(live,1,release,releaseCount,true);
 CHECK(releaseCount==1 && release[0]==72);
 // Pedal release must not overwrite the already latched notes.
 sidLatchArpReleasePool(live,0,release,releaseCount,false);
 CHECK(releaseCount==1 && release[0]==72);
 std::cout<<"OK full manual/no-pedal release chord, pedal parity and new phrase\n";
}
