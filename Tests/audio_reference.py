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


def restore_legacy_ahdsr_dsp(dsp: str) -> str:
    """Normalize only the three audited TEST0.2.46 AHDSR renderOne calls.

    This keeps historic SHA-256 regressions meaningful; it does not allow any
    other oscillator/filter/DSP edits. Independent Hold sample-clock behavior
    is covered by SidAhdsrHoldTest.cpp.
    """
    replacements = {
        'v.amp.tick(opSharedParam(34,i),opSharedParam(249,i),opSharedParam(37,i),opSharedParam(40,i),sampleRateHz)':
            'v.amp.tick(opSharedParam(34,i),opSharedParam(37,i),opSharedParam(40,i),sampleRateHz)',
        'v.mod.tick(param(64+i),param(252+i),param(67+i),param(70+i),sampleRateHz)':
            'v.mod.tick(param(64+i),param(67+i),param(70+i),sampleRateHz)',
        'filterEnvelope.tick(param(118),param(255),param(119),param(120),sampleRateHz)':
            'filterEnvelope.tick(param(118),param(119),param(120),sampleRateHz)',
    }
    for new, legacy in replacements.items():
        assert dsp.count(new)==1, 'Unreviewed AHDSR DSP modification: '+new
        dsp=dsp.replace(new,legacy,1)
    return dsp


def restore_live_automation_dsp(dsp: str) -> str:
    """Undo exactly the reviewed TEST0.2.47 live-control wiring for old hashes.

    The original TEST0.2.15 SHA-256 reference remains the authority for every
    unchanged sample equation. Unknown edits are never normalized: each of the
    seven intended modifications is matched verbatim and counted once.
    New live behavior is tested independently by SidLiveAutomationTest.cpp and
    verify_live_automation_wiring.py.
    """
    # TEST0.2.52: restore only the audited sample-synchronous glide routing
    # before the existing 0.2.49-0.2.51/historical DSP checksum normalizers.
    # All original pitch and synthesis equations remain byte-for-byte guarded.
    def restore_exact(new, old, label):
        nonlocal dsp
        assert dsp.count(new)==1, 'Unreviewed glide change: '+label
        dsp=dsp.replace(new,old,1)

    # 0.2.52 default-zero fast path: no pitch-motion calculations when off.
    restore_exact(' // Skip glide math entirely for legacy presets with Ping Pong Pan Off.\n'
                  ' std::array<bool,3> panPitchSyncActive{};\n'
                  ' for(int i=0;i<3;++i)panPitchSyncActive[(size_t)i]=\n'
                  '  param(259+i)!=0.f || voices[(size_t)i].pingPongPan.depth!=0.f;\n',
                  '', 'zero-depth CPU fast path')
    restore_exact('if(panPitchSyncActive[(size_t)i])panPitchMotion[(size_t)i].add(arpBefore,target,v.arpPitch);',
                  'panPitchMotion[(size_t)i].add(arpBefore,target,v.arpPitch);',
                  'SID Seq conditional motion')
    restore_exact('if(panPitchSyncActive[(size_t)i])panPitchMotion[(size_t)i].add(arpBefore,destination,v.arpPitch);',
                  'panPitchMotion[(size_t)i].add(arpBefore,destination,v.arpPitch);',
                  'classical conditional motion')
    restore_exact('if(panPitchSyncActive[(size_t)i] && !(integer(param(76+i))==1 && sidSeq[(size_t)i].latch.count>0))',
                  'if(!(integer(param(76+i))==1 && sidSeq[(size_t)i].latch.count>0))',
                  'portamento conditional motion')
    restore_exact('if(panPitchSyncActive[(size_t)i])panPitchCoefficient[(size_t)i]=panPitchMotion[(size_t)i].coefficient();',
                  'panPitchCoefficient[(size_t)i]=panPitchMotion[(size_t)i].coefficient();',
                  'conditional coefficient')
    restore_exact(
      ' std::array<bool,3> pingPongEvent{};\n'
      ' std::array<SidPitchGlideMotion,3> panPitchMotion{};\n'
      ' std::array<float,3> panPitchCoefficient{1.f,1.f,1.f};\n',
      ' std::array<bool,3> pingPongEvent{},pingPongAttack{};\n',
      'per-sample glide buffers')
    restore_exact('pingPongEvent[(size_t)i]=ahdsrReattack;',
                  'pingPongEvent[(size_t)i]=pingPongAttack[(size_t)i]=ahdsrReattack;',
                  'retrigger event')
    restore_exact('   const float arpBefore=v.arpPitch;\n'
                  '   v.arpPitch+=(target-v.arpPitch)*coefficient;\n'
                  '   if(std::abs(target-v.arpPitch)<0.00001f)v.arpPitch=target;\n'
                  '   panPitchMotion[(size_t)i].add(arpBefore,target,v.arpPitch);',
                  '   v.arpPitch+=(target-v.arpPitch)*coefficient;\n'
                  '   if(std::abs(target-v.arpPitch)<0.00001f)v.arpPitch=target;',
                  'SID Seq pitch sampling')
    restore_exact('    const float arpBefore=v.arpPitch;\n'
                  '    v.arpPitch+=(destination-v.arpPitch)*(glide<=0.f?1.f:limit(1.f/(float)(sampleRateHz*(0.002+glide*0.3)),0.0001f,1.f));\n'
                  '    panPitchMotion[(size_t)i].add(arpBefore,destination,v.arpPitch);',
                  '    v.arpPitch+=(destination-v.arpPitch)*(glide<=0.f?1.f:limit(1.f/(float)(sampleRateHz*(0.002+glide*0.3)),0.0001f,1.f));',
                  'classical arp pitch sampling')
    restore_exact('   const double noteBefore=v.noteSmooth;\n'
                  '   v.noteSmooth+=(v.note-v.noteSmooth)*smoothing;\n'
                  '   // SID Seq uses its own latched root instead of noteSmooth; in all other\n'
                  '   // modes the oscillator\'s pitch includes both portamento and arpPitch.\n'
                  '   if(!(integer(param(76+i))==1 && sidSeq[(size_t)i].latch.count>0))\n'
                  '    panPitchMotion[(size_t)i].add(noteBefore,v.note,v.noteSmooth);',
                  '   v.noteSmooth+=(v.note-v.noteSmooth)*smoothing;',
                  'portamento pitch sampling')
    restore_exact('  frequencies[(size_t)i]=std::min(sampleRateHz*0.45,440.*std::pow(2.,juce::jlimit(-130.,130.,(double)(pitch-69.)/12.)));\n  panPitchCoefficient[(size_t)i]=panPitchMotion[(size_t)i].coefficient();\n  if(pingPongEvent[(size_t)i]){\n   // One flip per audible arp grid or cyclical AHDSR Attack. When pitch\n   // slides, the side will use the exact SAME glide rate on this sample.\n   // When pitch jumps (or remains unchanged), switch immediately.\n   if(panPitchMotion[(size_t)i].moving() && panPitchCoefficient[(size_t)i]<1.f)\n    v.pingPongPan.advance();\n   else v.pingPongPan.advanceAtAttack(param(259+i));\n  }\n',
                  '  frequencies[(size_t)i]=std::min(sampleRateHz*0.45,440.*std::pow(2.,juce::jlimit(-130.,130.,(double)(pitch-69.)/12.)));\n',
                  'glide pan event after note and arp pitch')
    restore_exact('  } // Through Release, keep the clocked frequency arp; never retrigger the ADSR.\n }\n std::array<double,3> frequencies{};',
                  '  } // Through Release, keep the clocked frequency arp; never retrigger the ADSR.\n  if(pingPongEvent[(size_t)i]){\n   // The automatic AHDSR reattack is an Attack *on this sample*, not a\n   // continuous-pan modulation. Snap before the oscillator is mixed.\n   if(pingPongAttack[(size_t)i])v.pingPongPan.advanceAtAttack(param(259+i));\n   else v.pingPongPan.advance();\n  }\n }\n std::array<double,3> frequencies{};',
                  'original event placement')
    restore_exact('v.pingPongPan.tick(param(259+i),panPitchCoefficient[(size_t)i])',
                  'v.pingPongPan.tick(param(259+i))','output pan coefficient')

    # TEST0.2.49: narrowly undo only the new alternate-pan wiring. Every
    # occurrence is verified before the historic JSFX DSP checksum is computed.
    cycle = ('  // A cyclic AHDSR reattack also requests a pan flip. Merge with any\n'
             '  // simultaneous arp grid edge below instead of flipping twice.\n'
             '  const bool ahdsrReattack=param(256+i)>0.5f && v.gate && v.amp.stage==0;\n'
             '  pingPongEvent[(size_t)i]=pingPongAttack[(size_t)i]=ahdsrReattack;\n')
    assert dsp.count(cycle)==1, 'Unexpected Ping Pong AHDSR trigger wiring'
    dsp=dsp.replace(cycle,'',1)
    # TEST0.2.50: step events change stereo only; remove exact audited wiring
    # before comparing all unchanged synthesis equations to the old hash.
    new_wiring = [
      (' // At most one pan alternation per operator and sample: an AHDSR reattack\n'
       ' // may coincide with an arpeggiator grid edge.\n'
       ' std::array<bool,3> pingPongEvent{},pingPongAttack{};\n'),
      '   if(seq.lastTick)pingPongEvent[(size_t)i]=true;\n',
      '   if(event.edge && event.note>=0)pingPongEvent[(size_t)i]=true;\n',
      ('  if(pingPongEvent[(size_t)i]){\n'
       '   // The automatic AHDSR reattack is an Attack *on this sample*, not a\n'
       '   // continuous-pan modulation. Snap before the oscillator is mixed.\n'
       '   if(pingPongAttack[(size_t)i])v.pingPongPan.advanceAtAttack(param(259+i));\n'
       '   else v.pingPongPan.advance();\n'
       '  }\n'),
    ]
    for addition in new_wiring:
        assert dsp.count(addition)==1, 'Unexpected TEST0.2.50 arp pan DSP change: '+addition
        dsp=dsp.replace(addition,'\n' if addition.startswith('  if(pingPongEvent') else '',1)
    pan = ('  // Add a signed L/R alternating offset before the legacy pan clamp;\n'
           '  // old presets are bit-exact because Ping Pong Pan defaults to zero.\n'
           '  xfade[(size_t)i]=limit(param(19+i)+mod[(size_t)i]*param(49+i)+panMod[(size_t)i]\n'
           '                      +v.pingPongPan.tick(param(259+i)),-1.f,1.f);')
    original='  xfade[(size_t)i]=limit(param(19+i)+mod[(size_t)i]*param(49+i)+panMod[(size_t)i],-1.f,1.f);'
    assert dsp.count(pan)==1, 'Unexpected Ping Pong output pan wiring'
    dsp=dsp.replace(pan,original,1)
    # TEST0.2.48: only the three operator VCA calls gained an optional
    # gate-driven cycle. Normalize this exact block before the 0.2.47 audit;
    # any other modification still fails the historic DSP checksum.
    trigger_call = ('v.amp.tickTrigger(param(256+i)>0.5f,v.gate,\n'
                    '      opSharedParam(34,i),opSharedParam(249,i),opSharedParam(37,i),\n'
                    '      opSharedParam(40,i),opSharedParam(43,i),sampleRateHz)')
    previous_call = ('v.amp.tick(opSharedParam(34,i),opSharedParam(249,i),'
                     'opSharedParam(37,i),opSharedParam(40,i),'
                     'opSharedParam(43,i),sampleRateHz)')
    assert dsp.count(trigger_call)==1, 'Unreviewed TEST0.2.48 VCA DSP change'
    dsp=dsp.replace(trigger_call,previous_call,1)
    patches = {
      'v.amp.tick(opSharedParam(34,i),opSharedParam(249,i),opSharedParam(37,i),opSharedParam(40,i),opSharedParam(43,i),sampleRateHz)':
      'v.amp.tick(opSharedParam(34,i),opSharedParam(249,i),opSharedParam(37,i),opSharedParam(40,i),sampleRateHz)',
      'v.mod.tick(param(64+i),param(252+i),param(67+i),param(70+i),param(73+i),sampleRateHz)':
      'v.mod.tick(param(64+i),param(252+i),param(67+i),param(70+i),sampleRateHz)',
      'filterEnvelope.tick(param(118),param(255),param(119),param(120),param(121),sampleRateHz)':
      'filterEnvelope.tick(param(118),param(255),param(119),param(120),sampleRateHz)',
      '  const float pitch=arpBase+param(25+i)+param(28+i)/100.f+v.bend*param(91+i)\n'
      '    +mod[(size_t)i]*param(52+i)+pitchMod[(size_t)i]+v.arpPitch\n'
      '    +v.drift*param(13+i)*0.50f;':
      '  const float pitch=arpBase+param(25+i)+param(28+i)/100.f+v.bend*param(91+i)+v.drift\n'
      '    +mod[(size_t)i]*param(52+i)+pitchMod[(size_t)i]+v.arpPitch;',
      ' // Blend noise on the same sample the host changes its Character control,\n'
      ' // even at low oscillator frequency when neither LFSR clock advances.\n'
      ' const float noiseCharacter=limit(param(9),0.f,1.f);\n'
      ' const bool noiseCharacterChanged=noiseCharacter!=noiseCharacterLast;\n'
      ' noiseCharacterLast=noiseCharacter;\n': '',
      '    if(updated || (noiseCharacterChanged && sub==0))\n'
      '     v.noise=v.cleanNoiseValue*(1.f-noiseCharacter)+v.sidNoiseValue*noiseCharacter;':
      '    if(updated){\n'
      '     const float character=limit(param(9),0.f,1.f);\n'
      '     v.noise=v.cleanNoiseValue*(1.f-character)+v.sidNoiseValue*character;\n'
      '    }',
    }
    for new, legacy in patches.items():
        assert dsp.count(new)==1, 'Unreviewed live DSP alteration: '+new[:75]
        dsp=dsp.replace(new, legacy, 1)
    return dsp
