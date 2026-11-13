# TEST0.2.33 — Native accessible library browser

Base: TEST0.2.32. Adds reusable in-plugin Pattern and Song library browser.

- F1/F2 Ctrl+O opens library list within the plugin, with keyboard focus and spoken filenames.
- Arrow keys, Home/End, PageUp/Down, folders, Backspace/Escape, F5.
- Space/P audition; Ctrl+P auto preview on navigation (off by default); +/- preview length.
- Browser Pattern auditions use temporary, immutable audio snapshots with a 1–32 step limit. Browser Song auditions use the Song's embedded Pattern data with a short one-operator preview; no replacement of active synth or project song.
- All-100-Songs library: Left/Right select the slot to audition without changing the archive import target.
- Song imports confirm explicitly before replacing the current slot or the full 100-slot bank. Pattern imports preserve bank content by allocating free slots on collision.
- Song/Pattern formats, song transports, song-end chaining, and named saves remain unchanged.

Checks: Python regression suite, C++ standalone tests, packaging. Windows VST3 and NVDA require live verification in REAPER. 
