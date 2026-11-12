# Biscot1 Synth VST3 — TEST0.2.19.3 (12 November 2026)

## Fix: F1 audition when editing a value

The original `Biscot1 TrackerControl 0.166` calls `preview_pattern_step(current_step)`
after `change_value()` modifies a cell. The TEST0.2.19.2 native panel omitted
this call, so a user had to navigate away and return to hear the updated Note,
Gate, OP, Filter or LFO value.

- `Alt+Up/Down`, `Alt+PageUp/PageDown`: listen to the edited step again.
- `Alt+E` numeric entry and `Alt+Home/End` minimum/maximum share the same
  editable cell pathway and also retrigger the audition when the value changes.
- `Backspace` restores the default and auditions the new value.
- No preview at a numeric boundary if the value has not actually changed.
- Preview only steps with an explicit `Note`, matching the Lua's `has_note`
  safeguard. Parameter-only and empty steps are silent in isolated preview.
- During full Pattern playback (`Space`) an edit does not restart or
  interrupt the running transport. A restarted Pattern loads the latest edit.
- Publishing an updated immutable Pattern snapshot happens **before** the
  audition request; no UI data are read from the audio thread.

## Validation

- Standalone C++ test `TrackerPatternEditAuditionTest` covers OP/Filter/LFO
  edits, stopped/paused/audition/full-play statuses, empty and note-less rows,
  new immutable snapshot, and one-step clock restart.
- Static regression `verify_test02193_value_audition.py` checks the actual JUCE
  edit path, boundary guard, and registered GitHub Actions test.
- DSP, MIDI, idle optimizations and `.biscot` serialisation are unchanged.
- Windows VST3 compilation, REAPER playback and NVDA need GitHub Actions and
  a real host check.
