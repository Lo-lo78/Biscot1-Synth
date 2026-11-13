# Biscot1 TEST0.2.28 — Self-contained Song library

Based on TEST0.2.27, retaining the corrected TEST0.2.26 Song End chaining, preview steps and F1/F2 navigation.

- F1 Ctrl+S: exports a standalone `.biscot1-pattern` directly into `Documents/Biscot1-Synth/patterns` (no Singles directory).
- F1 Ctrl+O: imports a Pattern from the same folder.
- F2 Ctrl+S: writes ONE `.biscot1-song` directly into `Documents/Biscot1-Synth/songs`; the file embeds every required Pattern and every Song reachable via Song End or Loop Target, including the referenced Songs' own Patterns. No external dependencies.
- F2 Ctrl+O: validates and imports the single-file Song bundle. Remaps occupied Song and Pattern slots without overwriting existing entries, and updates Song End/Loop Target and Song Pattern references.
- Each Song file can be moved alone and later re-imported. Individual Patterns can be extracted afterwards by opening the Song and exporting the desired Pattern from F1.
- Existing `.biscot` preset behavior is unchanged.
- Format: `BISCOT1_SONG_BUNDLE_V2`; legacy V1 multi-file Song manifests are not imported.

The Song import still uses the native file chooser; the complete accessible sound-preview library browser is future work. Windows VST3 build and NVDA/REAPER testing are required.
