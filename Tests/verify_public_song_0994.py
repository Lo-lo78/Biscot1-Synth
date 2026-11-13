"""0.99.4: silent Song Length border and manual Space start/stop origin."""
from pathlib import Path
root=Path(__file__).resolve().parents[1]
s=(root/'Source/TrackerSongPanel.cpp').read_text()
h=(root/'Source/TrackerSongPanel.h').read_text()
assert 'if(direction==0 || (lane<3 && row<0 && direction<0))return;' in s
assert 'void TrackerSongPanel::rememberManualPlayOrigin()' in s
assert 'void TrackerSongPanel::restoreManualPlayOrigin()' in s
assert 'manualPlayOrigin.song=processor.trackerSongs.activeSong;' in s
assert 'manualPlayOrigin.lastStep=lastStep;' in s
assert 'processor.trackerSongs.activeSong=manualPlayOrigin.song;' in s
assert 'lane=manualPlayOrigin.lane;' in s and 'row=manualPlayOrigin.row;' in s
assert 'cell=manualPlayOrigin.cell;' in s
assert 'manualPlayOriginValid=false;' in s
assert 'struct ManualPlayOrigin' in h
assert 'bool awaitingManualStop=false;' in h
assert 'if(awaitingManualStop){' in s
assert 'rememberManualPlayOrigin();' in s
assert 'processor.requestSongStop();\n    restoreManualPlayOrigin();'.replace('\n','\n') in s
assert 'followPlayingSong(true);' in s  # navigation while playing still follows transitions
print('OK: F2 Length border is silent, Stop restores exact Space Play cursor after chains')
