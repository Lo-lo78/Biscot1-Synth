# Biscot1 VST3 TEST0.2.19.2 — F1 Lua cursor and same-parameter step navigation

- Fix a GUI-specific regression in TEST0.2.19.1: the ComboBox onChange callback could collapse the logical horizontal cursor to the currently visible cell while crossing a shorter step, despite the data-only cursor test succeeding. An unchanged ComboBox selection is ignored, preserving the Lua remembered column until the user explicitly moves horizontally.
- Add **Ctrl+Alt+Down** and **Ctrl+Alt+Up** in F1 to jump to the next or previous occurrence of the currently selected parameter ID in the same section, skipping unrelated steps. The match is announced and auditioned; boundaries are reported without moving the edit cursor. This shortcut is new, not present in the supplied Lua 0.166.
- Add independent C++ Release regression and static verification of the actual GUI event handler. No changes to synthesis DSP, audio clock, MIDI processor, pattern persistence, preset format, or F2 navigation.
- GitHub Actions performs Windows JUCE VST3 compile and new regression test; Windows + NVDA user verification remains required.
