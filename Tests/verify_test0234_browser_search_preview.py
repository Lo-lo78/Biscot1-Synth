from release_version_guard import release_version, release_test, release_cmake, release_artifact
"""TEST0.2.35: prefix search, persistent audition and two-arrow folder navigation."""
from pathlib import Path
r=Path(__file__).resolve().parents[1]
t=(r/'Source/TrackerLibraryBrowser.cpp').read_text()
p=(r/'Source/PresetBrowser.cpp').read_text()
s=(r/'Source/TrackerSongPlayback.h').read_text()
c=(r/'Source/PluginProcessor.cpp').read_text()
shared=(r/'Source/BrowserIncrementalSearch.h').read_text()
assert 'now-lastKeyTimeMs>1200.' in shared
assert 'nextPrefix=letter' in shared
assert 'name.startsWithIgnoreCase(wanted)' in shared
assert 'name.substring(j).startsWithIgnoreCase(wanted)' in shared
assert 'search.type(' in t and 'incrementalSearch.type (' in p
assert 'selectByInitial' in p and 'void PresetBrowser::parentFolder()' in p
assert 'code == juce::KeyPress::rightKey' in p and 'activateRow (row)' in p
assert 'if(code==juce::KeyPress::leftKey' in t and 'parentFolder();' in t
assert 'if(code==juce::KeyPress::rightKey' in t and 'enterFolder();' in t
assert 'mod.isCtrlDown() && !mod.isAltDown()' in t and 'archiveSongSlot' in t
assert "(letter('p') || code==32)" not in t
assert 'code==32' in t and 'automaticPreview=recalledAutomaticPreview(next)' in t
assert 'rememberAutomaticPreview(mode,automaticPreview)' in t
assert 'automaticPreview.patterns' in t and 'automaticPreview.songs' in t
assert 'compileTrackerSongLibraryPreview' in s and 'compileTrackerSongLibraryPreview(compileTrackerSong(song,slot,patterns),divisions)' in c
assert 'return std::make_shared<const TrackerPlayableSong>(song)' in s
assert 'compileTrackerSongRowPreview(*live,lane,row,steps)' in c
assert release_version in (r/'CMakeLists.txt').read_text()
print('OK TEST0.2.35 incremental search, navigation and three-operator library audio preview')
