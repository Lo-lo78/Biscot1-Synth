# Biscot1 Synth 0.99.6 — Accessible update guide

**16 November 2026 · Windows x64 VST3**

## New in 0.99.6 — AHDSR Hold (all seven envelopes)

All ADSR envelopes now add **Hold** after Attack and before Decay, independently:

- **Op1, Op2, Op3 Hold:** operator amplitude envelopes.
- **Op1, Op2, Op3 Mod Env Hold:** operator modulation envelopes.
- **Filter Hold:** shared filter envelope.

Each Hold runs from **0 to 5000 ms**, adjusted in **1 ms** steps; **0 ms** is the default, preserving earlier preset sound. During Hold the envelope stays at the peak reached by Attack, then Decay continues toward Sustain. Note Off starts Release even during Hold.

The main keyboard-accessible parameter pages place Hold immediately after Attack. In Alt+Q / F1 Pattern the parameter picker offers **OP Hold, OP Mod Env Hold and Filter Hold**, with the existing Song-lane mapping. Existing VST3 parameter IDs are preserved; the new IDs are 249–255.

## New in 0.99.6 — Accessible REAPER automation

From a main parameter page, select a parameter and press **Ctrl+Shift+A** in Grid, Value or Edit. This toggles the **active state** of its REAPER parameter envelope without removing its saved points. When active, NVDA announces **`, Auto`**. This is supported by all seven Hold parameters. The shortcut does not modify the Alt+Q tracker.

## Main navigation

- **Alt+A:** open the accessible About dialog; use Tab or Shift+Tab for its read-only text, Project, Contact and Close controls. Escape closes it.
- **Alt+Q:** open the integrated tracker. **F1** selects Pattern editing; **F2** selects Song editing.
- **1/2/3:** inside the *Steps* area, switch operator/section. Outside Steps they do not change the current operator.
- **Space in F2:** play/stop the Song. Stopping restores the position that was active when manual playback started, including after Song chaining.
- **Ctrl+Up at Song Length:** at the upper boundary, does nothing and remains silent.

## LFO controls inherited from 0.99.5

Each of LFO1, LFO2 and LFO3 has five additional, independent controls:

1. **Drift Amount (0–1; default 0):** progressively shifts phase between LFO cycles.
2. **Drift Cycle (1–32):** number of cycles before the phase-shift pattern repeats.
3. **Lace Source (Off, LFO1, LFO2, LFO3; default Off):** selects the cross-LFO or self-LFO source.
4. **Lace Rate (1–32):** subdivision ratio used by Lace.
5. **Lace Mix (0–1; default 0):** blends the Lace signal. Off/zero values preserve earlier presets' behavior.

These controls are exposed in the accessible LFO parameter pages and the Pattern/Song LFO parameter pickers, including DAW automation. LFO cycles remain host-synchronized as before.

## Other changes since version 0.99.1

- F1/F2 parameter pickers: short **Set,** label on already-inserted parameters. Ctrl+Enter inserts in F2 without leaving the picker; Enter returns to the step.
- Ping-Pong delay: circulating stereo echoes throughout nonzero depth values; Song-row audition uses the effects active at that row and lets delay tails ring out.
- F2 manual playback: the current position follows chained Songs, and Stop restores the starting Song, operator, row and cell.

## SID noise listening trial

This source snapshot also changes the SID-inspired 23-bit noise clock. Its tonal result is experimental and should be auditioned before final public distribution. Details: `NOISE_SID_23BIT_CLOCK_TRIAL.md` at the repository root.

## Updating

After the Windows build is successful, use the `Biscot1-0.99.6-Windows-x64.zip` build artifact for the GitHub release `v0.99.6`. For websites that require `*.vst3.zip`, attach the **identical archive contents** under `Biscot1-0.99.6-Windows-x64.vst3.zip`. Keep backups of presets and DAW projects.
