# Biscot1 Synth VST3 — TEST0.2.19.1

Fixes to the first audible F1/F2 tracker. Starting point: TEST0.2.19.

- Read named choice labels in step cells from the existing VST3 ParameterCatalog (Trigger/Legato, oscillator and LFO Wave, Arp Mode, LFO Mode, Off/On, Pitch Bend Mode, Chord Op1 Controls, etc.), with original Lua labels for Curve Ratio.
- Preserve the preferred horizontal cell across short or empty steps, using the same distinct visible/logical cursor as the original TrackerControl Lua 0.166.
- Preserve an independent preferred cell per OP/Filter/LFO section.
- Add a JUCE-free C++ test (including Release builds) and a Windows workflow regression step.

**Unchanged:** audio/MIDI DSP, sample-accurate Pattern transport, F1/F2 data serialization, preset format and CPU sleep. The extended Lua functionality (Curve evaluation, Sub Note, Step Recording, MIDI pattern assignment, Release Arp, F1 standalone export) remains pending. Full Windows JUCE/NVDA testing is still needed.
