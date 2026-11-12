# Biscot1 Synth VST3 – TEST0.2.19.4 (12 November 2026)

## Fix

- F1: pressing unmodified `1`, `2` or `3` once again selects OP, Filter or LFO.
- Root cause: `trackerShortcutLetterMatches()` explicitly accepts only a-z and had been reused for digits. A dedicated testable `trackerShortcutSectionIndex()` now examines both the JUCE key code and text character.
- F2: no change to its keyboard handling. Use Ctrl+Tab to switch parameter section, preserving incremental search, including digits.

## Operator arrangement from TrackerControl 0.166 / Biscot1 Tracker JSFX

- A Pattern contains OP parameter commands (plus Filter/LFO) without embedding three concurrent operator tracks.
- F3 Song has three independent lanes, corresponding to operators 1, 2, 3. Each can reference the same or different Pattern; the JSFX maps generic OP parameters to the lane's actual voice.
- The Filter and LFO generators are shared and the Song has ownership settings; native F3 mapping remains future work.

DSP, clock, MIDI and `.biscot` serialization are unchanged.
