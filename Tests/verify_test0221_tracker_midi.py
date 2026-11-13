from release_version_guard import release_version, release_test, release_cmake, release_artifact
from pathlib import Path
r=Path(__file__).resolve().parent.parent
panel=(r/'Source/TrackerPatternPanel.cpp').read_text()
processor=(r/'Source/PluginProcessor.cpp').read_text()
header=(r/'Source/PluginProcessor.h').read_text()
workflow=(r/'.github/workflows/build-windows.yml').read_text()
assert 'positionText(true)' in panel and 'positionText(bool includePattern)' in panel
assert 'setTab(section)' in panel and 'refresh(true);auditionCurrentStep()' in panel
assert 'Step Recording, ' in panel and 'Release Arpeggio, ' in panel
assert 'trackerRecordStep' in panel and 'popTrackerRecordedMidi' in panel
assert 'trackerMidiMap.store' in processor and 'handleTrackerExternalMidi(message)' in processor
assert 'trackerReleaseArpTick()' in processor and 'trackerTransport.activeSubNotes' in processor
assert 'TrackerMidiFeaturesTest.exe' in workflow and release_artifact in workflow
assert release_cmake in (r/'CMakeLists.txt').read_text()
print('OK TEST0.2.21 speech, sub note, step recording, release arp and MIDI trigger wiring')
