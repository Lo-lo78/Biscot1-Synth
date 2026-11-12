# Biscot1 Synth — TEST0.2.21.3 (12 November 2026)

## F1: three independent vertical regions

Unmodified Up/Down and PageUp/PageDown remain in the current navigation region:

- Header: Length, MIDI Note, Step Recording, Release Arpeggio; horizontal left/right changes the field. Vertical arrows do not enter steps.
- Steps: movement is clamped to step 0 .. Length-1; cannot descend into Repeat or rise into the header.
- Repeat and Speed/Transpose: Repeat is the top boundary and the last populated Speed/Transpose row is the bottom boundary; movement does not enter steps.

Home and End explicitly change regions. Home from any Repeat/Speed row restores the remembered last edited step, Home from steps reaches the header; End from header returns to the remembered step, End from steps enters Repeat, End from Repeat enters the first Speed row. L/M/S/R/P/D/T and shift reverse cycles are unchanged. Silent borders preserve the cell selection and avoid useless NVDA repeats. The parameter picker opened with Enter has **unchanged** navigation.

## Release Arpeggio: original semantics retained, not a mandatory arp sustain control

The original JSFX `schedule_release_arpeggio()` applies *only at the natural end of a Pattern*, playing pitch-only legato changes for the `Note` cells in the Pattern while the current VCA envelope is already releasing. It does not use `Sub Note`, and it does not continue the chord arp's normal note pool. No effect if the pattern contains only one distinct Note or the VCA has already fallen silent. The C++ implementation intentionally keeps this optional legacy behavior, default Off. The normal instrument's arpeggiator now sustains pitch movement through Release independently of it.

Critical distinction confirmed by the user: with an ordinary arpeggiated chord, **Off** keeps the normal arpeggiator cycling during the envelope Release; **On** substitutes the legacy Pattern-Note release sequencer and can sound like a single held pitch. This is not a global arp sustain switch. In particular, the legacy alternative ignores `Sub Note` and reads only explicit `Note` cells. When there is just one distinct explicit Note, repeating that note has no audible arpeggio. The user has also reported On apparently remaining static during their testing, so the audible behavior of the legacy alternative is **not yet verified as correct** on Windows. Do not claim full feature parity on the basis of unit tests.

Diagnostic REAPER test for that alternative (not required to use the normal Release arp): OP Arp Mode Off; OP Release 2 seconds; Pattern Length 4 Repeat 1; four explicit `Note` cells at steps 0..3 with pitches 60, 64, 67, 72; last Gate 1. If On still holds a fixed pitch through an audible Release tail, investigate the runtime release scheduler; don't assume user settings are wrong. This version intentionally does not alter the release scheduler or the established Off behavior.

## Compatibility / tests

The DSP, MIDI engine, Pattern playback/automation, preset state format and parameter mappings have not changed. Added independent C++ Release-mode assertions for all 3 F1 regions, including boundary stops, memory of last edit step and single-step/single-repeat patterns. Prior test suites remain in place. Full JUCE Windows build and NVDA testing still require GitHub Actions and REAPER.
