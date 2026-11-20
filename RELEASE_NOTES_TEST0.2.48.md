# Biscot1 Synth — TEST0.2.48 (AHDSR Trigger per operator)

Based on the official 0.99.6 sources plus TEST0.2.47 live envelope automation.

- Three new **Off/On** controls: `Op1 AHDSR Trigger`, `Op2 AHDSR Trigger`, `Op3 AHDSR Trigger`.
- **Off (factory default):** absolutely unchanged standard AHDSR, including sustain gate, natural release, CC64, legato and MIDI/arp behavior.
- **On:** with an active operator gate, the VCA envelope cycles through Attack, Hold, Decay, a one-sample Sustain transition, and a natural Release. When the Release falls below the existing silence threshold, the Attack starts again **without a new Note On**. On the final Note Off (or CC64 release, if active) looping stops and the current envelope finishes its natural Release. No ghost notes.
- The additional switches do not retrigger oscillator phase or noise, and do not change the three Mod ENVs or shared filter AHDSR.
- Accessible native parameter navigation: each switch follows its operator's `Release`. Available to REAPER automation and Pattern tracker parameter picker (`OP AHDSR Trigger`).
- New parameter IDs `s256`–`s258` are appended after existing Song Select / Song Transport; all earlier REAPER parameter indices are retained. Older presets omit these controls and explicitly load them as Off.
- No extra threads, allocations, new event schedulers or extra processing in silence; a few stage checks only on already-active voices.
- Standalone JUCE-free C++ test covers looping, MIDI gate release, default Off parity, and extreme Release values; CI includes the new regression test.

**Limitations:** While On, the Sustain stage is an instantaneous transition, not a timed hold. Sustain still defines the level from which Release begins; Hold already defines an adjustable peak plateau. Very long Release values (exponential time constants) produce long cycles. The Windows VST3 must be compiled and listened to in REAPER to verify the musical effect.
