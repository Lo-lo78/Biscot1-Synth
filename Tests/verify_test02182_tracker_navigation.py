"""Regression checks for native F1/F2 navigation and LFO mapping."""
from pathlib import Path
import hashlib, re
root=Path(__file__).resolve().parents[1]
panel=(root/'Source/TrackerPatternPanel.cpp').read_text()
header=(root/'Source/TrackerPatternPanel.h').read_text()
catalog=(root/'Source/TrackerParameterCatalog.h').read_text()
assert 'if(m.isCtrlDown() && code==juce::KeyPress::homeKey)' in panel
assert 'parameterIndex=0;lastParam' in panel
assert 'if(m.isCtrlDown() && code==juce::KeyPress::endKey)' in panel
assert 'parameterIndex=juce::jmax(0,(int)parameterList.size()-1)' in panel
assert 'if(code==juce::KeyPress::homeKey){move=-parameterIndex%paramHeight;}' in panel
assert 'if(code==juce::KeyPress::endKey){move=juce::jmin(count-1-parameterIndex,paramHeight-1-parameterIndex%paramHeight);}' in panel
assert panel.count('if(code==juce::KeyPress::homeKey)')>=2
assert 'trackerMoveWithinF1Region(row,delta,pat.length,pat.repeat)' in panel
assert 'trackerF1HomeTarget(row,currentPattern().length,lastRealStep)' in panel
assert 'trackerF1EndTarget(row,currentPattern().length,currentPattern().repeat,lastRealStep)' in panel
assert 'biscotParameterSearchCharacter(k)' in panel
assert 'parameterSearch.next(' in panel
assert 'm.isShiftDown()?-1:1' in panel
assert 'juce::Time::getMillisecondCounter()' in panel
assert 'ClipboardMode::steps' in panel
assert 'selectionAnchor' in header and 'selectionEnd' in header
assert 'void TrackerPatternPanel::pasteSelectedSteps()' in panel
assert 'void TrackerPatternPanel::eraseSelectedSteps(bool cut)' in panel
assert "if(m.isCtrlDown() && letter('x'))" in panel
assert 'if(code==juce::KeyPress::deleteKey && m.isCtrlDown() && m.isShiftDown()' in panel
assert 'if(code==juce::KeyPress::deleteKey && m.isCtrlDown() && !m.isAltDown())' in panel
assert 'std::vector<TrackerStep> stepClipboard;' in header
assert 'pat.steps[row+i]=stepClipboard[(size_t)i]' in panel
# All LFOs occupy a shared F2 LFO section, but each LFO is independently addressable.
for num in (1,2,3):
 for suffix in ('Mode','Rate','Wave','Square PWM','Cutoff Depth','OP Pitch Depth'):
  assert f'"LFO{num} {suffix}",3,' in catalog,(num,suffix)
assert 'const juce::Identifier trackerStateKey{"trackerPatternsV1"}' in (root/'Source/PluginProcessor.cpp').read_text()
assert 'renderOne(' in (root/'Source/PluginProcessor.cpp').read_text()
print('OK TEST0.2.21 navigation, multi-letter search, steps and all 3 LFOs')
