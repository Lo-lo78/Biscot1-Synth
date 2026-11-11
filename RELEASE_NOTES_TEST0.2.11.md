# Biscot1 Synth VST3 — TEST0.2.11

Fixed arpeggiator Trigger/Legato and natural Note Off release from TEST0.2.10.

- Trigger restarts the arp **pattern**, not the operator ADSR: the restart is scheduled on the next grid edge and BPM remains continuous.
- Legato leaves the running arp step counter and phase unchanged.
- Classical arps and SID Seq do not reattack the ADSR on each clock step.
- Without CC64, final Note Off stops the arp clock and the last frequency decays through the configured ADSR Release. With CC64 held, the arp keeps advancing until pedal release.
- In Chord Allocate, Op1 Mode Trigger is shared as before.
- No change to SID waves, output amp, filter, envelopes, parameter IDs or interface.

Git command:

```cmd
git add . && git commit -m "Biscot1 VST3 TEST0.2.11 correct arp Trigger Legato and final note release" && git push
```
