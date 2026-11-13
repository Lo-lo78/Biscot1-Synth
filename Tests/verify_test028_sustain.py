from pathlib import Path
p=Path(__file__).resolve().parents[1]
h=(p/'Source/PluginProcessor.h').read_text()
c=(p/'Source/PluginProcessor.cpp').read_text()
cat=(p/'Source/ParameterCatalog.h').read_text()
ed=(p/'Source/PluginEditor.cpp').read_text()
assert 'std::array<std::atomic<float>*,249>' in h
assert 'return 50.0;' in h
for i in range(3):
 for prefix,ix in [('Attack',34),('Decay',37),('Release',43)]:
  assert f'{{{ix+i}, "Op{i+1} {prefix}", 0.0f, 5.0f' in cat
 assert f'{{{227+i}, "Op{i+1} Sustain Pedal", 0.0f, 1.0f, 1.0f, 0.0f, "Off|On"}}' in cat
assert 'm.getControllerNumber()==64' in c
assert 'm.getControllerValue()>=64' in c
assert 'void BiscotProcessor::releasePedalLatches()' in c
assert 'v.pedalLatched=true' in c
assert 'result.affected[(size_t)i]=voices[(size_t)i].removeNewest(note)' in (p/'Source/SidMidiNoteState.h').read_text()
assert 'operatorPage.insert(afterSustain+1,sustainId)' in ed
assert 'SidSustainPedalTest.exe' in (p/'.github/workflows/build-windows.yml').read_text()
print('TEST0.2.8 ADSR / per-operator CC64 source checks passed')
