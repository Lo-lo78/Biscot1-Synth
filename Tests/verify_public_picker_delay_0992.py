from pathlib import Path
r=Path(__file__).resolve().parents[1]
p=(r/'Source/TrackerPatternPanel.cpp').read_text()
s=(r/'Source/TrackerSongPanel.cpp').read_text()
fx=(r/'Source/TrackerSongFxCore.h').read_text()
preview=(r/'Source/TrackerSongPlayback.h').read_text()
processor=(r/'Source/PluginProcessor.cpp').read_text()
assert 'parameterAlreadyInStep' in p and '"Set, "' in p
assert 'parameterAlreadyInStep' in s and '"Set, "' in s
assert 'insertSelectedParameter(m.isCtrlDown())' in s
assert 'else refresh(false); // Do not mask' in s
assert 'const float center=0.5f*(dl+dr)' in fx
assert 'left[index]=std::clamp(mono+feedback*dr' in fx
assert 'right[index]=std::clamp(feedback*dl' in fx
assert 'for(const auto& [fxRow,fx]:sourceFx)' in preview
assert 'auditionFx.present[(size_t)id]=true' in preview
assert 'for(auto& fx:songFx)fx.resetForAudition()' in processor
print('OK: F1/F2 Set prefixes, persistent Ctrl+Enter, stereo repeats and preview FX')
