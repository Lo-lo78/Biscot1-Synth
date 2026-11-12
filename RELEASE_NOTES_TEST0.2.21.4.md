# Biscot1 VST3 TEST0.2.21.4 — F1 Home/End semantics and Patterns/Singles export

- F1 Home/End stay inside current Header, Steps or Repeat/Speed/Transpose area; Ctrl+Home/End switch areas, restoring the last visited Step.
- Footer End reaches the last available Transpose even for partial speed rows; F1 Step Home/End keep the logical cell.
- F2 parameter picker retains its existing independent keyboard navigation. Alt+Home and Alt+End continue to set max/min values.
- Ctrl+S from F1 exports active Pattern to `Documents/Biscot1-Synth/Patterns/Singles/Pattern_NNN.biscot1-pattern`; creates `Patterns/Singles` and `Songs` folders. Collision-safe suffixes avoid silently overwriting different exported Patterns. The current editor session remembers the export target for subsequent Ctrl+S updates.
- Standalone Pattern text records all 3 parameter tabs, ordered cells, note mapping, length, Repeat, repeat Speed/Transpose and Release Arpeggio, with Unicode name support and validated read path for later library browsing.
- Existing Alt+S `.biscot` whole-preset saving unchanged. **F2 Song / Ctrl+S Song still not implemented.** Song will export relative references to `Patterns/<SongName>/`, not duplicate synth state.
- No changes to the audio processor, arp or DSP routines, transport or VST3 preset serialization.
- Regression tests include release-mode F1 navigation and Pattern export/import round trip. Windows JUCE compilation/NVDA verification are still required.
