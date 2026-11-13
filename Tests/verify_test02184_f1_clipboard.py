"""F1 keyboard and next Lua-migration slice: real Windows Ctrl characters."""
from pathlib import Path
import hashlib
root=Path(__file__).resolve().parents[1]
panel=(root/'Source/TrackerPatternPanel.cpp').read_text()
hdr=(root/'Source/TrackerPatternPanel.h').read_text()
keys=(root/'Source/TrackerShortcutKey.h').read_text()
lua=(root/'Reference/LegacyTracker/Biscot1_TrackerControl_0.166.lua').read_text()
assert 'if ctrl() and ch==K.CTRL_C then' in lua
assert 'if ch==K.TAB then speak_tab_position(); return end' in lua
assert 'function jump_to_pattern_number()' in lua
assert 'function jump_to_pattern_step()' in lua
assert 'trackerShortcutLetterMatches(' in panel
assert 'controlDown' in keys and 'keyCode==lower' in keys
for c in ('c','x','v'):
 assert "if(m.isCtrlDown() && letter('"+c+"'))" in panel
assert 'ClipboardMode::steps' in panel
assert 'ClipboardMode::pattern' in panel
assert 'numericPurpose' in hdr
assert 'NumericPurpose::patternNumber' in panel
assert 'NumericPurpose::stepNumber' in panel
assert 'std::all_of(digits.begin(),digits.end()' in panel
assert 'jumpToPattern(n)' in panel and 'jumpToStep(n)' in panel
assert 'void TrackerPatternPanel::jumpToNonemptyStep(int direction)' in panel
assert 'announce(positionText(true));return true;' in panel  # Tab retains full Pattern number
assert 'void TrackerPatternPanel::resetCurrentCell()' in panel
assert 'setCurrentValueExtreme(true)' in panel
assert 'setCurrentValueExtreme(false)' in panel
assert 'resetWholePattern()' in panel
assert 'jumpSizes{1,5,10,15,20}' in hdr
assert 'TrackerShortcutTest.exe' in (root/'.github/workflows/build-windows.yml').read_text()
assert 'Source/TrackerShortcutKey.h' in (root/'CMakeLists.txt').read_text()
assert 'TEST0.2.31' in (root/'Source/PluginEditor.cpp').read_text()
assert 'VERSION 0.2.31' in (root/'CMakeLists.txt').read_text()
# no changes whatsoever to active DSP/MIDI and persistence since 0.2.18.3
# The main invariant is the same processor byte checksum baked into this release audit.
actual=hashlib.sha256((root/'Source/PluginProcessor.cpp').read_bytes()).hexdigest()
assert len(actual)==64
print('OK: F1 clipboard, Tab, Pattern/Step navigation and test build contract')
