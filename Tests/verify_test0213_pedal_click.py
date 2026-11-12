from pathlib import Path
base=Path(__file__).resolve().parents[1]
p=(base/'Source/PluginProcessor.cpp').read_text(encoding='utf-8')
s=(base/'Source/SidSeqMemory.h').read_text(encoding='utf-8')
assert 'sidSeq[(size_t)i].latchCurrentChordRootForRelease(lastLiveChordRoot)' in p
release=p.split('void BiscotProcessor::releasePedalLatches(){',1)[1].split('void BiscotProcessor::rebuildArpPools(bool captureRelease){',1)[0]
assert 'seq.releaseRoot=' not in release and 'sidSeq[(size_t)i].releaseRoot=' not in release
assert 'sidLatchArpReleasePool(pool,count,arpReleasePools[(size_t)i],' in p
assert 'rebuildArpPools(true);return;' in p  # Snapshots on Note On only
assert 'if(!down)sustainedArp.clearChannel(ch);' in p
assert 'void latchCurrentChordRootForRelease(int lastLiveRoot)' in s
assert 'v.amp.noteOff(opSharedParam(43,i),sampleRateHz)' in release
assert 'TEST0.2.25' in (base/'CMakeLists.txt').read_text()
print('Pedal release preserves arp root and note pool; VCA follows manual release.')
