# Biscot1 Synth VST3 — TEST0.2.17.1

## Fixed

- New plugin instances with no saved `editorLastParameter` now select index **2** (`Audio Mode`), not index **3** (`MIDI Mode`).
- Existing projects with an explicitly persisted selection retain that selection, including if it is MIDI Mode or on another page. This preserves the remembered last parameter feature.
- Native entry focus and the ignored accessibility root from TEST0.2.17 remain unchanged, as do Grid navigation announcements, browser, incremental search, and all DSP/CPU paths.

## Check in REAPER and NVDA

1. Insert a **new** Biscot1 Synth instance and enter the FX editor: the first control should announce `Audio Mode, Stereo` (not `MIDI Mode`).
2. Navigate to a parameter on another page, close and reopen the FX editor: that parameter should be restored.
3. Previously saved instances may legitimately open on `MIDI Mode` if that position was stored by the old version. Such instances retain their saved position by design. Navigate to `Audio Mode` and save the REAPER project to update it.
4. Verify that the earlier NVDA `unknown` announcement remains absent and CPU idle remains unchanged.

No changes to DSP/CPU/MIDI/preset-state representation. Windows JUCE build and REAPER/NVDA tests require GitHub Actions and the user environment.
