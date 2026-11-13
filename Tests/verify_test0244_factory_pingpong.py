from pathlib import Path
r=Path(__file__).resolve().parents[1]
fx=(r/'Source/TrackerSongFxCore.h').read_text()
song=(r/'Source/TrackerSongPanel.cpp').read_text()
pattern=(r/'Source/TrackerPatternPanel.cpp').read_text()
editor=(r/'Source/PluginEditor.cpp').read_text()
processor=(r/'Source/PluginProcessor.cpp').read_text()
workflow=(r/'.github/workflows/build-windows.yml').read_text()
assert 'const float mono=0.5f*(inputL+inputR)' in fx
assert 'const float injectedR=inputR*(1.f-cross)' in fx
assert 'void TrackerSongPanel::resetCurrentValue()' in song
assert 'currentValue(songDefaultValue(id))' in song
assert 'code==juce::KeyPress::backspaceKey){resetCurrentValue();return true;}' in song
assert 'void TrackerPatternPanel::resetCurrentCell()' in pattern
assert 'if(row<0){changeValue(cell==0?16.:cell==1?-1.:0.);return;}' in pattern
assert "letter('i')){openFactoryResetConfirmation();return true;}" in editor
assert 'void BiscotEditor::finishFactoryReset(bool approved)' in editor
assert 'resetChoiceYes=false' in editor
assert 'void BiscotProcessor::resetToFactoryState()' in processor
assert 'trackerPatterns.reset();trackerSongs.reset();' in processor
assert 'state.state.removeAllProperties(nullptr)' in processor
assert 'parameter->setValueNotifyingHost' in processor
assert 'TrackerSongPingPongTest' in workflow
print('OK: Song ping pong stereo feedback, F1/F2 Backspace and guarded Alt+I factory reset')
