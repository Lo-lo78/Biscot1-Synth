from release_version_guard import release_version, release_test, release_cmake, release_artifact
from pathlib import Path
p=Path(__file__).resolve().parents[1]
cpp=(p/'Source/PluginProcessor.cpp').read_text()
hpp=(p/'Source/PluginProcessor.h').read_text()
core=(p/'Source/SidLfoCore.h').read_text()
assert 'l.noteTrigger(param(122+j*23)>0.5f,param(129+j*23)>0.5f,param(131+j*23))' in cpp
assert 'sidFilterLfoShouldTrigger(event.filterRetrigger' in cpp
assert 'triggerFilterAndLfos(note,sidFilterLfoShouldTrigger' in cpp
assert 'filterKeyNote=note;' in cpp
assert 'l.tick(inc,' in cpp
assert 'using Lfo = SidLfoCore;' in hpp
assert 'if(!active && !oneShot)active=true;' in core
assert core.index('smoothed=(float)') < core.index('const double squash=')
assert 'delayAge=1000000.' in core
assert 'LFO1 Rate Mod Source' in (p/'Source/ParameterCatalog.h').read_text()
assert release_test in (p/'CMakeLists.txt').read_text()
assert 'SidFilterLfoParityTest' in (p/'CMakeLists.txt').read_text()
print('OK: TEST0.2.16 filter gate, LFO event and JSFX signal-path wiring')
