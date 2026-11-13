# Biscot1 Synth VST3 — TEST0.2.35

## Accurate file library preview

The TEST0.2.34 browser preview unexpectedly assembled one clip per operator at row 0, skipped leading rests, suppressed Pattern repeats, and omitted Song End. It has been replaced with the actual compiled Song transport.

- Song file previews preserve all three lanes, their actual Song rows, starts, speeds, repeats, Pattern changes and Song FX including mixer, Filter/LFO, Sidechain and Delay.
- Song End follows only if the next Song exists within the file being auditioned; it never borrows a destination from the active project.
- Pattern previews begin at step 0 and preserve full repeats and initial rests.
- The default preview is Full. Space starts/stops playback; `-` shortens to 64, 32, 16, 8, 3, 1 divisions; `+` lengthens back to Full. The cutoff is independent of the underlying event data.
- Ctrl+P automatic preview recall and incremental filename search remain unchanged.
- The complete synth preset browser does not trigger an embedded Song. The synth DSP and the tracker/song bank serialization are unchanged.

## Test coverage

- New C++ regression compares original and library Song event traces, including leading blank rows, per-operator speeds, Pattern repeats, transpose, FX and Song End references.
- Additional source tests validate archive-only Song End, dual preview commands (-3 for F2 and -4 for library), adjustable cutoff and Pattern start at 0.
- Windows JUCE/VST3 build and REAPER/NVDA audition require GitHub Actions/user testing.
