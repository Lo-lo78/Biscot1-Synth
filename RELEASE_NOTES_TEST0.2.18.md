# Biscot1 Synth VST3 — TEST0.2.18

## First native tracker stage

- Alt+Q opens a native JUCE/Windows UIA Pattern Editor F1 and parameter picker F2.
- Uses the actual user-supplied Biscot1 TrackerControl 0.166 parameter list,
  Pattern model and editing navigation as the reference.
- 1000 indexed Patterns, 1024 steps per Pattern, up to 128 chronologically
  ordered cells per step across OP / Filter / LFO sections.
- Sparse step memory with editable Pattern Length, MIDI Note, Repeat,
  per-repeat Speed and Transpose. Default OP cells follow the Lua.
- Numeric entry, parameter insertion, delete, reorder, copy/paste,
  Pattern undo/redo, and section switching.
- Saves Pattern data directly into the VST3 state. This is also captured by
  the established Alt+S .biscot preset format and REAPER project state.
  No background Lua startup loader is required for this native data.
- Legacy Tracker JSFX, Lua Controller and Loader shipped in Reference/LegacyTracker.
- Audio/MIDI DSP (including deep idle CPU optimisation), all 232 existing
  parameter IDs, original synth editor and preset-browser format preserved.

## Scope: initial F1/F2 only

There is deliberately no Pattern playback or Song editor in this release.
F3–F6, step recording, independent Ctrl+S export and the new preset-folder
library will arrive in later versions. Pattern cells are not yet connected to
live synthesizer automation. Do not test this version as a functioning sequencer.

## Verification

Python static tests and 14 standalone C++ tests pass on Linux, including the
native TrackerPatternBankTest. Full Windows JUCE compilation and NVDA/REAPER
keyboard-focus testing still require GitHub Actions and installation.

## Git command

    git add . && git commit -m "Biscot1 VST3 TEST0.2.18 start native tracker F1 F2 and persistent Pattern bank" && git push
