"""Static regression for the optional native Song/Pattern library actions."""
from pathlib import Path
root=Path(__file__).resolve().parents[1]
pkg=(root/'Source/TrackerSongPackage.h').read_text()
song=(root/'Source/TrackerSongPanel.cpp').read_text()
pattern=(root/'Source/TrackerPatternPanel.cpp').read_text()
for label in ['BISCOT1_SONG_BUNDLE_V2','serializeTrackerPattern(',
              'deserializeTrackerPattern(', 'std::map<int,int> patternMap,songMap',
              'newPatterns.patterns.emplace(', 'newSongs.songs.emplace(',
              'atomicWrite(target,', 'Missing referenced Pattern', 'Missing embedded Pattern']:
 assert label in pkg,label
assert 'songpkg::exportSong(' in song
assert 'songpkg::importSong(' in song
assert 'deserializeTrackerPattern(' in pattern
assert 'bank.patterns.emplace(slot,std::move(imported))' in pattern
assert 'Source/TrackerSongPackage.h' in (root/'CMakeLists.txt').read_text()
print('OK: Song package export/import, dependency guard and Pattern chooser present')
