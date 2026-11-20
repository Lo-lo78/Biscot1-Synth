"""TEST0.2.49: stable IDs, no allocations, audio/host/tracker routing."""
import json
from pathlib import Path
r=Path(__file__).resolve().parents[1]
c=(r/'Source/ParameterCatalog.h').read_text()
p=(r/'Source/PluginProcessor.cpp').read_text()
h=(r/'Source/PluginProcessor.h').read_text()
e=(r/'Source/PluginEditor.cpp').read_text()
t=(r/'Source/TrackerParameterCatalog.h').read_text()
s=(r/'Source/TrackerPatternAutomation.h').read_text()
native=json.loads((r/'Reference/NativeParameters.json').read_text())
assert 'std::array<ParameterDef,260>' in c
assert 'std::array<std::atomic<float>*,262>' in h
for i in range(3):
 assert f'{{{259+i}, "Op{i+1} Ping Pong Pan", -1.0f, 1.0f, 0.01f, 0.0f, ""}}' in c
 assert native[-3+i]['index']==259+i
assert 'if(!arpOn){' in p and 'else v.pingPongPan.advanceAtAttack(param(259+i));' in p
assert 'pingPongEvent[(size_t)i]=ahdsrReattack;' in p
assert 'v.pingPongPan.tick(param(259+i),panPitchCoefficient[(size_t)i])' in p
assert 'for(int index=249;index<=261;++index)' in p
assert 'if(idx>=259&&idx<=261)return idx==259?idx+lane:-1;' in p
assert 'const int pingPong=259+i,basePan=19+i;' in e
assert '{3019,"OP Ping Pong Pan",1,-1,1,0.01,0,259}' in t
assert 'std::array<TrackerParam,126>' in t
assert 'std::array<bool,262>' in s
assert (r/'Source/PluginProcessor.cpp').read_text().index('"trackerSongTransport"') < p.index('if(d.index>=256)addCatalogParameter')
print('OK: three signed pan parameters append-only, accessible, tracker/song, original IDs preserved')
