# Biscot1 Synth — TEST0.2.39

## Song import: full tracker replacement

F2 Ctrl+O now asks **Yes or No** before importing either `.biscot1-song` or `.biscot1-all-song`.
Upon Yes, the import validates the complete package *first*, then atomically clears the previous Pattern bank and all previous Song slots. Imported Patterns keep their original 0–999 IDs; no search for spare slots and no accidental Pattern collisions. A single Song is placed at the currently selected Song slot (0–99), while the 100-Song archive restores its fixed bank. With No, Escape, an invalid file or failed validation, existing banks are unchanged. The synth sound and parameters are never replaced by a Song file.

## Preset safety and accessible confirmation

In the synth, Ctrl+O and Alt+B both open the complete `.biscot` preset browser. Selecting a file does not silently preview/replace instrument state; **Enter asks for a Yes/No confirmation** before loading the entire preset. Existing save-overwrite and deletion confirmations use the same controls. The default selection is No; Left/Down selects No; Right/Up selects Yes. Enter executes the selected choice; pressing Y or N immediately executes the corresponding choice; Escape is No. Both choices are keyboard focusable buttons. Alt+B inside Alt+Q is disabled silently.

## Pattern saves

Repeated F1 Ctrl+S exports are now duplicate-safe, creating `_2`, `_3`, etc. instead of silently overwriting the previous file.

## Cursor memory

Pattern F1 and Song F2 now save their row, column and region in the host/restored plugin state. Closing and reopening the plugin UI should restore the last navigator position, not always Length; switching Pattern/Song retains the matching area. Test with REAPER + NVDA/OSARA.

## Verification

Targeted Python regression is added to GitHub Actions. The full Windows JUCE/MSVC build and actual NVDA/OSARA interaction need to be checked on GitHub/REAPER.
