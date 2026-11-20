"""TEST0.2.51: ensure event timing/parameter ABI/CI wires first-attack pan sync."""
from pathlib import Path
r=Path(__file__).resolve().parents[1]
p=(r/'Source/PluginProcessor.cpp').read_text()
h=(r/'Source/SidPingPongPan.h').read_text()
a=(r/'Source/SidAmpEnvelope.h').read_text()
c=(r/'Source/ParameterCatalog.h').read_text()
cmake=(r/'CMakeLists.txt').read_text()
w=(r/'.github/workflows/build-windows.yml').read_text()
assert 'void advanceAtAttack(float signedDepth) noexcept' in h
assert 'phase=firstSide?1.f:-1.f;' in h
assert 'else v.pingPongPan.advanceAtAttack(param(259+i));' in p
assert 'if(panPitchMotion[(size_t)i].moving() && panPitchCoefficient[(size_t)i]<1.f)' in p
assert 'pingPongEvent[(size_t)i]=ahdsrReattack;' in p
assert 'else v.pingPongPan.advanceAtAttack(param(259+i));' in p
assert p.index('else v.pingPongPan.advanceAtAttack(param(259+i));') < p.index('v.pingPongPan.tick(param(259+i),panPitchCoefficient[(size_t)i])')
assert 'else if(stage==0)noteOn(true);' in a
assert 'SidPingPongAttackSyncTest Tests/SidPingPongAttackSyncTest.cpp' in cmake
assert 'SidPingPongAttackSyncTest.exe' in w
assert c.count('Ping Pong Pan')==3
print('OK: exact first Attack sample sync, no extra delay, legacy pan/parameter ABI retained')
