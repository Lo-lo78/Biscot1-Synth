# Biscot1 Synth 0.99.5 — TEST0.2.45

**REAPER VST3 parameter automation — Ctrl+Shift+A**

- On the main parameter pages (Grid / Value / Edit), Ctrl+Shift+A toggles the selected parameter’s **ACTIVE** REAPER envelope, using the REAPER VST3 host extension verified in LR-608 TEST36.3 and LJuno-116 TEST130.
- Existing envelope points are preserved. Enabling reveals and arms the lane; disabling removes ACTIVE and disarms/hides the lane without deleting the points.
- NVDA announces the selected parameter with **`, Auto`** when the envelope is active. One accessible notification is issued after toggling, without changing focus.
- Applies to all 12 main parameter sections (Global / MIDI, Operator 1–3, Filter, LFO 1–3, Arpeggiators, Ring / Sync, OSC / ENV Cross, LFO Cross / Rate), including Grid/Value/Edit.
- Tracker Alt+Q, Pattern, Song, preset browser, About, and factory reset dialogs are unchanged; the shortcut is deliberately not intercepted there.
- No VST3 parameter IDs were added or renumbered. No audio engine or preset format changes.
- The host fallback only invokes real Activate/Deactivate menu commands, never Show/Hide; unavailable APIs generate an accessible message instead of silently changing the wrong state.
- Added a dedicated pre-build regression check and explicit UTF-8 file reading for Windows Python CI.

**Testing:** source-level regression check is available locally; full MSVC VST3 build and REAPER/NVDA behavior require GitHub Actions and a REAPER test.
