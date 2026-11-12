# Biscot1 Synth VST3 — TEST0.2.19 (first audible native Pattern)

This is an **experimental first audio playback slice** of the original Biscot1 Tracker 0.166. It is not a complete port of F1/F2, the JSFX tracker scheduler, or the Song engine.

## Audition and transport

- Opening Alt+Q keeps the existing F1/F2 accessible editor, keyboard search, copy/paste, Undo and all approved navigation.
- Moving vertically to a real F1 step auditions **that step alone**, like the legacy TrackerControl Lua. Step inspection does not interrupt a full Pattern that is playing.
- **Space in F1:** start the Pattern from the **edit cursor**, not necessarily step 0. Press Space again while a full Pattern is playing to stop it.
- **Ctrl+Space:** request a pause and move the edit cursor to the **currently audible step** once the audio callback has acknowledged the pause. A later Space starts from that cursor position.
- Clock uses the REAPER tempo and the original JSFX default Pattern Division 16 (16th notes), with per-repeat speeds (0.25–16x) and per-repeat transposition. Step boundaries and Gate are processed at sample offsets, rather than a GUI timer.
- Note/Gate/Pause and direct F2 parameter commands (Op1, filter, LFO1–3) are sent to the original synth signal path. The generic Pattern operator previews through Op1, using temporary MIDI routing, without changing the user's saved MIDI Mode.
- F2 parameter edits are saved in the existing Pattern state and included in the current Alt+S `.biscot` preset and REAPER project state.

## Deliberate limits of this first audible test

- This version **does not yet** implement advanced per-parameter Curve interpolation, Curve Resolution/Ratio, Sub Notes, full Previous Value semantics, Release Arpeggio, MIDI step recording or MIDI keyboard Pattern triggers. Basic Previous Value holds the preceding Gate into the next step.
- F1's separate Ctrl+S export to `Patterns/Singles`, the Song editor and the final library layout are still future work.
- During full playback the audio thread uses a frozen immutable snapshot. If you modify a Pattern while it is playing, restart with Space to hear those changes. This prevents editor/audio data races while we establish stable playback.
- Pattern preview can override synth settings only while the temporary Pattern playback/release is active; the `.biscot` sound settings remain intact.

## Design and verification

- No modification to the existing SID oscillator, filter core, wave tables, output stage or `renderOne()` equations.
- Idle fast path is retained, including the short-circuit when no audio or MIDI processing is needed.
- JUCE-free transport tests check cursor start, Gate duration, Pause, step order, variable speed, transposition and one-step audition; prior DSP, MIDI and F1/F2 tests remain in the Windows workflow.
- Requires GitHub Actions Windows compilation and user tests in REAPER with NVDA before broader tracker feature work.
