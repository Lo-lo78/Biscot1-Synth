"""TEST0.2.48: new AHDSR Trigger controls preserve every older host ID."""
from pathlib import Path
r=Path(__file__).resolve().parents[1]
c=(r/'Source/ParameterCatalog.h').read_text()
p=(r/'Source/PluginProcessor.cpp').read_text()
e=(r/'Source/PluginEditor.cpp').read_text()
a=(r/'Source/SidAmpEnvelope.h').read_text()
t=(r/'Source/TrackerParameterCatalog.h').read_text()
for idx in range(256,259):
 assert f'{{{idx}, "Op{idx-255} AHDSR Trigger", 0.0f, 1.0f, 1.0f, 0.0f, "Off|On"}}' in c
assert 'for(const auto& d:catalog)if(d.index<256)addCatalogParameter(d);' in p
assert 'for(const auto& d:catalog)if(d.index>=256)addCatalogParameter(d);' in p
assert p.index('"trackerSongTransport"') < p.index('if(d.index>=256)addCatalogParameter')
assert 'v.amp.tickTrigger(param(256+i)>0.5f,v.gate' in p
assert 'if(idx>=256&&idx<=258)return idx==256?idx+lane:-1;' in p
assert 'if(stage==3)noteOff(release,sampleRate);' in a
assert 'else if(stage==0)noteOn(true);' in a
assert 'const int trigger=256+i,release=43+i;' in e
assert '{3018,"OP AHDSR Trigger",1,0,1,1,0,256}' in t
assert 'for(int index=249;index<=261;++index)' in p
print('OK: independent AHDSR Trigger controls, default Off, host parameter order preserved')
