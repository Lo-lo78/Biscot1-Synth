#include "TrackerSongParameters.h"
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "PitchBendRouting.h"
#include "SidArpReleasePool.h"
#include <algorithm>
#include <cmath>
#include <utility>

namespace biscot {
namespace {
constexpr double pi=3.14159265358979323846;
float limit(float x,float low,float high){return juce::jlimit(low,high,x);}
int integer(float value){return juce::roundToInt(value);}
float gain(float db){return std::pow(10.f,db/20.f);}
float randomUnit(uint32_t& s){s^=s<<13;s^=s>>17;s^=s<<5;return (float)(s&0xffffffu)/8388607.5f-1.f;}
}
juce::AudioProcessorValueTreeState::ParameterLayout BiscotProcessor::createLayout(){
 juce::AudioProcessorValueTreeState::ParameterLayout layout;
 for(const auto& d:catalog){
  const auto pid=juce::ParameterID{id(d.index),1};const auto name=juce::String(d.name);
  if(d.choices && d.choices[0]){
   juce::StringArray options;options.addTokens(juce::String(d.choices),"|","");
   layout.add(std::make_unique<juce::AudioParameterChoice>(pid,name,options,integer(d.initial)));
  }else if(d.step>=1.f){
   layout.add(std::make_unique<juce::AudioParameterInt>(pid,name,integer(d.low),integer(d.high),integer(d.initial)));
  }else{
   auto range=juce::NormalisableRange<float>(d.low,d.high,d.step>0?d.step:0.001f);
   layout.add(std::make_unique<juce::AudioParameterFloat>(pid,name,range,d.initial));
  }
 }
 return layout;
}
BiscotProcessor::BiscotProcessor():AudioProcessor(BusesProperties().withOutput("Mix",juce::AudioChannelSet::stereo(),true)
 .withOutput("Op1",juce::AudioChannelSet::stereo(),false)
 .withOutput("Op2",juce::AudioChannelSet::stereo(),false)
 .withOutput("Op3",juce::AudioChannelSet::stereo(),false)
 .withOutput("Filter",juce::AudioChannelSet::stereo(),false)),
 state(*this,nullptr,"BISCOT1",createLayout()){
 for(const auto& d:catalog)parameters[(size_t)d.index]=state.getRawParameterValue(id(d.index));
 for(auto& a:trackerOverrideMask)a.store(false,std::memory_order_relaxed);
 trackerPlayable.store(std::make_shared<const TrackerPlayablePattern>(compileTrackerPattern(trackerPatterns.at(0))));
 trackerMidiMap.store(std::make_shared<const TrackerMidiMap>(compileTrackerMidiMap(trackerPatterns)));
 songPlayable.store(std::make_shared<const TrackerPlayableSong>(compileTrackerSong(trackerSongs.at(0),0,trackerPatterns)));
 songMidiMap.store(std::make_shared<const TrackerSongMidiMap>(compileTrackerSongMidiMap(trackerSongs,trackerPatterns)));
 songChainMap.store(std::make_shared<const TrackerSongChainMap>(compileTrackerSongChainMap(trackerSongs,trackerPatterns)));
}
bool BiscotProcessor::isBusesLayoutSupported(const BusesLayout& l)const{
 if(l.outputBuses.size()!=5 || !l.inputBuses.isEmpty())return false;
 for(int i=0;i<5;++i){const auto a=l.outputBuses[(size_t)i];
  if(i==0 && a!=juce::AudioChannelSet::stereo())return false;
  if(i>0 && a!=juce::AudioChannelSet::stereo() && !a.isDisabled())return false;
 }
 return true;
}
void BiscotProcessor::prepareToPlay(double sr,int){
 sampleRateHz=sr>0?sr:44100.;for(auto& fx:songFx)fx.prepare(sampleRateHz);songFxModeActive=false;voices={};lfos={};filters={};pressed={};pressedCounts={};arpPoolCounts={};nextPress=0;heldNotes.clear();filterKeyNote=60;
 for(auto& seq:sidSeq)seq.clearAll();
 for(auto& s:outputStages)s.prepare(sampleRateHz);
 waveCore.prepare();
 for(int i=0;i<3;++i){voices[(size_t)i].noiseSeed^=(uint32_t)(i*0x9e3779b9u);
  voices[(size_t)i].cleanNoiseSeed=1234567ULL+(uint64_t)i*1111111ULL;
  voices[(size_t)i].sidNoiseSeed=i==0?5930003ULL:i==1?3592873ULL:7281093ULL;
  lfos[(size_t)i].reset(i);}
 filterEnvelope={};pedalDown.fill(false);sustainedArp.clear();arpReleaseCounts.fill(0);lastPlaying=false;idleSleep.reset();arpPoolCache.invalidate();
 trackerTransport.stop();trackerPlaybackSnapshot.reset();trackerAudibleStep.store(-1);trackerPlaybackStatus.store(0);
 trackerRequested.store(0);trackerClearOverrides();
 trackerReleaseArpActive=false;trackerMidiTriggerNote=-1;
 songTransport.stop();songPlaybackSnapshot.reset();songChainSnapshot.reset();songPlayingNumber.store(-1);songRequested.store(0);songPlaybackStatus.store(0);songMidiTriggerNote=-1;
 songMixActive=false; songMixer={1.f,1.f,1.f};songFilterOwner=1;songFilterPreference=1;songFilterMask=7;songLfoOwners={1,1,1};songLfoPreferences={1,1,1};songActiveNotes.fill(-1);songActiveSubCount.fill(0);
 for(auto& x:songAudibleRows)x.store(-1);
}
// The JSFX has a separate 16-event held-note stack for each operator, plus
// separate global filter and chord stacks. The caller has already updated those
// stacks so wasHeld reflects physical MIDI gates, not the envelope release tail.
void BiscotProcessor::noteOn(int i,int note,int channel,bool wasHeld,bool pedalPhrase){
 idleSleep.wake();
 auto& v=voices[(size_t)i];
 const int mode=integer(param(3));
 const bool arpOn=integer(param(76+i))>0;
 const bool opLegato=param((mode==1)?10:10+i)>0.5f;
 const bool soundingAlready=wasHeld || v.pedalLatched || (arpOn && pedalPhrase);
 const bool retrigger=sidShouldRetrigger(arpOn,soundingAlready,opLegato);
 const float portamento=opSharedParam(46,i);
 // In ARP mode, adding a held key must not retune the oscillator immediately.
 if(!arpOn || !soundingAlready){
  if(v.note<0 || portamento<=0)v.noteSmooth=note;
  v.note=note;
 }
 v.channel=channel;v.gate=true;v.pedalLatched=false;
 v.vel=limit(v.vel,0.f,1.f);
 if(!arpOn || !soundingAlready)v.drift=randomUnit(v.noiseSeed)*param(13+i)*0.50f;
 if(retrigger){
  // First note of an arpeggio starts the ADSR once. Subsequent Trigger
  // notes restart only the arp SEQUENCE (scheduled at the next clock edge).
  const bool resetAmp=integer(param(4))>=2 || (!arpOn && !opLegato && v.amp.stage!=0);
  v.amp.noteOn(resetAmp);
  v.mod.noteOn(resetAmp);
  if(param(4)>=1){v.phase=0;v.oldPhase=0;}
  if(param(4)>=3){v.noiseSeed=0x12345678u ^ (uint32_t)(i*0x9e3779b9u);v.noise=0.f;}
  // JSFX restarts the arp clock only on the first key of a phrase.
  if(!soundingAlready){v.arpStep=0;v.arpPhase=0;v.arpPitch=0;
   v.quantizedArp.reset();sidSeq[(size_t)i].resetClock();}
 }
}
void BiscotProcessor::triggerFilterAndLfos(int note,bool retrigger,int channel){
 // In a Song, three independent note streams share one physical filter and
 // three physical LFOs. Only each resource's owning lane may retrigger it.
 const bool songContext=songTransport.running && songPlaybackSnapshot!=nullptr;
 if(!songContext || songFilterOwner==channel){
  filterKeyNote=note;
  if(retrigger)filterEnvelope.noteOn(false);
 }
 if(!retrigger)return;
 for(int j=0;j<3;++j){
  if(songContext && songLfoOwners[(size_t)j]!=channel)continue;
  auto& l=lfos[(size_t)j];
  l.noteTrigger(param(122+j*23)>0.5f,param(129+j*23)>0.5f,param(131+j*23));
 }
}
void BiscotProcessor::noteOff(int channel,int note){
 // In Chord Allocate SID Seq uses the current live chord root.  If the last
 // physically held key is released under CC64, keep that exact root for the
 // sustained phrase and subsequent release.  Switching back to the older
 // per-voice latch root would cause an instantaneous pitch discontinuity.
 const int midiMode=integer(param(3));
 const int lastLiveChordRoot=heldNotes.chord.top();
 const auto result=heldNotes.noteOff(midiMode,channel,note);
 if(result.filterTop>=0)filterKeyNote=result.filterTop;
 for(int i=0;i<3;++i){
  if(!result.affected[(size_t)i])continue;
  auto& v=voices[(size_t)i];const int next=result.voiceTop[(size_t)i];
  if(next>=0){
   v.gate=true;
   // Return to the prior held note without restarting the VCA or Mod ENV.
   // While an arpeggio is active, retain its pitch base until the arp engine
   // determines the next step (as in the JSFX release latch logic).
   if(integer(param(76+i))<=0)v.note=next;
  }else{
   // MIDI CC64 is only active when this operator's Sustain Pedal is On.
   // The envelope continues normally while the pedal holds its last note;
   // no oscillator mute or artificial infinite/zero release is introduced.
   const bool sustainEnabled=opPedalEnabled(i);
   if(sustainEnabled && (pedalDown[(size_t)(channel-1)] || sustainedArp.any(i))){
    // An earlier pedal-held key on another MIDI channel may still be in
    // this mono operator's sustain pool.
    v.gate=true;v.pedalLatched=true;v.pedalChannel=channel;
   }else{
    v.gate=false;v.pedalLatched=false;
    if(sidSeq[(size_t)i].latch.count>0)
     sidSeq[(size_t)i].releaseRoot=sidSeq[(size_t)i].latchRoot;
    v.amp.noteOff(opSharedParam(43,i),sampleRateHz);
    v.mod.noteOff(param(73+i),sampleRateHz);
   }
  }
 }
 if(midiMode==1 && heldNotes.chord.empty() && lastLiveChordRoot>=0){
  for(int i=0;i<3;++i){
   if(voices[(size_t)i].pedalLatched && integer(param(76+i))==1)
    sidSeq[(size_t)i].latchCurrentChordRootForRelease(lastLiveChordRoot);
  }
 }
 if(result.filterReleased){
  bool pedalHolding=false;
  for(const auto& v:voices)pedalHolding|=v.pedalLatched;
  if(!pedalHolding)filterEnvelope.noteOff(param(121),sampleRateHz);
 }
}
// Called on CC64 release, also at block start when a user disables a pedal
// switch while a note is latched. It does not interfere with physical notes.
void BiscotProcessor::releasePedalLatches(){
 bool wasHolding=false;
 for(int i=0;i<3;++i){
  auto& v=voices[(size_t)i];
  if(!v.pedalLatched)continue;
  const bool enabled=opPedalEnabled(i);
  // A pedal on another MIDI channel may still be holding material for
  // this operator. Releasing one channel must not cut the other channel.
  if(enabled && (pedalDown[(size_t)(v.pedalChannel-1)] || sustainedArp.any(i)))continue;
  wasHolding=true;v.pedalLatched=false;
  if(heldNotes.voices[(size_t)i].empty()){
   v.gate=false;
   // No change to SID Seq releaseRoot here. CC64-up is an ADSR gate
   // transition, NOT a note event: re-basing the sequencer here would
   // cause a small pitch jump/click unique to pedal release.
   v.amp.noteOff(opSharedParam(43,i),sampleRateHz);
   v.mod.noteOff(param(73+i),sampleRateHz);
  }
 }
 if(wasHolding && heldNotes.filter.empty()){
  bool anyLatched=false;
  for(const auto& v:voices)anyLatched|=v.pedalLatched;
  if(!anyLatched)filterEnvelope.noteOff(param(121),sampleRateHz);
 }
}
SidArpPoolSettings BiscotProcessor::currentArpPoolSettings() const noexcept {
 SidArpPoolSettings settings;
 settings.midiMode=integer(param(3));
 for(int i=0;i<3;++i){
  settings.arpMode[(size_t)i]=integer(param(76+i));
  settings.octaveUp[(size_t)i]=integer(param(82+i));
  settings.octaveDown[(size_t)i]=integer(param(85+i));
  settings.pedalEnabled[(size_t)i]=opPedalEnabled(i);
 }
 return settings;
}
void BiscotProcessor::refreshArpPoolsIfNeeded(){
 // APVTS automation of MIDI mode, octaves, arp mode and pedal enable must
 // still take effect at the next block, without rebuilding on every block.
 const auto settings=currentArpPoolSettings();
 if(arpPoolCache.needsRefresh(settings))rebuildArpPools(true);
}
void BiscotProcessor::rebuildArpPools(bool captureRelease){
 const auto settings=currentArpPoolSettings();
 const int mode=settings.midiMode;
 for(int i=0;i<3;++i){
  // SID Seq (mode 1) has its own 128-step memory. With Arp Off (mode 0)
  // this pool is not read either. Preserve any previously latched release
  // pool; an automated mode change will cause a fresh rebuild next block.
  if(settings.arpMode[(size_t)i]<=1){arpPoolCounts[(size_t)i]=0;continue;}
  auto& pool=arpPools[(size_t)i];int count=0;
  std::array<std::pair<std::uint64_t,int>,640> ordered{};
  for(int ch=0;ch<16;++ch){
   if(mode==2 && ch!=i)continue;
   for(int note=0;note<128;++note){
    const auto sequence=std::max(pressed[(size_t)ch][(size_t)note],
      settings.pedalEnabled[(size_t)i] && pedalDown[(size_t)ch]
       ? sustainedArp.timestamp(i,ch+1,note) : std::uint64_t{0});
    if(sequence==0)continue;
    const int low=settings.octaveDown[(size_t)i],high=settings.octaveUp[(size_t)i];
    for(int oct=low;oct<=high && count<(int)pool.size();++oct){
     const int n=note+oct*12;
     if(n>=0 && n<=127)ordered[(size_t)count++]={sequence,n};
    }
   }
  }
  const int arpMode=settings.arpMode[(size_t)i];
  if(arpMode==5 || arpMode==1){
   std::sort(ordered.begin(),ordered.begin()+count,[](const auto& a,const auto& b){
    return a.first==b.first?a.second<b.second:a.first<b.first;
   });
  }else{
   std::sort(ordered.begin(),ordered.begin()+count,[](const auto& a,const auto& b){return a.second<b.second;});
  }
  for(int n=0;n<count;++n)pool[(size_t)n]=ordered[(size_t)n].second;
  arpPoolCounts[(size_t)i]=count;
  sidLatchArpReleasePool(pool,count,arpReleasePools[(size_t)i],
                         arpReleaseCounts[(size_t)i],captureRelease);
 }
 arpPoolCache.didRefresh(settings);
}
void BiscotProcessor::onMidi(const juce::MidiMessage& m){
 const int ch=m.getChannel(), mode=integer(param(3));
 if(m.isAllNotesOff()||m.isAllSoundOff()){
  for(auto& row:pressed)row.fill(0);
  for(auto& row:pressedCounts)row.fill(0);
  heldNotes.clear();pedalDown.fill(false);sustainedArp.clear();arpReleaseCounts.fill(0);
  for(auto& seq:sidSeq)seq.clearAll();
  rebuildArpPools();
  for(int i=0;i<3;++i){auto& v=voices[(size_t)i];v.gate=false;v.pedalLatched=false;
   v.amp.noteOff(opSharedParam(43,i),sampleRateHz);v.mod.noteOff(param(73+i),sampleRateHz);}
  filterEnvelope.noteOff(param(121),sampleRateHz);return;
 }
 if(m.isPitchWheel()){
  const float bend=limit(((float)m.getPitchWheelValue()-8192.f)/8192.f,-1.f,1.f);
  // Multi (default): keep the original channels 1-3 bend routing.
  // Omni: a pitch wheel from ANY MIDI channel bends all three SID voices.
  // Each voice still uses its own Op1/2/3 Pitch Bend range in renderOne().
  routePitchBend(ch,bend,mode,integer(param(226)),
                 [this](int voice,float amount){voices[(size_t)voice].bend=amount;});
  return;
 }
 if(m.isController()){
  if(m.getControllerNumber()==64){
   const bool down=m.getControllerValue()>=64;
   // The full release chord was latched during Note On. CC64-up changes
   // the amplitude gate only; never replace its note memory here.
   pedalDown[(size_t)(ch-1)]=down;
   if(!down)sustainedArp.clearChannel(ch);
   releasePedalLatches(); // same ADSR noteOff transition as a manual key release
   rebuildArpPools();
  }
  if(m.getControllerNumber()==123)onMidi(juce::MidiMessage::allNotesOff(ch));
  return;
 }
 if(m.isNoteOn()){
  const int note=m.getNoteNumber();
  auto& count=pressedCounts[(size_t)(ch-1)][(size_t)note];
  if(count<0xffffffffu)++count;
  pressed[(size_t)(ch-1)][(size_t)note]=++nextPress;
  const bool chordWasHeld=!heldNotes.chord.empty();
  std::array<bool,3> clockRunningBefore{};
  for(int i=0;i<3;++i){
   const auto& v=voices[(size_t)i];
   clockRunningBefore[(size_t)i]=v.gate &&
      (integer(param(76+i))==1 ? sidSeq[(size_t)i].step>=0 : v.quantizedArp.started);
  }
  std::array<bool,3> arpPhraseActive{};
  for(int i=0;i<3;++i)
   arpPhraseActive[(size_t)i]=sustainedArp.any(i) && opPedalEnabled(i);
  const bool filterWasPedalLatched=std::any_of(voices.begin(),voices.end(),
      [](const Voice& v){return v.pedalLatched;});
  const bool filterLegato=param(10)>0.5f;
  const auto event=heldNotes.noteOn(mode,ch,note,filterLegato);
  // Chord Allocate feeds every enabled SID Seq with the entire chord,
  // not just the note allocated to that operator (JSFX chord_note_on).
  if(mode==1){
   for(int i=0;i<3;++i)if(integer(param(76+i))==1){
    auto& seq=sidSeq[(size_t)i];
    if(!chordWasHeld && !arpPhraseActive[(size_t)i]){
     seq.clearSequence(note);seq.releaseRoot=note;
    }else seq.record(note); // continue the held CC64 phrase
   }
  }
  // All Operators / MIDI Channels: each operator owns its SID Seq and latch.
  for(int i=0;i<3;++i)if(event.trigger[(size_t)i]){
   auto& seq=sidSeq[(size_t)i];
   seq.snapshot(heldNotes.voices[(size_t)i]);
   if(mode!=1 && !event.wasHeld[(size_t)i] && !arpPhraseActive[(size_t)i])
    seq.releaseRoot=seq.latchRoot;
   if(mode!=1 && integer(param(76+i))==1){
    if(!event.wasHeld[(size_t)i] && !arpPhraseActive[(size_t)i])seq.clearSequence(note);
    else seq.record(note);
   }
  }
  // Sustain pedal counts as an uninterrupted phrase for filter/LFO Legato.
  // Arpeggio Trigger is musical transport only: no VCA reattack per step.
  bool onlyContinuingArps=true,anyRouted=false;
  for(int i=0;i<3;++i)if(event.trigger[(size_t)i]){
   anyRouted=true;
   const bool continuing=event.wasHeld[(size_t)i] || voices[(size_t)i].pedalLatched ||
                         arpPhraseActive[(size_t)i];
   if(integer(param(76+i))==0 || !continuing)onlyContinuingArps=false;
  }
  const bool suppressArpKeyTrigger=anyRouted && onlyContinuingArps;
  // The cutoff key-follow base tracks EVERY physical Note On, independently
  // of whether the current arp phrase suppresses an envelope retrigger.
  triggerFilterAndLfos(note,sidFilterLfoShouldTrigger(event.filterRetrigger,
                        filterLegato,filterWasPedalLatched,suppressArpKeyTrigger),ch);
  const float velocity=m.getFloatVelocity();
  for(int i=0;i<3;++i)if(event.trigger[(size_t)i]){
   voices[(size_t)i].vel=velocity;
   noteOn(i,note,ch,event.wasHeld[(size_t)i],arpPhraseActive[(size_t)i]);
  }
  // In Chord Allocate all enabled arpeggiators use the common chord material,
  // even when this Note On was assigned to only one of three oscillators.
  // Trigger starts its pattern at step 0 on the NEXT grid edge. Legato
  // preserves the existing step. Neither mode reattacks the envelope.
  for(int i=0;i<3;++i){
   const bool routed=(mode==1) || event.trigger[(size_t)i];
   const bool triggerMode=opSharedParam(10,i)<0.5f;
   if(integer(param(76+i))>0 &&
      sidArpRestartRequested(triggerMode,clockRunningBefore[(size_t)i],routed)){
    if(integer(param(76+i))==1)sidSeq[(size_t)i].requestRestart();
    else voices[(size_t)i].quantizedArp.requestRestart();
   }
  }
  rebuildArpPools(true);return;
 }
 if(m.isNoteOff()){
  const int note=m.getNoteNumber();
  auto& count=pressedCounts[(size_t)(ch-1)][(size_t)note];
  if(count>0)--count;
  if(count==0){
   const auto order=pressed[(size_t)(ch-1)][(size_t)note];
   if(order!=0 && pedalDown[(size_t)(ch-1)]){
    for(int i=0;i<3;++i){
     if(midiAddressesOperator(mode,i,ch) && opPedalEnabled(i))
      sustainedArp.hold(i,ch,note,order);
    }
   }
   pressed[(size_t)(ch-1)][(size_t)note]=0;
  }
  noteOff(ch,note);
  rebuildArpPools();return;
 }
}
float BiscotProcessor::polyBlep(double phase,double dt){
 dt=std::max(1.e-8,dt);
 if(phase<dt){const double p=phase/dt;return (float)(p+p-p*p-1.);}
 if(phase>1.-dt){const double p=(phase-1.)/dt;return (float)(p*p+p+p+1.);}
 return 0.f;
}
float BiscotProcessor::pulse(double phase,float pw,double dt)const{
 // Ends remain natural constant-level waveforms; DC blocker handles their bias.
 float out=phase<(double)pw?1.f:-1.f;
 if(pw>0.f && pw<1.f){
  out+=polyBlep(phase,dt);
  double p2=phase-pw;if(p2<0.)p2+=1.;
  out-=polyBlep(p2,dt);
 }
 return out;
}
float BiscotProcessor::saturate(float x){return x/(1.f+std::abs(x)*0.6f);}
float BiscotProcessor::processLfo(int i,const std::array<float,3>& prev){
 auto& l=lfos[(size_t)i];const int b=122+i*23;
 double oct=0.;
 for(int k=0;k<3;++k)if(integer(param(58+k))==i+1)
  oct+=voices[(size_t)k].mod.level*param(61+k)*4.;
 const int source=integer(param(220+i*2));
 if(source>0 && source<=3)oct+=prev[(size_t)(source-1)]*param(221+i*2)*4.;
 const double hz=std::max(0.001,tempoBpm)/240. *
   std::max(0.125f,param(b+1))*std::pow(2.,juce::jlimit(-8.,8.,oct));
 const double inc=hz/sampleRateHz;
 return l.tick(inc,integer(param(b+2)),param(b+5),param(b+6),
               param(b+7)>0.5f,param(b+8),param(b+3),param(b+4),sampleRateHz,param(231+i));
}
float BiscotProcessor::processFilter(float input,float registerCutoff,int channel,float resonanceMod){
 return (float)filters[(size_t)channel].process(input,registerCutoff,
   param(109)+resonanceMod,sampleRateHz,param(5)>0.5f,
   param(110)>0.5f,param(111)>0.5f,param(112)>0.5f);
}
void BiscotProcessor::renderOne(float& left,float& right,std::array<float,8>& stems){
 // All parameter groups are used by this intermediate DSP. Exact JSFX parity,
 // JSFX-derived wavetable and filter core retained; SID Seq uses a native clocked memory.
 std::array<float,3> amp{},mod{},lfo{},previous{},pitchMod{},pwmMod{},levelMod{},panMod{},oscOut{};
 float cutoffMod=0,resMod=0;
 for(int i=0;i<3;++i){auto& v=voices[(size_t)i];
  const bool wasActive=v.amp.stage!=0;
  amp[(size_t)i]=v.amp.tick(opSharedParam(34,i),opSharedParam(37,i),opSharedParam(40,i),sampleRateHz);
  mod[(size_t)i]=v.mod.tick(param(64+i),param(67+i),param(70+i),sampleRateHz);
  // JSFX retains SID Seq during Release and clears it only after the VCA
  // has completed its release tail (not immediately on Note Off).
  if(wasActive && v.amp.stage==0 && !v.gate){
   sidSeq[(size_t)i].clearAll();
   arpReleaseCounts[(size_t)i]=0;
   v.note=-1;v.arpPitch=0.f;v.arpPhase=0.;v.arpStep=0;v.quantizedArp.reset();
  }
  v.noteSmooth=v.noteSmooth<0 && v.note>=0?v.note:v.noteSmooth;
 }
 const float filterEnv=filterEnvelope.tick(param(118),param(119),param(120),sampleRateHz);
 for(int i=0;i<3;++i)previous[(size_t)i]=lfoUsedCached[(size_t)i]?lfos[(size_t)i].previous:0.f;
 for(int i=0;i<3;++i)lfo[(size_t)i]=lfoUsedCached[(size_t)i]?processLfo(i,previous):0.f;
 for(int i=0;i<3;++i)lfos[(size_t)i].previous=lfo[(size_t)i];
 for(int j=0;j<3;++j){const int base=122+j*23;
  cutoffMod+=lfo[(size_t)j]*param(base+10)*2047.f;
  for(int i=0;i<3;++i){
   pitchMod[(size_t)i]+=lfo[(size_t)j]*param(base+11+i);
   pwmMod[(size_t)i]+=lfo[(size_t)j]*param(base+14+i)*0.5f;
   levelMod[(size_t)i]+=lfo[(size_t)j]*param(base+17+i);
   panMod[(size_t)i]+=lfo[(size_t)j]*param(base+20+i);
  }
 }
 const auto route=[&](int target,float amount){
  switch(target){
   case 1:cutoffMod+=amount*2047.f;break;
   case 2:pitchMod[0]+=amount*24.f;break;
   case 3:pitchMod[1]+=amount*24.f;break;
   case 4:pwmMod[0]+=amount*0.5f;break;
   case 5:pwmMod[1]+=amount*0.5f;break;
   case 6:pitchMod[2]+=amount*24.f;break;
   case 7:pwmMod[2]+=amount*0.5f;break;
   case 8:levelMod[0]+=amount;break;
   case 9:levelMod[1]+=amount;break;
   case 10:levelMod[2]+=amount;break;
   case 11:resMod+=amount*15.f;break;
  }
 };
 for(int i=0;i<3;++i){
  const int oscTarget=i==0?203:i==1?205:104;
  const int envTarget=i==0?207:i==1?209:106;
  route(integer(param(oscTarget)),voices[(size_t)i].oscPrev*param(oscTarget+1));
  route(integer(param(envTarget)),(amp[(size_t)i]*2.f-1.f)*param(envTarget+1));
 }
 for(int i=0;i<3;++i){const int base=211+i*3,src=integer(param(base));
  if(src>=1 && src<=3)route(integer(param(base+1)),lfo[(size_t)(src-1)]*param(base+2));
 }
 // Extended Op2/Op3 event-linked envelope modulation.
 std::array<float,3> xfade{};
 const auto gateSource=[&](int src,int op)->float{
  if(src==0)return 0.f;
  if(src>=1&&src<=3)return 0.5f*(lfo[(size_t)(src-1)]+1.f);
  return mod[(size_t)op];
 };
 std::array<bool,3> ring{},sync{};
 for(int i=0;i<3;++i){
  const int ringSrc=integer(param(191+i)),syncSrc=integer(param(197+i));
  ring[(size_t)i]=ringSrc==0?param(94+i)>0.5f:gateSource(ringSrc,i)>=param(194+i)*0.01f;
  sync[(size_t)i]=syncSrc==0?param(97+i)>0.5f:gateSource(syncSrc,i)>=param(200+i)*0.01f;
 }
 // SID Seq is NOT an ordering of live notes. It is a 128-position
 // clock-inserted frequency memory independent for each operator. Other
 // arpeggio modes retain their existing routing and playback implementation.
 for(int i=0;i<3;++i){
  auto& v=voices[(size_t)i];const int arpMode=integer(param(76+i));
  if(arpMode==1){
   v.quantizedArp.reset(); // SID Seq has its own independent clock and memory
   auto& seq=sidSeq[(size_t)i];
   const bool chord=integer(param(3))==1;
   // JSFX parity: the SID Seq keeps changing frequency through the VCA
   // Release. The last recorded sequence/root remain latched even after
   // final Note Off or CC64-up; the clock sleeps only after ADSR idle.
   if(sidArpClockActive(v.gate,v.amp.stage) && seq.length>0){
    const int root=chord && heldNotes.chord.count>0
       ? heldNotes.chord.top() : seq.releaseRoot;
    seq.process(1,root,param(79+i),tempoBpm,sampleRateHz);
   }else seq.lastTick=false;
   const float target=seq.target;
   const float glide=std::max(0.f,param(88+i));
   const float coefficient=glide>0.f?1.f/(1.f+glide*glide*2000.f):1.f;
   v.arpPitch+=(target-v.arpPitch)*coefficient;
   if(std::abs(target-v.arpPitch)<0.00001f)v.arpPitch=target;
  }else if(arpMode>1 && v.note>=0 && sidArpClockActive(v.gate,v.amp.stage)){
   const bool live=arpPoolCounts[(size_t)i]>0;
   const auto& pool=live?arpPools[(size_t)i]:arpReleasePools[(size_t)i];
   const int count=live?arpPoolCounts[(size_t)i]:
      (v.amp.stage!=0 ? arpReleaseCounts[(size_t)i] : 0);
   const auto event=v.quantizedArp.process(pool,count,arpMode,param(79+i),
                                           tempoBpm,sampleRateHz,v.noiseSeed);
   if(event.note>=0){
    const float destination=(float)event.note-(float)v.note;
    const float glide=param(88+i);
    v.arpPitch+=(destination-v.arpPitch)*(glide<=0.f?1.f:limit(1.f/(float)(sampleRateHz*(0.002+glide*0.3)),0.0001f,1.f));
   }
  }else if(arpMode<=0){
   v.quantizedArp.reset();v.arpPitch=0;
  } // Through Release, keep the clocked frequency arp; never retrigger the ADSR.

 }
 std::array<double,3> frequencies{};
 for(int i=0;i<3;++i){auto& v=voices[(size_t)i];
  if(v.note<0 && v.amp.stage==0 && v.mod.stage==0){frequencies[(size_t)i]=0;continue;}
  if(v.note>=0){const float port=opSharedParam(46,i);
   const double smoothing=port<=0?1.:1.-std::exp(-1./(sampleRateHz*port));
   v.noteSmooth+=(v.note-v.noteSmooth)*smoothing;
  }
  float arpBase=(float)v.noteSmooth;
  if(integer(param(76+i))==1 && sidSeq[(size_t)i].latch.count>0){
   // In Chord Allocate the root is the top of the live chord while keys
   // are held. Other modes use the first note latched for this phrase.
   if(integer(param(3))==1 && !heldNotes.chord.empty())
    arpBase=(float)heldNotes.chord.top();
   else arpBase=(float)sidSeq[(size_t)i].releaseRoot;
  }
  const float pitch=arpBase+param(25+i)+param(28+i)/100.f+v.bend*param(91+i)+v.drift
    +mod[(size_t)i]*param(52+i)+pitchMod[(size_t)i]+v.arpPitch;
  frequencies[(size_t)i]=std::min(sampleRateHz*0.45,440.*std::pow(2.,juce::jlimit(-130.,130.,(double)(pitch-69.)/12.)));
 }
 // Advance all three SID phase accumulators together. This follows the JSFX
 // sub-sample order, including the source-oscillator MSB edges for Ring/Sync.
 const bool hot=ring[0]||ring[1]||ring[2]||sync[0]||sync[1]||sync[2];
 const double maxInc=std::max({frequencies[0],frequencies[1],frequencies[2]})/sampleRateHz;
 int oversample=1;
 if(hot){oversample=2;if(maxInc>0.032)oversample=4;
         if(maxInc>0.065)oversample=8;if(maxInc>0.105)oversample=16;}
 std::array<double,3> increments{frequencies[0]/sampleRateHz,frequencies[1]/sampleRateHz,
                                 frequencies[2]/sampleRateHz};
 std::array<double,3> widths{};
 for(int i=0;i<3;++i)widths[(size_t)i]=limit(
     0.5f+opSharedParam(31,i)/128.f+pwmMod[(size_t)i]+mod[(size_t)i]*param(55+i),0.f,1.f);
 std::array<double,3> accumulated{};
 for(int sub=0;sub<oversample;++sub){
  std::array<bool,3> msb{};
  for(int i=0;i<3;++i){
   auto& v=voices[(size_t)i];
   const int next=(i+1)%3;
   const bool phaseActive=v.note>=0||v.amp.stage!=0||
       ((ring[(size_t)next]||sync[(size_t)next]) && voices[(size_t)next].note>=0);
   v.oldPhase=v.phase;
   if(param(100+i)>0.5f){v.phase=0.;continue;}
   if(phaseActive){
    v.phase+=increments[(size_t)i]/oversample;
    if(v.phase>=1.)v.phase-=std::floor(v.phase);
    v.noiseClock+=increments[(size_t)i]*4./oversample;
    while(v.noiseClock>=1.){
     v.noiseClock-=1.;
     v.cleanNoiseSeed=(v.cleanNoiseSeed*1103515245ULL+12345ULL)%2147483647ULL;
     const double clean=(double)v.cleanNoiseSeed/1073741823.5-1.;
     const float character=limit(param(9),0.f,1.f);
     if(character<=0.f)v.noise=(float)clean;
     else{
      uint32_t sid=(uint32_t)(v.sidNoiseSeed%8388608ULL);
      if(sid==0)sid=8388607U;
      const uint32_t feedback=((sid>>17)^(sid>>22))&1U;
      sid=((sid<<1)&0x7fffffU)|feedback;
      v.sidNoiseSeed=sid;
      const int dac=((sid>>22)&1U)*128+((sid>>20)&1U)*64+
        ((sid>>16)&1U)*32+((sid>>13)&1U)*16+
        ((sid>>11)&1U)*8+((sid>>7)&1U)*4+
        ((sid>>4)&1U)*2+((sid>>2)&1U);
      const double coloured=dac/127.5-1.;
      v.noise=(float)(clean*(1.-character)+coloured*character);
     }
    }
   }
   msb[(size_t)i]=(v.oldPhase<0.5 && v.phase>=0.5);
  }
  for(int i=0;i<3;++i)if(sync[(size_t)i] && msb[(size_t)((i+2)%3)])
    voices[(size_t)i].phase=0.;
  for(int i=0;i<3;++i){
   const auto& v=voices[(size_t)i];
   if((v.note<0 && v.amp.stage==0) || param(100+i)>0.5f)continue;
   const int source=(i+2)%3;
   accumulated[(size_t)i]+=waveCore.render(integer(param(22+i)),v.phase,
      increments[(size_t)i]/oversample,widths[(size_t)i],v.noise,
      param(5)>0.5f,ring[(size_t)i],sync[(size_t)i],
      voices[(size_t)source].phase,increments[(size_t)source]/oversample);
  }
 }
 for(int i=0;i<3;++i){
  auto& v=voices[(size_t)i];
  double wave=accumulated[(size_t)i]/oversample;
  if(hot && (v.note>=0||v.amp.stage!=0)){
   // Exact JSFX aa_energy_comp() coefficient for Ring/Sync / BLEP path.
   const double x=std::min(1.,std::max(0.,increments[(size_t)i]*14.));
   const int shape=integer(param(22+i));
   double bonus=(shape==1||shape==2||shape==6||shape==7)?0.16:0.;
   if(ring[(size_t)i])bonus+=0.28;
   if(sync[(size_t)i])bonus+=0.14;
   double compensation=1.+x*(0.16+bonus);
   if(oversample>=8)compensation*=1.035;
   if(oversample>=16)compensation*=1.025;
   wave*=std::min(1.85,compensation);
  }
  // JSFX uses oscillator * envelope as the previous-sample cross-mod source.
  v.oscPrev=(float)wave*amp[(size_t)i];
  const float level=limit(opSharedParam(16,i)*(1.f+levelMod[(size_t)i]),0.f,2.f);
  oscOut[(size_t)i]=(float)wave*amp[(size_t)i]*level;
  if(songMixActive)oscOut[(size_t)i]*=songMixer[(size_t)i];
  if(songFxModeActive && songFx[(size_t)i].sidechainSource>0){
   const int src=songFx[(size_t)i].sidechainSource-1;
   oscOut[(size_t)i]*=songFx[(size_t)i].duckGain(amp[(size_t)src]);
  }
  xfade[(size_t)i]=limit(param(19+i)+mod[(size_t)i]*param(49+i)+panMod[(size_t)i],-1.f,1.f);
 }
 float filterInput[2]{},dry[2]{};
 const float filterInputGain=gain(param(6));
 for(int i=0;i<3;++i){
  const float x=oscOut[(size_t)i];
  // JSFX center pan is unity in BOTH channels. Previous equal-power pan
  // attenuated every centered voice by -3dB and changed the filter drive.
  const float pan=xfade[(size_t)i];
  float l=x*(1.f-std::max(0.f,pan));
  float r=x*(1.f+std::min(0.f,pan));
  if(songFxModeActive)songFx[(size_t)i].process(l,r,l,r,tempoBpm);
  if(param(114+i)>0.5f){filterInput[0]+=l*filterInputGain;filterInput[1]+=r*filterInputGain;}
  else if(i!=2 || param(103)<0.5f){
   dry[0]+=l;dry[1]+=r;
   stems[(size_t)i*2]=l;stems[(size_t)i*2+1]=r;
  }
 }
 const float keyFollow=(filterKeyNote-60.f)*param(113)*32.f;
 const float filterRegister=limit(param(108)+filterEnv*param(117)+cutoffMod+keyFollow,0.f,2047.f);
 const float filterGain=gain(param(7));
 const double masterGain=gain(param(8));
 const bool model6581=param(5)>0.5f;
 // JSFX 2.38 normalizes three automatically allocated chord voices, but not
 // single-note or separately addressed MIDI-channel modes.
 const double normTotal=0.0001+(integer(param(3))==1?3.0:1.0);
 const bool multichannel=param(2)>0.5f;
 for(int ch=0;ch<2;++ch){
  const float filtered=processFilter(filterInput[ch],filterRegister,ch,resMod)*filterGain;
  double mix=(dry[ch]*(model6581?1.122018454:1.0)+filtered)/normTotal;
  // The JSFX post-mix 6581 parallel rounding precedes the analogue output amp.
  if(model6581){
   const double dirty=SidOutputStage::softclip(mix*1.006)/1.006;
   mix=mix*0.88+dirty*0.12;
  }
  const float out=(float)(outputStages[(size_t)ch].process(mix,model6581)*masterGain);
  if(ch==0)left=out;else right=out;
  if(multichannel){
   // Same independent analogue stages as JSFX v2.38 multichannel mode.
   for(int i=0;i<3;++i){
    const int slot=2+2*i+ch;
    const double stem=stems[(size_t)(2*i+ch)]/normTotal;
    stems[(size_t)(2*i+ch)]=(float)(outputStages[(size_t)slot].process(stem,model6581)*masterGain);
   }
   const double filterStem=filtered/normTotal;
   stems[(size_t)(6+ch)]=(float)(outputStages[(size_t)(8+ch)].process(filterStem,model6581)*masterGain);
  }
 }
}
void BiscotProcessor::advanceIdleLfos(int samples) noexcept {
 for(int i=0;i<3;++i){
  const int b=122+i*23;
  SidIdleSleep::advanceFreeClock(lfos[(size_t)i],samples,tempoBpm,param(b+1),
       sampleRateHz,param(b)<0.5f,param(b+7)>0.5f);
 }
}
void BiscotProcessor::trackerSetOverride(int index,float value) noexcept {
 if(index<0 || index>=(int)trackerOverrideValues.size())return;
 trackerOverrideValues[(size_t)index].store(value,std::memory_order_relaxed);
 trackerOverrideMask[(size_t)index].store(true,std::memory_order_relaxed);
 trackerOverridesEnabled.store(true,std::memory_order_relaxed);
}
void BiscotProcessor::trackerClearOverrides() noexcept {
 trackerOverridesEnabled.store(false,std::memory_order_relaxed);
 for(auto& a:trackerOverrideMask)a.store(false,std::memory_order_relaxed);
}
void BiscotProcessor::requestTrackerStart(int step,bool audition) noexcept {
 trackerRequested.store((audition?2049:1)+juce::jlimit(0,1023,step),std::memory_order_release);
}
void BiscotProcessor::requestTrackerPause() noexcept {trackerRequested.store(-2,std::memory_order_release);}
void BiscotProcessor::requestTrackerStop() noexcept {trackerRequested.store(-1,std::memory_order_release);}
void BiscotProcessor::trackerReleaseNote() noexcept {
 for(int i=trackerTransport.activeSubCount-1;i>=0;--i)
  onMidi(juce::MidiMessage::noteOff(1,trackerTransport.activeSubNotes[(size_t)i]));
 trackerTransport.activeSubCount=0;
 if(trackerTransport.activeNote>=0){
  onMidi(juce::MidiMessage::noteOff(1,trackerTransport.activeNote));
  trackerTransport.activeNote=-1;
 }
}
void BiscotProcessor::trackerApplyStep(const TrackerPlayableRow& row,int transpose,double samples){
 // Lua stores ordered commands: OP, then Filter, then LFO. These dynamic
 // overrides are private to the audio thread, never written into APVTS.
 for(const auto& a:row.automation){
  // A carried Curve must resume at its correct fractional value when a
  // Pattern starts in the middle (Space from the editing cursor).
  trackerSetOverride(a.synthIndex,(float)trackerCurveAt(a,trackerTransport.currentStep,0.0));
 }
 // Preview Pattern's generic OP maps to Op1, as in the old Tracker JSFX.
 // Don't change the user's saved MIDI Mode or the sound outside playback.
 trackerSetOverride(3,2.f);
 if(row.pause){trackerReleaseNote();return;}
 if(row.note>=0){
  trackerReleaseNote();
  const int note=juce::jlimit(0,127,row.note+transpose);
  onMidi(juce::MidiMessage::noteOn(1,note,static_cast<uint8_t>(100)));
  trackerTransport.activeNote=note;trackerTransport.lastNote=note;
  // The JSFX adds Sub Notes to the SAME operator arp pool without reattacking
  // the envelope (arp mode is required in this step).
  for(int sub:row.subNotes){
   const int n=juce::jlimit(0,127,sub+transpose);
   onMidi(juce::MidiMessage::noteOn(1,n,static_cast<uint8_t>(100)));
   trackerTransport.activeSubNotes[(size_t)trackerTransport.activeSubCount++]=n;
  }
 }
 // A blank step must not rearm the preceding note's gate. The original
 // JSFX only schedules a gate for Note, or an explicitly written Gate cell.
 if(trackerTransport.activeNote>=0 && (row.note>=0 || row.gateExplicit))
  trackerTransport.gateFor(samples*row.gate,row.holdIntoNext);
 // A newly reached step may activate an LFO that was previously unused.
 lfoUsedCached=determineUsedLfos([this](int index){return param(index);});
}
// Launch note mappings without touching the message-thread editable bank.
// A mapped Note Off ends the performance; the trigger itself never sounds.
bool BiscotProcessor::handleTrackerExternalMidi(const juce::MidiMessage& m) noexcept {
 if(trackerStepRecording.load(std::memory_order_acquire)){
  if(m.isAllNotesOff() || m.isAllSoundOff())return false;
  if(m.isNoteOn())trackerMidiQueue.push({0,m.getNoteNumber()});
  else if(m.isController() && m.getControllerNumber()==64)
   trackerMidiQueue.push({m.getControllerValue()>=64?1:2,0});
  return true; // isolate recording keyboard from live synth and from mapped triggers
 }
 if(!m.isNoteOn() && !m.isNoteOff())return false;
 const int note=m.getNoteNumber();
 if(note<0||note>=128)return false;
 // Lua priority: a Song MIDI note wins over a Pattern assigned to the same key.
 const auto songMap=songMidiMap.load(std::memory_order_acquire);
 if(m.isNoteOff() && songMidiTriggerNote==note){
  songStopAudio(false);songMidiTriggerNote=-1;return true;
 }
 if(songMap && songMap->atNote[(size_t)note]){
  if(m.isNoteOff())return true;
  trackerReleaseNote();trackerTransport.stop();trackerReleaseArpActive=false;
  trackerPlaybackStatus.store(0,std::memory_order_relaxed);
  songStopAudio(false);
  trackerClearOverrides();
  songChainSnapshot=songChainMap.load(std::memory_order_acquire);
  for(auto& fx:songFx)fx.resetSidechain();
  songBegin(songMap->atNote[(size_t)note],0);
  songMidiTriggerNote=note;
  return true;
 }
 if(m.isNoteOff()){
  if(trackerMidiTriggerNote==note){
   trackerReleaseNote();trackerTransport.stop();trackerReleaseArpActive=false;
   trackerClearOverrides();
   trackerPlaybackStatus.store(0,std::memory_order_relaxed);
   trackerMidiTriggerNote=-1;
   trackerOverrideMask[3].store(false,std::memory_order_relaxed);
   return true;
  }
  // The JSFX consumes Note Off messages for any assigned note, even when
  // that note is not the current trigger.
  const auto mapping=trackerMidiMap.load(std::memory_order_acquire);
  return songTransport.running || (mapping && bool(mapping->atNote[(size_t)note]));
 }
 const auto mapping=trackerMidiMap.load(std::memory_order_acquire);
 if(!mapping || !mapping->atNote[(size_t)note]){
  if(songTransport.running)return true; // the Song owns all three operator channels
  if(!trackerTransport.running){trackerReleaseArpActive=false;trackerClearOverrides();songMixActive=false;}
  return false;
 }
 if(songTransport.running)songStopAudio(false);
 songMixActive=false;
 songMidiTriggerNote=-1;
 trackerReleaseNote();trackerTransport.stop();trackerReleaseArpActive=false;
 trackerClearOverrides();
 trackerPlaybackSnapshot=mapping->atNote[(size_t)note];
 trackerTransport.start(0,*trackerPlaybackSnapshot,false);
 trackerPlaybackStatus.store(1,std::memory_order_relaxed);
 trackerMidiTriggerNote=note;
 return true;
}
void BiscotProcessor::trackerReleaseArpTick() noexcept {
 if(!trackerReleaseArpActive || !trackerPlaybackSnapshot)return;
 // Stop as soon as Op1 VCA becomes silent: no hidden long-running work in idle.
 if(voices[0].amp.stage==0 || --trackerReleaseSamplesRemaining<=0){
  trackerReleaseArpActive=false;trackerClearOverrides();return;
 }
 if(--trackerReleaseStepRemain>0)return;
 const auto& p=*trackerPlaybackSnapshot;
 if(p.rows.empty()){trackerReleaseArpActive=false;return;}
 if(trackerReleaseStep >=p.length)trackerReleaseStep=0;
 trackerReleaseLastNote=trackerReleaseArpNote(p,trackerReleaseStep,trackerReleaseLastNote);
 ++trackerReleaseStep;
 trackerReleaseStepRemain=trackerReleaseStepSamples;
 if(trackerReleaseLastNote>=0){
  // Pitch-only legato: keep the VCA in Release. Never send Note On.
  auto& v=voices[0];
  v.note=juce::jlimit(0,127,trackerReleaseLastNote+trackerReleaseTranspose);v.noteSmooth=v.note;
 }
}
void BiscotProcessor::trackerRunCurveEvents(const TrackerPlayableRow& row) noexcept {
 // Segment events are sample accurate, but when multiple events land on the
 // same sample only the final value matters. Collapse them to one atomic write
 // (important at high BPM / 1024-point Curve Resolution).
 const double elapsed=trackerTransport.stepLength-trackerTransport.stepRemaining-1.;
 bool anyChanged=false;
 for(const int index:row.curveIndices){
  const auto& a=row.automation[(size_t)index];
  int& next=trackerCurveNextSegments[(size_t)index];
  const int end=a.segments();
  const int due=trackerCurveDueSegment(trackerTransport.stepLength,elapsed,end);
  if(due<next)continue;
  trackerSetOverride(a.synthIndex,(float)trackerCurveEventValue(a,trackerTransport.currentStep,due));
  next=due+1;
  anyChanged=true;
 }
 // Curves can activate previously unused LFO depth/routing at mid-step.
 if(anyChanged)lfoUsedCached=determineUsedLfos([this](int index){return param(index);});
}
void BiscotProcessor::requestSongStart(int row) noexcept {
 songRequested.store(1+juce::jlimit(0,4095,row),std::memory_order_release);
}
void BiscotProcessor::requestSongPause() noexcept {songRequested.store(-2,std::memory_order_release);}
void BiscotProcessor::requestSongStop() noexcept {songRequested.store(-1,std::memory_order_release);}
void BiscotProcessor::requestSongPreview(int lane,int row,int steps){
 // Never interrupt a full Song transport with a navigation preview.
 if(songPlaybackStatus.load(std::memory_order_acquire)==1)return;
 const auto live=songPlayable.load(std::memory_order_acquire);
 if(!live)return;
 const auto preview=compileTrackerSongRowPreview(*live,lane,row,steps);
 if(!preview)return;
 songPreviewPlayable.store(preview,std::memory_order_release);
 songRequested.store(-3,std::memory_order_release);
}
void BiscotProcessor::songBegin(const std::shared_ptr<const TrackerPlayableSong>& playable,int startRow) noexcept {
 if(!playable)return;
 songPlaybackSnapshot=playable;
 songPendingEndSong.store(-1,std::memory_order_release);
 songTransport.start(startRow,*playable);
 songMixer=playable->mixer;songMixActive=true;
 songMixerLastRevision=songMixerRevision.load(std::memory_order_acquire);
 songFilterPreference=playable->filterOwner;
 songLfoPreferences=playable->lfoOwners;
 songFilterOwner=songFilterPreference;songLfoOwners=songLfoPreferences;
 songFilterMask=playable->filterMask;
 for(int op=0;op<3;++op)trackerSetOverride(114+op,(songFilterMask&(1<<op))?1.f:0.f);
 songFxModeActive=true;
 songPlayingNumber.store(playable->songNumber,std::memory_order_relaxed);
 songPlaybackStatus.store(1,std::memory_order_relaxed);
 for(auto& x:songAudibleRows)x.store(-1,std::memory_order_relaxed);
 idleSleep.wake();
}
bool BiscotProcessor::songChainNext() noexcept {
 if(!songPlaybackSnapshot || !songChainSnapshot)return false;
 // F2 can change Song End while the Song is already sounding. An immutable
 // playback snapshot may therefore carry the old target; use the newest
 // message-thread publication at the chain boundary, never read the UI bank.
 const int current=songPlaybackSnapshot->songNumber;
 const int liveId=songPendingEndSong.load(std::memory_order_acquire);
 const int next=liveId==current?songPendingEndValue.load(std::memory_order_relaxed):songPlaybackSnapshot->songEnd;
 if(next<0||next>=100)return false;
 if(auto newer=songChainMap.load(std::memory_order_acquire))songChainSnapshot=std::move(newer);
 const auto& target=songChainSnapshot->byNumber[(size_t)next];
 if(!target)return false; // deliberately never guess an absent Song's contents
 // End -> next Song is a timeline transition, not a full DSP reset.
 // Keep oscillator/filter/LFO state and any running delay tails.
 for(int lane=0;lane<3;++lane)songReleaseLane(lane);
 songBegin(target,0);
 return true;
}
void BiscotProcessor::songApplyRowFx(int lane,const TrackerSongRowFx& fx) noexcept {
 const auto apply=[&](int id)->bool{return fx.present[(size_t)id];};
 if(apply(10))songFilterPreference=std::clamp((int)std::lround(fx.value[10]),1,3);
 if(apply(11))songFilterMask=std::clamp((int)std::lround(fx.value[11]),0,7);
 for(int j=0;j<3;++j)if(apply(12+j))
  songLfoPreferences[(size_t)j]=std::clamp((int)std::lround(fx.value[(size_t)(12+j)]),1,3);
 if(apply(11))for(int op=0;op<3;++op)
  trackerSetOverride(114+op,(songFilterMask&(1<<op))?1.f:0.f);
 // Sidechain and Delay are independent of the presence of a Pattern in the row.
 // Each explicitly written control updates that lane's persistent FX state.
 songFx[(size_t)lane].setRow(fx);
}
void BiscotProcessor::songResolveResources(const TrackerSongTick& tick) noexcept {
 std::array<int,4> claimantMask{};
 for(int lane=0;lane<3;++lane){
  const TrackerSongClip* clip=tick.lanes[(size_t)lane].clip;
  const auto& transport=songTransport.lanes[(size_t)lane];
  if(!clip && !transport.done && transport.currentPatternStep>=0 &&
      transport.audibleClipIndex>=0 && songPlaybackSnapshot &&
      transport.audibleClipIndex<(int)songPlaybackSnapshot->clips[(size_t)lane].size())
   clip=&songPlaybackSnapshot->clips[(size_t)lane][(size_t)transport.audibleClipIndex];
  if(!clip)continue;
  for(int resource=0;resource<4;++resource)
   if(clip->resourceFlags&(1<<resource))claimantMask[(size_t)resource]|=1<<lane;
 }
 songFilterOwner=trackerSongResolveOwner(claimantMask[0],songFilterPreference);
 for(int j=0;j<3;++j)
  songLfoOwners[(size_t)j]=trackerSongResolveOwner(claimantMask[(size_t)(j+1)],songLfoPreferences[(size_t)j]);
}
void BiscotProcessor::songReleaseLane(int lane) noexcept {
 const int channel=lane+1;
 for(int i=songActiveSubCount[(size_t)lane]-1;i>=0;--i)
  onMidi(juce::MidiMessage::noteOff(channel,songActiveSubs[(size_t)lane][(size_t)i]));
 songActiveSubCount[(size_t)lane]=0;
 if(songActiveNotes[(size_t)lane]>=0){
  onMidi(juce::MidiMessage::noteOff(channel,songActiveNotes[(size_t)lane]));
  songActiveNotes[(size_t)lane]=-1;
 }
}
void BiscotProcessor::songStopAudio(bool pause) noexcept {
 for(int lane=0;lane<3;++lane)songReleaseLane(lane);
 songTransport.stop();
 // Keep Song mixer and timbre throughout the natural VCA release tail.
 // Clearing either here creates an audible gain or filter discontinuity.
 songPlaybackStatus.store(pause?2:0,std::memory_order_relaxed);
 songPlayingNumber.store(-1,std::memory_order_relaxed);
}
int BiscotProcessor::songMapSlider(int idx,int lane,const TrackerSongClip&) const noexcept {
 if(idx<0||idx>=234)return -1;
 // Song Patterns use the generic Op1 columns, never the stored live MIDI mode.
 if(idx>=10&&idx<=102){return ((idx-10)%3)==0?idx+lane:-1;}
 // Routing is controlled exclusively by Song Filter Operators, not the Pattern.
 if(idx>=114&&idx<=116)return -1;
 if(idx==6||idx==7||(idx>=108&&idx<=121))return songFilterOwner==lane+1?idx:-1;
 // The three LFO wave generators are shared; depths belong to individual OPs.
 for(int j=0;j<3;++j){
  const int base=122+j*23;
  if(idx>=base&&idx<=base+10)return songLfoOwners[(size_t)j]==lane+1?idx:-1;
  if(idx>=base+11&&idx<=base+22){
   const int offset=idx-(base+11);
   return offset%3==0?idx+lane:-1;
  }
 }
 if(idx>=231&&idx<=233)return songLfoOwners[(size_t)(idx-231)]==lane+1?idx:-1;
 return idx;
}
void BiscotProcessor::songApplyStep(int lane,const TrackerSongLaneTick& e) noexcept {
 if(!e.clip||e.patternStep<0)return;
 const auto& clip=*e.clip;
 const auto& row=clip.pattern->rows[(size_t)e.patternStep];
 if(e.clipStarted){
  for(const auto& [index,value]:clip.initialParams){
   const int mapped=songMapSlider(index,lane,clip);
   if(mapped>=0)trackerSetOverride(mapped,value);
  }
 }
 songAudibleRows[(size_t)lane].store(e.songRow,std::memory_order_relaxed);
 trackerSetOverride(3,2.f); // MIDI channel 1/2/3 routes exactly to Op1/2/3
 trackerSetOverride(230,0.f); // independent OP controls even if preset uses chord master
 trackerSetOverride(10+lane,clip.legato?1.f:0.f);
 for(const auto& a:row.automation){
  const int slider=songMapSlider(a.synthIndex,lane,clip);
  if(slider>=0)trackerSetOverride(slider,(float)trackerCurveAt(a,e.patternStep,0.));
 }
 if(row.pause){songReleaseLane(lane);return;}
 if(row.note>=0){
  const int note=juce::jlimit(0,127,row.note+e.transpose);
  if(clip.legato && songActiveNotes[(size_t)lane]==note){
   // Identical-pitch legato must not push a duplicate hidden MIDI hold.
   for(int i=songActiveSubCount[(size_t)lane]-1;i>=0;--i)
    onMidi(juce::MidiMessage::noteOff(lane+1,songActiveSubs[(size_t)lane][(size_t)i]));
   songActiveSubCount[(size_t)lane]=0;
  }else if(clip.legato && songActiveNotes[(size_t)lane]>=0){
   // New voice joins the held stack before the old one leaves: no ADSR reattack.
   onMidi(juce::MidiMessage::noteOn(lane+1,note,static_cast<uint8_t>(100)));
   songReleaseLane(lane);
   songActiveNotes[(size_t)lane]=note;
  }else{
   songReleaseLane(lane);
   onMidi(juce::MidiMessage::noteOn(lane+1,note,static_cast<uint8_t>(100)));
   songActiveNotes[(size_t)lane]=note;
  }
  for(int sub:row.subNotes){
   if(songActiveSubCount[(size_t)lane]>=15)break;
   const int n=juce::jlimit(0,127,sub+e.transpose);
   onMidi(juce::MidiMessage::noteOn(lane+1,n,static_cast<uint8_t>(100)));
   songActiveSubs[(size_t)lane][(size_t)songActiveSubCount[(size_t)lane]++]=n;
  }
 }
 if(songActiveNotes[(size_t)lane]>=0&&(row.note>=0||row.gateExplicit)){
  auto& state=songTransport.lanes[(size_t)lane];
  state.gateRemaining=row.holdIntoNext?-1:std::max(1.,e.duration*row.gate);
 }
 songCurveNext[(size_t)lane].fill(1);
 lfoUsedCached=determineUsedLfos([this](int i){return param(i);});
}
void BiscotProcessor::songRunCurves(int lane,const TrackerSongClip& clip,int step) noexcept {
 const auto& row=clip.pattern->rows[(size_t)step];
 const auto& state=songTransport.lanes[(size_t)lane];
 const double elapsed=state.stepDuration-state.remaining-1.;
 bool changed=false;
 for(const int index:row.curveIndices){
  if(index<0||index>=(int)songCurveNext[(size_t)lane].size())continue;
  const auto& a=row.automation[(size_t)index];
  const int due=trackerCurveDueSegment(state.stepDuration,elapsed,a.segments());
  auto& next=songCurveNext[(size_t)lane][(size_t)index];
  if(due<next)continue;
  const int idx=songMapSlider(a.synthIndex,lane,clip);
  if(idx>=0){trackerSetOverride(idx,(float)trackerCurveEventValue(a,step,due));changed=true;}
  next=due+1;
 }
 if(changed)lfoUsedCached=determineUsedLfos([this](int i){return param(i);});
}
void BiscotProcessor::enterColdIdle() noexcept {
 // A transition occurs ONLY after the last VCA is idle and the output is
 // already below -100 dBFS. Clear histories once, never during the Release.
 for(auto& f:filters)f.reset();
 for(auto& stage:outputStages)stage.reset();
 for(int i=0;i<3;++i){
  auto& v=voices[(size_t)i];
  v.note=-1;v.noteSmooth=-1;v.phase=v.oldPhase=0.;
  v.noiseClock=0.;v.noise=0.f;v.oscPrev=0.f;v.arpPitch=0.f;
  v.arpPhase=0.;v.arpStep=0;v.gate=false;v.pedalLatched=false;
  v.amp={};v.mod={};v.quantizedArp.reset();
  sidSeq[(size_t)i].clearAll();
 }
 filterEnvelope={};
 arpReleaseCounts.fill(0);
 if(!trackerTransport.running&&!songTransport.running){
  trackerClearOverrides();songMixActive=false;
  songFxModeActive=false;
  for(auto& fx:songFx)fx.clearDelay();
 }
 // Never zero free LFO phases: restarting one would desynchronize BPM-based
 // modulations whenever a song inserts a rest.
}
void BiscotProcessor::processBlock(juce::AudioBuffer<float>& buffer,juce::MidiBuffer& midi){
 juce::ScopedNoDenormals denormals;
 buffer.clear();
 // UI -> audio command only; no locks and no reading the editable bank.
 // Apply F2 mixer edits without rebuilding the running Song or touching maps.
 const auto liveRevision=songMixerRevision.load(std::memory_order_acquire);
 if(liveRevision!=songMixerLastRevision){
  songMixerLastRevision=liveRevision;
  if(songTransport.running && songPlayingNumber.load(std::memory_order_relaxed)==songPendingMixerSong.load(std::memory_order_relaxed))
   for(int i=0;i<3;++i)songMixer[(size_t)i]=songPendingMixer[(size_t)i].load(std::memory_order_relaxed);
 }
 const int songCommand=songRequested.exchange(0,std::memory_order_acq_rel);
 if(songCommand!=0){
  const int patternStopped=trackerRequested.exchange(0,std::memory_order_acq_rel);
  (void)patternStopped;
  trackerReleaseNote();trackerTransport.stop();trackerReleaseArpActive=false;
  trackerMidiTriggerNote=-1;trackerPlaybackStatus.store(0,std::memory_order_relaxed);
  songStopAudio(songCommand==-2);
  if(songCommand>0){
   trackerClearOverrides();
   songMidiTriggerNote=-1;
   songChainSnapshot=songChainMap.load(std::memory_order_acquire);
   for(auto& fx:songFx)fx.resetSidechain();
   songBegin(songPlayable.load(std::memory_order_acquire),songCommand-1);
  }else if(songCommand==-3){
   trackerClearOverrides();
   songChainSnapshot.reset(); // auditions must never follow Song End
   songBegin(songPreviewPlayable.load(std::memory_order_acquire),0);
   songPlaybackStatus.store(3,std::memory_order_relaxed);
  }
 }
 const int trackerCommand=trackerRequested.exchange(0,std::memory_order_acq_rel);
 if(trackerCommand!=0){
  if(songTransport.running) songStopAudio(false);
  songMixActive=false;
  trackerReleaseNote();trackerReleaseArpActive=false;trackerMidiTriggerNote=-1;
  if(trackerCommand>0)trackerClearOverrides();
  const int audible=trackerTransport.currentStep;
  trackerTransport.stop();
  if(trackerCommand>0){
   trackerPlaybackSnapshot=trackerPlayable.load(std::memory_order_acquire);
   if(trackerPlaybackSnapshot){
    const bool audition=trackerCommand>=2049;
    const int wanted=(audition?trackerCommand-2049:trackerCommand-1);
    trackerTransport.start(wanted,*trackerPlaybackSnapshot,audition);
    trackerPlaybackStatus.store(audition?3:1,std::memory_order_relaxed);
   }
  }else{
   // Tracker's temporary MIDI routing must not leak into live keyboard play.
   trackerOverrideMask[3].store(false,std::memory_order_relaxed);
   trackerPlaybackStatus.store(trackerCommand==-2?2:0,std::memory_order_relaxed);
   trackerAudibleStep.store(audible,std::memory_order_relaxed);
   trackerPlaybackSnapshot.reset();
   if(idleSleep.asleep)trackerClearOverrides();
  }
 }
 // The JSFX returns immediately for a cold idle block. In particular, the
 // 16-channel arp list must not be rebuilt 3 times per block in silence.
 // MIDI-bearing blocks still go through the original sample-offset loop.
 const bool coldIdleNoMidi=idleSleep.asleep && midi.isEmpty() && !trackerTransport.running && !songTransport.running && !trackerReleaseArpActive;
 if(!coldIdleNoMidi){
  lfoUsedCached=determineUsedLfos([this](int index){return param(index);});
  releasePedalLatches(); // also honor changes to the Sustain Pedal switches
  for(int i=0;i<3;++i)if(!opPedalEnabled(i))sustainedArp.clearOperator(i);
  refreshArpPoolsIfNeeded();
 }
 if(auto* ph=getPlayHead())if(auto position=ph->getPosition()){
  if(auto bpm=position->getBpm())tempoBpm=juce::jlimit(1.,999.,*bpm);
  // JUCE 8 getIsPlaying() returns bool, not std::optional<bool>.
  // Restart free-running LFO phases only on the stopped -> playing edge.
  const bool running=position->getIsPlaying();
  if(!running && lastPlaying && songTransport.running){songStopAudio(false);songMidiTriggerNote=-1;}
  if(running && !lastPlaying){
   for(int i=0;i<3;++i)if(param(122+i*23)<0.5f){lfos[(size_t)i].phase=param(131+i*23);lfos[(size_t)i].age=0;}
  }
  lastPlaying=running;
 }
 if(coldIdleNoMidi){
  // Outputs are already zero, no audio-view creation, MIDI iterator, oscillator,
  // filter, modulation, pool preparation or sample loop. Preserve Free clocks.
  advanceIdleLfos(buffer.getNumSamples());
  return;
 }
 // Resolve bus views once per block: never allocate/copy bus metadata in @sample.
 auto mix=getBusBuffer(buffer,false,0);
 auto op1=getBusBuffer(buffer,false,1);
 auto op2=getBusBuffer(buffer,false,2);
 auto op3=getBusBuffer(buffer,false,3);
 auto filterOut=getBusBuffer(buffer,false,4);
 juce::MidiBuffer::Iterator it(midi);juce::MidiMessage message;int offset=0;bool more=it.getNextEvent(message,offset);
 // One immutable snapshot per block. The editor may publish newer rows,
 // but the sample loop never copies or allocates Pattern contents.
 const TrackerPlayablePattern* playback=trackerTransport.running?trackerPlaybackSnapshot.get():nullptr;
 const bool multichannel=param(2)>0.5f;
 // Zero-work idle: the output buffer was already cleared, so skip whole
 // silent spans rather than executing the SID DSP or writing zero samples.
 // MIDI is consumed at its exact sample offset, including inside these spans.
 for(int sample=0;sample<buffer.getNumSamples();){
  while(more && offset<=sample){
   if(!handleTrackerExternalMidi(message))onMidi(message);
   playback=trackerTransport.running?trackerPlaybackSnapshot.get():nullptr;
   more=it.getNextEvent(message,offset);
  }
  if(songTransport.running && songPlaybackSnapshot){
   const auto events=songTransport.tick(*songPlaybackSnapshot,sampleRateHz,tempoBpm);
   // Evaluate all three Song row control columns before any note-on.
   // This also processes effects on blank Song rows.
   for(int lane=0;lane<3;++lane)
    if(events.lanes[(size_t)lane].rowFx)
     songApplyRowFx(lane,*events.lanes[(size_t)lane].rowFx);
   bool resourceBoundary=false;
   for(const auto& laneEvent:events.lanes)
    if(laneEvent.rowStarted||laneEvent.clip||laneEvent.finished)
     {resourceBoundary=true;break;}
   if(resourceBoundary)songResolveResources(events);
   for(int lane=0;lane<3;++lane){
    const auto& e=events.lanes[(size_t)lane];
    if(e.release)songReleaseLane(lane);
    if(e.clip) songApplyStep(lane,e);
    if(e.finished)songReleaseLane(lane);
    const auto& current=songTransport.lanes[(size_t)lane];
    if(current.currentPatternStep>=0 && current.audibleClipIndex>=0 &&
       current.audibleClipIndex<(int)songPlaybackSnapshot->clips[(size_t)lane].size()){
     const auto& c=songPlaybackSnapshot->clips[(size_t)lane][(size_t)current.audibleClipIndex];
     if(current.currentPatternStep<(int)c.pattern->rows.size())
      songRunCurves(lane,c,current.currentPatternStep);
    }
   }
   if(events.finished && !songChainNext())songStopAudio(false);
  }
  if(playback && trackerTransport.running){
   const auto event=trackerTransport.tick(*playback,sampleRateHz,tempoBpm);
   if(event.gateOff)trackerReleaseNote();
   if(event.startStep>=0){
    const int step=event.startStep;
    trackerAudibleStep.store(step,std::memory_order_relaxed);
    const int repeat=trackerTransport.repeatIndex;
    const int transpose=repeat<(int)playback->repeatTransposes.size()?
      playback->repeatTransposes[(size_t)repeat]:0;
    trackerApplyStep(playback->rows[(size_t)step],transpose,
                     trackerTransport.stepSamples(*playback,sampleRateHz,tempoBpm));
    trackerCurveNextSegments.fill(1);
   }
   if(trackerTransport.running && trackerTransport.currentStep>=0){
    const auto& row=playback->rows[(size_t)trackerTransport.currentStep];
    if(!row.curveIndices.empty())trackerRunCurveEvents(row);
   }
   if(event.finished){
    trackerReleaseNote();
    if(playback->releaseArp && !playback->rows.empty()){
     const float release=std::max(param(43),param(121));
     if(release>0.f && trackerTransport.lastNote>=0){
      trackerReleaseArpActive=true;trackerReleaseLastNote=trackerTransport.lastNote;
      trackerReleaseSamplesRemaining=std::max(1.,sampleRateHz*release);
      const int lastRepeat=juce::jmax(0,trackerTransport.repeatIndex-1);
      const double repeatSpeed=lastRepeat<(int)playback->repeatSpeeds.size()?playback->repeatSpeeds[(size_t)lastRepeat]:1.0;
      trackerReleaseStepSamples=std::max(1.,sampleRateHz/(std::max(1.,tempoBpm)/60.*4.*std::clamp(repeatSpeed,0.25,16.)));
      trackerReleaseStepRemain=0;trackerReleaseStep=0;
      trackerReleaseTranspose=lastRepeat<(int)playback->repeatTransposes.size()?playback->repeatTransposes[(size_t)lastRepeat]:0;
      // The JSFX release arp changes pitch only: the old ADSR continues
      // releasing without a new trigger, despite OP Arp Mode being enabled.
      trackerSetOverride(76,0.f);voices[0].arpPitch=0.f;
     }
    }
    trackerOverrideMask[3].store(false,std::memory_order_relaxed);
    trackerPlaybackStatus.store(0,std::memory_order_relaxed);
    if(idleSleep.asleep)trackerClearOverrides();
    // Keep the immutable snapshot until the next request.  Never destroy a
    // large Pattern while rendering a sample.
   }
  }
  if(trackerReleaseArpActive)trackerReleaseArpTick();
  if(idleSleep.asleep){
   const int next=(trackerTransport.running||songTransport.running)?sample+1:(more?juce::jlimit(sample+1,buffer.getNumSamples(),offset):buffer.getNumSamples());
   advanceIdleLfos(next-sample);
   sample=next;
   continue;
  }
  float left=0,right=0;std::array<float,8> stems{};
  renderOne(left,right,stems);
  if(mix.getNumChannels()>0)mix.setSample(0,sample,left);
  if(mix.getNumChannels()>1)mix.setSample(1,sample,right);
  if(multichannel){
   auto put=[sample](juce::AudioBuffer<float>& out,float l,float r){
    if(out.getNumChannels()>0)out.setSample(0,sample,l);
    if(out.getNumChannels()>1)out.setSample(1,sample,r);
   };
   put(op1,stems[0],stems[1]);put(op2,stems[2],stems[3]);
   put(op3,stems[4],stems[5]);put(filterOut,stems[6],stems[7]);
  }
  float peak=std::max(std::abs(left),std::abs(right));
  if(multichannel)for(const float x:stems)peak=std::max(peak,std::abs(x));
  bool noGate=heldNotes.filter.empty();
  bool vcaIdle=true;
  for(const auto& v:voices){
   noGate=noGate && !v.gate && !v.pedalLatched;
   vcaIdle=vcaIdle && v.amp.stage==0 && v.amp.level<=0.0001f;
  }
  bool fxTail=false;
  if(songFxModeActive)for(const auto& fx:songFx)fxTail=fxTail||fx.hasTail();
  if(idleSleep.observe(noGate&&!fxTail,vcaIdle&&!fxTail,peak))enterColdIdle();
  ++sample;
 }
}
// TEST0.2.18: the Pattern bank lives in the VST3 state. This makes the bank
// self-contained in REAPER .RPP projects and in existing Alt+S .biscot presets.
// Nothing in this serializer is called from processBlock() or renderOne().
namespace {
const juce::Identifier trackerStateKey{"trackerPatternsV1"};
const juce::Identifier trackerSongStateKey{"trackerSongsV1"};
constexpr int trackerMagic=0x42545031; // BTP1
void writeTrackerBank(const TrackerPatternBank& b,juce::MemoryOutputStream& o){
 o.writeInt(trackerMagic);o.writeInt(b.activePattern);o.writeInt((int)b.patterns.size());
 for(const auto& [slot,p]:b.patterns){
  o.writeInt(slot);o.writeString(juce::String::fromUTF8(p.name.c_str()));
  o.writeInt(p.length);o.writeInt(p.midiNote);o.writeInt(p.repeat);o.writeInt(p.releaseArp?1:0);
  for(int i=0;i<p.repeat;++i){
   o.writeDouble(i<(int)p.repeatSpeeds.size()?p.repeatSpeeds[(size_t)i]:1.0);
   o.writeInt(i<(int)p.repeatTransposes.size()?p.repeatTransposes[(size_t)i]:0);
  }
  int nonempty=0;for(const auto& [st,row]:p.steps){
   if(st<0||st>=TrackerPatternBank::maxSteps)continue;
   for(const auto& a:row.tabs)if(!a.empty()){++nonempty;break;}
  }
  o.writeInt(nonempty);
  for(const auto& [st,row]:p.steps){
   if(st<0||st>=TrackerPatternBank::maxSteps)continue;
   bool any=false;for(const auto& a:row.tabs)if(!a.empty())any=true;
   if(!any)continue;
   o.writeInt(st);
   for(const auto& a:row.tabs){
    o.writeInt((int)a.size());for(const auto& c:a){o.writeInt(c.id);o.writeDouble(c.value);}
   }
  }
 }
}
bool readTrackerBank(const juce::String& text,TrackerPatternBank& result){
 juce::MemoryBlock binary;
 if(text.isEmpty()||!binary.fromBase64Encoding(text)||binary.getSize()>16*1024*1024)return false;
 juce::MemoryInputStream in(binary,false);
 if(in.readInt()!=trackerMagic)return false;
 TrackerPatternBank fresh;fresh.patterns.clear();
 const int current=in.readInt(),count=in.readInt();
 if(current<0||current>=1000||count<0||count>1000)return false;
 for(int x=0;x<count;++x){
  if(in.isExhausted())return false;
  const int slot=in.readInt();const auto name=in.readString();
  const int length=in.readInt(),midi=in.readInt(),repeats=in.readInt(),release=in.readInt();
  if(slot<0||slot>=1000||length<1||length>1024||midi< -1||midi>127||repeats<1||repeats>1000)return false;
  TrackerPattern pat(false);pat.name=name.toStdString();pat.length=length;pat.midiNote=midi;pat.repeat=repeats;pat.releaseArp=release!=0;
  pat.repeatSpeeds.clear();pat.repeatTransposes.clear();
  for(int y=0;y<repeats;++y){
   const double speed=in.readDouble();const int transpose=in.readInt();
   if(!std::isfinite(speed)||speed<0.25||speed>16||transpose< -96||transpose>96)return false;
   pat.repeatSpeeds.push_back(speed);pat.repeatTransposes.push_back(transpose);
  }
  const int rows=in.readInt();if(rows<0||rows>1024)return false;
  for(int y=0;y<rows;++y){
   const int step=in.readInt();if(step<0||step>=1024)return false;
   int total=0;
   for(int tab=0;tab<3;++tab){
    const int n=in.readInt();total+=n;if(n<0||total>128)return false;
    auto& a=pat.steps[step].tabs[(size_t)tab];
    for(int z=0;z<n;++z){
     const int id=in.readInt();const double value=in.readDouble();
     if(id<0||id>10000||!std::isfinite(value))return false;
     a.push_back({id,value});
    }
   }
  }
  if(!fresh.patterns.emplace(slot,std::move(pat)).second)return false;
 }
 fresh.activePattern=current;
 if(fresh.patterns.find(current)==fresh.patterns.end())fresh.patterns.try_emplace(current);
 result=std::move(fresh);return true;
}
}
// Song serialization is independent of Pattern V1. Old .biscot files remain
// readable; missing trackerSongsV1 means an empty Song bank. No audio thread IO.
namespace {
constexpr int trackerSongMagic=0x42545331; // BTS1
void writeSongBank(const TrackerSongBank& b,juce::MemoryOutputStream& out){
 out.writeInt(trackerSongMagic);out.writeInt(b.activeSong);out.writeInt((int)b.songs.size());
 for(const auto& [slot,s]:b.songs){
  out.writeInt(slot);out.writeString(juce::String::fromUTF8(s.name.c_str()));
  out.writeInt(s.midiNote);out.writeInt(s.songEnd);out.writeInt(s.loopTarget);out.writeInt(s.chainLoop?1:0);
  out.writeInt(s.filterOwner);out.writeInt(s.filterMask);
  for(int owner:s.lfoOwners)out.writeInt(owner);
  for(int volume:s.mixerVolumes)out.writeInt(volume);
  out.writeInt(s.mixerMuteMask);out.writeInt(s.mixerSoloLane);
  for(int lane=0;lane<3;++lane){
   out.writeInt(s.laneLengths[(size_t)lane]);
   const int repeats=s.laneRepeats[(size_t)lane];out.writeInt(repeats);
   for(int i=0;i<repeats;++i)
    out.writeDouble(i<(int)s.laneSpeeds[(size_t)lane].size()?s.laneSpeeds[(size_t)lane][(size_t)i]:1.0);
   const auto& a=s.lanes[(size_t)lane];
   int count=0;for(const auto& [step,entry]:a)if(step>=0&&step<4096&&!entry.empty())++count;
   out.writeInt(count);
   for(const auto& [step,entry]:a){
    if(step<0||step>=4096||entry.empty())continue;
    out.writeInt(step);out.writeInt((int)entry.order.size());
    for(int id:entry.order){out.writeInt(id);out.writeDouble(entry.values[(size_t)id]);}
    // Pattern range Start and Play Length belong to the Pattern group.
    if(entry.contains(1)){
     out.writeDouble(entry.values[5]);out.writeDouble(entry.values[6]);
    }
   }
  }
 }
}
bool readSongBank(const juce::String& text,TrackerSongBank& result){
 juce::MemoryBlock binary;
 if(text.isEmpty()||!binary.fromBase64Encoding(text)||binary.getSize()>128u*1024u*1024u)return false;
 juce::MemoryInputStream in(binary,false);
 const auto available=[&](int64_t n){return n>=0 && in.getPosition()<=in.getDataSize() &&
    n<=(int64_t)(in.getDataSize()-in.getPosition());};
 if(!available(12)||in.readInt()!=trackerSongMagic)return false;
 TrackerSongBank fresh;fresh.songs.clear();
 const int current=in.readInt(),count=in.readInt();
 if(current<0||current>=100||count<0||count>100)return false;
 for(int x=0;x<count;++x){
  if(!available(4))return false;
  const int slot=in.readInt();
  if(slot<0||slot>=100||fresh.songs.count(slot))return false;
  const auto name=in.readString();
  if(name.length()>1024||!available(4*15))return false;
  TrackerSong song;song.name=name.toStdString();
  song.midiNote=in.readInt();song.songEnd=in.readInt();song.loopTarget=in.readInt();song.chainLoop=in.readInt()!=0;
  song.filterOwner=in.readInt();song.filterMask=in.readInt();
  for(auto& v:song.lfoOwners)v=in.readInt();
  for(auto& v:song.mixerVolumes)v=in.readInt();
  song.mixerMuteMask=in.readInt();song.mixerSoloLane=in.readInt();
  if(song.midiNote< -1||song.midiNote>127||song.songEnd< -1||song.songEnd>99||
     song.loopTarget< -1||song.loopTarget>99||song.filterOwner<1||song.filterOwner>3||
     song.filterMask<0||song.filterMask>7||song.mixerMuteMask<0||song.mixerMuteMask>7||
     song.mixerSoloLane<0||song.mixerSoloLane>3)return false;
  for(int v:song.lfoOwners)if(v<1||v>3)return false;
  for(int v:song.mixerVolumes)if(v<0||v>100)return false;
  for(int lane=0;lane<3;++lane){
   if(!available(8))return false;
   const int len=in.readInt(),repeats=in.readInt();
   if(len<1||len>4096||repeats<1||repeats>1000||!available((int64_t)repeats*8+4))return false;
   song.laneLengths[(size_t)lane]=len;song.laneRepeats[(size_t)lane]=repeats;
   auto& speeds=song.laneSpeeds[(size_t)lane];speeds.clear();
   for(int i=0;i<repeats;++i){
    const double speed=in.readDouble();
    if(!std::isfinite(speed)||speed<0.25||speed>16.)return false;
    speeds.push_back(speed);
   }
   const int rows=in.readInt();if(rows<0||rows>4096)return false;
   auto& a=song.lanes[(size_t)lane];
   for(int r=0;r<rows;++r){
    if(!available(8))return false;
    const int step=in.readInt(),entries=in.readInt();
    if(step<0||step>=4096||entries<1||entries>18||a.count(step)||!available((int64_t)entries*12))return false;
    TrackerSongStep entry;
    for(int k=0;k<entries;++k){
     const int id=in.readInt();const double value=in.readDouble();
     if(!std::isfinite(value)||!entry.insert(id))return false;
     if(const auto* def=songParam(id)){
      if(value<def->min-1.0e-7||value>def->max+1.0e-7)return false;
     }
     entry.values[(size_t)id]=value;
    }
    if(entry.contains(1)){
     if(!available(16))return false;
     entry.values[5]=in.readDouble();entry.values[6]=in.readDouble();
     if(!std::isfinite(entry.values[5])||!std::isfinite(entry.values[6])||
        entry.values[5]<0||entry.values[5]>1023||entry.values[6]<1||entry.values[6]>1024)return false;
    }
    a.emplace(step,std::move(entry));
   }
  }
  fresh.songs.emplace(slot,std::move(song));
 }
 if(!available(0)||in.getPosition()!=in.getDataSize())return false;
 fresh.activeSong=current;
 if(!fresh.songs.count(current))fresh.songs.try_emplace(current);
 result=std::move(fresh);return true;
}
}
void BiscotProcessor::commitSongState(){
 // Song F2 Mute/Solo/Volume edits affect a currently playing Song next block.
 if(songPlayingNumber.load(std::memory_order_acquire)==trackerSongs.activeSong){
  if(const auto* live=trackerSongs.find(trackerSongs.activeSong)){
   songPendingEndValue.store(live->songEnd,std::memory_order_relaxed);
   songPendingEndSong.store(trackerSongs.activeSong,std::memory_order_release);
   for(int i=0;i<3;++i){
    const bool mute=(live->mixerMuteMask&(1<<i))!=0 || (live->mixerSoloLane>0&&live->mixerSoloLane!=i+1);
    songPendingMixer[(size_t)i].store(mute?0.f:(float)std::clamp(live->mixerVolumes[(size_t)i]/100.,0.,1.),std::memory_order_relaxed);
   }
   songPendingMixerSong.store(trackerSongs.activeSong,std::memory_order_relaxed);
   songMixerRevision.fetch_add(1,std::memory_order_release);
  }
 }
 songChainMap.store(std::make_shared<const TrackerSongChainMap>(compileTrackerSongChainMap(trackerSongs,trackerPatterns)),std::memory_order_release);
 songMidiMap.store(std::make_shared<const TrackerSongMidiMap>(compileTrackerSongMidiMap(trackerSongs,trackerPatterns)),std::memory_order_release);
 if(const auto* song=trackerSongs.find(trackerSongs.activeSong))
  songPlayable.store(std::make_shared<const TrackerPlayableSong>(compileTrackerSong(*song,trackerSongs.activeSong,trackerPatterns)),std::memory_order_release);
 juce::MemoryOutputStream stream;writeSongBank(trackerSongs,stream);
 state.state.setProperty(trackerSongStateKey,stream.getMemoryBlock().toBase64Encoding(),nullptr);
}
void BiscotProcessor::commitTrackerState(){
 songChainMap.store(std::make_shared<const TrackerSongChainMap>(compileTrackerSongChainMap(trackerSongs,trackerPatterns)),std::memory_order_release);
 songMidiMap.store(std::make_shared<const TrackerSongMidiMap>(compileTrackerSongMidiMap(trackerSongs,trackerPatterns)),std::memory_order_release);
 if(const auto* song=trackerSongs.find(trackerSongs.activeSong))
  songPlayable.store(std::make_shared<const TrackerPlayableSong>(compileTrackerSong(*song,trackerSongs.activeSong,trackerPatterns)),std::memory_order_release);
 juce::MemoryOutputStream stream;writeTrackerBank(trackerPatterns,stream);
 trackerMidiMap.store(std::make_shared<const TrackerMidiMap>(compileTrackerMidiMap(trackerPatterns)),std::memory_order_release);
 state.state.setProperty(trackerStateKey,stream.getMemoryBlock().toBase64Encoding(),nullptr);
 const auto* pattern=trackerPatterns.find(trackerPatterns.activePattern);
 if(pattern)trackerPlayable.store(std::make_shared<const TrackerPlayablePattern>(compileTrackerPattern(*pattern)),
                                  std::memory_order_release);
}
void BiscotProcessor::getStateInformation(juce::MemoryBlock& out){if(auto xml=state.copyState().createXml())copyXmlToBinary(*xml,out);}
void BiscotProcessor::setStateInformation(const void* ptr,int length){
 if(auto xml=getXmlFromBinary(ptr,length);xml && xml->hasTagName(state.state.getType())){
  auto tree=juce::ValueTree::fromXml(*xml);
  TrackerPatternBank restored;
  if(!readTrackerBank(tree.getProperty(trackerStateKey).toString(),restored))restored.reset();
  TrackerSongBank restoredSongs;
  if(!readSongBank(tree.getProperty(trackerSongStateKey).toString(),restoredSongs))restoredSongs.reset();
  state.replaceState(tree);
  trackerPatterns=std::move(restored);
  trackerSongs=std::move(restoredSongs);
  trackerMidiMap.store(std::make_shared<const TrackerMidiMap>(compileTrackerMidiMap(trackerPatterns)),std::memory_order_release);
  if(const auto* pat=trackerPatterns.find(trackerPatterns.activePattern))
   trackerPlayable.store(std::make_shared<const TrackerPlayablePattern>(compileTrackerPattern(*pat)),
                         std::memory_order_release);
  songChainMap.store(std::make_shared<const TrackerSongChainMap>(compileTrackerSongChainMap(trackerSongs,trackerPatterns)),std::memory_order_release);
 songMidiMap.store(std::make_shared<const TrackerSongMidiMap>(compileTrackerSongMidiMap(trackerSongs,trackerPatterns)),std::memory_order_release);
  if(const auto* song=trackerSongs.find(trackerSongs.activeSong))
   songPlayable.store(std::make_shared<const TrackerPlayableSong>(compileTrackerSong(*song,trackerSongs.activeSong,trackerPatterns)),std::memory_order_release);
  requestSongStop();requestTrackerStop();
 }
}
juce::AudioProcessorEditor* BiscotProcessor::createEditor(){return new BiscotEditor(*this);}
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new biscot::BiscotProcessor();}
