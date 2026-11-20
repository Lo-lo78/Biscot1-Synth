#pragma once
// Biscot1 SID-style envelopes, extended with an optional AHDSR peak hold.
// Stages 0..4 retain their original numeric meaning for arp and release logic.
// Hold is stage 5 and defaults to zero, so older presets preserve ADSR timing.
// During a running envelope the selected Hold/Release can also change via host automation.
#include <algorithm>
#include <cmath>

namespace biscot {
struct SidAmpEnvelope {
 float level=0.f, releaseCoefficient=0.f;
 float releaseSecondsCache=-1.f, holdSecondsCache=-1.f;
 int stage=0; // 0 idle, 1 attack, 2 decay, 3 sustain, 4 release, 5 hold
 int holdSamplesRemaining=0, holdSamplesElapsed=0;
 void noteOn(bool reset) noexcept {
  if(reset)level=0.f;
  holdSamplesRemaining=holdSamplesElapsed=0;
  stage=1;
 }
 void noteOff(float seconds,double sampleRate) noexcept {
  if(stage==0 || stage==4)return; // never interrupt/restart the release tail
  holdSamplesRemaining=holdSamplesElapsed=0;
  releaseSecondsCache=seconds;
  releaseCoefficient=1.f/(float)std::max(1.,(double)seconds*sampleRate);
  stage=4;
 }
 // Fast path: reading an automated Release is cheap at audio rate. Only a
 // *changed* value recalculates the exponential coefficient. No retrigger,
 // phase reset, allocation, or new DSP when the host does not automate it.
 void updateRelease(float seconds,double sampleRate) noexcept {
  if(stage==4 && seconds!=releaseSecondsCache){
   releaseSecondsCache=seconds;
   releaseCoefficient=1.f/(float)std::max(1.,(double)seconds*sampleRate);
  }
 }
 // Live-automation variant used by all three VCAs, all three Mod ENV and
 // the filter envelope, including their already released voices.
 float tick(float attack,float hold,float decay,float sustain,
            float release,double sampleRate) noexcept {
  updateRelease(release,sampleRate);
  return tick(attack,hold,decay,sustain,sampleRate);
 }
 // Optional oscillator VCA trigger-cycle: while a MIDI/pedal gate is held,
 // sustain lasts one sample and transitions to a genuine Release. Once that
 // release has fallen below the existing silence threshold the next Attack
 // starts from zero. A real Note Off cancels the repeating cycle immediately
 // and completes the ongoing natural release. Nothing loops in silence.
 // Off is a pure pass-through, preserving every original envelope sample.
 float tickTrigger(bool enabled,bool gate,float attack,float hold,float decay,
                   float sustain,float release,double sampleRate) noexcept {
  if(enabled && gate){
   if(stage==3)noteOff(release,sampleRate);
   else if(stage==0)noteOn(true);
  }
  return tick(attack,hold,decay,sustain,release,sampleRate);
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
    holdSecondsCache=hold;
    holdSamplesElapsed=0;
    holdSamplesRemaining=hold>0.f?(int)std::max(1LL,std::llround((double)hold*sampleRate)):0;
    stage=holdSamplesRemaining>0?5:2;
   }
  }
  else if(stage==5){
   // Unlike the original fixed timer, Hold automation shortens or extends
   // the *current* plateau, counted from the same Attack peak. The normal
   // unautomated path is exactly the original single decrement per sample.
   if(hold!=holdSecondsCache){
    holdSecondsCache=hold;
    const int total=hold>0.f?(int)std::max(1LL,std::llround((double)hold*sampleRate)):0;
    holdSamplesRemaining=std::max(0,total-holdSamplesElapsed);
   }
   level=1.f;
   if(holdSamplesRemaining<=0 || --holdSamplesRemaining==0)stage=2;
   ++holdSamplesElapsed;
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
