# Biscot1 Synth 0.99.5 — LFO Phase Drift & Lace, Tracker and Song Improvements

**Release date:** 15 November 2026  
**Platform:** Windows x64 · VST3  
**License:** AGPL-3.0-or-later

This update brings together the changes developed since the first public 0.99.1 release.

## New in 0.99.5: LFO Phase Drift and Lace

- Five new controls per LFO, available independently on **LFO1, LFO2 and LFO3**: Drift Amount, Drift Cycle, Lace Source, Lace Rate and Lace Mix.
- Drift creates a repeating phase displacement over 1–32 cycles, rather than merely changing the LFO rate.
- Lace can combine an LFO with LFO1/LFO2/LFO3, including the LFO itself, using adjustable rate and mix.
- Available in the accessible parameter pages, Pattern/Song parameter pickers and DAW automation.
- **Backward-compatible defaults:** Drift and Lace are disabled on existing presets.

## Tracker and Song updates from 0.99.2–0.99.4

- Clear **Set,** indication for parameters already inserted in F1 Pattern and F2 Song; Ctrl+Enter in F2 inserts while keeping the picker open.
- **True stereo Ping-Pong delay** throughout its nonzero depth range, with delay tails preserved during Song-row previews.
- Operator shortcut keys **1/2/3** switch operator only when editing Steps.
- Song playback follows Song End chaining correctly; manual Space-to-Stop restores the Song, operator, row and cell where playback started.
- Silent upper boundary when pressing Ctrl+Up on the F2 Song Length header.

## SID-inspired noise clock: experimental change

This source contains an experimental 23-bit SID noise-clock adjustment (16 SID LFSR updates per oscillator cycle while retaining the 4-update clean-noise stream). The musical impact has **not yet been confirmed through a listening comparison**; see `NOISE_SID_23BIT_CLOCK_TRIAL.md` in the source.

## Accessibility and compatibility

Biscot1 Synth remains keyboard-accessible with NVDA. **Alt+Q** opens the integrated Pattern/Song tracker, and **Alt+A** opens the accessible About dialog. Existing songs, presets and host automation remain supported; the new LFO parameters are append-only and disabled by default.

## Download

**Recommended release attachment:** `Biscot1-0.99.5-Windows-x64.vst3.zip` (identical content to the `Biscot1-0.99.5-Windows-x64.zip` build artifact; renamed for direct-download website compatibility).  
**Requirements:** Windows x64 and a VST3 host such as REAPER. The ZIP contains the VST3 plugin bundle.

Source code, issue reports and feedback: https://github.com/Lo-lo78/Biscot1-Synth
