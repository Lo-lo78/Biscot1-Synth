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
 for(int i=0;i<3;++i){voices[(size_t)i].noiseSeed^=(uint32_t)(i*0x9e3779b9u);lfos[(size_t)i].seed^=(uint32_t)(i*0x51ed270bu);lfos[(size_t)i].phase=param(131+23*i);}
 filterEnvelope={};lastPlaying=false;
}
float BiscotProcessor::Envelope::tick(float a,float d,float s,double sr){
 s=limit(s,0.f,1.f);
 if(stage==1){level+=a<=0?1.f:1.f/(float)(a*sr);if(level>=1.f){level=1;stage=2;}}
 else if(stage==2){level-=d<=0?1.f:(1.f-s)/(float)(d*sr);if(level<=s){level=s;stage=3;}}
 else if(stage==3){level=s;}
 else if(stage==4){level-=releaseStep;if(level<=0){level=0;stage=0;}}
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
float BiscotProcessor::processFilter(float input,float cutoff,int channel,float resonanceMod){
 auto& f=filters[(size_t)channel];
 // Topology-preserving SVF approximation, NOT exact SID 6581/8580 analogue circuit.
 const float nyquist=(float)sampleRateHz*0.42f;
 const float frequency=limit(cutoff,5.f,nyquist);
 const float g=std::tan((float)pi*frequency/(float)sampleRateHz);
 const float res=limit((param(109)+resonanceMod)/15.f,0.f,1.f);
 const float damping=1.75f-1.55f*res;
 const float denom=1.f+g*(g+damping);
 const float hp=(input-(damping+g)*f.ic1-f.ic2)/denom;
 const float bp=g*hp+f.ic1;
 const float lp=g*bp+f.ic2;
 f.ic1=g*hp+bp;f.ic2=g*bp+lp;
 float mix=0;
 if(param(110)>0.5f)mix+=lp;
 if(param(111)>0.5f)mix+=bp;
 if(param(112)>0.5f)mix+=hp;
 if(param(110)<0.5f && param(111)<0.5f && param(112)<0.5f)mix=input;
 if(param(5)>0.5f)mix=saturate(mix*1.35f)*1.2f;
 return mix;
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
  route(integer(param(envTarget)),amp[(size_t)i]*param(envTarget+1));
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
 for(int i=0;i<3;++i){auto& v=voices[(size_t)i];
  v.oldPhase=v.phase;
  if(param(100+i)>0.5f)v.phase=0;
  else {v.phase+=frequencies[(size_t)i]/sampleRateHz;v.phase-=std::floor(v.phase);}
 }
 for(int i=0;i<3;++i){
  const int modulator=(i+2)%3;
  auto& v=voices[(size_t)i];const auto& source=voices[(size_t)modulator];
  if(sync[(size_t)i] && source.oldPhase<0.5 && source.phase>=0.5)v.phase=0;
  const double p=v.phase,dt=std::max(1.e-7,frequencies[(size_t)i]/sampleRateHz);
  const float pwm=limit(0.5f+param(31+i)/128.f+pwmMod[(size_t)i]+mod[(size_t)i]*param(55+i)*0.5f,0.f,1.f);
  const float tri=(float)(1.-4.*std::abs(p-.5))*(ring[(size_t)i]?(source.phase<0.5?1.f:-1.f):1.f);
  const float saw=(float)(2.*p-1.)-polyBlep(p,dt);
  const float pul=pulse(p,pwm,dt);
  const float noise=(v.noiseSeed&0x8000u)?1.f:-1.f;
  if(v.phase<v.oldPhase){
   const float oldNoise=noise;
   const float rand=randomUnit(v.noiseSeed);
   const float mix=param(9);v.noise=rand*mix+oldNoise*(1.f-mix);
  }
  float wave=0;switch(integer(param(22+i))){
   case 0:wave=tri;break;case 1:wave=saw;break;case 2:wave=pul;break;
   case 3:wave=v.noise;break;
   case 4:wave=0.48f*tri+0.52f*saw+0.25f*tri*saw;break;
   case 5:wave=0.6f*tri+0.4f*pul;break;
   case 6:wave=0.5f*saw+0.5f*pul;break;
   case 7:wave=(tri+saw+pul+tri*saw*0.2f)/3.f;break;
  }
  const float previousOsc=wave*amp[(size_t)i];
  // Audio-rate routes read previous sample, never current (no algebraic loops).
  v.oscPrev=previousOsc;
  const float level=limit(param(16+i)*(1.f+levelMod[(size_t)i]),0.f,2.f);
  oscOut[(size_t)i]=wave*amp[(size_t)i]*level*v.vel;
  xfade[(size_t)i]=limit(param(19+i)+mod[(size_t)i]*param(49+i)+panMod[(size_t)i],-1.f,1.f);
 }
 float filterInput[2]{},dry[2]{};
 const float filterInputGain=gain(param(6));
 for(int i=0;i<3;++i){
  const float x=oscOut[(size_t)i];
  const float l=x*std::sqrt((1.f-xfade[(size_t)i])*0.5f);
  const float r=x*std::sqrt((1.f+xfade[(size_t)i])*0.5f);
  stems[(size_t)i*2]=l;stems[(size_t)i*2+1]=r;
  if(param(114+i)>0.5f){filterInput[0]+=l*filterInputGain;filterInput[1]+=r*filterInputGain;}
  else if(i!=2 || param(103)<0.5f){dry[0]+=l;dry[1]+=r;}
 }
 float keyFollow=0;
 for(const auto& v:voices)if(v.gate){keyFollow=(v.note-60.f)*param(113)*14.f;break;}
 const float filterRegister=limit(param(108)+filterEnv*param(117)+cutoffMod+keyFollow,0.f,2047.f);
 const float norm=filterRegister/2047.f;
 const float cutoff=param(5)>0.5f?4.f+std::pow(norm,1.38f)*12500.f:28.f+std::pow(norm,2.12f)*8170.f;
 const float filterGain=gain(param(7));
 const float masterGain=gain(param(8));
 const float maxGain=2.5f;
 for(int ch=0;ch<2;++ch){
  const float filtered=processFilter(filterInput[ch],cutoff,ch,resMod)*filterGain;
  stems[(size_t)6+ch]=filtered;
  float out=(dry[ch]+filtered)*masterGain;
  // C64 output analogue RC and DC blocker; no hard PWM-endpoint mute.
  auto& st=filters[(size_t)ch];
  const float alpha=1.f-std::exp(-2.f*(float)pi*std::min(16000.f,(float)sampleRateHz*0.45f)/(float)sampleRateHz);
  st.outLow+=(out-st.outLow)*alpha;
  const float dcCoef=std::exp(-2.f*(float)pi*1.6f/(float)sampleRateHz);
  const float y=st.outLow-st.prev+dcCoef*st.dc;
  st.prev=st.outLow;st.dc=y;
  out=saturate(y/maxGain)*maxGain;
  if(ch==0)left=out;else right=out;
 }
}
void BiscotProcessor::processBlock(juce::AudioBuffer<float>& buffer,juce::MidiBuffer& midi){
 juce::ScopedNoDenormals denormals;
 buffer.clear();
 rebuildArpPools(); // refresh octave/routing changes once per block, never per sample
 if(auto* ph=getPlayHead())if(auto position=ph->getPosition()){
  if(auto bpm=position->getBpm())tempoBpm=juce::jlimit(1.,999.,*bpm);
  if(auto running=position->getIsPlaying()){
   if(*running && !lastPlaying){
    for(int i=0;i<3;++i)if(param(122+i*23)<0.5f){lfos[(size_t)i].phase=param(131+i*23);lfos[(size_t)i].age=0;}
   }
   lastPlaying=*running;
  }
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
