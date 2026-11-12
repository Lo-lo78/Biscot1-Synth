# Biscot1 Synth — TEST0.2.21.1 (12 November 2026)

## Release arp: manual Note Off / CC64 parity

Fixed the native synth arpeggiator (independent of the Alt+Q tracker). Previously, as a manually played chord was released key by key, every MIDI Note Off rebuilt and replaced the *release* arp note pool with the shrinking physical note pool. By the final Note Off only the last key remained, so the pitch stopped arpeggiating during the envelope Release. The sustain-pedal path retained the chord, revealing the discrepancy.

The complete release pool is now captured on Note On and preserved through all Note Off messages, while the physical held-key pool continues to update normally. Explicit arp-configuration refreshes may also update the latch. The clock continues during the existing ADSR Release without retriggering amplitude. The release memory is cleared when the envelope finishes, as before. This does not alter the dedicated Alt+Q *Release Arpeggio* setting, which governs a Pattern's own end-of-playback pitch continuation.

## Accessible F1 header navigation

- **L:** Length
- **M:** MIDI Note
- **S:** Step Recording
- **R:** Release Arpeggio
- **P:** Repeat
- **D:** next Speed; **Shift+D:** previous Speed, wrapping through all Repeat entries
- **T:** next Transpose; **Shift+T:** previous Transpose, wrapping through all Repeat entries
- **Up from Repeat** or **Home from Repeat:** returns to the previously edited real step (for example, step 5), rather than Length-1. **End** from the step area still enters Repeat. Shortcut keys do not affect Alt+S or the parameter-picker search.

## Tests and compatibility

Adds JUCE-free regression tests `SidArpReleaseLatchTest` and `TrackerHeaderNavigationTest`, plus a static integration check. All prior Pattern/Curve/MIDI/CPU code is preserved except for the targeted arp-pool MIDI event bookkeeping. The DSP renderer is byte-identical to TEST0.2.21. The `.biscot` preset format has not changed.

This source package is prepared for Windows GitHub Actions. Actual Windows VST3 compilation and auditory/NVDA results must be confirmed on REAPER.
