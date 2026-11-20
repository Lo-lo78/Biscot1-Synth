// TEST0.2.49 signed per-operator note + AHDSR-cycle ping pong pan.
#include "../Source/SidPingPongPan.h"
#include "../Source/ParameterCatalog.h"
#ifdef NDEBUG
 #undef NDEBUG
#endif
#include <cassert>
#include <cmath>
#include <iostream>
using namespace biscot;
static void converge(SidPingPongPan& p,float amount){
 for(int n=0;n<2000;++n)p.tick(amount);
}
int main(){
 for(int i=0;i<3;++i){
  const auto& d=catalog[(size_t)(257+i)];
  assert(d.index==259+i && d.low==-1.f && d.high==1.f && d.initial==0.f);
 }
 SidPingPongPan op1,op2,op3;
 op1.prepare(48000.);op2.prepare(48000.);op3.prepare(48000.);
 for(int n=0;n<500;++n)assert(op1.tick(0.f)==0.f); // default is DSP-transparent
 // Alternating notes; signed amounts select which side is heard FIRST.
 op1.advance(); op2.advance();op3.advance();
 converge(op1,1.f);converge(op2,-1.f);converge(op3,.4f);
 assert(op1.tick(1.f)>.85f && op2.tick(-1.f)<-.85f);
 assert(op3.tick(.4f)>.35f);
 op1.advance();op2.advance(); // second Note On
 converge(op1,1.f);converge(op2,-1.f);
 assert(op1.tick(1.f)<-.85f && op2.tick(-1.f)>.85f);
 // The same edge is used by automatic AHDSR reattacks.
 op1.advance();converge(op1,1.f);assert(op1.tick(1.f)>.85f);
 // A change to zero while sounding is smooth; eventual Off is exactly 0.
 float old=op1.tick(1.f), first=op1.tick(0.f);
 assert(first>=0 && first<old && first>0);
 converge(op1,0.f);assert(op1.tick(0.f)==0.f);
 // Subsequent real note still alternates when the parameter was Off.
 op1.advance();converge(op1,-1.f);assert(op1.tick(-1.f)>.85f);
 // No crosstalk from other operators.
 assert(op3.tick(.4f)>.35f);
 std::cout<<"OK: signed ping-pong pan, alternating notes and cycles, zero default, smooth live automation\n";
}
