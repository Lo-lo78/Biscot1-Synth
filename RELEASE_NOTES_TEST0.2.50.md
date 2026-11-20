# Biscot1 Synth 0.99.6 — TEST0.2.50: Ping Pong Pan on arpeggiator steps

Based on TEST0.2.49. This is a test source update, not a public version bump.

## Bug corrected

When a user held 2, 4, 6 or 8 notes with an internal arpeggiator, the signed per-operator Ping Pong Pan could appear fixed. TEST0.2.49 flipped the stereo side for each incoming MIDI Note On, but an internal arpeggiator retuned the oscillator at its own clock edges without flipping pan. Even numbers of held MIDI notes therefore canceled the initial flips, leaving a constant stereo side during playback.

## Fix

- **Arp Off**: pan continues to alternate on actual routed MIDI Note On events, including non-arpeggiated tracker patterns and external MIDI arpeggios.
- **SID Seq**: pan now alternates on each `SidSeqMemory::lastTick` step event, independent of held note count or the note's pitch.
- **Classical arp modes**: pan now alternates on each valid `SidQuantizedArp::Event::edge` event, including up/down/random and the first scheduled step.
- Adding/removing physical notes during an active internal arpeggio no longer advances ping-pong until its next audible grid step. Trigger/Legato arpeggio clock timing is unchanged.
- The per-operator AHDSR Trigger continues to alternate pan on automatic cycles. If an AHDSR reattack and arpeggiator step coincide on one audio sample, their requests are merged into one side change (avoids an accidental double flip).
- Existing signed depth -1..+1, three independent operator states, 3 ms pan smoothing, song/tracker routing, parameter IDs/defaults, host order, and preset compatibility are preserved.
- No new allocations, synchronization, background threads, sample buffers or oscillator phase resets. The change uses two existing arpeggiator edge flags and three per-sample booleans. No new work in silence.

## Tests

- New `SidPingPongArpStepTest.cpp`: real clock stepping for 2/4/6/8 held notes, five classical arpeggiator modes, SID Seq, both signed directions, and merged AHDSR/arp edge. Runs on GitHub Windows workflow.
- New `verify_test0250_arp_pingpong.py`: processor and CI wiring checks.
- The legacy bit-exact DSP reference check has narrowly audited normalization for these newly added edge-routing statements; unchanged DSP equations retain their original fingerprint.

Windows VST3 compilation and REAPER listening still require verification.
