// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include "TrackerSongPlayback.h"
#include <algorithm>
#include <cmath>
#include <vector>

namespace biscot {
// Native per-operator Song FX. No allocation in the audio callback.
// Delay time is host-BPM synchronized: speed 1 = one quarter note,
// 2 = eighth, .5 = half note. Maximum history eight seconds.
// This is the native interpretation of legacy Tracker DSP events; the
// JSFX core's queue/gmem transport is intentionally not used by this VST3.
class TrackerSongFxCore {
public:
 int sidechainSource=0;
 float sidechainAmount=0.f;
 float delaySpeed=0.f,feedback=0.f,volume=1.f,pingpong=0.f;
 void prepare(double sr){
  sampleRate=std::max(1.,sr);
  const size_t capacity=std::max((size_t)2,(size_t)std::ceil(sampleRate*8.)+2);
  left.assign(capacity,0.f);right.assign(capacity,0.f);
  index=0;validSamples=0;tailTicks=0;duck=1.f;
  attack=(float)(1.-std::exp(-1./(sampleRate*0.005)));
  release=(float)(1.-std::exp(-1./(sampleRate*0.080)));
  sidechainSource=0;sidechainAmount=delaySpeed=feedback=pingpong=0.f;volume=1.f;
 }
 void clearDelay()noexcept{
  // Clear is for stopped/idle reset only, never on a Song chain transition.
  // Logical clear avoids clearing megabytes in a real-time audio callback.
  index=0;validSamples=0;tailTicks=0;
 }
 void resetSidechain()noexcept{sidechainSource=0;sidechainAmount=0.f;duck=1.f;}
 void setRow(const TrackerSongRowFx& fx)noexcept{
  if(fx.present[15])sidechainSource=std::clamp((int)std::lround(fx.value[15]),0,3);
  if(fx.present[16])sidechainAmount=(float)std::clamp(fx.value[16]/100.,0.,1.);
  // Explicit Source Off or Amount 0 resets both controls in JSFX 0.166.
  if((fx.present[15]&&sidechainSource==0)||(fx.present[16]&&sidechainAmount<=0))
   resetSidechain();
  if(fx.present[17])delaySpeed=(float)std::clamp(fx.value[17],0.,8.);
  if(fx.present[18])feedback=(float)std::clamp(fx.value[18]/100.,0.,0.96);
  if(fx.present[19])volume=(float)std::clamp(fx.value[19]/100.,0.,1.);
  if(fx.present[20])pingpong=(float)std::clamp(fx.value[20]/100.,0.,1.);
  if(fx.present[17]&&delaySpeed<=0.f)tailTicks=0;
 }
 float duckGain(float sourceEnvelope)noexcept{
  if(sidechainSource<=0||sidechainAmount<=0.f)return 1.f;
  const float wanted=1.f-sidechainAmount*std::clamp(sourceEnvelope,0.f,1.f);
  // Fast attack and 80 ms release: deterministic sample-rate-aware envelope.
  duck+=(wanted-duck)*(wanted<duck?attack:release);
  return duck;
 }
 void process(float inputL,float inputR,float& outL,float& outR,double bpm)noexcept{
  outL=inputL;outR=inputR;
  if(left.empty() || (delaySpeed<=0.f && tailTicks<=0))return;
  const size_t count=left.size();
  if(cachedBpm!=bpm || cachedSpeed!=delaySpeed){
   cachedBpm=bpm;cachedSpeed=delaySpeed;
   cachedSamples=std::clamp((size_t)std::lround(sampleRate*60./std::max(1.,bpm)/std::max(0.125f,delaySpeed)),(size_t)1,count-1);
  }
  const size_t samples=cachedSamples;
  const size_t read=index>=samples?index-samples:index+count-samples;
  const bool reached=validSamples>=samples;
  const float dl=reached?left[read]:0.f,dr=reached?right[read]:0.f;
  const float cross=pingpong;
  const float fedL=dl*(1.f-cross)+dr*cross;
  const float fedR=dr*(1.f-cross)+dl*cross;
  // Internal feedback cap guards against runaway energy on long note chains.
  // A centered synth voice has identical L and R samples. Swapping their
  // feedback alone cannot create audible Ping Pong. As depth approaches 100%,
  // feed the mono sum into the left delay input only, then cross-feed later
  // repeats to the right. Keep the dry signal and depth=0 stereo behaviour.
  const float mono=0.5f*(inputL+inputR);
  const float injectedL=inputL*(1.f-cross)+mono*cross;
  const float injectedR=inputR*(1.f-cross);
  left[index]=std::clamp(injectedL+feedback*fedL,-4.f,4.f);
  right[index]=std::clamp(injectedR+feedback*fedR,-4.f,4.f);
  if(++index>=count)index=0;
  if(validSamples<count)++validSamples;
  outL+=dl*volume;outR+=dr*volume;
  const float peak=std::max({std::abs(inputL),std::abs(inputR),std::abs(dl),std::abs(dr)});
  if(peak>0.000015f)tailTicks=std::max(tailTicks,(int)(samples+sampleRate*0.02));
  else if(tailTicks>0)--tailTicks;
 }
 bool hasTail()const noexcept{return tailTicks>0 && delaySpeed>0.f;}
private:
 std::vector<float> left,right;
 size_t index=0;
 size_t validSamples=0;
 int tailTicks=0;
 float duck=1.f,attack=0.001f,release=0.001f;
 double sampleRate=44100.;
 double cachedBpm=-1.;float cachedSpeed=-1.f;size_t cachedSamples=1;
};
} // namespace biscot
