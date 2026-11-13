from release_version_guard import release_version, release_test, release_cmake, release_artifact
"""0.2.35: portable keyboard-first native library with immutable short audio previews."""
from pathlib import Path
r=Path(__file__).resolve().parents[1]
h=(r/'Source/TrackerLibraryBrowser.h').read_text()
c=(r/'Source/TrackerLibraryBrowser.cpp').read_text()
editor=(r/'Source/PluginEditor.cpp').read_text()
pattern=(r/'Source/TrackerPatternPanel.cpp').read_text()
song=(r/'Source/TrackerSongPanel.cpp').read_text()
processor=(r/'Source/PluginProcessor.cpp').read_text()
transport=(r/'Source/TrackerPatternPlayback.h').read_text()
for key in ['Ctrl+P','F5','onClose','keyPressed','createAccessibilityHandler']:
 assert key in h+c,key
for key in ['juce::KeyPress::upKey','juce::KeyPress::downKey','juce::KeyPress::pageUpKey',
            'juce::KeyPress::homeKey','juce::KeyPress::escapeKey','juce::KeyPress::backspaceKey',
            'juce::KeyPress::returnKey','code==32','automaticPreview','archiveSongSlot',
            'Replace ALL current Patterns and Songs','single Song',
            'trackerPlaybackStatus.load','songPlaybackStatus.load',
            'deserializeTrackerPattern(','songpkg::importSong(','loadedPatterns->patterns.clear()',
            'requestLibraryPatternPreview','requestLibrarySongPreview','getChildFile("Biscot1-Synth")']:
 assert key in c,key
assert 'onLibraryBrowser' in pattern and 'onLibraryBrowser' in song
assert 'std::make_unique<TrackerLibraryBrowser>' in editor
assert 'libraryBrowser->open(TrackerLibraryBrowser::Mode::songs)' in editor
assert 'libraryBrowser->open(TrackerLibraryBrowser::Mode::patterns)' in editor
assert 'trackerLibraryPlayable.store' in processor
assert 'songPreviewPlayable.store' in processor
assert 'previewLimit>0 && previewPlayed>=previewLimit' in transport
assert 'if(audition || ' in transport
assert 'Source/TrackerLibraryBrowser.cpp' in (r/'CMakeLists.txt').read_text()
assert release_version in (r/'CMakeLists.txt').read_text()
print('OK TEST0.2.35 native library browser and bounded preview protection')
