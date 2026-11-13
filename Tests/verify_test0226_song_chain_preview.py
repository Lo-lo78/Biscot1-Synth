"""TEST0.2.35: source regressions for accessible Song End and step preview."""
from pathlib import Path
r=Path(__file__).resolve().parent.parent
processor=(r/'Source/PluginProcessor.cpp').read_text()
header=(r/'Source/PluginProcessor.h').read_text()
editor=(r/'Source/PluginEditor.cpp').read_text()
song=(r/'Source/TrackerSongPanel.cpp').read_text()
model=(r/'Source/TrackerSongBank.h').read_text()
playback=(r/'Source/TrackerSongPlayback.h').read_text()
workflow=(r/'.github/workflows/build-windows.yml').read_text()
assert 'VERSION 0.2.35' in (r/'CMakeLists.txt').read_text()
assert 'TEST0.2.35' in editor and 'TEST0.2.35' in workflow
assert 'songPendingEndSong' in header and 'songPendingEndValue' in processor
assert 'songChainNext()' in processor and 'songPendingEndValue.load' in processor
assert 'compileTrackerSongRowPreview' in playback
assert 'requestSongPreview' in processor and 'songPlaybackStatus.store(3' in processor
assert 'songCommand==-3 || songCommand==-4' in processor
assert 'values[23]=3' in model
assert 'Preview Steps' in (r/'Source/TrackerSongParameters.h').read_text()
assert 'previewCurrentStep()' in song
assert 'safe->announce(safe->positionText(true))' in song
assert 'safe->announce(safe->positionText(true))' in (r/'Source/TrackerPatternPanel.cpp').read_text()
assert 'lastTrackerWasSong' in editor
assert 'full?("Operator "' in song
assert 'TrackerSongEndPreviewTest.exe' in workflow
print('OK TEST0.2.35 Song End, preview, page focus and announcements')
