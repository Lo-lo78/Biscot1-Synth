"""TEST0.2.30 regression: the selected cell is the insertion position."""
from pathlib import Path
r=Path(__file__).resolve().parents[1]
panel=(r/'Source/TrackerPatternPanel.cpp').read_text()
ins=(r/'Source/TrackerPatternInsert.h').read_text()
cmake=(r/'CMakeLists.txt').read_text()
workflow=(r/'.github/workflows/build-windows.yml').read_text()
assert 'VERSION 0.2.30' in cmake
assert 'TEST0.2.30' in (r/'Source/PluginEditor.cpp').read_text()
assert 'insertTrackerParameter(candidate,row,tab,*def,cell)' in panel
assert 'pasteTrackerParameter(candidate,row,tab,cell,clipboard)' in panel
assert 'const int at=cell+1;' not in panel
assert 'trackerInsertionPosition(requestedPosition,(int)cells.size())' in ins
assert 'cells.insert(cells.begin()+at,{d.id,initial})' in ins
assert 'p.insert(step,tab,at,copied)' in ins
assert 'trackerArpActiveBefore(cells,at)' in ins
assert 'TrackerInsertionPositionTest Tests/TrackerInsertionPositionTest.cpp' in cmake
assert 'TrackerInsertionPositionTest.exe' in workflow
assert 'Biscot1-TEST0.2.30-Windows-x64' in workflow
print('OK F1 picker and clipboard insert at current cell, with test build contract')
