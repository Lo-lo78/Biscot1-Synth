import json,re,pathlib
root=pathlib.Path(__file__).resolve().parents[1]
jsfx=(root/'Reference/Biscot1Synth_v2.38.jsfx').read_text(encoding='utf-8-sig')
data=json.loads((root/'Reference/ParameterCatalog.json').read_text(encoding='utf-8'))
lines=re.findall(r'^slider(\d+):',jsfx,re.M)
assert len(data)==224,len(data)
assert [p['index'] for p in data]==list(map(int,lines))
assert len(set(lines))==len(lines)
for p in data:assert p['min']<=p['default']<=p['max'],p
assert data[-1]['index']==225
print('OK: all 224 real JSFX sliders captured, indices and ranges intact')
