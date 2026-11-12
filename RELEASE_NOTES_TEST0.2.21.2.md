# Biscot1 Synth — TEST0.2.21.2 (12 November 2026)

Fixes F1 clipboard and Enter-parameter picker insertion at the selected cell.
Pasting/inserting into the third cell now creates the new third cell, moving
old third and later cells right. Supports all three parameter sections.

- Keeps existing single-instance parameter rule (Sub Note may repeat).
- Maintains Curve / Curve Resolution / Curve Ratio musical dependencies.
- Avoids Undo entries or state updates when insertion is refused.
- Release Arpeggio is unchanged: it plays the Pattern's written pitches in
  the release tail and is not the same as the normal synthesis arpeggiator.
- Original DSP, MIDI, transport, NVDA speech and `.biscot` format unchanged.

Local isolated C++ and Python tests; full JUCE/MSVC build requires GitHub Actions.
