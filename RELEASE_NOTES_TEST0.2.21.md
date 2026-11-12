# Biscot1 Synth TEST0.2.21 — F1 speech and MIDI Pattern work

Based on TEST0.2.20.1 and legacy Biscot1 TrackerControl / Tracker JSFX 0.166.

- Compact speech when navigating F1: OP/Filter/LFO, step, parameter and value. Full Pattern number is spoken on Tab or when changing Pattern. Switching the three tabs with 1/2/3 likewise does not repeat the Pattern number.
- Restored **Step Recording** and **Release Arpeggio** entries to the Pattern header after Length and MIDI Note. Switch values with Alt+Up/Down or Alt+E (0/1); Step Recording is intentionally transient and not written to the preset.
- Step Recording MIDI Note On writes the note into the selected step and moves to the next. CC64 down (once per pedal press) writes Pause; pedal up re-arms it. Skipped positions become Pause, following the Lua. MIDI input used for recording is isolated from synth playback; only the UI thread updates Pattern data.
- MIDI Note mapped to a Pattern triggers its immutable compiled performance, and mapped Note Off stops it, consuming the mapping notes like the original JSFX. Unmapped notes still reach the synth. In the absence of Song support, only the Pattern map is checked.
- Sub Notes in a step are added to Op1's arpeggio note pool for the first audible Pattern playback, and are released with the primary note. Native pool is bounded to 15 sub notes plus the primary note, respecting the existing 16-event per-operator stack.
- First native Release Arpeggio: after natural completion, repeat the written pitch sequence during the operator/filter's release interval, *without generating any new Note On or retriggering ADSR*. Existing stereo DSP algorithms are unchanged.
- Reused native `.biscot` state format, no migration necessary. F2 Song editor is still reserved for a later version.

## First manual test with NVDA / REAPER

1. F1 arrows and 1/2/3 should announce section/step but not Pattern; Tab must still mention Pattern number.
2. F1 Header: Length, MIDI Note, Step Recording, Release Arpeggio. Use Alt+E or Alt+arrows to set Step Recording On and then navigate to a real step. Send MIDI Note Ons; the cursor should advance. CC64 down writes a Pause.
3. Set MIDI Note in the Pattern header to a trigger key and play that key from the MIDI input: the assigned Pattern should play, and the key's own note should not be heard.
4. Insert OP Arp Mode and multiple Sub Notes in a step. Test with a suitable arpeggiator mode/rate.
5. Enable Release Arpeggio and configure a nonzero OP Release; listen for pitch movement through the release tail after the Pattern ends.

## Scope and limitations

Release Arpeggio and Sub Notes need real REAPER audio tests to confirm full JSFX 0.166 musical parity, especially very long patterns, pedal behavior and interaction with the synth's own arpeggiator. The native three-lane Song motor and cross-preset Pattern browser are not included. Local tests cover JUCE-independent logic; Windows UIA/NVDA and full plugin compilation are checked by GitHub Actions and local REAPER use.
