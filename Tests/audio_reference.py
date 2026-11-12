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
