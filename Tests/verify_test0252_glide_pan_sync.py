"""TEST0.2.52: actual oscillator glide fractions drive Pan, no fixed stereo lag."""
from pathlib import Path
root=Path(__file__).resolve().parents[1]
p=(root/'Source/PluginProcessor.cpp').read_text()
h=(root/'Source/SidPingPongPan.h').read_text()
g=(root/'Source/SidPitchGlideMotion.h').read_text()
c=(root/'CMakeLists.txt').read_text()
w=(root/'.github/workflows/build-windows.yml').read_text()
assert 'panPitchMotion[(size_t)i].add(arpBefore,target,v.arpPitch);' in p
assert 'panPitchMotion[(size_t)i].add(arpBefore,destination,v.arpPitch);' in p
assert 'panPitchMotion[(size_t)i].add(noteBefore,v.note,v.noteSmooth);' in p
assert 'panPitchCoefficient[(size_t)i]=panPitchMotion[(size_t)i].coefficient();' in p
assert 'v.pingPongPan.tick(param(259+i),panPitchCoefficient[(size_t)i])' in p
assert 'if(noteGlides)v.pingPongPan.advance();' in p
assert 'else v.pingPongPan.advanceAtAttack(param(259+i));' in p
assert 'if(seq.lastTick)pingPongEvent[(size_t)i]=true;' in p
assert 'if(event.edge && event.note>=0)pingPongEvent[(size_t)i]=true;' in p
assert 'pingPongEvent[(size_t)i]=ahdsrReattack;' in p
assert 'float tick(float signedDepth,float pitchCoefficient=-1.f)' in h
assert 'depth+=(targetDepth-depth)*smoothing;' in h
assert 'phase+=(targetSide-phase)*coefficient;' in h
assert 'return (float)std::clamp(rate,0.,1.);' in g
assert 'SidPitchGlidePanSyncTest Tests/SidPitchGlidePanSyncTest.cpp' in c
assert 'SidPitchGlidePanSyncTest.exe' in w
assert (root/'Source/ParameterCatalog.h').read_text().count('Ping Pong Pan')==3
assert p.index('panPitchCoefficient[(size_t)i]=panPitchMotion[(size_t)i].coefficient();') < p.index('v.pingPongPan.tick(param(259+i),panPitchCoefficient[(size_t)i])')
print('OK: sample-synchronous portamento and both arp engines; snap on square jump; depth automation still de-clicked')
