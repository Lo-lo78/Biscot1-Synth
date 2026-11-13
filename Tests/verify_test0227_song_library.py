"""TEST0.2.30: Song/Pattern import-export does not regress TEST0.2.26."""
from pathlib import Path
r=Path(__file__).resolve().parents[1]
package=(r/'Source/TrackerSongPackage.h').read_text()
sp=(r/'Source/TrackerSongPanel.cpp').read_text()
pp=(r/'Source/TrackerPatternPanel.cpp').read_text()
play=(r/'Source/TrackerSongPlayback.h').read_text()
proc=(r/'Source/PluginProcessor.cpp').read_text()
assert 'songpkg::exportSong(' in sp and 'songpkg::importSong(' in sp
assert 'deserializeTrackerPattern(' in pp and 'letter(\'o\')' in pp
assert 'BISCOT1_SONG_BUNDLE_V2' in package
assert 'serializeTrackerPattern(*patterns.find(id),id)' in package
assert 'decode(payload,song,decodedId,embedded)' in package and 'auto newPatterns=patterns;auto newSongs=songs' in package
assert 'patterns=std::move(newPatterns);songs=std::move(newSongs)' in package
assert 'n>18' in package and 'id>23' in package  # Preview Steps stays portable
assert 'compileTrackerSongRowPreview' in play and 'songPendingEndValue.load' in proc
assert 'previewCurrentStep()' in sp
print('OK TEST0.2.30 Song package and Pattern library integration; previews retained')
