# Biscot1 Synth VST3 — TEST0.2.34

## Incremental filename search in every Biscot1 browser

- Ctrl+O in F1 (Pattern library), Ctrl+O in F2 (Song library), and the `.biscot` preset browser now use the same keyboard-first incremental search.
- Start typing the filename without entering an edit field: `p` finds the next matching file/folder, `pa` and `pat` progressively narrow the search. An inactivity gap of about 1.2 seconds starts a new query; repeating one letter cycles through matches. Case-insensitive and supports prefixes of words in names separated by spaces, periods, underscores, and hyphens.
- Both files and folders take part in the incremental search. Moving with arrows resets the search; the filename editor continues to receive ordinary text normally.
- **P is no longer an audio-preview shortcut**: it is free to navigate through filenames. The manual audio-preview key in the Pattern/Song library is **Space**.

## Consistent folder navigation

- Right Arrow enters the selected directory; Left Arrow and Backspace go to the parent directory, in the Pattern, Song, and `.biscot` preset browsers. Enter still loads a file or opens a folder.
- Inside a `.biscot1-all-song` package, **Ctrl+Left / Ctrl+Right** now select the slot to preview, 0–99. Unmodified Left/Right are reserved for folder navigation in every browser.
- The library browser restores selection to the directory just exited when navigating to the parent.

## Remembered Ctrl+P

- The Pattern and Song library each save their own Ctrl+P automatic-preview preference in JUCE user-level settings, across closing/reopening the browser and restarting REAPER. The preferences are not part of a preset and do not overwrite instrument state.
- Space remains available for explicit preview regardless of automatic mode. The initial choice is Off until the user changes it.

## Song library preview on three operators

- In the Song browser, preview compiles a short immutable arrangement from the first available Pattern on **each of the three operators**, starting them together and retaining each clip's Pattern Start, Speed, Transpose and Song mixer volume/mute/solo settings.
- Preview length remains selectable with `+/-`: 1, 3, 8, 16, 32 Pattern steps (default 3); it does not play full Pattern repeats or follow Song End.
- Song FX controls already present before the previewed clip (Filter/LFO ownership, Sidechain, Delay) are primed into the temporary snapshot where appropriate.
- To recognize all three operators even if the Song places its first clips on different rows, the library aligns each operator's first available clip at the start of the short preview. This is a summary audition, **not** a timeline-exact preview of all Song rows.
- **F2 Song step navigation audition remains one operator** at a time, as in the Lua reference. The full Song playback and DSP are unchanged.

## Verification

- 52/52 Python checks, including new browser regression checks.
- 33/33 standalone C++ tests in Release (including a new three-operator library preview test).
- Full JUCE VST3 Windows/MSVC build, audio and NVDA keyboard behavior still need REAPER/GitHub Actions testing.
