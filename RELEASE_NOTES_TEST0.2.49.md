# Biscot1 Synth 0.99.6 — TEST0.2.49: signed Ping Pong Pan per operator

Built on TEST0.2.48, including all live automation and AHDSR Trigger work.

- Appended exactly three host-visible, independent, accessible parameters: **Op1/Op2/Op3 Ping Pong Pan** (IDs s259, s260, s261; range -1..+1; default 0). All previous parameter IDs and host order, including Song Select and Song Transport, are preserved.
- **+1** places the first attack on the right and the next on the left; **-1** starts on the left, then on the right. The magnitude sets the alternate pan width; **0** keeps the classic Pan path identical.
- Per routed MIDI Note On the operator alternates its stereo side (whether VCA is Trigger or Legato). Its side is maintained through its Note Off/Release tail. Operators keep independent states.
- With the new per-operator **AHDSR Trigger On**, *each automatic Attack cycle* also alternates, so a held note makes a rhythmic left/right effect. The oscillator phase, MIDI notes, filter envelope and LFOs do not retrigger.
- A ~3 ms ramp avoids hard stereo jumps/clicks and follows live REAPER Pan Ping Pong automation. This is a signed offset summed with ordinary Pan + ENV Pan modulation + LFO Pan modulation and clamped to [-1,+1], before the original JSFX stereo gain law and output stems.
- Works in stereo/multichannel, regular MIDI/chord/channel routing, tracker Patterns and Songs. New tracker OP Ping Pong Pan (3019) is mapped per song lane.
- Old .biscot/REAPER presets lacking the appended controls get explicit zeros. Host's existing parameter indexes and sound are unchanged when new controls are zero.
- No buffers, allocation, threads, extra DSP for silence or oscillator-phase changes. One tiny smoothing step per active operator per rendered sample.
- Tests: SidPingPongPanTest.cpp, verify_test0249_pingpong.py; plus existing ADSR/Pan/tracker checks.
- Native Windows VST3 build and in-REAPER listening remain to be done.
