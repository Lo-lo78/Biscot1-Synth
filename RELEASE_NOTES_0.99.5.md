# Biscot1 Synth 0.99.5 — LFO Phase Drift and Lace

**Release date:** 15 November 2026. **Platform:** Windows x64 / VST3. **License:** AGPL-3.0-or-later.

This release also includes the changes documented in 0.99.2, 0.99.3 and 0.99.4, which were not previously published as stable GitHub releases.

- Five new parameters on each of LFO1, LFO2 and LFO3: Drift Amount (0–1, default Off), Drift Cycle (1–32 cycles), Lace Source (Off/LFO1/LFO2/LFO3), Lace Rate (1–32) and Lace Mix (0–1, default Off).
- Drift shifts each successive LFO cycle in phase and repeats after the selected integer number of cycles. Unlike a rate multiplier, a larger Cycle value lengthens the complete repeating phrase. Works with Sine, Triangle, Saw Up, Saw Down, Square and sample-based waveforms.
- Lace alternates subdivisions of the current wave and another LFO; Mix sets the blend, with one-sample source history for deterministic feedback-safe routing.
- Both functions are disabled on all existing presets and remain clocked from the existing host-synchronized LFO rate. Note trigger and host restart reset Drift cycles; cold idle preserves Free cycle continuity.
- Available in the accessible main LFO pages and Pattern/Song LFO parameter pickers, including host automation.
- Original wave, filter, tracker, song, and audio effect behavior remains unchanged with the new controls Off.
- This is a Biscot1 extension inspired by SIDStation techniques, not a claim of exact SIDStation DSP emulation.

## GitHub Actions build fix (same version)

- Corrected one unmatched closing brace inside `BiscotProcessor::processLfo()` in `Source/PluginProcessor.cpp` that caused MSVC C2059 at `return baseWave;` and cascading compiler errors.
- Added a dedicated scope regression check to the Windows workflow. LFO DSP behavior and parameter defaults are unchanged.

## Included improvements since the 0.99.1 public release

- **0.99.2:** F1/F2 picker Set indicator and Ctrl+Enter feedback, true stereo Ping-Pong delay at intermediate depths and Song-row-aware effect preview.
- **0.99.3:** operator-switching digits only inside tracker Steps; manual Song transport remembers the actual position reached after Song End chaining.
- **0.99.4:** silent Ctrl+Up at the Song Length header boundary; Play/Stop restores the exact starting Song, row, operator and selected cell.

## SID noise-clock experiment in this source snapshot

The supplied source includes a 23-bit SID-inspired noise-clock experiment: the SID LFSR advances at 16 updates per oscillator cycle while the clean-noise generator retains 4 updates per cycle. Clock advance is independent of the Noise Character blend. This is documented in `NOISE_SID_23BIT_CLOCK_TRIAL.md` as a **listening trial**, not an already user-verified sonic improvement. Compare with earlier builds before treating its tonal change as final.

## Installation and release artifact

Build with the existing GitHub Actions Windows workflow and check that it passes. Download its `Biscot1-0.99.5-Windows-x64.zip` artifact and attach the matching ZIP to the stable GitHub release tagged `v0.99.5`. If the website requires a `*.vst3.zip` filename for direct downloads, rename an **identical copy** to `Biscot1-0.99.5-Windows-x64.vst3.zip` before attaching it; do not rebuild or change its contents. Do not attach an older build with a mismatched version. The archive contains the Windows x64 VST3 plugin bundle.
