"""Regressions for typed, accessible library names and space echo."""
from pathlib import Path
r=Path(__file__).resolve().parents[1]
p=(r/'Source/TrackerPatternPanel.cpp').read_text()
ph=(r/'Source/TrackerPatternPanel.h').read_text()
s=(r/'Source/TrackerSongPanel.cpp').read_text()
sh=(r/'Source/TrackerSongPanel.h').read_text()
b=(r/'Source/TrackerSongPackage.h').read_text()
f=(r/'Source/TrackerLibraryFileNames.h').read_text()
preset=(r/'Source/PresetBrowser.cpp').read_text()
for required in [
 'openNumericEditor(NumericPurpose::savePatternName,',
 'if(purpose==NumericPurpose::savePatternName)',
 'saveCurrentPattern(txt);return true;',
 'patternExportFiles[slot]=file;',
 'if(!candidate.exists()){file=candidate;break;}',
 'TemporaryFile temporary(file)',
 'getChildFile("patterns")',
 'getChildFile("songs")',
 'numeric.grabKeyboardFocus();numeric.selectAll();',
 'numeric.setVisible(false);navigator.grabKeyboardFocus();',
]:assert required in p,required
for required in [
 'NumericPurpose::savePatternName',
 'juce::TextEditor::keyPressed(k)',
 'announceToActiveScreenReader(*this,"spazio")',
]: assert required in ph,required
for required in [
 'openNumeric(EditPurpose::saveSongName,',
 'openNumeric(EditPurpose::saveAllSongsName,',
 'if((savingSong||savingAll)&&input.isEmpty())',
 'numeric.setVisible(true);numeric.grabKeyboardFocus();numeric.selectAll();',
 'numeric.setVisible(false);navigator.grabKeyboardFocus();',
 'songpkg::exportSong(',
 'songpkg::exportAllSongs(',
 'const TrackerSong before=song();',
 'else song().name=before.name;',
]: assert required in s,required
for required in [
 'EditPurpose::saveSongName','EditPurpose::saveAllSongsName',
 'EditPurpose::songName',
 'juce::TextEditor::keyPressed(k)',
 'announceToActiveScreenReader(*this,"spazio")',
]: assert required in sh,required
assert 'requestedFileName' in b
assert 'const juce::String filename=chosenName.isNotEmpty()' in b
assert 'if(!candidate.exists()){target=candidate;break;}' in b
assert 'const juce::String ext=all?".biscot1-all-song":".biscot1-song"' in b
assert 'name=name.substring(0,90)' in f
assert 'name.replaceCharacter(c' in f
assert '"CON"' in f and '"LPT"' in f
assert 'parent.mode == Mode::save || parent.mode == Mode::rename || parent.mode == Mode::folder' in preset
assert 'announceToActiveScreenReader (*this, "spazio")' in preset
assert 'newSongs.songs=loadedSongs' in b
assert 'const int dest=selectedDestination' in b
print('OK TEST0.2.35 named library save, typed spaces and focus')
