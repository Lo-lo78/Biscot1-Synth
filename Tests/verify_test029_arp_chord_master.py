from pathlib import Path
import re
r=Path(__file__).resolve().parents[1]
p=(r/'Source/PluginProcessor.cpp').read_text()
h=(r/'Source/PluginProcessor.h').read_text()
cat=(r/'Source/ParameterCatalog.h').read_text()
ed=(r/'Source/PluginEditor.cpp').read_text()
sus=(r/'Source/SidArpSustain.h').read_text()
w=(r/'.github/workflows/build-windows.yml').read_text()
assert 'std::array<ParameterDef,247>' in cat
assert len(re.findall(r'^  \{\d+, "',cat,re.M))==247
assert '{230, "Chord Op1 Controls", 0.0f, 1.0f, 1.0f, 0.0f, "Independent|Op1 Master"}' in cat
assert 'global.insert(pos,{226,230});' in ed
assert 'std::array<std::atomic<float>*,249>' in h
for base in (16,31,34,37,40,43,46):
 assert f'opSharedParam({base},i)' in p,(base,p.count(f'opSharedParam({base},i)'))
for base in (22,25,28,49,52,55,64,67,70,73,91):
 assert f'param({base}+i)' in p, f'{base} was unexpectedly shared'
assert 'opSharedParam(227,op)' in h
assert 'const bool soundingAlready=wasHeld || v.pedalLatched || (arpOn && pedalPhrase);' in p
assert '(!arpOn && !opLegato && v.amp.stage!=0)' in p
assert 'const bool filterWasPedalLatched=' in p
assert 'sustainedArp.hold(i,ch,note,order)' in p
assert 'sustainedArp.clearChannel(ch)' in p
assert 'sustainedArp.clearOperator(i)' in p
assert 'arpReleasePools' in h and 'arpReleaseCounts' in p
assert 'seq.record(note); // continue the held CC64 phrase' in p
assert 'sidArpClockActive(v.gate,v.amp.stage) && seq.length>0' in p
assert 'SidArpSustainTest.exe' in w
assert 'std::array<Channels,3>' in sus
print('OK: independent defaults, scoped Chord master, Trigger and CC64 arp retention')
