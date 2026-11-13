# Biscot1 Synth VST3 — TEST0.2.30

## Independent Song library formats

- F2 Ctrl+S: export ONLY the selected Song to `Documents/Biscot1-Synth/songs/*.biscot1-song`, with its referenced Patterns embedded. Song End and Loop Target are stored as numeric addresses, even if the other Song does not exist.
- F2 Alt+S: export ALL 100 fixed Song slots (including empty positions) to a single `*.biscot1-all-song`, with every referenced Pattern embedded.
- F2 Ctrl+O: open either format. Single Song loads into the CURRENT selected Song slot and replaces that Song; all-Song file replaces the entire 100-slot Song bank by fixed position. No per-pattern sidecar files.
- During import, embedded Pattern IDs are remapped if occupied; references within the imported Song(s) are updated. Existing synth parameters remain unchanged.
- The earlier V2 Song bundle remains readable for migration.
- In the F2 Song view, Alt+S now exports the Song bank; full `.biscot` preset saving remains accessible in the preset browser outside F2.

Note: importing one Song overwrites the selected Song slot; importing All Songs overwrites the entire Song bank. Back up the current preset before testing.

Static checks only; Windows VST3 build and NVDA/REAPER tests are still required.
