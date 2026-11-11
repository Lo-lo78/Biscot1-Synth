from pathlib import Path
r=Path(__file__).resolve().parents[1]
h=(r/'Source/PluginProcessor.h').read_text()
c=(r/'Source/PluginProcessor.cpp').read_text()
s=(r/'Source/SidSeqMemory.h').read_text()
w=(r/'.github/workflows/build-windows.yml').read_text()
assert 'std::array<SidSeqMemory,3> sidSeq' in h
assert 'if(!chordWasHeld)seq.clearSequence(note)' in c
assert 'else seq.record(note)' in c
assert 'seq.snapshot(heldNotes.voices[(size_t)i])' in c
assert 'const int count=chord?heldNotes.chord.count:seq.latch.count' in c
assert 'const float target=seq.process(' in c
assert 'sidSeq[(size_t)i].clearAll();' in c
assert 'sidSeq[(size_t)i].releaseRoot' in c
assert 'std::array<int,capacity*2>' in s
assert 'SidSeqMemoryTest.exe' in w
print('SID Seq JSFX memory integration checks passed')
