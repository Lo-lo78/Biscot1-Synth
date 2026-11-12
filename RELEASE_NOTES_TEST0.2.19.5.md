# Biscot1 Synth VST3 - TEST0.2.19.5 (12 November 2026)

## Accessible numeric editing

- F1 Enter edits the selected numeric value directly; applies to Length, MIDI Note, Repeat, Speed, Transpose and every populated OP/Filter/LFO cell. The existing numeric text control retains focus and NVDA accessibility.
- F1 and F2 Alt+Enter now open the Pattern number prompt (0–999); switching from F2 preserves the parameter-list page and announces the newly selected Pattern.
- F2 Enter continues inserting the current parameter and returning to F1; Ctrl+Enter continues inserting while remaining in F2.
- F1 Alt+E remains an alias for manual numeric editing. On an empty step Enter announces that a parameter must first be inserted from F2.
- Manual values accept both decimal commas and dots; invalid text is rejected rather than being silently interpreted as zero. Existing per-parameter range clamping and live audition remain unchanged.

## Not yet complete from original Lua 0.166

F1: Step Recording (also a missing header cell), Release Arpeggio, MIDI trigger of Patterns, independent Pattern export/save, full Lua command and audition parity.
F2: Previous Value, Curve, Curve Resolution/Ratio, Sub Note execution and insertion dependencies/ordering, plus complete Filter/LFO command behavior.

The native DSP, MIDI engine, clock, saved .biscot format and idle optimizations are unchanged.
