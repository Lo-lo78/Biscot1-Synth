from pathlib import Path
root=Path(__file__).resolve().parents[1]
p=(root/'Source/PluginProcessor.cpp').read_text()
h=(root/'Source/PluginProcessor.h').read_text()
seq=(root/'Source/SidSeqMemory.h').read_text()
clock=(root/'Source/SidQuantizedArp.h').read_text()
w=(root/'.github/workflows/build-windows.yml').read_text()
assert 'SidQuantizedArp quantizedArp' in h
assert 'v.quantizedArp.process(pool,count,arpMode' in p
assert 'if(edge)' in clock and 'std::copy_n(candidate.begin()' in clock
assert 'restartPending' in clock and 'requestRestart()' in clock
assert 'restartPending' in seq and 'requestRestart()' in seq
assert 'sidArpClockActive(v.gate,v.amp.stage)' in p
assert 'sidArpRestartRequested(triggerMode,clockRunningBefore' in p
assert 'v.amp.noteOn(true);v.mod.noteOn(true);' not in p
assert 'sidArpStepRetriggers(' not in p
assert 'noteOn(i,note,ch,event.wasHeld[(size_t)i],arpPhraseActive[(size_t)i]);' in p
assert 'SidQuantizedArpTest.exe' in w
assert 'SidQuantizedArpTest.cpp' in (root/'CMakeLists.txt').read_text()
print('TEST0.2.11 ARP transport-only Trigger, Legato and natural Release: OK')
