# Biscot1 TEST0.2.36 — Tracker navigation and Song import diagnostics

- Alt+B is disabled inside Alt+Q F1/F2; the synth preset browser remains outside the tracker.
- Silent list boundaries in the Pattern/Song library browser.
- Switching Pattern or Song preserves the cursor region and, where possible, its step/column.
- F1/F2 switches preserve the semantic cursor region (header, steps, Repeat, Speed).
- F1 direct shortcuts L/M/S/R/P/D/T open the numeric editor and restore the original cursor after Enter/Escape.
- F2 L/M/P/R/D/S direct numeric editing preserves the original cursor.
- Song library loader tolerates UTF-8 BOM and CRLF in the header and reports an unknown header to diagnose previously saved files.

**Important:** The reported `Unsupported Song library format` cannot be conclusively fixed without examining one of the actual `.biscot1-song` or `.biscot1-all-song` files produced on the user's Windows installation. This version adds diagnostics and BOM tolerance, but does not assert compatibility with unknown file formats. JUCE/MSVC compilation and NVDA tests remain pending.
