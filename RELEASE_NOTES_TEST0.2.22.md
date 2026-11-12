# Biscot1 Synth VST3 TEST0.2.22 — Native F2 Song: first editable milestone

**From the tested TEST0.2.21.4 baseline.** No changes to SID oscillators, ADSR/filter/output DSP, MIDI process, F1 Pattern clock, Curve automation or `.biscot` preset header.

## Implemented

- **Alt+Q, F2** opens a new accessible Song grid. **F1** returns to Patterns. Key **1, 2, 3** selects independent Operator 1/2/3 lanes; **4** accesses Song-global Song End.
- 100 Song slots (0..99), three sparse lanes with maximum 4096 steps each, separate lengths, repeats and per-repeat speeds; saved Song values are ordered as in the legacy Lua 0.166.
- Song steps can reference a Pattern (0..999) with Speed, Transpose, Repeat, Pattern Start, Play Length, Start Offset and the Lua's Filter/LFO, legato, sidechain/delay parameter IDs. Pattern is an ordered block exposing its auxiliary fields and read-only counters.
- **Enter** while on a Song step opens a 17-parameter insertion grid (same F4 list/order as Lua); Enter adds and returns, Ctrl+Enter adds and stays, Esc backs out. Incremental search for parameters is available.
- **Alt+E** edits numeric values; **Alt+Up/Down** changes values, **Alt+Home/End** jumps to min/max. **Ctrl+Up/Down** selects Song slots; **Alt+Enter** jumps by Song number; **Ctrl+J** jumps by step number. Tab speaks full location. Home/End moves inside the current area and Ctrl+Home/End changes areas.
- New independent `trackerSongsV1` state property, stored with existing Pattern state in REAPER projects and Alt+S `.biscot` presets. Presets created before TEST0.2.22 continue to load with an empty Song bank.
- Entering Song disables active Step Recording in F1 and stops F1 preview; this prevents MIDI events from editing the hidden Pattern.
- New `TrackerSongBankTest` is included in GitHub Actions, with a dedicated static integration validation test.

## Deliberately not implemented in this first phase

- Song audio playback on Op1/Op2/Op3. Space announces that Song playback is pending. It does not silently start a Pattern and claim Song success.
- Standalone Song Ctrl+S and linked `Patterns/<SongName>` exports, import/browser, MIDI Song triggers, mute/solo routing and Song Undo/Redo. These are covered by `SONG_MIGRATION_PLAN.md`.

**Local validation:** standalone C++ tests in Release and Python integration assertions. The full JUCE VST3 Windows compilation, NVDA navigation and host-state round trip still need GitHub Actions / REAPER validation.
