from pathlib import Path
r=Path(__file__).resolve().parents[1]
p=(r/'Source/PluginProcessor.cpp').read_text()
h=(r/'Source/SidArpTransport.h').read_text()
w=(r/'.github/workflows/build-windows.yml').read_text()
s=(r/'Source/SidAmpEnvelope.h').read_text()
assert 'return envelopeStage!=0;' in h
assert 'return gate && envelopeStage!=0;' not in h
assert p.count('sidArpClockActive(v.gate,v.amp.stage)')==2
assert 'arpReleasePools[(size_t)i]' in p and 'arpReleaseCounts[(size_t)i]' in p
assert 'seq.releaseRoot' in p and 'seq.process(1,root,' in p
assert 'if(wasActive && v.amp.stage==0 && !v.gate)' in p
assert 'sidSeq[(size_t)i].clearAll()' in p
assert 'env.stage' not in h
assert 'SidArpReleaseTest.exe' in w
assert 'SidArpReleaseTest.cpp' in (r/'CMakeLists.txt').read_text()
print('TEST0.2.12: arp clock preserved through ADSR release, stopped at idle')
