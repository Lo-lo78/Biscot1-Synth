# The standalone DSP test verifies the common envelope. This integration guard
# ensures every live VCA/Mod ENV/filter is actually passed its automation value.
from pathlib import Path
r=Path(__file__).resolve().parents[1]
p=(r/'Source/PluginProcessor.cpp').read_text()
h=(r/'Source/PluginProcessor.h').read_text()
e=(r/'Source/SidAmpEnvelope.h').read_text()
for route in (
 'opSharedParam(40,i),opSharedParam(43,i),sampleRateHz)',
 'param(70+i),param(73+i),sampleRateHz)',
 'param(120),param(121),sampleRateHz)',
):
 assert route in p, f'Live envelope Release not wired: {route}'
assert 'float tick(float attack,float hold,float decay,float sustain,' in e
assert 'if(stage==4 && seconds!=releaseSecondsCache)' in e
assert 'if(hold!=holdSecondsCache)' in e
assert 'v.drift=randomUnit(v.noiseSeed)' in p
assert '+v.drift*param(13+i)*0.50f' in p
assert 'updated || (noiseCharacterChanged && sub==0)' in p
assert 'noiseCharacterLast=-1.f' in h
assert 'SidLiveAutomationTest.cpp' in (r/'CMakeLists.txt').read_text()
print('OK: seven live envelopes, live Drift/Noise Character and cached-coefficient wiring')
