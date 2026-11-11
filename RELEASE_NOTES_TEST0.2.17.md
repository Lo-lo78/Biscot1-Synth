# Biscot1 Synth VST3 — TEST0.2.17

## Accessible entry in REAPER / NVDA

- Brought `BiscotEditor::createAccessibilityHandler()` in line with LB-303:
  `createIgnoredAccessibilityHandler(*this)` suppresses the generic editor
  root from the accessibility tree, without hiding the native Grid/Value/Edit.
- Initial focus transfer now allows a temporarily null focused component,
  matching LB-303, but never steals focus from REAPER or Value/Edit.
- Preserved the delayed native UIA Grid focus and single selection speech.
- No startup synthetic speech notifications, no added control descriptions.
- A regression check is included in GitHub Actions.

## Unchanged

Audio, DSP, idle path, MIDI, arp, ADSR, LFO and presets are identical to
TEST0.2.16. The 0.005c/0.006c idle reading previously reported in REAPER is
for TEST0.2.16; TEST0.2.17 requires user verification for NVDA focus only.
