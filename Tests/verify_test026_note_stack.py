from pathlib import Path
r=Path(__file__).resolve().parents[1]
h=(r/'Source/PluginProcessor.h').read_text()
c=(r/'Source/PluginProcessor.cpp').read_text()
st=(r/'Source/SidMidiNoteState.h').read_text()
assert 'SidMidiNoteState heldNotes;' in h
assert 'heldNotes.noteOn(mode,ch,note,filterLegato)' in c
assert 'heldNotes.noteOff(integer(param(3)),channel,note)' in c
assert 'param((mode==1)?10:10+i)' in c
assert 'filterEnvelope.noteOn(false);' in c
assert 'pressedCounts' in c
assert 'static constexpr int capacity = 16' in st
assert 'removeNewest(int note)' in st
assert 'for(int i=count-1;i>=0;--i)' in st
assert 'Source/SidMidiNoteState.h' in (r/'CMakeLists.txt').read_text()
assert 'SidMidiNoteStateTest' in (r/'.github/workflows/build-windows.yml').read_text()
print('MIDI note-stack and routing structural checks passed')
