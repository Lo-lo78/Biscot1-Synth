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
