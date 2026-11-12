# Biscot1 VST3 — TEST0.2.18.3 (Windows Release test fix)

## What happened
GitHub Actions successfully compiled and linked the JUCE Windows VST3 in TEST0.2.18.2.
The job failed afterward when running `TrackerSearchStepTest.exe` (exit code 1).
On Windows the test target is compiled in `Release` with `NDEBUG`: calls written
inside `assert(...)` were entirely eliminated. This skipped the `p.insert(...)`
operations required to prepare the LFO test data, causing invalid access.

## Fix
- Both `TrackerSearchStepTest.cpp` and `TrackerPatternBankTest.cpp` use a portable,
  always-evaluated `CHECK` macro with a useful failure message.
- An additional static check protects the Release regression tests.
- Version label and GitHub Actions artifact updated to TEST0.2.18.3.

All music code (PluginProcessor including audio/MIDI/idle, tracker UI, bank storage)
is unchanged from TEST0.2.18.2. This is a build-validation hotfix only.

## Git
`git add . && git commit -m "Biscot1 VST3 TEST0.2.18.3 fix Release-mode tracker regression tests" && git push`
