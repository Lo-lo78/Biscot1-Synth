#pragma once
// Musical arpeggiator transport decisions, independent of the SID audio DSP.
// Trigger restarts the pattern on the next clock edge, not the amplitude ADSR.
// Legato leaves the sequence position unchanged; both preserve BPM phase.
namespace biscot {
inline bool sidArpRestartRequested(bool triggerMode,bool wasRunning,bool addressesOperator) noexcept {
 return triggerMode && wasRunning && addressesOperator;
}
// When no key and no CC64 latch is holding the operator, do not run its arp
// during the natural release tail. Its current pitch remains frozen while the
// ADSR fades, and the musical transport stops. No hard mute is performed.
inline bool sidArpClockActive(bool gate,int envelopeStage) noexcept {
 return gate && envelopeStage!=0;
}
} // namespace biscot
