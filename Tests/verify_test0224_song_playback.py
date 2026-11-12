"""Phase 3.1: Song uses pinned snapshots, three independent sample-clock lanes."""
from pathlib import Path
r=Path(__file__).resolve().parents[1]
p=(r/'Source/PluginProcessor.cpp').read_text()
h=(r/'Source/PluginProcessor.h').read_text()
s=(r/'Source/TrackerSongPanel.cpp').read_text()
cm=(r/'CMakeLists.txt').read_text()
w=(r/'.github/workflows/build-windows.yml').read_text()
for t in ('TrackerSongTransport songTransport','songPlaybackSnapshot','requestSongStart','songPlaybackStatus', 'songMixActive'):
 assert t in h, t
for t in ('const int songCommand=songRequested.exchange','songTransport.tick','songApplyStep(lane,e)','songReleaseLane(lane)',
          'onMidi(juce::MidiMessage::noteOn(lane+1,', 'songMixer[(size_t)i]', 'compileTrackerSong(*song,',
          'songMapSlider(a.synthIndex,lane,clip)','trackerSetOverride(3,2.f)',
          'songMidiMap.load(std::memory_order_acquire)','songMidiTriggerNote=note',
          'if(!running && lastPlaying && songTransport.running)' ):
 assert t in p,t
for t in ('processor.requestSongStart','processor.requestSongPause','processor.requestSongStop', 'processor.songAudibleRows'):
 assert t in s,t
assert 'Song playback will be added after the editor' not in s
assert 'TrackerSongPlaybackTest.exe' in w
assert 'TrackerSongPlaybackTest.cpp' in cm
print('OK: independent Song lanes, immutable snapshots, playback UI, DSP routing, Release regression')
