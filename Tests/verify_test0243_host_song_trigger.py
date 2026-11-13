from pathlib import Path
r=Path(__file__).resolve().parents[1]
cpp=(r/'Source/PluginProcessor.cpp').read_text()
h=(r/'Source/PluginProcessor.h').read_text()
assert '"trackerSongTrigger"' in cpp
assert '"Song Trigger",0,100,0' in cpp
assert 'transportStarted && hostSelection>0' in cpp
assert 'hostSongPending=hostSelection>0?hostSelection-1:-1' in cpp
assert 'songRequested.store(5000+hostSongPending' in cpp
assert 'bank->byNumber[(size_t)selected]' in cpp
assert 'hostSongTriggerParameter' in h
assert 'hostSongLastSelection=-1' in h
assert 'hostSongTriggerParameter=state.getRawParameterValue' in cpp
print('OK: native Song Trigger automation and DAW Play restart')
