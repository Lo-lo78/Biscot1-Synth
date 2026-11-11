# Biscot1 VST3 — TEST0.2.12

Restored the original JSFX behaviour: both SID Seq and the five classical arpeggiator modes continue stepping over the last held chord **during the ADSR Release**. The clock keeps the existing BPM and notes remain latched until the envelope has completed its natural exponential release, after which the memory is cleared. This includes two cases: no CC64 and CC64-up after having held a chord with the sustain pedal. No per-step retrigger or hard mute is introduced; Trigger/Legato keeps its musical-sequence meaning.

The incorrect gate-only transport check in TEST0.2.11 has been removed. Added standalone C++ release regression and automated source audit; GitHub Actions runs both before creating the VST3 artifact. Core DSP and GUI are unchanged. Real-time REAPER/NVDA testing remains necessary.

Git:

```cmd
git add . && git commit -m "Biscot1 VST3 TEST0.2.12 restore arp during ADSR Release" && git push
```
