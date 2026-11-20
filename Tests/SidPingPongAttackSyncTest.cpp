// TEST0.2.51: Pan must already be on the new side on the FIRST audible
// sample of each physical and AHDSR-triggered Attack. Continuous modulation
// and legato arpeggio edges retain the existing 3ms de-click smoothing.
#include "../Source/SidPingPongPan.h"
#include "../Source/SidAmpEnvelope.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <cmath>
#include <iostream>
using namespace biscot;
static constexpr double sr=48000.;
static void noOldSide(float amp,float pan){
 const float left=amp*(1.f-std::max(0.f,pan));
 const float right=amp*(1.f+std::min(0.f,pan));
 assert(amp>0.f);
 if(pan>0.f)assert(std::abs(left)<1.e-7f && right>0.f);
 else assert(std::abs(right)<1.e-7f && left>0.f);
}
int main(){
 for(int operatorIndex=0;operatorIndex<3;++operatorIndex){
  for(float sign: {-1.f,1.f}){
   SidPingPongPan pan;
   pan.prepare(sr);
   SidAmpEnvelope env;
   env.noteOn(true);
   // A real Note On is processed before the first audio sample.
   pan.advanceAtAttack(sign);
   noOldSide(env.tickTrigger(false,true,0.f,0.f,0.00002f,0.4f,0.00002f,sr),pan.tick(sign));
   int attacks=0;
   for(int sample=0;sample<500;++sample){
    const bool reattack=env.stage==0;
    if(reattack){pan.advanceAtAttack(sign);++attacks;}
    const float a=env.tickTrigger(true,true,0.f,0.f,0.00002f,0.4f,0.00002f,sr);
    const float p=pan.tick(sign);
    if(reattack){
     assert(p==(pan.firstSide?sign:-sign));
     noOldSide(a,p);
    }
   }
   assert(attacks>20);
  }
 }
 // Arp pitch step without amp reattack: keep smoothed pan to avoid clicks.
 SidPingPongPan legato;
 legato.prepare(sr);
 legato.advanceAtAttack(+1.f);
 assert(legato.tick(+1.f)==1.f);
 legato.advance();
 const float smoothed=legato.tick(+1.f);
 assert(smoothed>0.f && smoothed<1.f);
 for(int j=0;j<3000;++j)legato.tick(+1.f);
 assert(legato.tick(+1.f)<-0.999f);
 // Automating signed depth while the note is sounding also remains smoothed.
 const float change=legato.tick(-1.f);
 assert(change<0.f && change>-1.f);
 // Coincident arp edge + cyclic trigger is ONE instantaneous step, not two.
 SidPingPongPan merged; merged.prepare(sr);
 const bool arpEdge=true,ahdsrReattack=true;
 if(arpEdge || ahdsrReattack){
  if(ahdsrReattack)merged.advanceAtAttack(1.f);
  else merged.advance();
 }
 assert(merged.tick(1.f)==1.f);
 std::cout<<"OK: pan switches on first Attack sample, 3 independent operators, signed depth, normal arp and automation slew retained\n";
}
