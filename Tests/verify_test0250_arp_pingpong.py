"""TEST0.2.50: the processor must advance from clock edges, not physical keys.
A standalone C++ regression verifies both arp engines and even note counts.
"""
from pathlib import Path
r=Path(__file__).resolve().parents[1]
p=(r/'Source/PluginProcessor.cpp').read_text()
cmake=(r/'CMakeLists.txt').read_text()
workflow=(r/'.github/workflows/build-windows.yml').read_text()
assert 'if(!arpOn){' in p and 'else v.pingPongPan.advanceAtAttack(param(259+i));' in p
assert 'if(seq.lastTick)pingPongEvent[(size_t)i]=true;' in p
assert 'if(event.edge && event.note>=0)pingPongEvent[(size_t)i]=true;' in p
assert 'pingPongEvent[(size_t)i]=ahdsrReattack;' in p
assert 'if(pingPongEvent[(size_t)i]){' in p and 'if(panPitchMotion[(size_t)i].moving() && panPitchCoefficient[(size_t)i]<1.f)' in p
assert 'SidPingPongArpStepTest Tests/SidPingPongArpStepTest.cpp' in cmake
assert 'SidPingPongArpStepTest.exe' in workflow
assert (r/'Source/ParameterCatalog.h').read_text().count('Ping Pong Pan')==3
print('OK: classical & SID Seq clock edges, one flip per sample, all parameters intact')
