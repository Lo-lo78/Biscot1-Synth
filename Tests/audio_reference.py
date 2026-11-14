"""Strip only explicitly approved native Song FX additions before comparison
with the known byte-for-byte original SID DSP reference (TEST0.2.15).
This preserves the historical protection of synth core equations.
"""
def without_native_song_fx(dsp):
    patches={
       '  if(songMixActive)oscOut[(size_t)i]*=songMixer[(size_t)i];\n':'',
       '''  if(songFxModeActive && songFx[(size_t)i].sidechainSource>0){
   const int src=songFx[(size_t)i].sidechainSource-1;
   oscOut[(size_t)i]*=songFx[(size_t)i].duckGain(amp[(size_t)src]);
  }
''':'',
       '  if(songFxModeActive)songFx[(size_t)i].process(l,r,l,r,tempoBpm);\n':'',
       '''  float l=x*(1.f-std::max(0.f,pan));
  float r=x*(1.f+std::min(0.f,pan));
''':'''  const float l=x*(1.f-std::max(0.f,pan));
  const float r=x*(1.f+std::min(0.f,pan));
'''
    }
    for old,new in patches.items():
        assert old in dsp, 'Native Song FX change missing/unexpected; audit required: '+old[:55]
        dsp=dsp.replace(old,new)
    return dsp


def restore_legacy_sid_noise_dsp(dsp: str) -> str:
    """Restore only the audited SID noise clock experiment for legacy DSP hashes.

    The complete replacement region has a fixed SHA-256 fingerprint, so this
    does NOT suppress any other DSP changes. The old byte-for-byte reference
    checksum still detects changes anywhere outside this single audited block.
    """
    import hashlib

    noise_start='    // The clean noise retains its original 4 updates per oscillator cycle.\n'
    noise_end='   }\n   msb[(size_t)i]='
    assert dsp.count(noise_start)==1 and dsp.count(noise_end)==1
    noise_block=dsp[dsp.index(noise_start):dsp.index(noise_end,dsp.index(noise_start))]
    assert hashlib.sha256(noise_block.encode()).hexdigest()=='ce013539add218a58aefa55ee3e88f351288665c7cb26175c0afb66712ca70f7', 'SID noise implementation changed unexpectedly'
    legacy_noise_block='    v.noiseClock+=increments[(size_t)i]*4./oversample;\n    while(v.noiseClock>=1.){\n     v.noiseClock-=1.;\n     v.cleanNoiseSeed=(v.cleanNoiseSeed*1103515245ULL+12345ULL)%2147483647ULL;\n     const double clean=(double)v.cleanNoiseSeed/1073741823.5-1.;\n     const float character=limit(param(9),0.f,1.f);\n     if(character<=0.f)v.noise=(float)clean;\n     else{\n      uint32_t sid=(uint32_t)(v.sidNoiseSeed%8388608ULL);\n      if(sid==0)sid=8388607U;\n      const uint32_t feedback=((sid>>17)^(sid>>22))&1U;\n      sid=((sid<<1)&0x7fffffU)|feedback;\n      v.sidNoiseSeed=sid;\n      const int dac=((sid>>22)&1U)*128+((sid>>20)&1U)*64+\n        ((sid>>16)&1U)*32+((sid>>13)&1U)*16+\n        ((sid>>11)&1U)*8+((sid>>7)&1U)*4+\n        ((sid>>4)&1U)*2+((sid>>2)&1U);\n      const double coloured=dac/127.5-1.;\n      v.noise=(float)(clean*(1.-character)+coloured*character);\n     }\n    }\n'
    return dsp.replace(noise_block, legacy_noise_block, 1)
