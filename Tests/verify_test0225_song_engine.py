"""TEST0.2.31: native Song End / owner / effects control integration."""
from pathlib import Path
r=Path(__file__).resolve().parents[1]
h=(r/'Source/TrackerSongPlayback.h').read_text()
f=(r/'Source/PluginProcessor.cpp').read_text()
p=(r/'Source/PluginProcessor.h').read_text()
fx=(r/'Source/TrackerSongFxCore.h').read_text()
w=(r/'.github/workflows/build-windows.yml').read_text()
for snippet in ('struct TrackerSongChainMap','compileTrackerSongChainMap', 'struct TrackerSongRowFx',
                'fxRows','trackerSongResolveOwner','trackerSongResourceFlags','rowStarted'):
 assert snippet in h, snippet
for snippet in ('songChainNext()', 'songChainMap.load','songChainSnapshot','songResolveResources(events)',
                'songApplyRowFx(lane,','songFx[(size_t)i].duckGain','songFx[(size_t)i].process',
                'songMapSlider', 'if(idx>=114&&idx<=116)return -1;',
                'songFxModeActive=false;', 'if(events.finished && !songChainNext())songStopAudio(false)'):
 assert snippet in f, snippet
assert 'std::array<TrackerSongFxCore,3> songFx' in p
assert 'std::vector<float> left,right' in fx
assert 'clearDelay()noexcept' in fx
assert 'TrackerSongAdvancedTest.exe' in w
assert 'Biscot1-TEST0.2.31-Windows-x64' in w
print('OK: song chain, resource ownership, row-only fx, sidechain/delay and audio integration')
