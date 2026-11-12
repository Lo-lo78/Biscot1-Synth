# Biscot1 Synth VST3 — TEST0.2.24 — Song F2, first audible three-operator engine

## The first audible Song playback milestone (Phase 3, part A)

This is the first build to drive **Op1, Op2 and Op3 simultaneously from three independent Song lanes** in native JUCE code, without invoking the JSFX or Lua at runtime.

- In Alt+Q **F2 Song**, pressing **Space** starts from the current Song row (or row 0 when outside the step grid); pressing it again stops. **Ctrl+Space** pauses and moves the editing cursor to the last *actually scheduled* Song row on the selected operator lane. Press Space to start from that row again. Pausing does not silently mark an unsounding row as played.
- The Song `Pattern` parameter references an F1 Pattern by number. Each lane uses its own Pattern Start, Play Length, Pattern repeats, Song-row Repeat, Song-row Speed, transposition and lane-cycle speeds. Blank Song rows consume grid time without generating MIDI notes. An optional Start Offset delays a row by 0..31/32 of a Song grid beat.
- MIDI is routed to **channels 1, 2 and 3** internally, addressing the existing three SID operators independently. Each lane owns its Note Off and Sub Notes. Legato changes note without restarting the VCA; repeated equal-pitch legato does not leave duplicated held notes.
- Three operator Song mixer volumes, Mute and Solo now affect both normal audio and signal routed to the shared filter, with the final mixer state maintained during natural Release. The entire synth DSP architecture, SID wave generation and output stages are unchanged except for one conditional Song gain multiplier.
- Song Filter and LFO owner selections restrict which operator can apply shared parameter changes and trigger the corresponding shared resource. Shared Filter Operators mask is applied. The immutable transport and Pattern snapshots are prepared on the editor/message thread; no editable Pattern or Song map is read inside `processBlock`.
- Song MIDI Note mapping starts the assigned Song on Note On and stops it on corresponding Note Off. On a conflicting MIDI note assignment, Song wins over Pattern, following the legacy JSFX. Song performance stops when REAPER transport changes from Playing to Stopped.
- Previous F1 Pattern playback is kept separate; switching to F1 stops Song, starting Song stops Pattern. Alt+S still saves the whole `.biscot` containing synth, Pattern and Song.

## Important limits — not yet full Lua parity

This build is **Phase 3 first audible integration**, not the last Phase 3 milestone. Song End chaining/loop, the row-level Sidechain and Delay audio DSP, automatic resource arbitration across empty rows, and advanced Song continuous resume need their own tests and implementation. **Ctrl+S in F2** and portable Song/Pattern package import remain Phase 4. Pressing Ctrl+Space pauses and release-tails the three voices; Space restarts from the saved Song row rather than continuing at an intra-pattern sample position. Full sample-exact host-transport restart mapping and multi-Song chaining remain to be added.

## Initial REAPER/NVDA acceptance test

1. In F1 create two contrasting Patterns, e.g. Pattern 0 (Note 60) and Pattern 1 (Note 72) with audible Gate and a nonzero OP level.
2. In F2, Operator 1 row 0 insert `Pattern` and set it to `0`. Press `2` and Operator 2 row 0 insert Pattern `1`. Set Operator 3 as desired.
3. Press **Space**. Both operators should sound on their independent notes. Press **Ctrl+Space**, check the announced row, then press **Space** again.
4. Try an Op2 Solo and Mute, adjust the volume with +/- (while stopped before restarting Song), and set different Song-row transpositions/speeds. Use Tab for full location speech.
5. Save a full `.biscot` with **Alt+S**; reopen it and verify all three Song assignments remain intact.

## Technical verification

JUCE-free Song playback transport test checks three simultaneous lanes, sparse Song rows, timing, windows, repeats, offset and mixer compile. Python checks native callbacks, song MIDI and pinned snapshots. Legacy C++/Python regression tests are run in Release mode. JUCE Windows VST3 compilation, actual REAPER audio and NVDA were **not** executed locally and require GitHub Actions and user audition.
