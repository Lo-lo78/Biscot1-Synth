# Biscot1 Synth TEST0.2.44 — Factory reset and Song Delay Ping Pong

- Fix Song Delay Ping-Pong Depth: centered operator audio is injected into the left delay line as depth increases; repeats alternately feed the opposite channel instead of remaining indistinguishable on centered stereo sources. Dry sound unchanged.
- In F1 Pattern, Backspace resets header Length/MIDI Note/recording/Release Arp, Repeat, Speed/Transpose and the selected step parameter to its initial value.
- In F2 Song, Backspace resets Length/MIDI Note, Repeat, Speed, Song End or the selected parameter to its initial value; it does not remove the parameter from the step (Delete retains that role).
- Outside Alt+Q, Alt+I opens an accessible Yes/No factory reset dialog (No by default, Y immediate Yes, N/Escape immediate No, arrow selection and Enter). Reset clears all synth controls, Song Trigger, Pattern and Song banks and editor metadata, but never library files on disk.
- Playback remains the established Song Trigger mode. Seeking through DAW project time still does not imply seeking the Song: future optional Follow DAW mode would require an event-to-beat seeking engine with deterministic chains and tempo mapping.
- Requires full Windows VST3/REAPER test; local Python/C++ standalone tests cover DSP and reset shortcut patterns.
