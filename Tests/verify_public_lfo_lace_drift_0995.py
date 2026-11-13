"""Public 0.99.5 append-only LFO Phase Drift / Lace wiring."""
from pathlib import Path
import json,re
r=Path(__file__).resolve().parents[1]
cat=(r/'Source/ParameterCatalog.h').read_text()
tracker=(r/'Source/TrackerParameterCatalog.h').read_text()
h=(r/'Source/PluginProcessor.h').read_text()
p=(r/'Source/PluginProcessor.cpp').read_text()
core=(r/'Source/SidLfoCore.h').read_text()
editor=(r/'Source/PluginEditor.cpp').read_text()
native=json.loads((r/'Reference/NativeParameters.json').read_text())
assert 'std::array<ParameterDef,247>' in cat
assert 'std::array<TrackerParam,121>' in tracker
assert 'std::array<std::atomic<float>*,249>' in h
assert [d['index'] for d in native[-15:]]==list(range(234,249))
for i in range(1,4):
 for suffix in ('Drift Amount','Drift Cycle','Lace Source','Lace Rate','Lace Mix'):
  assert f'"LFO{i} {suffix}"' in cat
  assert f'"LFO{i} {suffix}"' in tracker
assert 'const int extra=234+i*5;' in p
assert 'const int laceSource=integer(param(extra+2));' in p
assert 'l.cycles=0;' in core or 'cycles=0;' in core
assert 'const double x=phase+shift-std::floor(phase+shift);' in core
assert 'first=234+i*5' in editor
assert 'SidLfoLaceDriftTest' in (r/'CMakeLists.txt').read_text()
assert 'SidLfoLaceDriftTest.exe' in (r/'.github/workflows/build-windows.yml').read_text()
print('OK 0.99.5: 15 append-only LFO parameters, tracker/audio/host parity')
