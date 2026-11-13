#pragma once
// Biscot1 JSFX 2.38 LFO state and signal ordering.
// Kept JUCE-independent so the audio-rate logic can be regression-tested.
#include <algorithm>
#include <cmath>

namespace biscot {
struct SidLfoCore {
 double phase=0., age=2., delayAge=1000000.;
 float output=0.f, previous=0.f, hold=0.f, smoothed=0.f;
 double seed=7654321.;
 unsigned long long cycles=0;
 bool active=true;

 void reset(int index) noexcept {
  phase=0.;age=2.;delayAge=1000000.;cycles=0;
  output=previous=hold=smoothed=0.f;active=true;
  seed=index==0?7654321.:(index==1?8765432.:9876543.);
 }

 // JSFX lfo_note_trigger(): every permitted note trigger restarts Delay;
 // Phase and One Shot age restart only for Trigger mode or One Shot mode.
 void noteTrigger(bool triggerMode,bool oneShot,double startPhase) noexcept {
  delayAge=0.;
  if(triggerMode||oneShot){
   phase=startPhase-std::floor(startPhase);
   age=0.;cycles=0;
  }
  active=true;
 }

 float tick(double phaseInc, int wave, double delaySec, double smoothAmount,
            bool oneShot, double oneShotLength, double upperSquash,
            double lowerSquash, double sampleRate, double squarePwm=0.5,
            double driftAmount=0., int driftCycle=5) noexcept {
  const double sr=std::max(1.,sampleRate);
  // JSFX: a One Shot that finished becomes free-running immediately when
  // One Shot is disabled, without requiring an extra MIDI Note On.
  if(!active && !oneShot)active=true;
  if(active){
   delayAge+=1./sr;
   const double oldPhase=phase;
   phase+=phaseInc;
   if(phase>=1.){const double whole=std::floor(phase);phase-=whole;cycles+=(unsigned long long)whole;}
   // JSFX Sample Hold and Smooth Random draw at half-cycle AND wrap.
   const bool update=(phase<oldPhase) || ((wave==5||wave==6) && oldPhase<0.5 && phase>=0.5);
   if(update){
    seed=std::fmod(seed*1103515245.0+12345.,2147483647.);
    hold=(float)(seed/1073741823.5-1.);
   }
   age+=phaseInc;
   if(oneShot && age>=std::clamp(oneShotLength,0.,1.))active=false;
  }
  double raw=0.;
  if(active){
   // Clock-polyrhythm: N different phase positions repeat every N complete
   // LFO cycles, irrespective of the actual rate. Zero retains JSFX parity.
   const int period=std::clamp(driftCycle,1,32);
   const double depth=std::clamp(driftAmount,0.,1.);
   const double shift=(period>1 && depth>0.) ?
       depth*(double)(cycles%(unsigned long long)period)/(double)period:0.;
   const double x=phase+shift-std::floor(phase+shift);
   switch(wave){
    case 0:raw=std::sin(6.2831853071795864769*x);break;
    case 1:raw=4.*std::abs(x-0.5)-1.;break;
    case 2:raw=2.*x-1.;break;
    case 3:raw=1.-2.*x;break;
    case 4:raw=x<std::clamp(squarePwm,0.05,0.95)?1.:-1.;break;
    default:
     if(depth>0. && period>1){
      // Repeatable Sample Hold and Smooth Random cells, unlike an unbounded PRNG.
      const unsigned long long cell=cycles%(unsigned long long)period;
      const unsigned long long half=x>=0.5?1ULL:0ULL;
      unsigned long long hash=(cell*2ULL+half+1ULL)*0x9e3779b97f4a7c15ULL;
      hash^=hash>>30;hash*=0xbf58476d1ce4e5b9ULL;
      hash^=hash>>27;hash*=0x94d049bb133111ebULL;
      hash^=hash>>31;
      raw=(double)(hash&0xffffffULL)/8388607.5-1.;
     }else raw=hold;
     break;
   }
   const double fade=delaySec<=0. ? 1. : std::min(1.,delayAge/std::max(0.000001,delaySec));
   raw*=fade;
  }
  // JSFX applies Smooth to the full bipolar signal, then applies the
  // independent upper and lower attenuation to the *smoothed* output.
  const double amt=std::clamp(smoothAmount,0.,1.);
  const double effective=(wave==6)?std::max(0.12,amt):amt;
  const double coefficient=effective<=0. ? 1. :
     1.-std::exp(-1./std::max(1.,(0.001+effective*effective*0.249)*sr));
  smoothed=(float)(coefficient>=1. ? raw : smoothed+(raw-smoothed)*coefficient);
  const double squash=smoothed>=0. ? upperSquash:lowerSquash;
  output=(float)(smoothed*(1.-std::clamp(squash,0.,1.)));
  return output;
 }
};
} // namespace biscot
