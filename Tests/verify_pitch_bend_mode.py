"""Static wiring checks for one native-only Pitch Bend Mode combo."""
import json,re
from pathlib import Path
root=Path(__file__).resolve().parents[1]
jsfx=(root/'Reference/Biscot1Synth_v2.38.jsfx').read_text(encoding='utf-8-sig')
original=json.loads((root/'Reference/ParameterCatalog.json').read_text())
native=json.loads((root/'Reference/NativeParameters.json').read_text())
cat=(root/'Source/ParameterCatalog.h').read_text()
processor=(root/'Source/PluginProcessor.cpp').read_text()
header=(root/'Source/PluginProcessor.h').read_text()
editor=(root/'Source/PluginEditor.cpp').read_text()
assert len(original)==224 and len(native)==4
assert original[0]['index']==2 and original[-1]['index']==225
assert original==json.loads((root/'Reference/ParameterCatalog.json').read_text())
assert len(re.findall(r'^slider\d+:',jsfx,re.M))==224
assert len(re.findall(r'^  \{\d+, "',cat,re.M))==228
assert 'std::array<ParameterDef,228> catalog' in cat
assert '{226, "Pitch Bend Mode", 0.0f, 1.0f, 1.0f, 0.0f, "Multi|Omni"}' in cat
assert 'std::array<std::atomic<float>*,230> parameters' in header
assert 'routePitchBend(ch,bend,mode,integer(param(226))' in processor
assert 'std::find(global.begin(),global.end(),3)+1,226' in editor
assert 'TEST0.2.8' in editor
assert 'LFO3 Rate Mod Depth' in cat
assert native[0]['index']==226 and native[0]['enum']==['Multi','Omni']
print('OK: 224 original JSFX parameters + append-only Pitch Bend and Sustain Pedal controls, accessible next to MIDI Mode')
