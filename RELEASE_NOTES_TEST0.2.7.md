# Biscot1 Synth VST3 – TEST0.2.7

## Second JSFX migration parity step: SID Seq

- Mode `SID Seq` for each of three operators now uses a 128-position dynamic frequency sequence with per-operator clock and step position, following `sid_seq_clear`, `sid_seq_record` and `sid_seq_process` in JSFX v2.38.
- New notes are inserted after the step currently playing, including insertion into the middle and overwrite at capacity. First step sounds immediately.
- Chord Allocate sends each incoming note to all enabled SID Seq memories; their clocks remain independent.
- The note snapshot and pitch root are retained for Release in All Operators and MIDI Channels modes and cleared after the voice amplitude envelope completes.
- The JSFX square-of-depth ARP Glide coefficient is applied to SID Seq independently.
- Retained the VST3's existing Up, Down, Pendulum, Held Order and Random algorithms, the user-validated SID oscillator/filter/output DSP, keyboard navigation, preset browser, and append-only Pitch Bend Mode.
- Adds a JUCE-independent C++ regression executable `SidSeqMemoryTest` and GitHub Actions validation.

**Not yet ported:** full JSFX details for non-SID Seq arpeggio modes, global filter/trigger and LFO state corner cases, complete idle CPU optimization, and tracker Alt+Q. Full Windows compilation and a MIDI/REAPER listening test remain necessary.
