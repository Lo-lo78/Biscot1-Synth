# Biscot1 Synth VST3 — TEST0.2.31

## Hotfix: Windows compilation of the Song editor

- Fixed `Source/TrackerSongPanel.cpp`: the closing brace of the Song End branch in `TrackerSongPanel::currentValue()` was accidentally included in a `//` comment.
- This prevented MSVC from recognizing the end of the method and caused many C2601 errors followed by C1075 in GitHub Actions.
- This release does not change the audio DSP or Song/Pattern storage semantics.
- Retained `Ctrl+S` for one portable `.biscot1-song` in F2 and `Alt+S` for the fixed 100-slot `.biscot1-all-song` bank; patterns remain embedded in the Song files.
- Added a regression test for brace/comment balance in the Song panel.

### Validation

The source regression tests and JUCE-free C++ tests are run locally. The complete Windows VST3 build and NVDA/REAPER test require a new GitHub Actions run.
