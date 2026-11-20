from pathlib import Path
import json,re
r=Path(__file__).resolve().parents[1]
cat=(r/'Source/ParameterCatalog.h').read_text()
tracker=(r/'Source/TrackerParameterCatalog.h').read_text()
head=(r/'Source/PluginProcessor.h').read_text()
proc=(r/'Source/PluginProcessor.cpp').read_text()
editor=(r/'Source/PluginEditor.cpp').read_text()
core=(r/'Source/SidAmpEnvelope.h').read_text()
assert len(re.findall(r'^  \{\d+, "',cat,re.M))==260
assert 'std::array<ParameterDef,260>' in cat
assert 'std::array<TrackerParam,126>' in tracker
assert 'std::array<std::atomic<float>*,262>' in head
old=(r/'Reference/ParameterCatalog.json')
assert [x['index'] for x in json.loads(old.read_text())]==list(range(2,226))
native=json.loads((r/'Reference/NativeParameters.json').read_text())
assert [x['index'] for x in native[-13:-6]]==list(range(249,256))
for i in range(3):
 for index,name in ((249+i,f'Op{i+1} Hold'),(252+i,f'Op{i+1} Mod Env Hold')):
  assert f'{{{index}, "{name}", 0.0f, 5.0f, 0.001f, 0.0f, ""}}' in cat
assert '{255, "Filter Hold", 0.0f, 5.0f, 0.001f, 0.0f, ""}' in cat
for call in ('opSharedParam(249,i)','param(252+i)','param(255)'):
 assert call in proc
assert 'i==255)g=4' in editor
assert 'operatorPage.insert(afterSustain+1,sustainId)' in editor
assert 'pair<int,int>{34+i,249+i}' in editor
assert 'filterPage.insert(filterAttack+1,255)' in editor
assert 'holdSamplesRemaining' in core and 'stage=holdSamplesRemaining>0?5:2;' in core
assert 'if(idx>=249&&idx<=251)' in proc
assert 'if(idx>=252&&idx<=254)' in proc
assert 'if(idx==255)' in proc
assert 'sid==255' in (r/'Source/TrackerSongPlayback.h').read_text()
assert '262' in (r/'Source/TrackerPatternAutomation.h').read_text()
assert 'for(int index=249;index<=261;++index)' in proc
assert 'SidAhdsrHoldTest.cpp' in (r/'CMakeLists.txt').read_text()
print('OK: all seven AHDSR parameters, historic IDs, editor, REAPER automation, tracker and legacy presets')
