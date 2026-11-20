#pragma once
// Per-operator note/trigger-cycle alternating pan. Signed amount selects the
// first side (+1 right / -1 left), magnitude controls depth, 0 is silent/off.
// State is on the audio thread only. No allocations or oscillator phase resets.
#include <algorithm>
#include <cmath>
namespace biscot {
struct SidPingPongPan {
 bool firstSide=false;  // First audible Note On (Arp Off) or grid edge (Arp On) selects signed side.
 float offset=0.f;
 float smoothing=1.f;
 void prepare(double sampleRate) noexcept {
  firstSide=false;
  offset=0.f;
  // Dezipper note-on retriggers / live depth changes; ~3 ms time constant.
  smoothing=1.f-std::exp(-1.f/(float)(0.003*std::max(1.,sampleRate)));
 }
 void advance() noexcept { firstSide=!firstSide; }
 float tick(float signedDepth) noexcept {
  const float target=(firstSide?1.f:-1.f)*std::clamp(signedDepth,-1.f,1.f);
  // At default zero no added math: maintain exact old centered-pan signal.
  if(target==0.f && offset==0.f)return 0.f;
  offset+=(target-offset)*smoothing;
  if(std::abs(target-offset)<0.000001f)offset=target;
  return offset;
 }
};
} // namespace biscot
