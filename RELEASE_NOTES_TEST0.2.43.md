# Biscot1 Synth TEST0.2.43 — host Song Trigger automation

The native VST3 now exposes **Song Trigger** to REAPER as an automatable plugin parameter, following the legacy `Biscot1 Tracker` JSFX slider6 semantics:

- `0`: Off (stops playback initiated by Song Trigger).
- `1..100`: Song `0..99` respectively, including all three operator lanes and Song End transitions.
- When an automated value changes, starts that Song from row 0.
- On every host stopped-to-playing transition, a non-zero Song Trigger re-launches the Song, including when REAPER restores the same automation value.
- On host Play-to-Stop transition, existing native logic stops the running Song. Free-running LFO phase reset and host BPM remain unchanged.
- Previously implemented MIDI Song-note and Pattern-note mappings still work; no Lua or shared GMEM is needed.
- Supports host automation and project/preset state via APVTS; no synth DSP slider numbering changed.
- No requirement that F1/F2 UI be open while the DAW plays.

Known limits: the host transport currently follows Play/Stop edges and BPM but does not reconstruct position within the Song after a seek performed while playback continues. A new Play after Stop restarts Song row 0. Song Trigger itself does not contain synth timbre; presets `.biscot` save parameters and bank data.
