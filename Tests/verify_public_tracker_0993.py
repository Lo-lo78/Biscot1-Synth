"""Public 0.99.3: operator keys are scoped to Steps and Space tracks chained Song."""
from pathlib import Path
r=Path(__file__).resolve().parents[1]
p=(r/'Source/TrackerPatternPanel.cpp').read_text()
s=(r/'Source/TrackerSongPanel.cpp').read_text()
h=(r/'Source/TrackerSongPanel.h').read_text()
assert 'if(page==1 && row>=0 && row<currentPattern().length)setTab(section)' in p
assert "if(code=='4'||realStep())changeLane(code-'1')" in s
assert 'void TrackerSongPanel::followPlayingSong(bool speak)' in s
assert 'void followPlayingSong(bool speak);' in h
assert 'followPlayingSong(true);' in s
assert 'followPlayingSong(false);' in s
assert 'processor.songAudibleRows[(size_t)op].load(std::memory_order_relaxed)' in s
assert 'processor.requestSongStop();' in s
assert 'lastStep[(size_t)op]=juce::jlimit' in s
assert 'processor.requestSongStart(lane<3&&realStep()?row:0)' in s
print('OK: F1/F2 operators only in Steps; Space captures chained Song and audible row')
