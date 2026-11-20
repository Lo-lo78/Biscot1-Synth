#pragma once
// Audio-thread helper: derive one pan coefficient from the actual semitone
// movement already computed by the operator's portamento and arpeggiator.
// This follows the combined pitch trajectory when both are changing.
#include <algorithm>
#include <cmath>
namespace biscot {
struct SidPitchGlideMotion {
 double remaining=0.,advanced=0.;
 double remainingAbs=0.,advancedAbs=0.;
 void add(double pitchBefore,double pitchTarget,double pitchAfter) noexcept {
  const double delta=pitchTarget-pitchBefore;
  const double moved=pitchAfter-pitchBefore;
  remaining+=delta; advanced+=moved;
  remainingAbs+=std::abs(delta);advancedAbs+=std::abs(moved);
 }
 bool moving() const noexcept {return remainingAbs>1.e-7;}
 float coefficient() const noexcept {
  if(!moving())return 1.f; // No actual pitch slide: a square pan step.
  // If opposite slides cancel in total, use their absolute weighted rate,
  // rather than dividing by (almost) zero or inverting the pan curve.
  const double rate=std::abs(remaining)>1.e-7
     ?advanced/remaining:advancedAbs/remainingAbs;
  if(!std::isfinite(rate) || rate<0.)
   return (float)std::clamp(advancedAbs/remainingAbs,0.,1.);
  return (float)std::clamp(rate,0.,1.);
 }
};
} // namespace biscot
