# Biscot1 Synth — TEST0.2.40

## F2 Song parameter-picker borders: silent, like F1 Pattern

In Alt+Q > F2 Song, pressing Enter on a step opens the Song parameter picker.
Previously, Up/Down/Left/Right/PageUp/PageDown at the first or last available
parameter could refresh and announce the same parameter, because movement was
clamped but unconditionally announced. Navigation now refreshes and speaks only
when the selected parameter actually changes. Home/End and Ctrl+Home/End share
the same behavior; when the movement is impossible there is no repeated speech.

Parameter search, insertion, Enter, Ctrl+Enter, Esc and focus behavior remain
unchanged. The same boundary semantics are already used by F1 Pattern.

## Validation

A new regression guard in GitHub Actions compares F1 and F2 silent-edge behavior.
The Windows VST3 compilation and NVDA/OSARA interactions still require testing.
The Song library `Unsupported Song library format` issue remains a separate open issue.
