# Biscot1 VST3 TEST0.2.18.2 — Tracker navigation and F1 migration

Based on the verified TEST0.2.18.1 MSVC C2131 fix. No changes to audio DSP,
MIDI event processing, CPU idle, existing synth parameters or tracker bank format.

## F1 — Pattern step grid
- Home moves backward through Speed → Repeat → the last Step → Length.
- End moves forward through Length → first Step → Repeat → Speed.
  These directions are the user's requested reversal of TEST0.2.18.1.
- Shift+Up/Down or Shift+PageUp/PageDown selects steps (the **first** Shift+arrow
  selects the current step; subsequent presses extend the range).
- Ctrl+C copies a selected range of whole steps (OP, Filter and LFO);
  Ctrl+X cuts it, Ctrl+V pastes starting at the focused step. A single
  parameter remains independently copyable when no step selection is active.
- Delete on a step selection clears the selected steps. Ctrl+Delete initializes
  only the currently selected section at the current step (OP gets its eight
  initial cells, Filter/LFO get empty cells); Ctrl+Shift+Delete clears the
  selected section throughout the Pattern.
- Each edit remains a single per-Pattern undo event (Ctrl+Z / Ctrl+Shift+Z).
- F1 data persists in the VST3 saved state, presets and REAPER projects.

## F2 — Parameter grid
- Plain Home/End retain **exactly** the existing current-column behavior.
- Ctrl+Home selects the first parameter in the current OP/Filter/LFO list.
- Ctrl+End selects the last parameter in that list.
- Multi-character incremental keyboard search matches prefixes of the whole
  name or individual words (for example `Rate`, `Cutoff`, `Square`), with a
  1000ms typing window, repeated-letter cycling and Shift reverse.
- Ctrl+Tab continues switching sections, without changing the grid layout.

## Three LFOs in one tab
- This is the original Lua design: F2 lists LFO1, LFO2 and LFO3 controls
  consecutively within one LFO section. They are not mandatory/fixed cells.
  Add an LFO1 Rate cell, LFO2 Mode cell, and LFO3 Square PWM cell on the same
  step in any order. The Song's LFO Owner routing belongs to future F3/F4.

## Still pending
The native tracker does not yet have a playback engine, Song F3/F4, browser
F5, standalone Pattern/Song exporting, or diagnostics F6. This is a UI/data
migration build only. Test focus and NVDA with the complete GitHub Windows VST3.

## Git
`git add . && git commit -m "Biscot1 VST3 TEST0.2.18.2 fix tracker navigation and add search and step clipboard" && git push`
