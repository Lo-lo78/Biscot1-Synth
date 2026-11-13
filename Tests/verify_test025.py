from release_version_guard import release_version, release_test, release_cmake, release_artifact
"""Static guard for accessible preset browser. The Windows+NVDA run is still mandatory."""
from pathlib import Path
r=Path(__file__).resolve().parents[1]
ed=(r/'Source/PluginEditor.cpp').read_text()
head=(r/'Source/PluginEditor.h').read_text()
manager=(r/'Source/PresetManager.cpp').read_text()
browser=(r/'Source/PresetBrowser.cpp').read_text()
c=(r/'CMakeLists.txt').read_text()
processor=(r/'Source/PluginProcessor.cpp').read_text()
for name in ['PresetBrowser.cpp','PresetManager.cpp','ScreenReaderAnnouncer.cpp']:
 assert name in c,name
assert release_test in ed and release_cmake in c
assert 'setAccessible(false)' in ed
assert 'void BiscotEditor::rememberLocation()' in ed
assert 'void BiscotEditor::restoreLocation()' in ed
assert 'editorLastParameter' in ed
assert 'Alt+B' in ed and 'Alt+S' in ed
assert "if(letter('b')){openPresetBrowser(false);return true;}" in ed
assert "if(letter('s')){openPresetBrowser(true);return true;}" in ed
assert 'parameterSearch.next(' in ed and 'biscotParameterSearchCharacter(k)' in ed
assert 'Alt+L' not in ed[:ed.index('BiscotEditor::BiscotEditor(')]
assert 'source.setDescription(text);' not in ed
assert 'grid.setSelectedId(i+1,juce::dontSendNotification);' in ed
assert 'class PresetBrowser' in (r/'Source/PresetBrowser.h').read_text()
assert 'std::unique_ptr<PresetBrowser> presetBrowser;' in head
assert '"Biscot1-Synth"' in manager
assert '"*.biscot"' in manager and 'hasFileExtension ("biscot")' in manager
assert 'State: ' in manager and 'getStateInformation' in manager and 'setStateInformation' in manager
assert 'manager.restore (original)' in browser and 'manager.commit (entry.file)' in browser
assert 'manager.instrument()' not in browser
assert 'void BiscotProcessor::getStateInformation' in processor
print('OK: accessible editor navigation, native preset save/load and state restore (static)')
