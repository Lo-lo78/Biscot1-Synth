# Biscot1 Synth 0.99.6 — AHDSR Hold and REAPER Automation

**16 November 2026 | Windows x64 VST3**

## Seven AHDSR envelopes

- Independent **Hold** after Attack on all three operator amplitude envelopes (Op1/2/3).
- Independent Hold on all three operator modulation envelopes (Mod Env1/2/3).
- Independent Hold on the shared Filter envelope.
- Range: **0–5 seconds**, **1 ms** resolution, default **0** to preserve existing presets.
- Note Off enters Release during Hold; legacy SID envelope curves, Trigger, Legato and sustain-pedal behavior are preserved.
- Existing VST3 parameter IDs are unchanged; seven new parameters are appended at 249–255.
- Hold is also available in the Alt+Q F1 Pattern parameter picker (OP Hold, OP Mod Env Hold, Filter Hold).

## Accessible REAPER automation

- **Ctrl+Shift+A** activates/deactivates the selected REAPER parameter envelope from the main Grid, Value or Edit.
- Previously recorded automation points are preserved; this controls the envelope's active state, not only its visibility.
- NVDA announces **`, Auto`** when active, including the new Hold parameters.

## Additional information

- Includes the existing Phase Drift and Lace on all three LFOs from version 0.99.5.
- Updated Alt+A About, English and Italian guides, release metadata and GitHub Actions packaging.
- All previous presets, tracker Songs and Pattern data remain supported. The SID noise-clock listening trial contained in this source is still experimental.

**Recommended GitHub release attachment:** `Biscot1-0.99.6-Windows-x64.vst3.zip`, containing the same VST3 bundle as the `Biscot1-0.99.6-Windows-x64.zip` build artifact.

**Project:** https://github.com/Lo-lo78/Biscot1-Synth
