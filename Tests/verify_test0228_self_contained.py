from pathlib import Path
r=Path(__file__).resolve().parents[1]
p=(r/'Source/TrackerSongPackage.h').read_text()
s=(r/'Source/TrackerSongPanel.cpp').read_text()
f=(r/'Source/TrackerPatternPanel.cpp').read_text()
browser=(r/'Source/TrackerLibraryBrowser.cpp').read_text()
assert 'BISCOT1_SONG_BUNDLE_V2' in p
assert 'BISCOT1_SINGLE_SONG_BUNDLE_V3' in p and 'BISCOT1_ALL_SONG_BUNDLE_V3' in p
assert 'if(!single)for(int ref:{s.songEnd,s.loopTarget})' in p
assert 'detail::hexBytes(payload)' in p and 'decodePayload(hex,payload' in p
assert 'if(legacy)' in p and 'songMap.at(s.songEnd)' in p and 'songMap.at(s.loopTarget)' in p
assert 'patternMap.at((int)step.values[1])' in p
assert 'TrackerPatternBank newPatterns;' in p
assert 'TrackerSongBank newSongs;' in p
assert 'getChildFile("patterns")' in f and 'getChildFile("Singles")' not in f
assert '"songs"' in browser
print('OK 0.2.28: standalone Song, chain remapping, simple library folders')
