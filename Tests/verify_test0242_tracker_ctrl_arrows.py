"""TEST0.2.42: Ctrl+vertical arrows consistently navigate regions and banks."""
from pathlib import Path
root=Path(__file__).resolve().parents[1]
p=(root/'Source/TrackerPatternPanel.cpp').read_text(encoding='utf-8')
s=(root/'Source/TrackerSongPanel.cpp').read_text(encoding='utf-8')
assert 'if(m.isShiftDown()){' in p and 'jumpToPattern(next)' in p
assert 'trackerF1HomeTarget(row,pat.length,lastRealStep)' in p
assert 'trackerF1EndTarget(row,pat.length,pat.repeat,lastRealStep)' in p
assert 'Ctrl+Home/End no longer change tracker regions' in p
assert 'jumpToStep(juce::jlimit(0,currentPattern().length-1' not in p
assert 'if(!picker&&m.isCtrlDown()&&!m.isAltDown()&&' in s
assert 'if(m.isShiftDown())changeSong(processor.trackerSongs.activeSong+direction)' in s
assert 'else navigateArea(direction)' in s
assert 'Region navigation moved to Ctrl+Up/Down' in s
assert 'Ctrl+Shift+Up/Down: Song' in s
print('OK: Ctrl+Up/Down regions and Ctrl+Shift+Up/Down banks in F1/F2')
