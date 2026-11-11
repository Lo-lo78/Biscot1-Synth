# Biscot1 Synth VST3 — TEST0.2.9

## CC64 and arpeggiators
- On MIDI CC64 Down, any operator with Sustain Pedal On keeps its released notes in an independent, channel-aware arpeggio pool. Physical key release no longer removes them from Up, Down, Up/Down, Held, or Random arpeggio modes.
- With pedal held, incoming notes join the current pool. SID Seq records new notes at the existing clock step without clearing its recorded 128-step memory.
- CC64 Up removes pedal-retained notes for that MIDI channel. The last ARP pool continues clocking over the natural Release tail until the VCA finishes.
- Disabling an operator's Sustain Pedal switch also clears its retained arp notes. All Notes Off clears everything.
- This is still a three-operator SID synth, not 16-part multitimbrality; pitch bend routing is unchanged.

## Chord Op1 Controls
- A new stable parameter `s230`, `Chord Op1 Controls`, is placed next to Pitch Bend Mode in Global / MIDI.
- Values: **Independent (default)** / **Op1 Master**.
- Only in `Chord Allocate 3 Mono Ops`, Op1 Master uses Op1 values for **Portamento, Attack, Decay, Sustain ADSR, Release, Sustain Pedal (CC64 enable), Level, and PWM** on all three operators.
- No parameter automation IDs are moved. Op2 / Op3 stored values are not overwritten; switching back to Independent restores them.
- Oscillator wave, tuning, pitch bend, pan, modulation, LFO and arp rates remain operator-specific.
- The preexisting Chord Allocate Op1 Trigger / Legato rule is preserved in both settings; retriggering held or pedal-latched notes in Trigger mode now starts a new VCA attack.
- MIDI Modes other than Chord Allocate ignore the new selector.

## Verification limits
Independent C++ tests cover arp-note retention, channel clearing, SID Seq extension, Trigger/Legato, and master mode selection. Static source checks cover the DSP routing and unchanged JSFX original catalog. Actual REAPER/NVDA testing and Windows JUCE build remain required.
