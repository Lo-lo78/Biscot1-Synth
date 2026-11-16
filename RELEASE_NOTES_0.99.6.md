# Biscot1 Synth 0.99.6 — Seven AHDSR Hold Envelopes and REAPER Automation

**Release date:** 16 November 2026. **Platform:** Windows x64 VST3. **License:** AGPL-3.0-or-later.

## AHDSR Hold: all seven envelopes (TEST0.2.46)

- Three independent **operator amplitude** envelopes: Op1 Hold, Op2 Hold, Op3 Hold.
- Three independent **operator modulation** envelopes: Op1 Mod Env Hold, Op2 Mod Env Hold, Op3 Mod Env Hold.
- One shared **Filter** envelope: Filter Hold.
- Every envelope now has Attack, **Hold**, Decay, Sustain and Release. Hold maintains the level reached after Attack before Decay begins.
- Range: 0–5 seconds, at 1 ms resolution. Every Hold defaults to **0 seconds**, keeping the old envelope sound and existing preset/project behavior.
- A Note Off during Hold starts Release. Existing SID-inspired envelope curves, legato/retrigger and per-operator CC64 sustain-pedal handling are retained.
- The seven VST3 host parameters were **appended at IDs 249–255**; earlier parameter IDs were not renumbered. Old `.biscot` presets without Hold initialize the new controls to zero.
- The native tracker F1 Pattern parameter picker supports **OP Hold**, **OP Mod Env Hold**, and **Filter Hold**, including existing Song lane mapping.

## REAPER accessible parameter automation (TEST0.2.45)

- In the main parameter pages, **Ctrl+Shift+A** activates/deactivates the selected VST3 parameter's REAPER envelope (from Grid, Value or Edit).
- Automation points already recorded are preserved: it changes the active state, not just the envelope's visibility.
- NVDA announces the `, Auto` suffix when automation is active. The new Hold parameters have the same automation support.

## Preserved features, compatibility and accessibility

- The three LFOs keep 0.99.5 **Phase Drift / Lace**, including self-Lace modulation.
- SID-inspired waveforms, filter topology, arpeggiator, Patterns, Songs, preset manager and audio effects are unchanged by the 0.99.6 release packaging.
- Alt+A provides an updated read-only About dialog. Documentation is updated in Italian and English; original release and TEST histories are retained.
- The 23-bit noise-clock listening trial already present in the source remains experimental, not a claim of a separately verified sound improvement. See `NOISE_SID_23BIT_CLOCK_TRIAL.md`.

## Build and publication

Run the Windows GitHub Actions workflow. It should create the artifact **`Biscot1-0.99.6-Windows-x64.zip`** containing the VST3 plugin bundle. For a website that expects `*.vst3.zip` links, attach an identical copy named **`Biscot1-0.99.6-Windows-x64.vst3.zip`** to the GitHub release tagged **`v0.99.6`**. No change to the archive contents is required.

Full native Windows VST3 compilation and functional audio/NVDA checks must still pass on GitHub Actions and in REAPER before publication.

Developer notes: `RELEASE_NOTES_TEST0.2.45.md` and `RELEASE_NOTES_TEST0.2.46.md`.
