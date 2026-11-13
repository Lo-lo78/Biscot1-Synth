from release_version_guard import release_version, release_test, release_cmake, release_artifact
"""Regression for F1 numeric section selection and Lua F2 behavior."""
from pathlib import Path
r=Path(__file__).resolve().parents[1]
source=(r/'Source/TrackerPatternPanel.cpp').read_text()
helper=(r/'Source/TrackerShortcutKey.h').read_text()
test=(r/'Tests/TrackerShortcutTest.cpp').read_text()
workflow=(r/'.github/workflows/build-windows.yml').read_text()
assert 'trackerShortcutSectionIndex(code,(int)k.getTextCharacter())' in source
assert 'if(page==1 && row>=0 && row<currentPattern().length)setTab(section)' in source
assert "letter('1')" not in source and "letter('2')" not in source and "letter('3')" not in source
assert "digit='1';digit<='3'" in helper
assert 'trackerShortcutSectionIndex(digit,0)==i' in test
assert 'trackerShortcutSectionIndex(0,digit)==i' in test
assert 'TrackerShortcutTest.exe' in workflow
assert release_cmake in (r/'CMakeLists.txt').read_text()
assert release_test in (r/'Source/PluginEditor.cpp').read_text()
assert release_artifact in workflow
# Do not alter F2's search/section behavior.
assert 'm.isCtrlDown()&&code==juce::KeyPress::tabKey&&page==2' in source
assert 'biscotParameterSearchCharacter(k)' in source
print('OK: F1 section digits, F2 Ctrl+Tab/search, release workflow and version')
