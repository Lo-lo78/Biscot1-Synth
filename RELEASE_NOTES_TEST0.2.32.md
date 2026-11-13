# Biscot1 Synth VST3 — TEST0.2.32

## Named library exports and accessible Space echo

- Alt+Q F1 Ctrl+S opens the actual Pattern filename TextEditor, selected initial text, Enter accepts and Esc cancels.
- Alt+Q F2 Ctrl+S opens the portable single-Song filename editor; a successful save also updates that Song's internal display name.
- Alt+Q F2 Alt+S opens the full 100-slot Song-bank filename editor, without renaming any individual Song.
- In these fields, typing a space inserts it and explicitly announces `spazio` to the active screen reader through the focused TextEditor; Song Ctrl+R rename uses the same speech improvement.
- The existing synth `.biscot` preset browser also echoes `spazio` during save, rename, and new-folder naming.
- File naming sanitizes invalid Windows characters, path traversal and device names, preserves intentional spaces, and keeps suffix protection against unrelated existing files.
- Unchanged: `.biscot1-pattern`, `.biscot1-song`, `.biscot1-all-song`; stored Pattern references and static 100-song slot import; synth/host preset `.biscot`; Song End, Preview Steps and DSP.
- F1 Alt+S still opens the separate `.biscot` preset browser.

## Pending

- Native accessible browser with sound previews (currently native file chooser on Ctrl+O).
- Complete GitHub Actions Windows/MSVC VST3 build and NVDA/REAPER functional audition.
