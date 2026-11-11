#pragma once
// SID amplitude and modulation envelopes: same sample-by-sample equations as
// Biscot1Synth_v2.38.jsfx (including the exponential release threshold).
// Range 0..5 seconds is selected by the host parameter catalogue.
#include <algorithm>

namespace biscot {
struct SidAmpEnvelope {
 float level=0.f, releaseCoefficient=0.f;
 int stage=0; // idle / attack / decay / sustain / release
 void noteOn(bool reset) noexcept { if(reset)level=0.f; stage=1; }
 void noteOff(float seconds,double sampleRate) noexcept {
  if(stage==0 || stage==4)return; // do not restart a running release
  releaseCoefficient=1.f/(float)std::max(1.,(double)seconds*sampleRate);
  stage=4;
 }
 float tick(float attack,float decay,float sustain,double sampleRate) noexcept {
  sustain=std::clamp(sustain,0.f,1.f);
  if(stage==1){level+=attack<=0.f?1.f:1.f/(float)(attack*sampleRate);if(level>=1.f){level=1.f;stage=2;}}
  else if(stage==2){level-=decay<=0.f?1.f:(1.f-sustain)/(float)(decay*sampleRate);if(level<=sustain){level=sustain;stage=3;}}
  else if(stage==3){level=sustain;}
  else if(stage==4){level-=std::max(0.0000001f,level)*std::max(0.0000000001f,releaseCoefficient);
    if(level<=0.0001f){level=0.f;stage=0;}}
  return level;
 }
};
} // namespace biscot
