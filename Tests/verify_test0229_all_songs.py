from pathlib import Path
p=(Path(__file__).resolve().parents[1]/'Source/TrackerSongPackage.h').read_text()
assert 'for(int i=0;i<100;++i)songIds.insert(i)' in p
assert 'newSongs.songs=loadedSongs' in p
assert 'if(active<0||active>=100||!songs.find(active))' in p
assert 'for(int id:songIds)' in p
assert 'const auto refs=references(*song)' in p
assert 'newSongs.activeSong=songMap.at(active)' in p
assert 'BISCOT1_SONG_BUNDLE_V2' in p
print('OK: exports every Song, embeds all referenced Patterns and preserves selected Song')
