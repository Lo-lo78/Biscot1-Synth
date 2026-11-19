# Biscot1 Synth 0.99.6 — TEST0.2.47: Live automation during Release

**Status:** TEST source based on the user's official 0.99.6 AHDSR source. Not a new public release. Historical versions, VST3 parameter IDs, preset format and GitHub release metadata are unchanged.

## Fixes

- **Op1/Op2/Op3 Release (VCA):** the release time is no longer frozen at MIDI Note Off. REAPER can shorten or lengthen the Release **while the envelope is already releasing**, with no further Note On. An automated zero terminates the envelope on its next audio sample. A completed envelope is never resurrected by increasing Release.
- **Op1/Op2/Op3 Mod Env Release:** the same continuous control now works for modulation envelope tails, including when the corresponding MIDI key has been released.
- **Filter Release:** likewise follows automation throughout the filter envelope's active release stage.
- **Seven AHDSR Hold controls:** a change during the *current Hold plateau* adjusts that plateau's remaining duration. Shortening Hold below elapsed time ends Hold and proceeds into Decay; extending it preserves elapsed time. A Hold change during an already completed stage does not rewind the envelope.
- **Op1/Op2/Op3 Drift:** the random per-note detuning draw remains latched at Note On, but its amount now responds to automation on the sounding or releasing voice. The same random sequence and static pitch amount are preserved when Drift stays unchanged.
- **Noise Character:** recombines the two already-advancing noise streams as soon as the control changes; it no longer waits for the next noise-clock edge. Neither SID nor clean noise clock is reset by automation.

## CPU / compatibility

- The new Release coefficient is recomputed **only when an automated value changes**, not every sample of a fixed Release. Hold duration is recalculated only when Hold changes.
- No new DSP threads, memory allocations, filter resets, Note On events, envelopes, parameters or persistent state were added. Existing cold-idle skipping and free-running LFO clocks remain intact.
- Default sound and the amplitude/filter envelope equations when controls are static are preserved. No changes to CC64 sustain, arpeggiator note memory, multi-output configuration, preset serialization or keyboard accessibility.
- Trigger/Legato, Hard Restart and LFO start phase are **event-based settings by design**; changing their value does not retroactively generate a MIDI event. This test does not pretend they can produce sound after all voices have ended.

## How to test in REAPER

1. Put a long MIDI note on the Biscot1 track, ending before the point where you want the sound to stop. Set Op1 Release to 0.320 or 5.000 seconds.
2. Activate an envelope for **Op1 Release** (Ctrl+Shift+A from the Biscot1 parameter editor) and place an automation point at **0.000** during the Release tail, e.g. 480 MIDI ticks after the Note Off. The voice must stop there, without another note.
3. Reverse the change: increase Release during its tail and verify it continues from the *current envelope level*, without a fresh Attack.
4. Repeat for **Op2 Release, Op3 Release, all three Mod Env Release and Filter Release** (for the modulation/filter controls, configure a clearly audible modulation amount).
5. Automate an **Op Hold** while the Hold stage is in progress, and then **Op Drift** and **Noise Character** on a long sounding note.
6. Check CC64 pedal, Arp release, multi-output channels, existing `.biscot` presets and CPU use while silent.

## Automated verification already performed on the source

- `SidLiveAutomationTest`: 7 early-cuts after Note Off, extended Release, 5-second Release cut, Hold shortening/extension, no ghost notes and unchanged fixed-parameter envelope values.
- 25 C++ standalone regression executables passed on Linux (DSP / MIDI / sustain / LFO / arps / tracker / Song).
- 71 Python verification scripts passed, including historic DSP SHA-256 parity checks with only the explicitly audited TEST0.2.47 changes normalized out.
- **Still required:** Windows VST3 compilation via GitHub Actions and the final playback/NVDA test inside REAPER. This source package is not a compiled plugin.
