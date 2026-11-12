# Biscot1 Synth VST3 — TEST0.2.20 (12 November 2026)

Native Pattern automation and F2 insertion rules, based on `Biscot1 TrackerControl 0.166` and `Biscot1 Tracker.jsfx`.

- **Previous value** now inherits the nearest real parameter value in prior steps. Only parameters to the right of the marker inherit; chains of markers and parameter-only steps are supported.
- **Curve** performs bipolar (-100 to +100) stepped smoothing toward the next explicit value of the same parameter, even across empty steps. Positive Curve begins smoothing early; negative Curve finishes late.
- **Curve Resolution** (2..1024) and **Curve Ratio** (`Off`, `1 to 1` through `2 to 2`) now change playback in the audio clock rather than being stored-only cells. The legacy 32-point behavior is kept with Resolution absent.
- **F2**: Filter and LFO parameter lists now include `Previous value`, `Curve`, `Pause`, like the Lua. Duplicate parameter insertion navigates to the existing cell. Adding Resolution/Ratio before a Curve automatically inserts Curve +100; Resolution goes before Ratio. Sub Note insertion checks that an active OP Arp Mode is present, and still permits multiple entries.
- Immutable Pattern automation is compiled in a single message-thread pass; the audio thread only reads the pinned snapshot and advances already-compiled events. Coincident Curve Resolution events are combined into one sample-level override update for fast tempos.
- **Unchanged**: SID DSP engine, preset `.biscot` serialization, existing pattern navigation, clipboard, MIDI processing and idle suspension.
- **Not yet ported**: native Sub Note audio pool, Step Recording MIDI, Release Arpeggio, keyboard-triggered Pattern bank and complete F3 Song.

### Tests

`TrackerCurvePreviousTest` covers inheritance chains, positive/negative curves, step gaps, Ratio/Resolution alternation, sample offsets, optimized/reference compiler equivalence and F2 insertion constraints. The previous independent C++ regression suite remains available. Full Windows VST3 build and NVDA behavior must still be confirmed with GitHub Actions and REAPER.
