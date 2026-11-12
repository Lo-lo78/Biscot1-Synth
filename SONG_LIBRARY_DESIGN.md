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
