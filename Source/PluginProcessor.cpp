#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>
namespace biscot {
static constexpr double pi=3.14159265358979323846;
static float clampf(float x,float lo,float hi){return juce::jlimit(lo,hi,x);}
juce::AudioProcessorValueTreeState::ParameterLayout BiscotProcessor::createLayout(){
 juce::AudioProcessorValueTreeState::ParameterLayout l;
 for(const auto& d:catalog){
   juce::NormalisableRange<float> range(d.low,d.high,d.step>0?d.step:0.001f);
   l.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{id(d.index),1},d.name,range,d.initial));
 }
 return l;
}
BiscotProcessor::BiscotProcessor():AudioProcessor(BusesProperties().withOutput("Mix",juce::AudioChannelSet::stereo(),true)
 .withOutput("Op1",juce::AudioChannelSet::stereo(),false)
 .withOutput("Op2",juce::AudioChannelSet::stereo(),false)
 .withOutput("Op3",juce::AudioChannelSet::stereo(),false)
 .withOutput("Filter",juce::AudioChannelSet::stereo(),false)),state(*this,nullptr,"BISCOT1",createLayout()){
 for(const auto& d:catalog) parameters[(size_t)d.index]=state.getRawParameterValue(id(d.index));
}
bool BiscotProcessor::isBusesLayoutSupported(const BusesLayout& l) const {
 if(l.outputBuses.size()!=5 || !l.inputBuses.isEmpty()) return false;
 for(int i=0;i<5;++i){auto a=l.outputBuses[(size_t)i]; if(i==0 && a!=juce::AudioChannelSet::stereo()) return false;
 if(i>0 && a!=juce::AudioChannelSet::stereo() && !a.isDisabled())return false;}
 return true;
}
void BiscotProcessor::prepareToPlay(double sr,int){sampleRateHz=sr>0?sr:44100.;filterLpL=filterLpR=dcL=dcR=prevInL=prevInR=0;voices={};}
void BiscotProcessor::onMidi(const juce::MidiMessage& m){
 const int ch=m.getChannel(),mode=juce::roundToInt(param(3));
 if(m.isAllNotesOff()||m.isAllSoundOff()){for(auto& v:voices){v.gate=false;v.stage=4;}return;}
 if(m.isPitchWheel()){for(int i=0;i<3;++i)if((mode==0 && ch==1)||(mode!=0 && ch==i+1))voices[(size_t)i].bend=(m.getPitchWheelValue()-8192)/8192.f;return;}
 if(m.isNoteOn()){
  int n=m.getNoteNumber();
  if(mode==0){for(auto& v:voices){v.note=n;v.channel=ch;v.gate=true;v.stage=1;v.env=0;}return;}
  if(mode==2){if(ch<=3){auto& v=voices[(size_t)(ch-1)];v.note=n;v.channel=ch;v.gate=true;v.stage=1;v.env=0;}return;}
  for(auto& v:voices)if(v.note==n && v.gate)return;
  for(auto& v:voices)if(!v.gate){v.note=n;v.channel=ch;v.gate=true;v.stage=1;v.env=0;return;}
  return;
 }
 if(m.isNoteOff()){
  for(auto& v:voices)if(v.gate && v.note==m.getNoteNumber() && (mode!=2 || v.channel==ch)){v.gate=false;v.stage=4;}
 }
}
void BiscotProcessor::renderOne(float& left,float& right,std::array<float,6>& stems){
 // Initial native-audio implementation only: oscillator, basic ADSR, three MIDI modes,
 // per-operator bend/wave/PWM/level/pan, approximate shared LP filter.
 // NOT the final v2.38 oscillator/filter/modulation/AA port.
 float mono[3]{}, pan[3]{};
 for(int i=0;i<3;++i){
  auto& v=voices[(size_t)i]; const int base=i;
  float a=param(34+base),d=param(37+base),s=param(40+base),r=param(43+base);
  if(v.stage==1){v.env+=1.0/(sampleRateHz*std::max(0.001f,a));if(v.env>=1){v.env=1;v.stage=2;}}
  else if(v.stage==2){v.env-=(1.0-s)/(sampleRateHz*std::max(0.001f,d));if(v.env<=s){v.env=s;v.stage=3;}}
  else if(v.stage==3)v.env=s;
  else if(v.stage==4){v.env-=1.0/(sampleRateHz*std::max(0.001f,r));if(v.env<=0){v.env=0;v.stage=0;v.note=-1;}}
  if(v.note<0)continue;
  float bendRange=param(91+base), semis=param(25+base)+0.01f*param(28+base)+v.bend*bendRange;
  double f=440.*std::pow(2.,((double)v.note-69.+semis)/12.);
  v.phase+=f/sampleRateHz;v.phase-=std::floor(v.phase);
  double p=v.phase;
  float tri=(float)(1.-4.*std::abs(p-0.5));
  float saw=(float)(2.*p-1.);
  float pulse=(float)(p<(param(31+base)+64.f)/128.f?1.:-1.);
  // DC rejection after output allows natural pulse endpoints rather than forced oscillator mute.
  int wave=juce::roundToInt(param(22+base));
  float x=wave==0?tri:wave==1?saw:wave==2?pulse:wave==3?(float)(std::sin(p*12781.3)*std::sin(p*5870.1)):
    wave==4?0.5f*(tri+saw):wave==5?0.5f*(tri+pulse):wave==6?0.5f*(saw+pulse):(tri+saw+pulse)/3.f;
  mono[i]=x*(float)v.env*param(16+base);
  pan[i]=param(19+base);
 }
 float dryL=0,dryR=0,fltInL=0,fltInR=0;
 for(int i=0;i<3;++i){
  float x=mono[i]; if(i==2 && param(101)>0.5f && param(114)<0.5f)x=0;
  float l=x*std::sqrt((1.f-pan[i])*0.5f),rr=x*std::sqrt((1.f+pan[i])*0.5f);
  stems[(size_t)i*2]=l;stems[(size_t)i*2+1]=rr;
  if(param(112+i)>0.5f){fltInL+=l;fltInR+=rr;}else{dryL+=l;dryR+=rr;}
 }
 // Deliberately simplified filter: the 6581/8580 nonlinear filters are NOT migrated yet.
 float cut=clampf(param(106),20.f,18000.f);
 float g=1.f-std::exp(-2.f*(float)pi*cut/(float)sampleRateHz);
 filterLpL+=g*(fltInL-filterLpL);filterLpR+=g*(fltInR-filterLpR);
 float filteredL=filterLpL,filteredR=filterLpR;
 if(param(108)<0.5f){filteredL=fltInL;filteredR=fltInR;}
 left=(dryL+filteredL)*std::pow(10.f,param(8)/20.f);
 right=(dryR+filteredR)*std::pow(10.f,param(8)/20.f);
 // Lightweight final DC-blocker.
 float dl=left-prevInL+0.995f*dcL,dr=right-prevInR+0.995f*dcR;
 prevInL=left;prevInR=right;dcL=dl;dcR=dr;left=dl;right=dr;
 stems[0]=stems[0];
}
void BiscotProcessor::processBlock(juce::AudioBuffer<float>& buffer,juce::MidiBuffer& midi){
 juce::ScopedNoDenormals noDenormals;
 buffer.clear();
 juce::MidiBuffer::Iterator it(midi);juce::MidiMessage msg;int offset=0;bool more=it.getNextEvent(msg,offset);
 for(int sample=0;sample<buffer.getNumSamples();++sample){
  while(more && offset<=sample){onMidi(msg);more=it.getNextEvent(msg,offset);}
  float l=0,r=0;std::array<float,6> stems{};renderOne(l,r,stems);
  if(buffer.getNumChannels()>0)buffer.setSample(0,sample,l);
  if(buffer.getNumChannels()>1)buffer.setSample(1,sample,r);
  // Non-mix buses are opt-in and currently muted until verified parity.
 }
}
void BiscotProcessor::getStateInformation(juce::MemoryBlock& dest){auto xml=state.copyState().createXml();copyXmlToBinary(*xml,dest);}
void BiscotProcessor::setStateInformation(const void* p,int size){auto xml=getXmlFromBinary(p,size);if(xml && xml->hasTagName(state.state.getType()))state.replaceState(juce::ValueTree::fromXml(*xml));}
juce::AudioProcessorEditor* BiscotProcessor::createEditor(){return new BiscotEditor(*this);}
} // namespace biscot
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new biscot::BiscotProcessor();}
