#pragma once

namespace biscot {
// Pitch Bend Mode: 0 = Multi (original behaviour), 1 = Omni.
// This intentionally does not modify note allocation or accept notes on new
// channels in MIDI Channels 1-3 mode. Omni means *pitch wheel* from any of the
// sixteen MIDI channels, NOT MIDI-channel note Omni or a new fourth voice.
// The callback receives an operator index (0..2) and the normalized bend.
template <class Assign>
void routePitchBend(int midiChannel, float bend, int midiMode,
                    int pitchBendMode, Assign&& assign) {
 if (midiChannel < 1 || midiChannel > 16) return;
 if (pitchBendMode == 1 || (midiMode == 0 && midiChannel == 1)) {
  for (int op = 0; op < 3; ++op) assign(op,bend);
 } else if (midiChannel <= 3) {
  assign(midiChannel-1,bend);
 }
}
} // namespace biscot
