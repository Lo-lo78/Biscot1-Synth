"""Early regression test for MSVC C2662 in the F1 cursor-region getter.

The full JUCE/MSVC build validates the actual source; this check catches
a missing const overload before the expensive JUCE compilation.
"""
from pathlib import Path
import re

root = Path(__file__).resolve().parents[1]
header = (root / "Source/TrackerPatternPanel.h").read_text(encoding="utf-8")
source = (root / "Source/TrackerPatternPanel.cpp").read_text(encoding="utf-8")

# The mutable accessor is still necessary for in-place parameter edits.
mutable = r"TrackerPattern\s*&\s*currentPattern\(\s*\)\s*\{"
read_only = r"const\s+TrackerPattern\s*&\s*currentPattern\(\s*\)\s*const\s*\{"
assert re.search(mutable, header), "Missing mutable currentPattern() for grid editing"
assert re.search(read_only, header), "Missing const currentPattern() needed by cursorRegion() const (MSVC C2662)"
assert "const_cast" not in header, "Do not circumvent the F1 read-only accessor with const_cast"

# Both calls were the actual MSVC C2662 errors (lines 871-872 in 0.2.37).
match = re.search(r"int\s+TrackerPatternPanel::cursorRegion\(\s*\)\s*const\s*\{([^}]+)\}", source)
assert match, "Missing cursorRegion() const"
assert match.group(1).count("currentPattern()") == 2, "Unexpected cursorRegion() access path"
print("OK: F1 cursorRegion() const has a matching read-only Pattern accessor (MSVC C2662 guard)")
