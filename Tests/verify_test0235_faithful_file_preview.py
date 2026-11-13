"""TEST0.2.35: faithful file playback, user-configurable cut, no data mutation."""
from pathlib import Path
p=Path(__file__).resolve().parents[1]
b=(p/'Source/TrackerLibraryBrowser.cpp').read_text()
s=(p/'Source/TrackerSongPlayback.h').read_text()
c=(p/'Source/PluginProcessor.cpp').read_text()
h=(p/'Source/PluginProcessor.h').read_text()
assert 'requestLibraryPatternPreview(pattern,0,previewSteps)' in b
assert 'first playable' not in b
assert 'compileTrackerSongLibraryPreview' in s
assert 'return std::make_shared<const TrackerPlayableSong>(song)' in s
assert 'songLibraryChainMap.store' in c
assert 'compileTrackerSongChainMap(sourceSongs,patterns)' in c
assert 'songRequested.store(-4' in c
assert 'songCommand==-3 || songCommand==-4' in c
assert 'songLibraryPreviewRunning=library' in c
assert 'songChainSnapshot=library?songLibraryChainMap' in c
assert 'if(!songLibraryPreviewRunning)' in c
assert 'if(songLibraryPreviewRunning)songPlaybackStatus.store(3' in c
assert 'songLibraryPreviewSamplesLeft' in c
assert 'songLibraryPreviewDivisions' in h
assert 'previewSteps=0' in b
assert 'code==32' in b and 'Preview stopped' in b
assert 'constexpr std::array<int,7> lengths' in b
assert 'requestLibrarySongPreview(*s,slot,*previewPatterns,*previewSongs,previewSteps)' in b
assert 'compileTrackerSongRowPreview(*live,lane,row,steps)' in c
assert 'Biscot1' in (p/'Source/PresetBrowser.cpp').read_text()
assert '0.2.36' in (p/'CMakeLists.txt').read_text()
print('OK: faithful Pattern and Song library auditions, archive-only chaining and length limit')
