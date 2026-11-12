# Biscot1 Synth VST3 — TEST0.2.18.4 (F1 clipboard and navigation)

## Ctrl+C, Ctrl+X, Ctrl+V fixed

In the native JUCE ComboBox on Windows, Ctrl+letter may deliver a control
character (e.g. Ctrl+C = ASCII 3) instead of a normal text character. The
old `letter()` handler only checked the text character against 'c', 'x', 'v'.
The new `TrackerShortcutKey.h` recognizes both ordinary key codes and ASCII
control characters, with a separate C++ Release-mode regression test.

- Ctrl+C / Ctrl+X copy or cut the current F1 cell; if a step range is
  selected they copy/cut the complete steps across OP, Filter and LFO.
- Ctrl+V inserts the last copied cell or pastes the complete step range.
- Ctrl+Alt+Shift+C copies the whole current Pattern; Ctrl+V pastes it into
  the target Pattern, with an undo snapshot.

## More of the original Lua F1 controls

- Tab speaks Pattern number, section, step, cell/parameter and value without
  moving keyboard focus. This matches `speak_tab_position()`.
- Enter opens an accessible Pattern number edit (0–999).
- Ctrl+J opens an accessible step number edit (0–current length minus one).
- Ctrl+PageUp/Down searches earlier/later nonempty steps in any section.
- Alt+Left/Right chooses the vertical jump size (1, 5, 10, 15, 20).
- Alt+Home/End sets current F1 numeric value to minimum/maximum.
- Backspace restores the original value of the current Pattern cell.
- Ctrl+Shift+Alt+Delete initializes the current Pattern and supports Undo.
- Ctrl+Shift+Up/Down advances to the previous/next step for future transport.

The existing user-approved Home/End directions in F1 and F2 are not changed.
This remains an editor-only TEST: no tracker audio playback, F3/F4 Song, F5 browser
or F6 diagnostics yet. No changes to the audio processor, renderOne(), MIDI,
CPU idle, state persistence schema or VST3 parameter IDs.

## Tests

The new standalone C++ shortcut test executes in Release as well as Debug.
GitHub Actions runs a new static integration check plus the additional C++ test.
The full JUCE Windows VST3 build is left to GitHub Actions.
