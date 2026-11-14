from release_version_guard import release_version, release_test, release_cmake, release_artifact
from audio_reference import without_native_song_fx
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
assert release_cmake in (r/'CMakeLists.txt').read_text()
assert release_test in w
# Byte-for-byte regression: JSFX-derived DSP and the complete MIDI event
# dispatch are unchanged from the audible TEST0.2.15 reference.
dsp=p[p.index('void BiscotProcessor::renderOne('):p.index('void BiscotProcessor::advanceIdleLfos(')]
midi=p[p.index('void BiscotProcessor::onMidi('):p.index('float BiscotProcessor::polyBlep(')]
assert 'if(songMixActive)oscOut[(size_t)i]*=songMixer[(size_t)i];' in dsp
# Keep the historic byte-for-byte DSP guard while allowing ONLY the audited
# experimental SID noise-clock block; verify the changed region separately.
noise_start='    // The clean noise retains its original 4 updates per oscillator cycle.\n'
noise_end='   }\n   msb[(size_t)i]='
assert dsp.count(noise_start)==1 and dsp.count(noise_end)==1
noise_block=dsp[dsp.index(noise_start):dsp.index(noise_end,dsp.index(noise_start))]
assert hashlib.sha256(noise_block.encode()).hexdigest()=='ce013539add218a58aefa55ee3e88f351288665c7cb26175c0afb66712ca70f7', 'SID noise implementation changed unexpectedly'
legacy_noise_block='    v.noiseClock+=increments[(size_t)i]*4./oversample;\n    while(v.noiseClock>=1.){\n     v.noiseClock-=1.;\n     v.cleanNoiseSeed=(v.cleanNoiseSeed*1103515245ULL+12345ULL)%2147483647ULL;\n     const double clean=(double)v.cleanNoiseSeed/1073741823.5-1.;\n     const float character=limit(param(9),0.f,1.f);\n     if(character<=0.f)v.noise=(float)clean;\n     else{\n      uint32_t sid=(uint32_t)(v.sidNoiseSeed%8388608ULL);\n      if(sid==0)sid=8388607U;\n      const uint32_t feedback=((sid>>17)^(sid>>22))&1U;\n      sid=((sid<<1)&0x7fffffU)|feedback;\n      v.sidNoiseSeed=sid;\n      const int dac=((sid>>22)&1U)*128+((sid>>20)&1U)*64+\n        ((sid>>16)&1U)*32+((sid>>13)&1U)*16+\n        ((sid>>11)&1U)*8+((sid>>7)&1U)*4+\n        ((sid>>4)&1U)*2+((sid>>2)&1U);\n      const double coloured=dac/127.5-1.;\n      v.noise=(float)(clean*(1.-character)+coloured*character);\n     }\n    }\n'
baseline_dsp=dsp.replace(noise_block,legacy_noise_block,1)
assert hashlib.sha256(without_native_song_fx(baseline_dsp).encode()).hexdigest()=='d6e631be7980f9840d4c44e30b3dd6bd02c632f8cb5ea6df40297c0aa6e27914'

assert 'rebuildArpPools(true);return;' in midi and 'noteOff(ch,note);' in midi
assert 'if(!down)sustainedArp.clearChannel(ch);' in midi
assert 'm.isAllNotesOff()' in midi and 'm.isPitchWheel()' in midi

print('OK: fast idle, audited SID noise update, unchanged legacy DSP remainder, MIDI event and arp pool invariants')
