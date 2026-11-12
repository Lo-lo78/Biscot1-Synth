"""Check remapped F1 Enter/Alt+E/Alt+Enter and parameter picker insertion."""
from pathlib import Path
r=Path(__file__).resolve().parents[1]
panel=(r/'Source/TrackerPatternPanel.cpp').read_text()
header=(r/'Source/TrackerPatternPanel.h').read_text()
cmake=(r/'CMakeLists.txt').read_text()
workflow=(r/'.github/workflows/build-windows.yml').read_text()
# Alt+Enter must be reached BEFORE the F2 insertion branch.
assert panel.index('if(m.isAltDown() && !m.isCtrlDown() && !m.isShiftDown() && code==juce::KeyPress::returnKey)') < panel.index('if(page==2){',panel.index('if(m.isAltDown() && !m.isCtrlDown() && !m.isShiftDown() && code==juce::KeyPress::returnKey)'))
assert 'openNumericEditor(NumericPurpose::patternNumber,"Go to Pattern 0 to 999"' in panel
assert """if(!m.isCtrlDown() && !m.isAltDown() && !m.isShiftDown() && code==juce::KeyPress::returnKey){
  if(row>=0 && row<currentPattern().length){""" in panel
assert 'void TrackerPatternPanel::openSelectedValueEditor()' in panel
assert 'void openSelectedValueEditor();' in header
assert 'value=pat.length;name="Length"' in panel
assert 'value=pat.midiNote;name="MIDI Note"' in panel
assert 'value=pat.repeat;name="Repeat"' in panel
assert 'name="Speed "+juce::String(idx+1)' in panel
assert 'name="Transpose "+juce::String(idx+1)' in panel
assert 'value=selected->value' in panel
assert "if(letter('e')){openSelectedValueEditor();return true;}" in panel
assert 'parseTrackerManualNumeric(txt.toStdString(),newValue)' in panel
assert """if(code==juce::KeyPress::returnKey){
   if(const auto* def=currentParameter())""" in panel
assert 'if(!m.isCtrlDown()){page=1;refresh(true);}' in panel
assert 'if(page==2){refresh(false);announce("Pattern "+juce::String(target)' in panel
assert 'VERSION 0.2.21.3' in cmake
assert 'TEST0.2.21.3' in (r/'Source/PluginEditor.cpp').read_text()
assert 'TrackerManualNumericTest.exe' in workflow
assert 'verify_test02195_enter_value.py' in workflow
print('OK: Enter value, Alt+Enter Pattern, F2 insertion, numeric validation, version')
