# Biscot1 Synth 0.99.5 — LFO Phase Drift and Lace

- Five new parameters on each of LFO1, LFO2 and LFO3: Drift Amount (0–1, default Off), Drift Cycle (1–32 cycles), Lace Source (Off/LFO1/LFO2/LFO3), Lace Rate (1–32) and Lace Mix (0–1, default Off).
- Drift shifts each successive LFO cycle in phase and repeats after the selected integer number of cycles. Unlike a rate multiplier, a larger Cycle value lengthens the complete repeating phrase. Works with Sine, Triangle, Saw Up, Saw Down, Square and sample-based waveforms.
- Lace alternates subdivisions of the current wave and another LFO; Mix sets the blend, with one-sample source history for deterministic feedback-safe routing.
- Both functions are disabled on all existing presets and remain clocked from the existing host-synchronized LFO rate. Note trigger and host restart reset Drift cycles; cold idle preserves Free cycle continuity.
- Available in the accessible main LFO pages and Pattern/Song LFO parameter pickers, including host automation.
- Original wave, filter, tracker, song, and audio effect behavior remains unchanged with the new controls Off.
- This is a Biscot1 extension inspired by SIDStation techniques, not a claim of exact SIDStation DSP emulation.
