# Biscot1 Synth VST3 – TEST0.2.15

## Point 4: cold idle / CPU optimizations

- The audio processor skips complete silent sample spans while still consuming MIDI at the exact offsets. Audio buffers are cleared and no SID oscillators, filter, envelopes or output stages run during cold idle.
- Cold idle is entered **only after** all physical/CC64 gates have closed, the three audible VCA envelopes have finished their release, AND the output of the enabled stereo buses has remained below -100 dBFS for 64 consecutive samples. No hard cut to an audible release or arpeggio is introduced.
- The filter, output-stage and voice histories are reset **once** on the transition into cold idle, never during the release. Physical pedal state and free-LFO phase are preserved.
- The three Free LFOs keep their BPM-based phases moving during idle, at span/block rate rather than at every sample. One Shot and Trigger LFOs do not advance in idle. LFOs with no active destinations are suspended while sounding, following the JSFX modulation-source usage rules.
- New square-wave duty control for each of the three LFOs: `LFO1 Square PWM` (`s231`), `LFO2 Square PWM` (`s232`), `LFO3 Square PWM` (`s233`). Range 0.050–0.950; default 0.500, shown to four decimals in Grid/Value. Only the Square LFO waveform is affected, not SID oscillator PWM.

## Accessibility

- Initial focus transfer uses the LB-303 deferred native UI Automation ComboBox handoff, avoiding premature focus on an unlabelled root. No eager constructor focus, no repeated title/instruction speech, no changes to arrow/Grid speech. Actual NVDA behaviour requires a Windows/REAPER test.

## Compatibility

- The 224 original JSFX parameter IDs (`s002`–`s225`) and five existing native-only parameters (`s226`–`s230`) are untouched; the three new parameters are appended. Previous `.biscot` preset snapshots retain stable parameters; new controls default to 0.500.
- No changes to the SID wavetable, filter formulas, Ring/Sync, arpeggios, ADSR equations, MIDI routing, preset browser or Alt+Q reservation.
- Windows VST3 compilation is not performed locally; GitHub Actions runs the existing suite plus new C++/static tests. The upcoming point 5 remains a full audio/MIDI REAPER comparison.
