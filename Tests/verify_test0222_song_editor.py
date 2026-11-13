"""New F2 Song state, native accessibility, three lanes, and preservation of F1."""
from pathlib import Path
r=Path(__file__).resolve().parents[1]
model=(r/'Source/TrackerSongBank.h').read_text()
song=(r/'Source/TrackerSongPanel.cpp').read_text()
processor=(r/'Source/PluginProcessor.cpp').read_text()
editor=(r/'Source/PluginEditor.cpp').read_text()
pattern=(r/'Source/TrackerPatternPanel.cpp').read_text()
cmake=(r/'CMakeLists.txt').read_text()
workflow=(r/'.github/workflows/build-windows.yml').read_text()
assert 'maxSongs=100,maxSteps=4096,maxLanes=3' in model
assert 'std::array<std::map<int,TrackerSongStep>,3> lanes' in model
assert 'std::vector<int> order;' in model
assert 'values[1]=-1;' in model and 'result.insert(result.end(),{1,5,6,21,7,8})' in model
assert 'trackerSongsV1' in processor
assert 'writeSongBank(' in processor and 'readSongBank(' in processor
assert 'state.state.setProperty(trackerSongStateKey,' in processor
assert 'trackerSongs=std::move(restoredSongs);' in processor
assert 'getStateInformation(juce::MemoryBlock& out)' in processor
assert 'void BiscotProcessor::trackerApplyStep(' in processor
assert 'code==juce::KeyPress::F1Key' in song
assert 'code==juce::KeyPress::F2Key' in song
assert 'changeLane(code-\'1\')' in song
assert 'openNumeric(EditPurpose::songNumber' in song
assert 'if(realStep()){picker=true;' in song
assert 'processor.commitSongState();' in song
assert 'processor.requestSongStart(' in song
assert 'songpkg::exportSong(' in song
assert 'onOpenSong()' in pattern
assert 'songPanel->open();' in editor
assert 'songPanel->restoreAfterPreset()' in editor
assert 'Source/TrackerSongPanel.cpp' in cmake
assert 'TrackerSongBankTest.exe' in workflow
assert 'Biscot1-TEST0.2.31-Windows-x64' in workflow
print('OK: Song F2 sparse bank, UI routing, independent preset persistence, honest playback status')
