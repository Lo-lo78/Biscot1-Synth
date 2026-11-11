from pathlib import Path
r=Path(__file__).resolve().parents[1]
p=(r/'Source/PluginProcessor.cpp').read_text()
q=(r/'Source/SidQuantizedArp.h').read_text()
s=(r/'Source/SidSeqMemory.h').read_text()
a=(r/'Source/SidArpTransport.h').read_text()
# Do not re-trigger amplitude/modulation envelopes on clock edges.
assert 'sidArpStepRetriggers' not in p
assert 'v.amp.noteOn(true);v.mod.noteOn(true);' not in p
assert 'sidArpClockActive(v.gate,v.amp.stage)' in p
assert 'if(integer(param(76+i))==1)sidSeq[(size_t)i].requestRestart()' in p
assert 'else voices[(size_t)i].quantizedArp.requestRestart()' in p
assert 'const bool routed=(mode==1) || event.trigger[(size_t)i]' in p
assert 'const bool triggerMode=opSharedParam(10,i)<0.5f' in p
assert 'if(restartPending){step=0;restartPending=false;}' in q
assert 'if(restartPending){step=0;restartPending=false;}' in s
assert 'return gate && envelopeStage!=0;' in a
assert 'SidArpTransport.h' in (r/'CMakeLists.txt').read_text()
print('TEST0.2.11: no step envelope retrigger; musical Trigger/Legato; gated arp transport OK')
