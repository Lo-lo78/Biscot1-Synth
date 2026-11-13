from pathlib import Path
r=Path(__file__).resolve().parents[1]
panel=(r/'Source/TrackerPatternPanel.cpp').read_text()
processor=(r/'Source/PluginProcessor.cpp').read_text()
playback=(r/'Source/TrackerPatternPlayback.h').read_text()
cat=(r/'Source/TrackerPatternAutomation.h').read_text()
cmake=(r/'CMakeLists.txt').read_text()
workflow=(r/'.github/workflows/build-windows.yml').read_text()
assert 'VERSION 0.2.30' in cmake
assert 'TEST0.2.30' in (r/'Source/PluginEditor.cpp').read_text()
assert 'Biscot1-TEST0.2.30-Windows-x64.zip' in workflow
assert 'TrackerCurvePreviousTest Tests/TrackerCurvePreviousTest.cpp' in cmake
assert 'TrackerPatternAutomation.h' in playback
assert 'trackerCompileAllAutomation(pattern,flattened)' in playback
assert 'trackerRunCurveEvents(row)' in processor
assert 'trackerCurveNextSegments.fill(1)' in processor
assert 'trackerCurveDueSegment' in cat
assert 'trackerCurveAt' in processor and 'trackerCurveEventValue' in processor
assert 'insertTrackerParameter(candidate,row,tab,*def,cell)' in panel
assert 'for(const int shared:{3,4,6})' in panel
assert 'Sub Note requires OP Arp Mode' in panel
assert 'std::make_shared<const TrackerPlayablePattern>' in processor
print('OK TEST0.2.21 Previous, Curve, Ratio, Resolution, F2 and immutable audio integration')
