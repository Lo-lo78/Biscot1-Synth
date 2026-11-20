#pragma once
// Signed alternating pan, driven by the oscillator's *actual pitch glide*.
// In step mode the stereo side switches on the same sample as the pitch.
// Under portamento/arp glide, the stereo side advances by the exact same
// per-sample fraction as the pitch, even when glide speed changes live.
// Only independent automation of the amount uses the legacy 3 ms dezipper.
// All state belongs to one operator on the audio thread; no allocations.
#include <algorithm>
#include <cmath>
namespace biscot {
struct SidPingPongPan {
 bool firstSide=false;
 float phase=0.f;           // -1 = left, +1 = right; exact pitch-synchronized motion
 float depth=0.f;           // signed [-1,+1] amount, de-clicked for automation
 float smoothing=1.f;       // depth changes only; NEVER delays a pitch-driven flip
 bool depthInitialized=false;
 void prepare(double sampleRate) noexcept {
  firstSide=false; phase=0.f; depth=0.f; depthInitialized=false;
  smoothing=1.f-std::exp(-1.f/(float)(0.003*std::max(1.,sampleRate)));
 }
 // A new edge updates the desired side. tick() applies pitch coefficient.
 void advance() noexcept {firstSide=!firstSide;}
 // A square pitch jump or an Attack with no glide must be instantaneous;
 // commit the depth too to avoid a first-Attack leak from stale automation.
 void advanceAtAttack(float signedDepth) noexcept {
  advance();phase=firstSide?1.f:-1.f;
  depth=std::clamp(signedDepth,-1.f,1.f);depthInitialized=true;
 }
 // pitchCoefficient is supplied from the *same audio sample* as the
 // portamento/arpPitch update. 1 = square jump. The default preserves the
 // historical 3 ms standalone/de-click behavior in existing unit tests.
 float tick(float signedDepth,float pitchCoefficient=-1.f) noexcept {
  const float targetDepth=std::clamp(signedDepth,-1.f,1.f);
  if(!depthInitialized){depth=targetDepth;depthInitialized=true;}
  else {
   depth+=(targetDepth-depth)*smoothing;
   if(std::abs(targetDepth-depth)<0.000001f)depth=targetDepth;
  }
  const float targetSide=firstSide?1.f:-1.f;
  const float coefficient=pitchCoefficient<0.f?smoothing:std::clamp(pitchCoefficient,0.f,1.f);
  // No event has chosen a stereo side yet. With zero depth this is the
  // exact old output path (no extra pan offset on legacy presets).
  if(phase!=targetSide && (firstSide || phase!=0.f)){
   phase+=(targetSide-phase)*coefficient;
   if(std::abs(targetSide-phase)<0.000001f)phase=targetSide;
  }
  return phase*depth;
 }
};
} // namespace biscot
