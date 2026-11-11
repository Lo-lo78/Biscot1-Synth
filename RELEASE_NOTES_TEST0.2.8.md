# Biscot1 Synth VST3 TEST0.2.8 — ADSR / MIDI Sustain Pedal

- Corrected Chord Allocate Note Off handling: only the voice actually owning the released note is affected.
- All three amplifier Attack / Decay / Release retain the original JSFX 0..5 second ranges, with 0.001 second parameter resolution.
- Extended the reported plugin tail from 5 to 50 seconds so REAPER can retain the natural long exponential Release (5-second time constant can take around 45 seconds to reach the JSFX silence threshold).
- Extracted the JSFX envelope state machine into testable `SidAmpEnvelope.h`. The release tail is no longer restarted by a duplicate Note Off.
- Added three independent **Op1/Op2/Op3 Sustain Pedal** switches (Off/On; default Off), with stable new VST parameter IDs `s227`, `s228`, `s229`.
- When enabled, MIDI CC64 >= 64 on the note's channel defers the corresponding operator's release. CC64 < 64 releases latched voices at each operator's own Release time. Disabling the switch also releases a latch.
- Off means CC64 is ignored for that operator. Amp Sustain (ADSR Sustain 0..1) remains a different control.
- Parameter positions and IDs 2..226, existing DSP, wavetable, filter and output stage remain unchanged.
- The previous 224 JSFX parameters still map to their original VST IDs. Each new switch is presented adjacent to its operator's Amp Sustain.
- Added native standalone ADSR tests and chord gate regression to GitHub Actions.
- No Windows VST3 build or listening test has been run here.

**Note:** a fresh instance defaults `Op2 Level` and `Op3 Level` to 0, as in the JSFX. Raise `Op3 Level` when evaluating its ADSR; setting `Op3 Release` to 5 does not automatically create a five-second *total silence time*, as the original JSFX has an exponential tail.
