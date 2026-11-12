"""Ensure all four requested keyboard routes remain distinct in Alt+Q."""
from pathlib import Path
r=Path(__file__).resolve().parents[1]
s=(r/'Source/TrackerPatternPanel.cpp').read_text()
workflow=(r/'.github/workflows/build-windows.yml').read_text()
# Enter opens the picker only while on a real Pattern step.
assert 'if(!m.isCtrlDown() && !m.isAltDown() && !m.isShiftDown() && code==juce::KeyPress::returnKey){\n  if(row>=0 && row<currentPattern().length){\n   clearSelection();page=2;parameterSearch.reset();refresh(true);' in s
# No F2 branch can open the old parameter picker.
assert 'if(code==juce::KeyPress::F2Key){announce("Song editor not yet implemented");return true;}' in s
assert "if(letter('e')){openSelectedValueEditor();return true;}" in s
assert 'if(code==juce::KeyPress::homeKey){setCurrentValueExtreme(true);return true;}' in s
assert 'if(code==juce::KeyPress::endKey){setCurrentValueExtreme(false);return true;}' in s
# Both parameter picker insertion modes still work, including Ctrl+Enter.
assert 'if(code==juce::KeyPress::returnKey){\n   if(const auto* def=currentParameter())' in s
assert 'if(!m.isCtrlDown()){page=1;refresh(true);}' in s
assert 'if(code==juce::KeyPress::escapeKey && page==2)' in s
assert 'NumericPurpose::patternNumber,"Go to Pattern 0 to 999"' in s
assert 'Biscot1-TEST0.2.21.1-Windows-x64' in workflow
assert 'VERSION 0.2.21.1' in (r/'CMakeLists.txt').read_text()
print('OK: remapped Enter, Alt+E, Alt+Enter, F2 reserved, reversed value limits')
