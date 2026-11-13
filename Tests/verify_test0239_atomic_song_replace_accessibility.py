"""0.2.39: confirmed, atomic replacement of tracker banks and accessible focus."""
from pathlib import Path
r=Path(__file__).resolve().parents[1]
pkg=(r/'Source/TrackerSongPackage.h').read_text()
lib=(r/'Source/TrackerLibraryBrowser.cpp').read_text()
lh=(r/'Source/TrackerLibraryBrowser.h').read_text()
preset=(r/'Source/PresetBrowser.cpp').read_text()
ph=(r/'Source/PresetBrowser.h').read_text()
p=(r/'Source/TrackerPatternPanel.cpp').read_text()
s=(r/'Source/TrackerSongPanel.cpp').read_text()
e=(r/'Source/PluginEditor.cpp').read_text()
# Import decodes and validates every Song/Pattern/linked reference before replacing.
assert pkg.index('if(!in.eof()||!loadedSongs.count(active))') < pkg.index('TrackerPatternBank newPatterns;')
assert 'TrackerPatternBank newPatterns;' in pkg and 'TrackerSongBank newSongs;' in pkg
assert 'newPatterns.patterns.clear();' in pkg and 'newSongs.songs.clear();' in pkg
assert 'newPatterns.patterns.emplace(id,pat)' in pkg
assert 'newPatterns.patterns.count(dest)' not in pkg
assert 'newPatterns.activePattern=newPatterns.patterns.begin()->first;' in pkg
assert 'newSongs.songs[dest]=loadedSongs.at(active)' in pkg
assert 'const int dest=selectedDestination' in pkg
assert 'newSongs.songs=loadedSongs' in pkg
assert pkg.index('patterns=std::move(newPatterns);songs=std::move(newSongs)') > pkg.index('for(auto& lane:s.lanes)')
# Both Song file types require explicit Yes/No before any mutation.
assert 'if(mode==Mode::songs)' in lib and 'confirming=true' in lib
assert 'finishConfirmation(bool approve)' in lib and 'if(importSelected(file,status))close(true,status)' in lib
assert 'if(code==juce::KeyPress::returnKey){finishConfirmation(confirmationYes)' in lib
assert "if(ch=='y'||code=='Y'){finishConfirmation(true)" in lib
assert "if(ch=='n'||code=='N'){finishConfirmation(false)" in lib
assert 'ConfirmationButton yes' in lh and 'ConfirmationButton' in ph
assert 'code==juce::KeyPress::escapeKey){finishConfirmation(false)' in lib
# The preset browser never changes synth state during ordinary file navigation.
assert 'previewRow (row);' not in preset[preset.index('void PresetBrowser::selectedRowsChanged'):preset.index('void PresetBrowser::previewRow')]
assert 'manager.load (target)' in preset and 'Mode::loadReplace' in preset
assert 'cancelConfirmation()' in preset and 'confirmationYes' in preset
assert 'code == juce::KeyPress::returnKey' in preset
# F1/F2 cursor positions are written to the host state and restored on constructor.
assert 'readCursor();refreshParameterList();refresh();' in p
assert 'readCursor();refresh();' in s
assert 'persistCursor();' in p and 'persistCursor();' in s
assert 'trackerPatternRow' in p and 'trackerSongRow' in s
assert 'const auto saved=patternExportFiles.find(slot)' not in p
assert 'page=1;refresh();setVisible(true)' not in p
# Alt+B works only in synth view; Ctrl+O aliases preset open there.
assert 'openPresetBrowser(false);return true;' in e
assert "letter('o')" in e
assert 'Preset browser is available outside Alt+Q' not in p+s
print('OK 0.2.39: atomic full tracker import, explicit confirmations, cursor state, quiet Alt+B')
