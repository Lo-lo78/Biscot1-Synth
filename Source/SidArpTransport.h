#pragma once
// Musical arpeggiator transport decisions, independent of the SID audio DSP.
// Trigger restarts the pattern on the next clock edge, not the amplitude ADSR.
// Legato leaves the sequence position unchanged; both preserve BPM phase.
namespace biscot {
inline bool sidArpRestartRequested(bool triggerMode,bool wasRunning,bool addressesOperator) noexcept {
 return triggerMode && wasRunning && addressesOperator;
}
// The original JSFX keeps the arpeggio frequency sequence moving THROUGH
// the natural amplitude Release, even when neither a key nor CC64 is down.
// The latch stores the last complete note material and the oscillator still
// follows the sequencer, only with the ADSR amplitude decaying naturally.
// Clock stops ONLY when the amp envelope reaches idle. A pedal latch keeps
// the envelope sustained; on CC64-up the same clock continues through Release.
inline bool sidArpClockActive(bool /*gate*/,int envelopeStage) noexcept {
 return envelopeStage!=0;
}
} // namespace biscot
