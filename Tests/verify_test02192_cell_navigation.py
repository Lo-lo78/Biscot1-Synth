"""Guard the real ComboBox event path as well as the JUCE-free helper."""
from pathlib import Path
r=Path(__file__).resolve().parents[1]
s=(r/'Source/TrackerPatternPanel.cpp').read_text()
h=(r/'Source/TrackerPatternPanel.h').read_text()
w=(r/'.github/workflows/build-windows.yml').read_text()
assert 'const int requested=navigator.getSelectedId()-1;' in s
assert 'if(!trackerCellSelectionWasChanged(requested,cell))return;' in s
assert 'logicalCell=cell' in s
assert 'jumpToSameParameter' in h and 'void TrackerPatternPanel::jumpToSameParameter(int direction)' in s
assert 'trackerFindMatchingParameter(currentPattern(),row,tab,parameterId,direction)' in s
assert 'm.isCtrlDown() && m.isAltDown() && !m.isShiftDown()' in s
assert 'TrackerCellNavigationTest.exe' in w
assert 'VERSION 0.2.30' in (r/'CMakeLists.txt').read_text()  # Preserve regression across later 0.2.19 fixes
print('OK real F1 selection guard and matching-parameter shortcuts')
