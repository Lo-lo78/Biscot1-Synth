# Biscot1 Synth — 0.99.1
Release date: 13 November 2026

First public VST3 release. Accessible native SID synthesizer, Pattern/Song tracker, Song End chains, host BPM and Song Trigger automation, libraries and presets.

Alt+A opens the native accessible About panel, matching LJuno-116's structure: version, release date, AGPL-3.0-or-later license, GitHub project and contact. Tab and Shift+Tab navigate; Esc/Alt+C close and restore focus.

Project: https://github.com/Lo-lo78/Biscot1-Synth
Contact: vmanolo301@gmail.com

Build correction: replaced unsupported JUCE TextEditor::setKeyHandler with an accessible TextEditor subclass; public version remains 0.99.1.

## Pre-publication host transport refinement

- Split the previous Song Trigger into Song Select (0–99) and Song Transport (Stop, Pause, Play).
- Song Select never starts a Song; default transport Stop preserves manual control.
- Stop resets the Song playback; Pause freezes three-lane sequencing and releases active notes; Play resumes after Pause or starts at the beginning after Stop.
- A new REAPER Play starts the selected Song only if the Song Transport parameter is Play.
- The old host parameter identifier `trackerSongTrigger` is retained for Song Select; previous automation values need to be revised for the new 0–99 mapping.
