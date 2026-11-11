# Biscot1 Synth VST3 — TEST0.2.14

## Point 3 — Filter ADSR / LFO and Trigger alignment

Starting point: TEST0.2.13, with the smooth sustain-pedal release fix retained.

- Filter ADSR Op1 Trigger/Legato routing has been made explicit, including the pedal and continuous arpeggio cases. A note that joins an arpeggio updates filter key follow without restarting its envelope.
- Each LFO now starts at the JSFX state: phase zero, age two, Delay clock 1,000,000 seconds, active. Deterministic seeds 7654321 / 8765432 / 9876543.
- Note-triggered LFO Delay always restarts; phase/age only restart with Trigger or One Shot enabled.
- Delay, Smooth, One Shot and separate upper/lower squash now follow the original operation order. One Shot can be disabled after completion and the LFO will resume.
- Added JUCE-free tests in CMake and the Windows GitHub Actions workflow.

**Not part of this release:** idle CPU sleep / cold reset optimization (point 4), full JSFX-VST audio and MIDI parity session (point 5), tracker (Alt+Q).

No oscillator, filter core or output-stage DSP coefficients changed. No VST parameter ID changes.
