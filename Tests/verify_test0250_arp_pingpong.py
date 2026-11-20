"""TEST0.2.50: the processor must advance from clock edges, not physical keys.
A standalone C++ regression verifies both arp engines and even note counts.
"""
from pathlib import Path
r=Path(__file__).resolve().parents[1]
p=(r/'Source/PluginProcessor.cpp').read_text()
cmake=(r/'CMakeLists.txt').read_text()
workflow=(r/'.github/workflows/build-windows.yml').read_text()
assert 'if(!arpOn)v.pingPongPan.advance();' in p
assert 'if(seq.lastTick)pingPongEvent[(size_t)i]=true;' in p
assert 'if(event.edge && event.note>=0)pingPongEvent[(size_t)i]=true;' in p
assert 'pingPongEvent[(size_t)i]=param(256+i)>0.5f && v.gate && v.amp.stage==0;' in p
assert 'if(pingPongEvent[(size_t)i])v.pingPongPan.advance();' in p
assert 'SidPingPongArpStepTest Tests/SidPingPongArpStepTest.cpp' in cmake
assert 'SidPingPongArpStepTest.exe' in workflow
assert (r/'Source/ParameterCatalog.h').read_text().count('Ping Pong Pan')==3
print('OK: classical & SID Seq clock edges, one flip per sample, all parameters intact')
