"""Static regression checks for ported JSFX output stage and accessible navigation."""
from pathlib import Path
import re, json
root=Path(__file__).resolve().parents[1]
cpp=(root/'Source/PluginProcessor.cpp').read_text()
head=(root/'Source/SidOutputStage.h').read_text()
hed=(root/'Source/PluginEditor.h').read_text()
ed=(root/'Source/PluginEditor.cpp').read_text()
readme=(root/'README.md').read_text()
jsfx=(root/'Reference/Biscot1Synth_v2.38.jsfx').read_text()
assert 'function sid_output_amp(' in jsfx
for value in ('0.012','0.003','1.10','1.025','0.96','0.99','0.085','0.022','1.022','1.004'):
    assert value in head, value
for fragment in ('x / (1.0 + std::abs(x))', '1.0 - std::exp(-2.0*pi*std::min(16000.0,sr*0.45)/sr)',
                 'std::exp(-2.0*pi*1.6/sr)', 'previousLowpass = lowpass;', 'previousHighpass = output;'):
    assert fragment in head, fragment
assert 'outputStages[(size_t)ch].process(mix,model6581)*masterGain' in cpp
assert 'std::array<SidOutputStage,10> outputStages{};' in (root/'Source/PluginProcessor.h').read_text()
assert 'const double normTotal=0.0001+(integer(param(3))==1?3.0:1.0)' in cpp
assert 'const float l=x*(1.f-std::max(0.f,pan));' in cpp
assert 'const float r=x*(1.f+std::min(0.f,pan));' in cpp
assert 'const double dirty=SidOutputStage::softclip(mix*1.006)/1.006;' in cpp
assert 'out=saturate(y/maxGain)*maxGain;' not in cpp
assert 'Alt+Shift+Left/Right: section' in ed
assert 'm.isAltDown() && m.isShiftDown()' in ed
assert 'changeGroup(code==juce::KeyPress::rightKey?1:-1)' in ed
assert 'IncrementalParameterSearch parameterSearch;' in hed
assert 'parameterSearch.next(' in ed
assert 'biscotParameterSearchCharacter(k)' in ed
assert 'typeSearch(' not in ed
assert 'std::wstring' in ed
assert 'test0.2.2' in readme.lower()
print('OK: stage equations, signal chain, 10 independent output states, page keys and LB-303 search')
