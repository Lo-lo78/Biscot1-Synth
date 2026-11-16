#pragma once
// Biscot1 SID-style envelopes, extended with an optional AHDSR peak hold.
// Stages 0..4 retain their original numeric meaning for arp and release logic.
// Hold is stage 5 and defaults to zero, so older presets preserve ADSR timing.
#include <algorithm>
#include <cmath>

namespace biscot {
struct SidAmpEnvelope {
 float level=0.f, releaseCoefficient=0.f;
 int stage=0; // 0 idle, 1 attack, 2 decay, 3 sustain, 4 release, 5 hold
 int holdSamplesRemaining=0;
 void noteOn(bool reset) noexcept {
  if(reset)level=0.f;
  holdSamplesRemaining=0;
  stage=1;
 }
 void noteOff(float seconds,double sampleRate) noexcept {
  if(stage==0 || stage==4)return; // never interrupt/restart the release tail
  holdSamplesRemaining=0;
  releaseCoefficient=1.f/(float)std::max(1.,(double)seconds*sampleRate);
  stage=4;
 }
 // Old signature retained for existing SID/arp/filter parity tests.
 float tick(float attack,float decay,float sustain,double sampleRate) noexcept {
  return tick(attack,0.f,decay,sustain,sampleRate);
 }
 float tick(float attack,float hold,float decay,float sustain,double sampleRate) noexcept {
  sustain=std::clamp(sustain,0.f,1.f);
  if(stage==1){
   level+=attack<=0.f?1.f:1.f/(float)(attack*sampleRate);
   if(level>=1.f){
    level=1.f;
    // Capture hold duration when Attack reaches its peak. Exactly N further
    // output samples stay at 1.0 before the original Decay equation resumes.
    holdSamplesRemaining=hold>0.f?(int)std::max(1LL,std::llround((double)hold*sampleRate)):0;
    stage=holdSamplesRemaining>0?5:2;
   }
  }
  else if(stage==5){
   level=1.f;
   if(holdSamplesRemaining>0 && --holdSamplesRemaining==0)stage=2;
  }
  else if(stage==2){
   level-=decay<=0.f?1.f:(1.f-sustain)/(float)(decay*sampleRate);
   if(level<=sustain){level=sustain;stage=3;}
  }
  else if(stage==3){level=sustain;}
  else if(stage==4){
   level-=std::max(0.0000001f,level)*std::max(0.0000000001f,releaseCoefficient);
   if(level<=0.0001f){level=0.f;stage=0;}
  }
  return level;
 }
};
} // namespace biscot
