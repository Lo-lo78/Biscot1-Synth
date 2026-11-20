// TEST0.2.48: per-operator AHDSR Trigger, default Off and live gate handling.
#include "../Source/SidAmpEnvelope.h"
#include "../Source/ParameterCatalog.h"
#ifdef NDEBUG
 #undef NDEBUG
#endif
#include <cassert>
#include <iostream>
#include <array>
using namespace biscot;
int main(){
 constexpr double sr=1000.;
 for(int i=0;i<3;++i){
  const auto &d=catalog[(size_t)(254+i)];
  assert(d.index==256+i && d.initial==0.f && d.high==1.f && d.step==1.f);
 }
 // Off is numerically identical to the 0.99.6 legacy envelope.
 SidAmpEnvelope classic,unchanged;
 classic.noteOn(true);unchanged.noteOn(true);
 for(int n=0;n<3000;++n){
  if(n==200){classic.noteOff(.3f,sr);unchanged.noteOff(.3f,sr);}
  const float a=classic.tick(.05f,.02f,.1f,.7f,.3f,sr);
  const float b=unchanged.tickTrigger(false,n<200,.05f,.02f,.1f,.7f,.3f,sr);
  assert(a==b && classic.stage==unchanged.stage);
 }
 // With On, one held Note On produces multiple A-H-D-S-R cycles;
 // neither a repeated Note On nor any new MIDI note is required.
 SidAmpEnvelope loop;loop.noteOn(true);
 int attacks=0, releases=0;
 int previous=loop.stage;
 for(int n=0;n<10000;++n){
  loop.tickTrigger(true,true,.004f,.003f,.005f,.6f,.008f,sr);
  if(loop.stage==1 && previous==0)++attacks;
  if(loop.stage==4 && previous==3)++releases;
  previous=loop.stage;
 }
 assert(attacks>=3 && releases>=3);
 // Real Note Off during automatic release must NOT cause a ghost attack.
 if(loop.stage!=4){
  for(int n=0;n<1000 && loop.stage!=4;++n)
   loop.tickTrigger(true,true,.004f,.003f,.005f,.6f,.008f,sr);
 }
 assert(loop.stage==4);
 loop.noteOff(.008f,sr);
 for(int n=0;n<200;++n)loop.tickTrigger(true,false,.004f,.003f,.005f,.6f,.008f,sr);
 assert(loop.stage==0 && loop.level==0.f);
 for(int n=0;n<100;++n){
  assert(loop.tickTrigger(true,false,.004f,.003f,.005f,.6f,.008f,sr)==0.f);
  assert(loop.stage==0);
 }
 // Gate release during an Attack leads into the normal Release and silence.
 SidAmpEnvelope early;early.noteOn(true);
 early.tickTrigger(true,true,.2f,.05f,.1f,.6f,.02f,sr);
 early.noteOff(.02f,sr);
 for(int n=0;n<400;++n)early.tickTrigger(true,false,.2f,.05f,.1f,.6f,.02f,sr);
 assert(early.stage==0);
 // Automation to zero Release may create short fast cycles, never NaN/Inf.
 SidAmpEnvelope fast;fast.noteOn(true);
 for(int n=0;n<1000;++n){
  const float x=fast.tickTrigger(true,true,0.f,0.f,0.f,.1f,0.f,sr);
  assert(x>=0.f && x<=1.f);
 }
 std::cout<<"OK: AHDSR Trigger on/off, seven-phase gate, no ghost, legacy parity\n";
}
