# Biscot1 Synth 0.99.5 — SID 23-bit Noise Clock listening trial

This is an **experimental listening build**, not a new public release.

## Scope
- Correct the 23-bit SID-inspired LFSR stepping rate to **16 updates per oscillator cycle**, corresponding to the rising edge of phase accumulator bit 19 on a 24-bit SID oscillator.
- Preserve the original **4 updates per oscillator cycle** for the modern clean-noise generator, with a separate fractional clock.
- Advance the SID LFSR independently of the Noise Character mix value so that changing or automating the mix cannot change the pseudo-random sequence.
- Keep the existing 23-bit feedback taps, eight-bit output selection, seed, Noise Character range/default, oscillator waves, filters, envelope, LFO Lace and Phase Drift, and all preset parameters unchanged.
- Reset both fractional clocks upon entry into cold idle. No changes to the SID Test Bit or Hard Restart semantics in this experiment.

## Listening comparison in REAPER
1. Use the previous **0.99.5** VST3 as the baseline. Build the test ZIP in the same GitHub workflow and install the resulting VST3.
2. Use exactly the same saved preset and the same MIDI sequence. For a clear difference use a single operator with **Noise** waveform, `Noise Character = 1.0`, filter off or open, no delay and no reverb.
3. Compare sustained noise notes and short percussion at different pitches; the new SID noise sequence should change more rapidly at the same pitch.
4. Repeat with `Noise Character = 0.0`: clean noise should retain its former clock and sequence.
5. Try values between 0 and 1 to hear blending. If the noise is too bright or metallic, keep the original sound and reconsider the clock scaling.

This update has static and deterministic clock-simulation tests. It has **not** been built with MSVC/GitHub Actions or auditioned through REAPER in this environment.
