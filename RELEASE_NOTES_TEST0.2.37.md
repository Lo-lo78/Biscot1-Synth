# Biscot1 Synth TEST0.2.37 — CI regression test version guard

13 November 2026

- Fixed the immediate GitHub Actions failure: `Tests/verify_test021_build_fixes.py` expected TEST0.2.35 while the project was TEST0.2.36.
- Removed stale pinned TEST0.2.35/0.2.36 version asserts from all applicable Python regression tests, now consistently checking the version derived from CMakeLists.txt.
- Added an early CI release-number consistency check comparing CMake, accessible editor title, and GitHub artifact names, plus a guard against future pinned regression test versions.
- Version advanced to 0.2.37, preserving the implemented TEST0.2.36 tracker navigation changes and Song-format diagnostics.
- **Song file loading/previewing**: the reported `Unsupported Song library format` requires separate verification with a file actually written by the plugin. This release does not claim to repair that runtime failure.
- Full JUCE/MSVC Windows VST3 build must still be verified by GitHub Actions.
