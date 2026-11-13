# Biscot1 native Song migration — plan based on TrackerControl Lua 0.166

Original Lua: F3 Song + F4 Song parameter list. Native VST3 maps Song to **F2**, keeps F1 Pattern and moves each parameter list to **Enter**, as agreed.

## Phase 1 — TEST0.2.22 (implemented)

- Add a sparse, independent Song bank with **100 Song slots**, **3 operator lanes**, **up to 4096 steps per lane**, independent Length, Repeat and per-repeat Speed. Keep the Song-global end, MIDI note and future chaining, routing, mixer and delay fields in the model.
- Implement an accessible native **F2 Song** editor. Key **1/2/3** selects the Operator lane; **4** selects the Song-global area. Tab announces Song number, lane and position. Enter opens the original 17-item Song parameter list; Ctrl+Enter adds and remains there. Alt+E edits a numeric value. Ctrl+Up/Down changes Song slot. Ctrl+J jumps to an editable Song step. Esc from the picker returns to F2; F1 returns to Pattern; Esc from F2 exits Alt+Q.
- A `Pattern` cell is an ordered group, expanding into Pattern Start, Play Length and read-only counters, as in Lua; no allocation of empty steps. All 17 Lua Song parameter IDs are retained with stable numerical IDs.
- **Persist Song bank in `.biscot` and the REAPER project**, alongside the existing Pattern bank, using an independent `trackerSongsV1` property. Previous presets without that property load with an empty Song bank. No modification to oscillators/filters/audio timing.
- **Not yet implemented:** Song playback, copy/paste/undo/redo, Song solo/mute audio, Song Ctrl+S export, Song browser/import. Space announces the pending playback rather than falsely indicating it is running.

## Phase 2 — TEST0.2.23: editing fidelity (implemented, pending Windows/NVDA verification)

- Native F2 now supports selection of steps, cut/copy/paste for steps/parameters/whole Song, independent undo/redo, ordered Pattern groups, Song renaming, and saved mute/solo/volume. Tab and logical cursor navigation are retained. Advanced Lua-only operations (copy-to-following steps, F3-to-F1 jump with return context, detailed per-lane audition) are still pending refinement.
- Start Offset, Pattern Position, Pattern Remaining, Play Length, Filter/LFO owner/mask, sidechain and delay parameter fields remain editable/labelled; their *audible* implementation is Phase 3.
- Test user workflow: two different Patterns in separate operator lanes, save/reopen `.biscot`, verify every Song parameter still present.

## Phase 3 — audible Song engine (TEST0.2.24, first integration)

- **TEST0.2.24 implemented:** Pin an immutable Song + referenced Pattern snapshot per playback session. REAPER-synchronized, sample-accurate audio scheduling across Op1/Op2/Op3, lane speeds/repeats, transposition, pattern windows and legato.
- **TEST0.2.24 implemented:** basic Filter/LFO ownership, Filter routing mask, mixer/mute/solo and MIDI Song triggers. **Still pending:** sidechain and delay DSP, Song End/chain loop, advanced resource reallocation across empty Song rows.
- **TEST0.2.24 implemented:** F1/Song performance separation; Space from Song cursor and Ctrl+Space pause at the audible Song row (restarts that row, not mid-Pattern).
- Compare timings and behavior against `Biscot1 Tracker(8).jsfx`, Lua 0.166 and the startup loader. Do not read mutable UI data or write files in processBlock().

## Phase 4 — export, import and library

- Keep **Alt+S** as the entire self-contained patch `.biscot`: synthesis state + all Patterns + all Songs.
- Keep **Ctrl+S in F1** as the current Pattern in `Documents/Biscot1-Synth/Patterns/Singles/`.
- Add **Ctrl+S in F2** to save `Songs/<SongName>.biscot1-song` and precisely the referenced Patterns to `Patterns/<SongName>/`.
- Store relative Pattern paths and stable original ID mapping in the Song. Write and validate all Pattern dependencies before committing the Song file. Do not overwrite unrelated exports.
- Implement accessible browser + preview, importing individual Patterns from another `.biscot` without replacing its synth sound; remap colliding Pattern IDs when importing Song packages.

Phase 3 is staged: TEST0.2.24 plays three operator lanes, but more advanced original JSFX effects and Song chaining must still be ported and verified. Phase 4 covers portable Song packages.


## TEST0.2.25: advanced Song engine

- Song End chain/repeat, immutable multi-Song bank and Pattern Start state priming implemented.
- Pattern-first shared Filter/LFO ownership implemented, including sparse FX-only Song rows.
- Native Sidechain and Delay DSP, plus live F2 mixer update implemented for REAPER testing.
- Phase 4 remains: per-Song Ctrl+S, dependent Pattern folder export/import, collision-safe Song package management.
- Exact legacy JSFX effect response and advanced continuous pause/resume remain subject to listening tests.

## TEST0.2.26 — Song-End UI synchronization and operator audition

- Fixed live Song End updates and visible F2 follow-on when Song End starts the next Song. Song End waits for all three lane lengths/repeats, including blank Song rows.
- Ported the Lua F3 short Pattern preview to native F2: selected lane only, default three steps, now configurable independently per Song row via optional `Preview Steps` (ID 23).
- F1/F2 entry announcements, compact F2 parameter speech and persistent last-opened Alt+Q page; each pane retains its own edit cursor.
- Native Windows VST3/NVDA listening tests remain necessary. Phase 4 standalone Song/Pattern package export remains not implemented.


## TEST0.2.27 — Phase 4 (first integration)

- Based on verified TEST0.2.26 **Song Chain Preview Focus**, preserving Song End, per-step Preview Steps, independent Pattern/Song navigation and NVDA speech.
- F2 Ctrl+S exports Song and referenced Pattern dependencies to separate, relative library directories.
- F1/F2 Ctrl+O use native file chooser to import a Pattern/Song and allocate free IDs for collisions without replacing synthesis settings.
- The package parser also supports new Song Preview Steps field (ID 23).
- **Remaining:** custom accessible browser with audio audition; multi-Song Song End chain packages; full JUCE Windows/NVDA end-to-end testing.

## TEST0.2.28 — Phase 4 revised: portable self-contained Song bundles

The previous Phase 4 multi-file proposal above is superseded. `Ctrl+S` in F1 saves only the selected Pattern in `Documents/Biscot1-Synth/patterns`. `Ctrl+S` in F2 saves a **single** `.biscot1-song` file in `Documents/Biscot1-Synth/songs`, embedding all referenced Patterns plus chained Songs reachable through Song End / Loop Target and their referenced Patterns. No file under `patterns` is required to reload a Song. F2 `Ctrl+O` imports the bundle and remaps occupied Song/Pattern numbers and internal links. No synthesis parameters are changed. Native file chooser is temporary until the complete accessible library browser is implemented.

## TEST0.2.29 — Save all Songs in one package

F2 Ctrl+S exports all instantiated Songs in the bank and embeds all their referenced Patterns, whether or not linked by Song End. F2 Ctrl+O imports the whole package and retains active Song selection. See RELEASE_NOTES_TEST0.2.29.md.

## TEST0.2.30–0.2.32 — Current library behavior (overrides earlier proposals)

- F2 Ctrl+S exports ONE selected Song and its Patterns to one `.biscot1-song`. Import places it in the actively selected destination slot; Song End / Loop Target are kept as numbers even if the referenced Song is absent.
- F2 Alt+S exports the full fixed 100-Song bank and associated Patterns to one `.biscot1-all-song`. Import restores all 100 Song slots, including empty entries. Neither import changes synth settings.
- F1 Ctrl+S exports one `.biscot1-pattern`. Both Song types embed referenced Patterns; the folder `patterns` holds independent exports only.
- TEST0.2.32 opens a real, focused JUCE TextEditor before each tracker export, with user-editable filename, Enter/ESC, and explicit spoken `spazio` on space typed in the name field. The chosen filename is sanitized for Windows paths and collisions are suffixed.
- F1 Alt+S remains the separate complete preset `.biscot` browser.
- Still pending: native accessible Pattern/Song browser with auditory preview, and Windows/NVDA/REAPER end-to-end test.

## TEST0.2.33 — Native library browsing milestone

Ctrl+O in F1/F2 now opens native JUCE library browser with audible temporary previews and independent bank import, filename navigation, folders, confirmation for Song replacement and automatic preview toggle. Existing `.biscot1-song` / `.biscot1-all-song` / `.biscot1-pattern` formats are unchanged. The Song preview previews the earliest playable operator/clip, not a complete arrangement; multitrack Song preview and asynchronous loading of very large bundles remain possible future enhancements.


## TEST0.2.34 — Library usability follow-up

All three Biscot1 file browsers now support incremental filename/word-prefix search, Right-to-open-folder and Left/Backspace-to-parent. The tracker library frees P for search (Space previews), remembers Ctrl+P auto-preview independently for Pattern/Song, and moves `.biscot1-all-song` slot selection to Ctrl+Left/Right. Song library audio audition combines three operators from their first available clips, with bounded duration; F2 per-row audition remains one lane. No source or preset format change; Windows JUCE/NVDA test pending.


### TEST0.2.35 preview fidelity
The file library now uses the same immutable compiled Song timeline as F2 playback, without altering row alignment, event timing or loops. Full audition is default; +/- chooses an external preview length only. Song End within the archive is supported through an isolated immutable Song graph, not the live project bank.


## TEST0.2.39 — current replacement policy (supersedes collision remapping above)

Before every single- or all-Song import, a keyboard-operable Yes/No confirmation protects the **entire current Pattern and Song banks**. The loader validates the whole file before clearing anything, then atomically replaces both banks. Original Pattern indexes are restored exactly (0–999), never remapped into free destination slots. For a single Song, only the imported Song is placed at the selected Song number and other Song slots are cleared, to avoid dangling Pattern references; for an all-Song archive, all fixed 100 Song slots are restored. Song imports do **not** change any synth parameters/timbre. Standalone Pattern imports still add to the existing bank, using a free destination on collision. Descriptions of Song collision remapping above are superseded.
