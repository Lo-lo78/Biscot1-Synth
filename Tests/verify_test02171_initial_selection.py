"""New instances start at Audio Mode; previously saved locations remain authoritative."""
from pathlib import Path
import re
root=Path(__file__).resolve().parents[1]
src=(root/'Source/PluginEditor.cpp').read_text(encoding='utf-8')
cat=(root/'Source/ParameterCatalog.h').read_text(encoding='utf-8')
workflow=(root/'.github/workflows/build-windows.yml').read_text(encoding='utf-8')
restore=src.split('void BiscotEditor::restoreLocation(){',1)[1].split('void BiscotEditor::openPresetBrowser(',1)[0]
assert 'getProperty(juce::Identifier("editorLastParameter"),2)' in restore
assert 'getProperty(juce::Identifier("editorLastParameter"),3)' not in restore
assert 'const auto found=std::find(entries.begin(),entries.end(),previous);' in restore
assert 'group=g;' in restore and 'selected=lastSelected[(size_t)g];' in restore
assert 'proc.state.state.setProperty(juce::Identifier("editorLastParameter"),index,nullptr);' in src
assert '{2, "Audio Mode",' in cat and '{3, "MIDI Mode",' in cat
assert 'grid.setSelectedId(selected+1,juce::dontSendNotification);' in src
assert 'return createIgnoredAccessibilityHandler(*this);' in src
assert 'verify_test02171_initial_selection.py' in workflow
assert 'TEST0.2.23' in src and 'TEST0.2.23' in workflow
print('OK: fresh UI starts at Audio Mode, remembered parameter still restores, native accessibility focus retained')
