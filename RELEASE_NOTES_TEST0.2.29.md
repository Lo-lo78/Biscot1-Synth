# Biscot1 TEST0.2.29 — All-Songs standalone library

Based on TEST0.2.28, retaining Song End chaining, audition, navigation memory and self-contained package import/export.

- F2 Ctrl+S exports **all instantiated Songs in the Song bank**, regardless of Song End links, with every referenced Pattern embedded in **one** `.biscot1-song` file under `Documents/Biscot1-Synth/songs`.
- If the bank contains one Song, the package has one Song; if it contains three, it contains three (up to 100). The current Song remains selected upon import.
- Song End and Loop Target references are validated and remain internally consistent; missing referenced Song makes export fail explicitly rather than creating a broken package.
- F2 Ctrl+O imports the complete package transactionally and remaps occupied Pattern and Song IDs; original bank data is not overwritten.
- F1 Ctrl+S/Ctrl+O continue to save and load individual Patterns in `patterns`. Synth presets and DSP are unchanged.
- The native file chooser remains; a custom accessible audio-preview browser is not yet implemented.

The Windows VST3 build and NVDA/REAPER test are still required.
