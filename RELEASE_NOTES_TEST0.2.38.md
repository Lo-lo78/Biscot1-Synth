# Biscot1 Synth VST3 — TEST0.2.38

13 November 2026

## Windows build fix (TEST0.2.37 GitHub Actions log)

- Fixed the actual MSVC C2662 compile error in `Source/TrackerPatternPanel.cpp`,
  lines 871 and 872: the `cursorRegion() const` method was invoking
  `currentPattern()` which had only a mutable overload.
- Added `const TrackerPattern& currentPattern() const` to the panel class,
  preserving the existing mutable overload for editing. The fix is read-only
  and does not alter the tracker grid, transport, DSP or file formats.
- Added `Tests/verify_test0238_pattern_const_correctness.py` and an early
  GitHub Actions step, to detect accidental removal of the overload before
  the Windows JUCE compilation.
- Updated CMake, accessible editor title, workflow and artifact name to 0.2.38.
- `Unsupported Song library format` remains an independent runtime issue;
  this build-focused release does not claim to fix it.

**The full JUCE/MSVC Windows VST3 build must be rechecked on GitHub Actions.**
