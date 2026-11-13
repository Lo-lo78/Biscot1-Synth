# Biscot1 TEST0.2.27 — standalone Song/Pattern library (integration)

Based on TEST0.2.26 Song Chain Preview Focus, preserving Song End chaining, per-step preview, Preview Steps, separate F1/F2 navigation and screen-reader announcements.

- F1 Ctrl+S: export standalone `.biscot1-pattern` to Documents/Biscot1-Synth/Patterns/Singles.
- F1 Ctrl+O: import a standalone Pattern via system file chooser, assigning a free slot if necessary.
- F2 Ctrl+S: export the selected Song and all referenced Patterns into a relative two-folder package, with manifest saved last.
- F2 Ctrl+O: import a Song and dependent Patterns; check all files before changing banks, remap occupied pattern/song IDs.
- Alt+S: existing full `.biscot` preset workflow unchanged.

Limitations: standalone Song export currently refuses Song End chains to another Song, as it packages one Song only. Dialog is native file selection, not yet the complete keyboard-driven browser with sound audition. Windows VST3 build and NVDA/OSARA use require real-world testing.
