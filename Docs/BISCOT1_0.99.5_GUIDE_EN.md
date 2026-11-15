# Biscot1 Synth 0.99.5 — Accessible update guide

**15 November 2026 · Windows x64 VST3**

## Main navigation

- **Alt+A:** open the accessible About dialog; use Tab or Shift+Tab for its read-only text, Project, Contact and Close controls. Escape closes it.
- **Alt+Q:** open the integrated tracker. **F1** selects Pattern editing; **F2** selects Song editing.
- **1/2/3:** inside the *Steps* area, switch operator/section. Outside Steps they do not change the current operator.
- **Space in F2:** play/stop the Song. Stopping restores the position that was active when manual playback started, including after Song chaining.
- **Ctrl+Up at Song Length:** at the upper boundary, does nothing and remains silent.

## New LFO controls (0.99.5)

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

After the Windows build is successful, use the `Biscot1-0.99.5-Windows-x64.zip` build artifact for the GitHub release `v0.99.5`. For sites that recognise `*.vst3.zip` for a direct download, upload the **same ZIP contents** with the release filename `Biscot1-0.99.5-Windows-x64.vst3.zip`. Keep a backup of your saved presets and projects before replacing a previous VST3 build.
