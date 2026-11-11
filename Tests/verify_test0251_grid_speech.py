"""Regression: keyboard Grid selection must speak, while focus entry remains quiet."""
from pathlib import Path
r=Path(__file__).resolve().parents[1]
s=(r/'Source/PluginEditor.cpp').read_text()
start=s.index('void BiscotEditor::select(int i){')
end=s.index('void BiscotEditor::move(', start)
selection=s[start:end]
assert 'if(i==selected)return;' in selection  # silent boundaries
assert 'grid.setSelectedId(i+1,juce::dontSendNotification);' in selection
assert selection.index('selectedFromGrid();') < selection.index('announce(grid,labelFor(selected));')
assert selection.count('announce(grid,labelFor(selected));')==1
assert 'select(found); // select() makes the ONE concise navigation announcement' in s
assert 'if(c==Control::value)announce(value,labelFor(selected));' not in s
for cmd in [
    'move(-1,0)', 'move(1,0)', 'move(0,-1)', 'move(0,1)',
    'select(start)', 'select(juce::jmin(start+rowsPerColumn-1',
    'select(juce::jmax(start,selected-5))',
    'select(juce::jmin(juce::jmin(start+rowsPerColumn-1',
    'select(0)', 'select((int)groups[(size_t)group].size()-1)',
]:
    assert cmd in s,cmd
# Do not regress the previous focus/no-duplicate fix.
assert 'source.setDescription(text);' not in s
assert 'juce::AccessibilityHandler::clearCurrentlyFocusedHandler();' in s
assert 'if(focused!=nullptr && focused!=safe.getComponent())return;' in s
assert 'grid.setTitle({});grid.setDescription({});' in s
assert 'TEST0.2.17' in s
print('OK: Grid speech restored once per effective selection; entry focus still quiet')
