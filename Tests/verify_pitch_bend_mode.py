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
assert len(original)==224 and len(native)==5
assert original[0]['index']==2 and original[-1]['index']==225
assert original==json.loads((root/'Reference/ParameterCatalog.json').read_text())
assert len(re.findall(r'^slider\d+:',jsfx,re.M))==224
assert len(re.findall(r'^  \{\d+, "',cat,re.M))==229
assert 'std::array<ParameterDef,229> catalog' in cat
assert '{226, "Pitch Bend Mode", 0.0f, 1.0f, 1.0f, 0.0f, "Multi|Omni"}' in cat
assert 'std::array<std::atomic<float>*,231> parameters' in header
assert 'routePitchBend(ch,bend,mode,integer(param(226))' in processor
assert 'global.insert(pos,{226,230});' in editor
assert 'TEST0.2.11' in editor
assert 'LFO3 Rate Mod Depth' in cat
assert native[-1]['index']==230 and native[-1]['enum']==['Independent','Op1 Master']
assert native[0]['index']==226 and native[0]['enum']==['Multi','Omni']
print('OK: 224 original JSFX parameters + append-only Pitch Bend and Sustain Pedal controls, accessible next to MIDI Mode')
