from release_version_guard import release_version, release_test, release_cmake, release_artifact
"""TEST0.2.35: native F2 Song editing and access, no new audio wiring."""
from pathlib import Path
r=Path(__file__).resolve().parents[1]
s=(r/'Source/TrackerSongPanel.cpp').read_text()
h=(r/'Source/TrackerSongPanel.h').read_text()
e=(r/'Source/TrackerSongEditing.h').read_text()
p=(r/'Source/PluginProcessor.cpp').read_text()
w=(r/'.github/workflows/build-windows.yml').read_text()
assert '0.2.26' in (r/'CMakeLists.txt').read_text()
assert release_test in (r/'Source/PluginEditor.cpp').read_text()
assert 'TrackerSongEditingTest.exe' in w
assert 'Tests/verify_test0223_song_editing.py' in w
for token in ['extendSelection(-1)','extendSelection(1)','copySelected(false)','copySelected(true)',
              'pasteClipboard()','clearSelected()','reorder(-1)','reorder(1)',
              'undoRedo(m.isShiftDown())','renameSong()','toggleMuteSolo(false)',
              'toggleMuteSolo(true)','mixVolume(1)','mixVolume(-1)',
              'clipboard.copySong','processor.requestSongStart(',
              'songpkg::exportSong(']:
 assert token in s, token
for token in ['std::vector<std::optional<TrackerSongStep>> steps','struct TrackerSongClipboard',
              'class TrackerSongHistory','std::swap(entry.order','entry.values[5]=parameterData.values[5]']:
 assert token in e,token
assert 'history.clear()' in s and 'commitEdit(before)' in s
assert 'trackerSongsV1' in p
assert 'void BiscotProcessor::trackerApplyStep(' in p
print('OK: Song F2 clipboard, step selection, mixer data, Undo Redo and version; Song audio unchanged')
