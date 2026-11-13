"""Guard the two F1/F2 accessibility fixes before building on Windows."""
from pathlib import Path
root=Path(__file__).resolve().parents[1]
ui=(root/'Source/TrackerPatternPanel.cpp').read_text()
header=(root/'Source/TrackerPatternPanel.h').read_text()
labels=(root/'Source/TrackerCellPresentation.h').read_text()
workflow=(root/'.github/workflows/build-windows.yml').read_text()
assert 'logicalCell' in header and 'lastLogicalCell' in header
assert 'cell=trackerVisibleCell(logicalCell,navigator.getNumItems())' in ui
assert 'trackerF1HomeTarget(row,pat.length,lastRealStep)' in ui and 'lastRealStep=row' in ui
assert 'return juce::String(trackerCellLabel(c).c_str());' in ui
assert 'tracker->synthIndex' in labels and 'synth.choices' in labels
assert 'trackerCellValue(cell.id,cell.value)' in labels
assert 'TrackerCellPresentationTest.exe' in workflow
assert 'VERSION 0.2.31' in (root/'CMakeLists.txt').read_text()
print('OK: synth-native choice labels and persistent F1 logical cell')
