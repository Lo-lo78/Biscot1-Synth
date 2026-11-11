# Biscot1 Synth VST3 — TEST0.2.10

Base: TEST0.2.9. This update changes the MIDI scheduling of classical
arpeggiators and ADSR retriggering at arpeggiator clock boundaries.

## Fixes

1. The active pitch pool in classical ARP modes is frozen between pulses.
   Notes inserted via MIDI keyboard or sustained CC64 enter the next clock
   step; they do not audibly change pitch at the moment of MIDI Note On.
2. New keys in an already running ARP phrase do not reset its phase/clock,
   drift or amplitude/modulation envelopes, nor the filter/LFO note trigger.
3. Mode Trigger reattacks the amplitude and modulation ADSR at each subsequent
   arpeggiator step. Mode Legato allows continuous envelopes and uninterrupted
   clock. The first arp step is never double-triggered.
4. Once the sustain pedal releases and the amplitude envelope enters its
   Release phase, additional ARP steps never re-trigger that envelope.
5. SID Seq continues to use its original sequence memory and clock, with
   a tick flag for the same Trigger/Legato behavior.

No new parameter indices, no changes to SID filter, oscillator/wavetable,
analogue output or accessible editor. C++ standalone tests included.

## Git

```cmd
git add . && git commit -m "Biscot1 VST3 TEST0.2.10 quantize arpeggio note updates and fix Trigger Legato" && git push
```
