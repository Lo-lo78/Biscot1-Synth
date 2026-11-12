"""Regression for F1 numeric section selection and Lua F2 behavior."""
from pathlib import Path
r=Path(__file__).resolve().parents[1]
source=(r/'Source/TrackerPatternPanel.cpp').read_text()
helper=(r/'Source/TrackerShortcutKey.h').read_text()
test=(r/'Tests/TrackerShortcutTest.cpp').read_text()
workflow=(r/'.github/workflows/build-windows.yml').read_text()
assert 'trackerShortcutSectionIndex(code,(int)k.getTextCharacter())' in source
assert 'if(section>=0){setTab(section);return true;}' in source
assert "letter('1')" not in source and "letter('2')" not in source and "letter('3')" not in source
assert "digit='1';digit<='3'" in helper
assert 'trackerShortcutSectionIndex(digit,0)==i' in test
assert 'trackerShortcutSectionIndex(0,digit)==i' in test
assert 'TrackerShortcutTest.exe' in workflow
assert 'VERSION 0.2.21' in (r/'CMakeLists.txt').read_text()
assert 'TEST0.2.21' in (r/'Source/PluginEditor.cpp').read_text()
assert 'Biscot1-TEST0.2.21-Windows-x64' in workflow
# Do not alter F2's search/section behavior.
assert 'm.isCtrlDown()&&code==juce::KeyPress::tabKey&&page==2' in source
assert 'biscotParameterSearchCharacter(k)' in source
print('OK: F1 section digits, F2 Ctrl+Tab/search, release workflow and version')
