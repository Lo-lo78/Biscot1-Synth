# Biscot1 Synth VST3 — TEST0.2.20.1 (12 November 2026)

- Alt+Q F1 **Enter** opens the parameter picker from a real Pattern step; **F2** is reserved for the forthcoming Song page (not yet implemented).
- **Alt+E** is the only exact numeric entry command in F1, including metadata such as Length, Repeat, Speed and Transpose.
- **Alt+Enter** continues to jump to a numbered Pattern from F1 and from the parameter picker.
- In the parameter picker, **Enter** inserts and returns to F1; **Ctrl+Enter** inserts and stays; **Esc** now returns to F1.
- In F1, **Alt+Home = maximum** and **Alt+End = minimum**, as requested. Unmodified Home/End and the parameter-picker navigation remain unchanged.
- No DSP, MIDI, automation, playback or `.biscot` format modifications.
- `verify_test02201_enter_picker.py` guards these keyboard decisions.
- F1/F2 Lua parity is not finished. Native Sub Note audio, MIDI Step Recording, Release Arpeggio, MIDI Pattern triggering, and standalone Pattern exports remain future work, together with further rare Lua commands.
