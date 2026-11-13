# Biscot1 Synth — proposed F2 Song library (design only, TEST0.2.21.4)

## Two different sharing units

- `.biscot`: the self-contained synth preset, including all synthesis parameters, all native Patterns, and eventually all Songs. Alt+S saves this whole snapshot; the REAPER host state uses the same model.
- `.biscot1-pattern`: one standalone Pattern, including every step and ordered OP / Filter / LFO cell, 1–1000 repeats with individual Speed / Transpose, MIDI note mapping, Pattern length and Release Arpeggio; no synth parameters.
- `.biscot1-song`: one Song with all three operator lanes, repeats/transpose/mixer/routing and **relative references** to exactly the Patterns it uses; no synth parameters.

## Global library paths

Under the Documents directory:

```text
Biscot1-Synth/
  Patterns/
    Singles/
      Pattern_004.biscot1-pattern
      Pattern_004_2.biscot1-pattern
    SID6581/
      Pattern_004.biscot1-pattern
      Pattern_012.biscot1-pattern
  Songs/
    SID6581.biscot1-song
  My Preset/
    My Preset.biscot
```

**Implemented in TEST0.2.21.4:** F1 Ctrl+S exports the active Pattern into `Patterns/Singles`; directories `Patterns/Singles` and `Songs` are created as necessary. Exported files have a stable encoding suitable for a future library importer; existing unrelated files are not silently overwritten. Alt+S is unchanged.

**NOT IMPLEMENTED in TEST0.2.21.4:** the F2 Song interface, Song Ctrl+S, the Song file format, Song references, and the Song/Pattern import browser.

## Future F2 Ctrl+S transaction

1. Select Song and use its user-defined display name; sanitize only its filesystem basename. Prevent collisions with other Song package names.
2. Determine the **used Pattern IDs** in its three lanes, including any duplicated references. Export each distinct Pattern exactly once, with all its settings, to `Patterns/<SongName>/`.
3. Save `Songs/<SongName>.biscot1-song` with an explicit dependency manifest: `Pattern ID -> ../Patterns/<SongName>/<file>`. Keep the actual numeric ID and song-lane references; filenames alone are not stable identities.
4. Ensure all dependencies were saved successfully before confirming the Song. Commit the Song file last so no published Song points to missing Patterns. Detect and resolve future linked-Song/chain dependencies before labeling a package fully portable.
5. A standalone Song package is the **two-part relative directory tree**, not just the `.biscot1-song` file. It plays with the currently loaded Biscot1 synth patch; it does not carry the saved timbre. For exactly reproducible sounds, distribute the `.biscot` preset.

## Future load/import

- Read the Song and the manifest relative to the Song's own folder, never through a hardcoded `C:\\Users\\...` path.
- Validate every dependency and file version before applying changes.
- Remap Pattern numbers to free slots when a destination bank already uses a referenced ID. Update **all three Song lanes** consistently. Do not silently overwrite unrelated patterns.
- Use the imported Pattern data for an accessible audition list, and eventually allow copying any single Pattern from another `.biscot` preset without loading its synth sound.

## Current implementation: TEST0.2.32 (supersedes the early design above)

The original external dependency scheme was replaced. Only two tracker library directories are needed beneath the user Documents `Biscot1-Synth`: lower-case `patterns` for standalone `.biscot1-pattern` files, and lower-case `songs` for both `.biscot1-song` (portable single Song) and `.biscot1-all-song` (fixed 100-Song archive). Songs embed every Pattern needed and need no dependent files. Single Song imports target the current Song slot; all-Song imports replace slots 0–99. Both formats leave the synth alone.

Saving via F1 Ctrl+S, F2 Ctrl+S or F2 Alt+S first focuses an editable filename field, initially selected. Enter saves, Esc cancels, and typing a space inserts the character and explicitly speaks `spazio`. Source notes under TEST0.2.21.4 above are historical proposals only.


## TEST0.2.39 — current replacement policy (supersedes collision remapping above)

Before every single- or all-Song import, a keyboard-operable Yes/No confirmation protects the **entire current Pattern and Song banks**. The loader validates the whole file before clearing anything, then atomically replaces both banks. Original Pattern indexes are restored exactly (0–999), never remapped into free destination slots. For a single Song, only the imported Song is placed at the selected Song number and other Song slots are cleared, to avoid dangling Pattern references; for an all-Song archive, all fixed 100 Song slots are restored. Song imports do **not** change any synth parameters/timbre. Standalone Pattern imports still add to the existing bank, using a free destination on collision. Descriptions of Song collision remapping above are superseded.
