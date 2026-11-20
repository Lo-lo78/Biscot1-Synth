# Biscot1 Synth 0.99.6 — TEST0.2.51: immediate Ping Pong Pan at AHDSR Attack

Based on TEST0.2.50; test source only, public 0.99.6 version unchanged.

## Audible defect

The Ping Pong Pan side was correctly advanced when the gate/clock requested a new Attack, but the output pan offset then moved through the original 3 ms smoothing filter. With an AHDSR Trigger cycle or a retriggered MIDI note, this let a very short fragment of the fresh Attack appear on the **previous** stereo side before reaching the intended one.

## Change

- `SidPingPongPan::advanceAtAttack(signedDepth)` flips the per-operator side and sets the offset to its full signed target **on the same audio sample**. The subsequent output `tick()` does not add an unnecessary ramp.
- On the automatic AHDSR Trigger (amp stage 0 + held gate), the DSP records the reattack in the envelope stage and commits the new pan side before rendering/mixing the first Attack sample.
- In normal Arp Off note routing, a new silent/resetting VCA Attack also commits the target side immediately. A legato/retrigger event that carries over a nonzero envelope does **not** hard-switch its pan: it retains click-reducing smoothing.
- Internal SID Seq and classical arpeggio grid edges that **do not** retrigger the amp keep the existing 3 ms de-clicking ramp, rather than jumping abruptly across a sustained waveform.
- Coincident AHDSR reattack and arp grid edge still count as **one** alternation, preserving the TEST0.2.50 even-note arpeggio fix.
- Depth automation still uses the previous smooth ramp; the three signed [-1,+1] parameters and their indexes are unchanged. Zero depth remains exactly transparent.
- No delay buffers, allocations, additional threads or oscillator phase resets. The new branch only applies to events already detected at audio rate.

## Regression

- `SidPingPongAttackSyncTest.cpp` tests first audible samples for all three operators, both signed starting sides, repeated cyclic reattacks, full stereo isolation, smoothing on non-retriggering arp steps, and smooth depth automation.
- `verify_test0251_pan_attack_sync.py` validates native processor wiring, parameter count and Windows CI inclusion.
- The historical byte-for-byte DSP comparison removes *only* the audited routing additions; unchanged DSP equations remain checksum guarded.

Windows VST3 compilation and actual playback verification in REAPER still need running via GitHub Actions and listening tests.
