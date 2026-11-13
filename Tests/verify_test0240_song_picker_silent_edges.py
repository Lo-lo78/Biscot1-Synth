"""0.2.40: F2 Song parameter-picker navigation must be silent at its borders.

Song and Pattern pickers both use clamped indexes, but only refresh/speak when
that clamp actually changes the selection. The key handler consumes keys at
borders so JUCE cannot move focus out of Alt+Q.
"""
from pathlib import Path
import re
root = Path(__file__).resolve().parents[1]
song = (root/'Source/TrackerSongPanel.cpp').read_text(encoding='utf-8')
pattern = (root/'Source/TrackerPatternPanel.cpp').read_text(encoding='utf-8')
song_picker = song.split(' if(picker){\n  if(code==juce::KeyPress::returnKey)',1)[1].split(' if(m.isAltDown()&&!m.isCtrlDown()&&letter(\'e\'))',1)[0]
pattern_picker = pattern.split(' if(page==2){\n  if(m.isCtrlDown()',1)[1].split(' // Enter on a Pattern step',1)[0]
assert 'if(next!=parameterIndex){' in pattern_picker, 'F1 silent-edge rule must remain'
assert 'const int next=juce::jlimit(0,(int)songParameters.size()-1,pickerIndex+move);' in song_picker
assert 'if(next!=pickerIndex){pickerIndex=next;search.reset();refresh(true);}' in song_picker
assert 'if(move){pickerIndex=juce::jlimit' not in song_picker
for key in ('upKey','downKey','leftKey','rightKey','pageUpKey','pageDownKey','homeKey','endKey'):
    assert f'code==juce::KeyPress::{key}' in song_picker,key
assert 'return true;' in song_picker
print('OK: F2 Song parameter-picker silent borders match F1 Pattern for all navigation keys')
