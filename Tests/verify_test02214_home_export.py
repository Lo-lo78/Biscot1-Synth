from release_version_guard import release_version, release_test, release_cmake, release_artifact
"""TEST0.2.35: local F1 Home/End, region Ctrl+Home/End and portable Pattern export."""
from pathlib import Path
root=Path(__file__).resolve().parents[1]
ui=(root/'Source/TrackerPatternPanel.cpp').read_text()
header=(root/'Source/TrackerHeaderNavigation.h').read_text()
codec=(root/'Source/TrackerPatternText.h').read_text()
panel_h=(root/'Source/TrackerPatternPanel.h').read_text()
cmake=(root/'CMakeLists.txt').read_text()
workflow=(root/'.github/workflows/build-windows.yml').read_text()
assert release_cmake in cmake
assert 'trackerF1AreaEdge(row,cell,pat.length,pat.repeat,' in ui
assert 'm.isCtrlDown() && !m.isAltDown() && !m.isShiftDown()' in ui
assert 'trackerF1HomeTarget(row,pat.length,lastRealStep)' in ui
assert 'trackerF1EndTarget(row,pat.length,pat.repeat,lastRealStep)' in ui
assert 'trackerF1AreaEdge(' in header and 'return {length,0,true}' in header
assert 'return {length+1+idx/5, (idx%5)*2+1, true}' in header
assert 'if(row==length && repeats>0)return length+1' not in header
assert 'if(m.isCtrlDown() && !m.isAltDown() && !m.isShiftDown() && letter(\'s\'))' in ui
assert 'openNumericEditor(NumericPurpose::savePatternName,' in ui
assert 'saveCurrentPattern(txt);return true;' in ui
assert 'getChildFile("patterns")' in ui
assert 'getChildFile("songs")' in ui
assert 'TemporaryFile temporary(file)' in ui
assert 'patternExportFiles.clear()' in ui
assert 'serializeTrackerPattern(currentPattern(),slot)' in ui
assert 'std::map<int,juce::File> patternExportFiles;' in panel_h
assert 'BISCOT1_TRACKER_PATTERN_V1' in codec
assert 'deserializeTrackerPattern(' in codec
assert 'TrackerPatternTextTest' in cmake and 'TrackerPatternTextTest.exe' in workflow
assert release_test in workflow
print('OK TEST0.2.35 F1 navigation regions, Ctrl+S Pattern library and validated codec')
