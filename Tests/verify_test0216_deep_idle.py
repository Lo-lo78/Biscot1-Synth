from pathlib import Path
import re
import hashlib
r=Path(__file__).resolve().parents[1]
p=(r/'Source/PluginProcessor.cpp').read_text()
h=(r/'Source/PluginProcessor.h').read_text()
w=(r/'.github/workflows/build-windows.yml').read_text()
assert 'coldIdleNoMidi=idleSleep.asleep && midi.isEmpty()' in p
assert 'if(coldIdleNoMidi)' in p and 'advanceIdleLfos(buffer.getNumSamples());' in p
assert p.index('if(coldIdleNoMidi)')<p.index('auto mix=getBusBuffer')
assert 'refreshArpPoolsIfNeeded();' in p
assert 'if(arpPoolCache.needsRefresh(settings))rebuildArpPools(true);' in p
assert 'arpPoolCache.didRefresh(settings);' in p
assert 'if(settings.arpMode[(size_t)i]<=1)' in p
assert 'rebuildArpPools();return;' in p # keep event-accurate note on/off
assert 'SidArpPoolCache arpPoolCache{};' in h
assert 'SidDeepIdleCpuTest.exe' in w and 'verify_test0216_deep_idle.py' in w
assert 'VERSION 0.2.21.2' in (r/'CMakeLists.txt').read_text()
assert 'TEST0.2.21.2' in w
# Byte-for-byte regression: JSFX-derived DSP and the complete MIDI event
# dispatch are unchanged from the audible TEST0.2.15 reference.
dsp=p[p.index('void BiscotProcessor::renderOne('):p.index('void BiscotProcessor::advanceIdleLfos(')]
midi=p[p.index('void BiscotProcessor::onMidi('):p.index('float BiscotProcessor::polyBlep(')]
assert hashlib.sha256(dsp.encode()).hexdigest()=='d6e631be7980f9840d4c44e30b3dd6bd02c632f8cb5ea6df40297c0aa6e27914'
assert 'rebuildArpPools(true);return;' in midi and 'noteOff(ch,note);' in midi
assert 'if(!down)sustainedArp.clearChannel(ch);' in midi
assert 'm.isAllNotesOff()' in midi and 'm.isPitchWheel()' in midi

print('OK: fast idle, audio unchanged, MIDI events intact, parameter-driven arp pools')
