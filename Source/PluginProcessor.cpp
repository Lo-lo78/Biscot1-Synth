#include "PluginProcessor.h"
#include "PluginEditor.h"
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
 sampleRateHz=sr>0?sr:44100.;voices={};lfos={};filters={};pressed={};arpPoolCounts={};nextPress=0;
 for(auto& s:outputStages)s.prepare(sampleRateHz);
 waveCore.prepare();
 for(int i=0;i<3;++i){voices[(size_t)i].noiseSeed^=(uint32_t)(i*0x9e3779b9u);
  voices[(size_t)i].cleanNoiseSeed=1234567ULL+(uint64_t)i*1111111ULL;
  voices[(size_t)i].sidNoiseSeed=i==0?5930003ULL:i==1?3592873ULL:7281093ULL;
  lfos[(size_t)i].seed^=(uint32_t)(i*0x51ed270bu);lfos[(size_t)i].phase=param(131+23*i);}
 filterEnvelope={};lastPlaying=false;
}
float BiscotProcessor::Envelope::tick(float a,float d,float s,double sr){
 s=limit(s,0.f,1.f);
 if(stage==1){level+=a<=0?1.f:1.f/(float)(a*sr);if(level>=1.f){level=1;stage=2;}}
 else if(stage==2){level-=d<=0?1.f:(1.f-s)/(float)(d*sr);if(level<=s){level=s;stage=3;}}
 else if(stage==3){level=s;}
 else if(stage==4){level-=std::max(0.0000001f,level)*std::max(0.0000000001f,releaseCoefficient);if(level<=0.0001f){level=0;stage=0;}}
 return level;
}
void BiscotProcessor::noteOn(int i,int note,int channel){
 auto& v=voices[(size_t)i];const bool legato=param(10+i)>0.5f && v.gate;
 const float portamento=param(46+i);
 if(v.note<0 || portamento<=0)v.noteSmooth=note;
 v.note=note;v.channel=channel;v.gate=true;
 v.vel=limit(v.vel,0.f,1.f);
 v.drift=randomUnit(v.noiseSeed)*param(13+i)*0.50f;
 if(!legato){
  v.amp.noteOn(integer(param(4))>=2);
  v.mod.noteOn(integer(param(4))>=2);
  if(param(4)>=1){v.phase=0;v.oldPhase=0;}
  if(param(4)>=3){v.noiseSeed=0x12345678u ^ (uint32_t)(i*0x9e3779b9u);v.noise=0.f;}
  v.arpStep=0;v.arpPhase=0;v.arpPitch=0;
  for(int j=0;j<3;++j){const int b=122+j*23;
   if(param(b)>0.5f || param(b+7)>0.5f){lfos[(size_t)j].phase=param(b+9);lfos[(size_t)j].age=0;lfos[(size_t)j].delayAge=0;lfos[(size_t)j].active=true;}
  }
 }
 filterEnvelope.noteOn(integer(param(4))>=2);
}
void BiscotProcessor::noteOff(int channel,int note){
 for(int i=0;i<3;++i){auto& v=voices[(size_t)i];
  if(v.gate && v.note==note && (integer(param(3))!=2 || channel==v.channel)){
   v.gate=false;v.amp.noteOff(param(43+i),sampleRateHz);v.mod.noteOff(param(73+i),sampleRateHz);
  }
 }
 bool any=false;for(const auto& v:voices)any|=v.gate;
 if(!any)filterEnvelope.noteOff(param(121),sampleRateHz);
}
void BiscotProcessor::rebuildArpPools(){
 const int mode=integer(param(3));
 for(int i=0;i<3;++i){
  auto& pool=arpPools[(size_t)i];int count=0;
  std::array<std::pair<std::uint64_t,int>,640> ordered{};
  for(int ch=0;ch<16;++ch){
   if(mode==2 && ch!=i)continue;
   for(int note=0;note<128;++note){
    const auto sequence=pressed[(size_t)ch][(size_t)note];
    if(sequence==0)continue;
    const int low=integer(param(85+i)),high=integer(param(82+i));
    for(int oct=low;oct<=high && count<(int)pool.size();++oct){
     const int n=note+oct*12;
     if(n>=0 && n<=127)ordered[(size_t)count++]={sequence,n};
    }
   }
  }
  const int arpMode=integer(param(76+i));
  if(arpMode==5 || arpMode==1){
   std::sort(ordered.begin(),ordered.begin()+count,[](const auto& a,const auto& b){
    return a.first==b.first?a.second<b.second:a.first<b.first;
   });
  }else{
   std::sort(ordered.begin(),ordered.begin()+count,[](const auto& a,const auto& b){return a.second<b.second;});
  }
  for(int n=0;n<count;++n)pool[(size_t)n]=ordered[(size_t)n].second;
  arpPoolCounts[(size_t)i]=count;
 }
}
void BiscotProcessor::onMidi(const juce::MidiMessage& m){
 const int ch=m.getChannel(), mode=integer(param(3));
 if(m.isAllNotesOff()||m.isAllSoundOff()){
  for(auto& row:pressed)row.fill(0);rebuildArpPools();
  for(int i=0;i<3;++i){auto& v=voices[(size_t)i];v.gate=false;
   v.amp.noteOff(param(43+i),sampleRateHz);v.mod.noteOff(param(73+i),sampleRateHz);}
  filterEnvelope.noteOff(param(121),sampleRateHz);return;
 }
 if(m.isPitchWheel()){
  const float bend=limit(((float)m.getPitchWheelValue()-8192.f)/8192.f,-1.f,1.f);
  if(mode==0 && ch==1)for(auto& v:voices)v.bend=bend;
  else if(ch>=1 && ch<=3)voices[(size_t)(ch-1)].bend=bend;
  return;
 }
 if(m.isController()){
  if(m.getControllerNumber()==123){onMidi(juce::MidiMessage::allNotesOff(ch));}return;
 }
 if(m.isNoteOn()){
  const int note=m.getNoteNumber();pressed[(size_t)(ch-1)][(size_t)note]=++nextPress;rebuildArpPools();
  const float velocity=m.getFloatVelocity();
  if(mode==0){for(int i=0;i<3;++i){voices[(size_t)i].vel=velocity;noteOn(i,note,ch);}return;}
  if(mode==2){if(ch<=3){voices[(size_t)(ch-1)].vel=velocity;noteOn(ch-1,note,ch);}return;}
  // Chord allocation: one MIDI note per operator, reuse released voices first.
  for(const auto& v:voices)if(v.gate && v.note==note)return;
  int index=-1;
  for(int i=0;i<3;++i)if(!voices[(size_t)i].gate){index=i;break;}
  if(index<0)index=0;
  voices[(size_t)index].vel=velocity;noteOn(index,note,ch);return;
 }
 if(m.isNoteOff()){
  pressed[(size_t)(ch-1)][(size_t)m.getNoteNumber()]=0;rebuildArpPools();
  if(mode!=2)noteOff(ch,m.getNoteNumber());
  else if(ch<=3)noteOff(ch,m.getNoteNumber());
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
 for(int k=0;k<3;++k)if(integer(param(58+k))==i+1)oct+=voices[(size_t)k].mod.level*param(61+k)*4.;
 const int source=integer(param(220+i*2));
 if(source>0 && source<=3)oct+=prev[(size_t)(source-1)]*param(221+i*2)*4.;
 const float rate=param(b+1);
 const double hz=std::max(0.001,tempoBpm)/240. *std::max(0.125f,rate)*std::pow(2.,juce::jlimit(-8.,8.,oct));
 const double inc=std::min(0.45,hz/sampleRateHz);
 const double oldPhase=l.phase;
 if(l.active){l.phase+=inc;if(l.phase>=1.)l.phase-=std::floor(l.phase);l.age+=inc;l.delayAge+=1./sampleRateHz;}
 const int wave=integer(param(b+2));
 if(l.phase<oldPhase || ((wave==5||wave==6) && oldPhase<0.5 && l.phase>=0.5))l.hold=randomUnit(l.seed);
 float raw=0.f;const double p=l.phase;
 switch(wave){
  case 0: raw=(float)std::sin(2*pi*p);break;
  case 1: raw=(float)(4*std::abs(p-0.5)-1);break;
  case 2: raw=(float)(2*p-1);break;
  case 3: raw=(float)(1-2*p);break;
  case 4: raw=p<0.5?1.f:-1.f;break;
  default:raw=l.hold;break;
 }
 if(param(b+7)>0.5f && l.age>=limit(param(b+8),0.f,1.f))l.active=false;
 if(!l.active)raw=0;
 const float delay=param(b+5);
 raw*=delay<=0?1.f:limit((float)(l.delayAge/delay),0.f,1.f);
 raw*=raw>=0?(1.f-param(b+3)):(1.f-param(b+4));
 const float smooth=std::max(param(b+6),wave==6?0.12f:0.f);
 const float coeff=smooth<=0?1.f:1.f-std::exp(-1.f/(float)(sampleRateHz*(0.001+0.249*smooth*smooth)));
 l.smoothed+=(raw-l.smoothed)*coeff;
 l.output=l.smoothed;return l.output;
}
float BiscotProcessor::processFilter(float input,float registerCutoff,int channel,float resonanceMod){
 return (float)filters[(size_t)channel].process(input,registerCutoff,
   param(109)+resonanceMod,sampleRateHz,param(5)>0.5f,
   param(110)>0.5f,param(111)>0.5f,param(112)>0.5f);
}
void BiscotProcessor::renderOne(float& left,float& right,std::array<float,8>& stems){
 // All parameter groups are used by this intermediate DSP. Exact JSFX parity,
 // wavetable AA/oversampling, SID-accurate filter, and SID Seq ARP remain future tasks.
 std::array<float,3> amp{},mod{},lfo{},previous{},pitchMod{},pwmMod{},levelMod{},panMod{},oscOut{};
 float cutoffMod=0,resMod=0;
 for(int i=0;i<3;++i){auto& v=voices[(size_t)i];
  amp[(size_t)i]=v.amp.tick(param(34+i),param(37+i),param(40+i),sampleRateHz);
  mod[(size_t)i]=v.mod.tick(param(64+i),param(67+i),param(70+i),sampleRateHz);
  v.noteSmooth=v.noteSmooth<0 && v.note>=0?v.note:v.noteSmooth;
 }
 const float filterEnv=filterEnvelope.tick(param(118),param(119),param(120),sampleRateHz);
 for(int i=0;i<3;++i)previous[(size_t)i]=lfos[(size_t)i].previous;
 for(int i=0;i<3;++i)lfo[(size_t)i]=processLfo(i,previous);
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
 // Per-operator arpeggiator, driven by the host BPM and held MIDI keys.
 for(int i=0;i<3;++i){
  auto& v=voices[(size_t)i];const int arpMode=integer(param(76+i));
  if(arpMode>0 && v.note>=0){
   const auto& pool=arpPools[(size_t)i];const int count=arpPoolCounts[(size_t)i];
   if(count>0){
    v.arpPhase+=tempoBpm/240.*std::max(0.125f,param(79+i))/sampleRateHz;
    if(v.arpPhase>=1.){
     v.arpPhase-=std::floor(v.arpPhase);v.arpStep++;
     if(arpMode==6)randomUnit(v.noiseSeed);
    }
    int pos=v.arpStep%count;
    if(arpMode==3)pos=count-1-pos;
    else if(arpMode==4){
     const int cycle=std::max(1,2*count-2);
     pos=v.arpStep%cycle;if(pos>=count)pos=cycle-pos;
    }
    else if(arpMode==6)pos=(int)(v.noiseSeed%(uint32_t)count);
    const float destination=(float)pool[(size_t)pos]-(float)v.note;
    const float glide=param(88+i);
    v.arpPitch+=(destination-v.arpPitch)*(glide<=0.f?1.f:limit(1.f/(float)(sampleRateHz*(0.002+glide*0.3)),0.0001f,1.f));
   }
  }else v.arpPitch=0;
 }
 std::array<double,3> frequencies{};
 for(int i=0;i<3;++i){auto& v=voices[(size_t)i];
  if(v.note<0 && v.amp.stage==0 && v.mod.stage==0){frequencies[(size_t)i]=0;continue;}
  if(v.note>=0){const float port=param(46+i);
   const double smoothing=port<=0?1.:1.-std::exp(-1./(sampleRateHz*port));
   v.noteSmooth+=(v.note-v.noteSmooth)*smoothing;
  }
  const float pitch=(float)v.noteSmooth+param(25+i)+param(28+i)/100.f+v.bend*param(91+i)+v.drift
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
     0.5f+param(31+i)/128.f+pwmMod[(size_t)i]+mod[(size_t)i]*param(55+i),0.f,1.f);
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
  const float level=limit(param(16+i)*(1.f+levelMod[(size_t)i]),0.f,2.f);
  oscOut[(size_t)i]=(float)wave*amp[(size_t)i]*level;
  xfade[(size_t)i]=limit(param(19+i)+mod[(size_t)i]*param(49+i)+panMod[(size_t)i],-1.f,1.f);
 }
 float filterInput[2]{},dry[2]{};
 const float filterInputGain=gain(param(6));
 for(int i=0;i<3;++i){
  const float x=oscOut[(size_t)i];
  // JSFX center pan is unity in BOTH channels. Previous equal-power pan
  // attenuated every centered voice by -3dB and changed the filter drive.
  const float pan=xfade[(size_t)i];
  const float l=x*(1.f-std::max(0.f,pan));
  const float r=x*(1.f+std::min(0.f,pan));
  if(param(114+i)>0.5f){filterInput[0]+=l*filterInputGain;filterInput[1]+=r*filterInputGain;}
  else if(i!=2 || param(103)<0.5f){
   dry[0]+=l;dry[1]+=r;
   stems[(size_t)i*2]=l;stems[(size_t)i*2+1]=r;
  }
 }
 float keyFollow=0;
 for(const auto& v:voices)if(v.gate){keyFollow=(v.note-60.f)*param(113)*32.f;break;}
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
void BiscotProcessor::processBlock(juce::AudioBuffer<float>& buffer,juce::MidiBuffer& midi){
 juce::ScopedNoDenormals denormals;
 buffer.clear();
 rebuildArpPools(); // refresh octave/routing changes once per block, never per sample
 if(auto* ph=getPlayHead())if(auto position=ph->getPosition()){
  if(auto bpm=position->getBpm())tempoBpm=juce::jlimit(1.,999.,*bpm);
  // JUCE 8 getIsPlaying() returns bool, not std::optional<bool>.
  // Restart free-running LFO phases only on the stopped -> playing edge.
  const bool running=position->getIsPlaying();
  if(running && !lastPlaying){
   for(int i=0;i<3;++i)if(param(122+i*23)<0.5f){lfos[(size_t)i].phase=param(131+i*23);lfos[(size_t)i].age=0;}
  }
  lastPlaying=running;
 }
 // Resolve bus views once per block: never allocate/copy bus metadata in @sample.
 auto mix=getBusBuffer(buffer,false,0);
 auto op1=getBusBuffer(buffer,false,1);
 auto op2=getBusBuffer(buffer,false,2);
 auto op3=getBusBuffer(buffer,false,3);
 auto filterOut=getBusBuffer(buffer,false,4);
 juce::MidiBuffer::Iterator it(midi);juce::MidiMessage message;int offset=0;bool more=it.getNextEvent(message,offset);
 for(int sample=0;sample<buffer.getNumSamples();++sample){
  while(more && offset<=sample){onMidi(message);more=it.getNextEvent(message,offset);}
  float left=0,right=0;std::array<float,8> stems{};
  renderOne(left,right,stems);
  if(mix.getNumChannels()>0)mix.setSample(0,sample,left);
  if(mix.getNumChannels()>1)mix.setSample(1,sample,right);
  if(param(2)>0.5f){
   auto put=[sample](juce::AudioBuffer<float>& out,float l,float r){
    if(out.getNumChannels()>0)out.setSample(0,sample,l);
    if(out.getNumChannels()>1)out.setSample(1,sample,r);
   };
   put(op1,stems[0],stems[1]);put(op2,stems[2],stems[3]);
   put(op3,stems[4],stems[5]);put(filterOut,stems[6],stems[7]);
  }
 }
}
void BiscotProcessor::getStateInformation(juce::MemoryBlock& out){if(auto xml=state.copyState().createXml())copyXmlToBinary(*xml,out);}
void BiscotProcessor::setStateInformation(const void* ptr,int length){
 if(auto xml=getXmlFromBinary(ptr,length);xml && xml->hasTagName(state.state.getType()))state.replaceState(juce::ValueTree::fromXml(*xml));
}
juce::AudioProcessorEditor* BiscotProcessor::createEditor(){return new BiscotEditor(*this);}
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new biscot::BiscotProcessor();}
