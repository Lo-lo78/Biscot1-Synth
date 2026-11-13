# Biscot1 Synth VST3 — TEST0.2.41

## Windows/MSVC build correction: PresetBrowser.cpp

GitHub Actions for TEST0.2.40 reached full JUCE VST3 compilation and stopped
on `Source/PresetBrowser.cpp:47` with compiler errors **C3535**, **C2440** and
**C2665**. The `for (auto* b : { &accept, &cancel, &back })` expression
mixed `ConfirmationButton*` and `juce::TextButton*`; the compiler could not
deduce one pointer type for the initializer list.

The constructor now registers `accept`, `cancel` and `back` separately with
`addAndMakeVisible()`. These are the exact same controls with the same actions:
no change to preset loading, confirmations, Song/Pattern controls, MIDI or DSP.

A dedicated Python regression check runs at the start of the Windows workflow.
Version labels, artifact name and release documentation were updated to 0.2.41.

The full Windows JUCE VST3 build must still be verified on GitHub Actions.
The previously reported `Unsupported Song library format` is not addressed
in this compilation-only patch.
