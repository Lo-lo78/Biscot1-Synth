# Biscot1 Synth 0.99.6 — TEST0.2.52: Pitch-locked Ping Pong Pan

This builds on TEST0.2.51; public plugin version stays 0.99.6.

## Audible issue

Ping Pong Pan previously used an independent, fixed ~3 ms stereo smoothing
regardless of the oscillator's Portamento or internal arp Glide. This made
fast glide steps sound late, and made hard/square pitch changes lag by ~3 ms.
The previous first-Attack-only snap fixed leakage at ADSR onset but did not
solve the mismatched pan/pitch trajectories in legato arpeggio steps.

## Fix

- The existing three signed [-1,+1] Ping Pong Pan parameters are unchanged.
- For each active oscillator the Pan side now follows the **actual semitone
  progress** calculated on that same audio sample by:
  - per-operator Portamento (`Op1/2/3 Portamento`),
  - native SID Seq (`Op1/2/3 Arp Glide Depth`),
  - the classical arpeggiator glide (Up/Down/Up-Down/Press Order/Random).
- No portamento or glide, or no moving pitch: stereo switches **immediately**
  on the same sample as the hard pitch change (including a new Attack).
- With an active glide: stereo uses the same normalized per-sample movement,
  not a separate fixed time. The curves remain locked even for very fast
  glide or when glide is automated while a note is playing.
- If a new arpeggio step occurs before the previous glide finishes, the Pan
  retargets from the actual intermediate position, without an extra delay.
- If both Portamento and Arp Glide move at the same time, their combined
  actual semitone motion determines the single Pan trajectory (with a safe
  fallback for opposing motions that cancel numerically).
- The existing AHDSR Trigger and even-size arpeggio fixes remain in place.
  One simultaneous AHDSR reattack and arp step counts as one Pan transition.
- Independent live automation of the *Ping Pong Pan amount* still uses the
  short 3 ms de-click; this no longer introduces timing lag on stereo side
  transitions caused by a pitch/arp step.

## CPU, ABI and compatibility

- No new parameters, buffers, threads, latency, or allocations.
- Legacy presets with all Ping Pong Pan amounts at zero skip the extra glide
  motion calculation entirely.
- Existing parameter IDs, factory values, tracker/Song mappings, host state,
  ADSRs, noise, phase and original oscillator pitch equations are unchanged.

## Regression

- `SidPitchGlidePanSyncTest.cpp` checks square/hard steps, exact normalized
  sample-by-sample matching across native SID Seq, classical arpeggiator,
  operator Portamento, fast glide, live speed changes, unfinished glide
  retargeting, mixed Portamento+Arp Glide and signed stereo directions.
- `verify_test0252_glide_pan_sync.py` guards native processor wiring and CI.
- The original `audio_reference.py` checksum still verifies that unchanged
  historical DSP calculations have not been modified.
- Older 0.2.49, 0.2.50 and 0.2.51 tests have their static assertions updated
  for the new synchronized routing; they continue to test the same behavior.

Windows VST3 compilation and real listening tests in REAPER remain to be done.
