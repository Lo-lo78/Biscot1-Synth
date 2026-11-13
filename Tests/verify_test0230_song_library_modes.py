from pathlib import Path
root=Path(__file__).resolve().parents[1]
p=(root/'Source/TrackerSongPackage.h').read_text()
u=(root/'Source/TrackerSongPanel.cpp').read_text()
browser=(root/'Source/TrackerLibraryBrowser.cpp').read_text()
for part in ['BISCOT1_SINGLE_SONG_BUNDLE_V3','BISCOT1_ALL_SONG_BUNDLE_V3',
             '.biscot1-song','.biscot1-all-song','for(int i=0;i<100;++i)songIds.insert(i)',
             'newSongs.songs=loadedSongs','newSongs.songs[dest]=loadedSongs.at(active)',
             'if(legacy)','if(!single)for(int ref:']:
    assert part in p,part
assert "songpkg::exportAllSongs(" in u
assert "songpkg::exportSong(" in u
assert 'file.hasFileExtension(".biscot1-all-song")' in browser
assert "onPresetBrowser(true)" not in u
print('OK TEST0.2.35 song library modes')
