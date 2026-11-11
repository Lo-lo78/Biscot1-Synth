# Biscot1 Synth VST3 TEST0.2.13 — Smooth CC64 Arp Release

- CC64-up no longer changes the SID Seq pitch root as the volume envelope enters Release; removes a potential note-reference discontinuity.
- When the last physical chord key is released under CC64, the current chord root is latched for the pedal-held phrase and its later Release.
- Classical arpeggiators snapshot the complete active note pool before dropping CC64-held notes; MIDI events only affect pitch on clock edges.
- VCA Release remains the identical exponential curve used for manual key release; all arpeggios keep running through the natural Release tail until idle.
- No new parameters. SID audio DSP, accessibility, browser, MIDI mode, Trigger and Legato unchanged.
- Added standalone C++ tests and GitHub Actions verification. Full Windows JUCE build and audible behavior still require validation in REAPER.
