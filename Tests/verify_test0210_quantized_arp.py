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
assert 'v.amp.noteOn(true);v.mod.noteOn(true);' in p
assert 'sidArpStepRetriggers(seq.lastTick' in p
assert 'sidArpStepRetriggers(event.edge' in p
assert 'if(!suppressArpKeyTrigger)' in p
assert 'noteOn(i,note,ch,event.wasHeld[(size_t)i],arpPhraseActive[(size_t)i]);' in p
assert 'if(!arpOn || !soundingAlready){' in p
assert 'if(!arpOn || !soundingAlready)v.drift=' in p
assert 'bool lastTick=false' in seq
assert 'SidQuantizedArpTest.exe' in w
assert 'SidQuantizedArpTest.cpp' in (root/'CMakeLists.txt').read_text()
print('TEST0.2.10 quantized arpeggio, Trigger/Legato and CC64 wiring: OK')
