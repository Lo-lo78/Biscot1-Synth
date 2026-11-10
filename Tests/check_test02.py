"""Static source coverage and navigation contract. NOT a sound or Windows build test."""
from pathlib import Path
import re, json
root=Path(__file__).resolve().parents[1]
cpp=(root/'Source/PluginProcessor.cpp').read_text(encoding='utf-8')
h=(root/'Source/PluginProcessor.h').read_text(encoding='utf-8')
ed=(root/'Source/PluginEditor.cpp').read_text(encoding='utf-8')
data=json.loads((root/'Reference/ParameterCatalog.json').read_text(encoding='utf-8'))
assert len(data)==224
choices=[x for x in data if x.get('enum')]
assert len(choices)>20
assert 'std::make_unique<juce::AudioParameterChoice>' in cpp
assert 'std::make_unique<juce::AudioParameterInt>' in cpp
assert 'std::make_unique<juce::AudioParameterFloat>' in cpp
assert 'grid.setSelectedId(i+1,juce::sendNotificationSync)' in ed
assert 'return owner.handleKey(k,Control::grid)' in (root/'Source/PluginEditor.h').read_text()
assert 'return owner.handleKey(k,Control::value)' in (root/'Source/PluginEditor.h').read_text()
for key in ['juce::KeyPress::rightKey','juce::KeyPress::leftKey','juce::KeyPress::homeKey',
            'juce::KeyPress::pageUpKey','juce::KeyPress::tabKey']:
    assert key in ed, key
assert 'void BiscotEditor::focusGained' in ed
assert 'UiaRaiseNotificationEvent' in ed
for fragment in ['voices[(size_t)i].oscPrev','filterEnvelope.tick','processLfo','param(220+i*2)',
                 'param(191+i)','param(197+i)','param(122+i*23)','BiscotProcessor::rebuildArpPools',
                 'std::sort(ordered.begin(),ordered.begin()+count,', 'processFilter(filterInput[ch],cutoff,ch,resMod)']:
    assert fragment in cpp,fragment
assert 'std::array<std::array<int,640>,3> arpPools' in h
# Mirror group routing in editor and ensure every source slider occurs in exactly one page.
def group(i,name):
    if 122<=i<=190:return 5+(i-122)//23
    if 76<=i<=93:return 8
    if 94<=i<=103 or 191<=i<=202:return 9
    if 104<=i<=107 or 203<=i<=210:return 10
    if 58<=i<=63 or 211<=i<=225:return 11
    if 108<=i<=121:return 4
    if name.startswith('Op1 '):return 1
    if name.startswith('Op2 '):return 2
    if name.startswith('Op3 '):return 3
    return 0
counts=[0]*12
for p in data:counts[group(p['index'],p['name'])]+=1
assert all(counts),counts
assert sum(counts)==224
print('OK: TEST0.2 accessibility navigation, native combos, DSP route coverage; groups:',counts)
