# Biscot1 Synth VST3 — TEST0.2.23 — Native Song editing, phase 2

## Editing and accessibility (F2 Song)

- Shift+Up/Down and Shift+PageUp/PageDown select Song steps; the first keypress selects the current step, following presses extend or shrink the selection.
- Ctrl+C, Ctrl+X and Ctrl+V: copy/cut/paste a selected block of Song steps or the current parameter. Empty Song steps in a copied block remain empty when pasted. Paste starts at the current step/cell.
- Ctrl+Alt+Shift+C copies the entire current Song; Ctrl+V restores it to the selected Song slot while preserving the destination Song name.
- Ctrl+Left/Right reorders parameter groups within a Song step. A Pattern with Pattern Start and Play Length moves as a single group.
- Delete removes a selected block or the current Song parameter. Ctrl+Delete clears the current Song step; Ctrl+Shift+Delete clears the current operator lane; Ctrl+Alt+Shift+Delete resets the current Song.
- Ctrl+Z and Ctrl+Shift+Z provide independent Song histories, ten undo levels per Song (in-memory session, not saved to presets).
- Ctrl+R edits the Song name; Tab announces the name along with the Song number, operator and cursor position.
- 1/2/3 select operator lanes, 4 selects Song global as before. M toggles the selected operator's Song Mute; S toggles Song Solo; + and - adjust Song Volume by one percentage point. These values are stored in the .biscot state. They do not alter synthesizer output until phase 3 connects Song playback.
- Existing F1 Pattern editor, Song parameter picker and Pattern/Song state persistence are preserved.

## Phase boundaries

The Song does not yet play, and Ctrl+S in F2 does not yet export a standalone Song/Patterns package. The audio engine remains unchanged. Phase 3 will supply multi-operator playback; phase 4 will implement library I/O and linked Pattern exports.

## Validation

The new pure C++ Song editing regression test covers grouped Pattern copy, empty-step clipboard, ordered parameters, undo/redo, whole Song paste and destructive editing. A static test checks keyboard routes and unchanged audio status. Full Windows VST3 compilation and NVDA verification remain for GitHub Actions and REAPER.
