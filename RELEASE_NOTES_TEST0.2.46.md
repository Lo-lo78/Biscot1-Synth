# Biscot1 Synth — TEST0.2.46 (16 November 2026)

## New: AHDSR Hold for all seven SID-style envelopes

- Op1 / Op2 / Op3 amplitude Hold, each independent, added after Attack.
- Op1 / Op2 / Op3 Mod Env Hold, each independent, added after Mod Env Attack.
- Global Filter Hold, added after Filter Attack.
- Range 0 to 5 seconds in 1 ms increments; defaults 0 for old preset sound parity.
- Host parameter IDs s249–s255 appended only; historic IDs unchanged.
- F1 Pattern cells: OP Hold, OP Mod Env Hold and Filter Hold, with Song lane mapping.
- New Hold stage uses SID envelope's existing Attack / Decay / Sustain / Release equations without changing attack/release rate curves. Note Off during Hold enters Release. Pedal and retrigger retain their original logic.
- Existing .biscot preset/project state without Hold loads the seven controls as zero.
- REAPER Ctrl+Shift+A per-parameter automation retains its normal behavior.

**TEST ONLY**: public version remains 0.99.5. Compile full VST3 on GitHub Actions.
