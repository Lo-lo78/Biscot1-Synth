# Biscot1 Synth 0.99.7 — AHDSR Trigger and Pitch-Synchronized Ping Pong Pan

**Release date:** 21 November 2026. **Platform:** Windows x64 VST3. **License:** AGPL-3.0-or-later.

This release consolidates the changes tested in **TEST0.2.47 through TEST0.2.52** following the public 0.99.6 source. The DSP and host-facing controls are taken from TEST0.2.52; this packaging update only changes About, version metadata, guides, release text and corresponding CI checks.

## Live automation for active voices (TEST0.2.47)

- All three **operator VCA Release** envelopes, all three **Mod Env Release** envelopes and the shared **Filter Release** follow REAPER automation *while a release tail is already sounding*, even after MIDI Note Off. An automated Release of 0 ends that tail immediately; increasing Release during an active tail extends it without restarting Attack. Fully finished envelopes do not restart.
- Any of the **seven Hold** parameters can be shortened or extended during its active Hold stage, without rewinding completed stages.
- **Op1/2/3 Drift** changes affect existing sounding/releasing voices while the per-note random draw remains fixed. **Noise Character** changes take effect without waiting for a new noise clock edge.
- Event-triggered modes are not retroactive: automation cannot manufacture a MIDI Note On after all voices have ended.

## Independent operator AHDSR Trigger (TEST0.2.48)

- **Op1 AHDSR Trigger**, **Op2 AHDSR Trigger**, and **Op3 AHDSR Trigger** are accessible Off/On parameters (IDs **256–258**, default Off).
- When On and the operator MIDI gate remains held, its *amplitude* envelope repeatedly runs Attack → Hold → Decay → a one-sample Sustain transition → Release, then starts a new Attack after its Release finishes. Turning off the gate or releasing CC64 ends the cycle with a natural Release.
- This does not retrigger oscillator phase or the three modulation envelopes/filter envelope. Off preserves previous AHDSR behavior.
- Available in REAPER automation and the Alt+Q F1 Pattern picker as **OP AHDSR Trigger**, with existing per-Song-lane mapping.

## Signed Ping Pong Pan for all three operators (TEST0.2.49–52)

- **Op1 Ping Pong Pan**, **Op2 Ping Pong Pan**, **Op3 Ping Pong Pan**: IDs **259–261**, range **-1 to +1** (0.01 steps), default **0**. At positive depth the first attack is on the right; at negative depth the first attack is on the left. Absolute depth defines stereo excursion. Values are offsets added to the existing operator Pan/modulation, not a replacement for it.
- The three operators alternate their stereo sides independently on ordinary notes and automatic AHDSR Trigger reattacks. The selected side is held through Release.
- Internal **SID Seq** and **classical arpeggiator** modes alternate at each *actual audible step*, not just at incoming MIDI Note On; this also works with **2, 4, 6 or 8 held notes** and avoids double-flipping when arp and AHDSR edges coincide.
- When the pitch **jumps**, including a new unglided Attack, the pan switches on the *same audio sample*. When **operator Portamento** or **Arp Glide** is active, the stereo movement follows the **actual per-sample semitone progression** of the pitch, including live speed changes, incomplete/retargeted steps and simultaneous glide sources. No fixed 3 ms lag is added to pitch-driven side changes.
- Only **live automation of the Ping Pong Pan amount itself** retains a short ~3 ms de-clicking ramp. With Pan depth 0, the original stereo path is unchanged.
- The new control is available in accessible operator parameters, the Alt+Q F1 Pattern picker (**OP Ping Pong Pan**), Song lane mapping and REAPER automation.

## Compatibility

- The six new host parameters are **appended only**, at IDs **256–261**, following 0.99.6's seven Hold parameters (249–255). Older parameter IDs and ordering remain unchanged.
- Older `.biscot` presets and saved REAPER projects lacking the new switches default to **Trigger Off / Ping Pong Pan 0**. No existing preset data, Pattern/Song format or tracker arrangement is changed by the packaging.
- Existing **Ctrl+Shift+A** accessible active-envelope toggle (preserving automation points), **`, Auto`** NVDA indicator, AHDSR Hold, all three LFO Phase Drift/Lace sections, CC64 handling and multi-output routing remain supported.
- The source's **23-bit SID noise-clock listening trial** is still experimental; no verified tonal improvement is claimed.

## Build and publication

- GitHub Actions builds **`Biscot1-0.99.7-Windows-x64.zip`**; the tag for publication is **`v0.99.7`**. If the website requires `.vst3.zip`, attach an identical copy as **`Biscot1-0.99.7-Windows-x64.vst3.zip`**.
- The source includes standalone and Python regression tests for TEST0.2.47–52. Native Windows VST3 build, playback, automation and NVDA behavior should still be confirmed in REAPER before publishing.

**Prior public release:** [RELEASE_NOTES_0.99.6.md](RELEASE_NOTES_0.99.6.md). Detailed development notes are preserved as `RELEASE_NOTES_TEST0.2.47.md` through `RELEASE_NOTES_TEST0.2.52.md`.
