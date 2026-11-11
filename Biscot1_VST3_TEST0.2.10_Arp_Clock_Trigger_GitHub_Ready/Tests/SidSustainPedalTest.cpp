#include "../Source/SidAmpEnvelope.h"
#include "../Source/SidMidiNoteState.h"
#include <array>
#include <cassert>
#include <cmath>
#include <iostream>
using namespace biscot;
int main(){
 constexpr double sr=1000.;
 for(int op=0;op<3;++op){
  SidAmpEnvelope e; e.noteOn(true);
  int attackSamples=0;
  while(e.stage==1 && attackSamples<5100){e.tick(5.f,5.f,0.2f,sr);++attackSamples;}
  assert(e.stage==2 && attackSamples>=4980 && attackSamples<=5020);
  assert(std::abs(e.level-1.f)<0.0001f);
  int decaySamples=0;
  while(e.stage==2 && decaySamples<5100){e.tick(5.f,5.f,0.2f,sr);++decaySamples;}
  assert(e.stage==3 && decaySamples>=4980 && decaySamples<=5020);
  assert(std::abs(e.level-0.2f)<0.0002f);
  e.noteOff(5.f,sr);assert(e.stage==4);
  const auto previous=e.level;
  e.tick(5.f,5.f,0.2f,sr);assert(e.level<previous && e.level>0.f);
  e.noteOff(0.f,sr); // duplicate noteOff must NOT kill the tail
  assert(e.stage==4);
  for(int n=0;n<55000 && e.stage!=0;++n)e.tick(5.f,5.f,0.2f,sr);
  assert(e.stage==0 && e.level==0.f);
 }
 // Releasing one chord key must not release the other two operator gates.
 SidMidiNoteState s;
 s.noteOn(1,1,60,false);s.noteOn(1,1,64,false);s.noteOn(1,1,67,false);
 auto off=s.noteOff(1,1,60);
 assert(off.affected[0] && !off.affected[1] && !off.affected[2]);
 off=s.noteOff(1,1,67);
 assert(!off.affected[0] && !off.affected[1] && off.affected[2]);
 off=s.noteOff(1,1,64);
 assert(!off.affected[0] && off.affected[1] && !off.affected[2]);
 std::cout<<"ADSR 0-5 s and operator-specific Note Off regression OK\n";
}
