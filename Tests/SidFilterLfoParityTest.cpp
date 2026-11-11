#include "../Source/SidLfoCore.h"
#include "../Source/SidFilterLfoTrigger.h"
#include "../Source/SidAmpEnvelope.h"
#include "../Source/SidMidiNoteState.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>

// JSFX v2.38 lfo_process() expressed independently in C++ as a numerical
// reference. No production helper is used by this reference implementation.
struct RefLfo {
 double phase=0, age=2, delay=1000000, seed=7654321, sh=0, smooth=0, out=0;
 bool active=true;
 void start(bool mode,bool oneShot,double ph){
  delay=0;
  if(mode||oneShot){phase=ph-std::floor(ph);age=0;}
  active=true;
 }
 double sample(double increment, int wave, double delaySeconds, double smoothAmount,
               bool oneShot,double length,double topSquash,double bottomSquash,double sr){
  if(!active&&!oneShot)active=true;
  if(active){
   delay+=1/sr;
   double before=phase;
   phase+=increment;
   if(phase>=1)phase-=std::floor(phase);
   bool wrap=(phase<before),mid=(before<0.5&&phase>=0.5);
   if(wrap||((wave==5||wave==6)&&mid)){
    seed=std::fmod(seed*1103515245+12345,2147483647.);
    sh=seed/1073741823.5-1;
   }
   age+=increment;
   if(oneShot&&age>=std::clamp(length,0.,1.))active=false;
  }
  double raw=0.;
  if(active){
   if(wave==0)raw=std::sin(6.2831853071795864769*phase);
   else if(wave==1)raw=4*std::abs(phase-0.5)-1;
   else if(wave==2)raw=2*phase-1;
   else if(wave==3)raw=1-2*phase;
   else if(wave==4)raw=phase<0.5?1:-1;
   else raw=sh;
   if(delaySeconds>0)raw*=std::min(1.,delay/std::max(0.000001,delaySeconds));
  }
  double value=std::clamp(smoothAmount,0.,1.);
  if(wave==6)value=std::max(value,0.12);
  double factor=value<=0?1:1-std::exp(-1/std::max(1.,(0.001+value*value*0.249)*sr));
  if(factor>=1)smooth=raw;
  else smooth+=(raw-smooth)*factor;
  out=smooth*(1-std::clamp(smooth>=0?topSquash:bottomSquash,0.,1.));
  return out;
 }
};

static void check(bool c,const char* m){if(!c){std::fprintf(stderr,"FAIL: %s\n",m);std::exit(1);}}
static void near(double a,double b,double tolerance,const char* m){if(std::abs(a-b)>tolerance){std::fprintf(stderr,"FAIL %s: %.12f != %.12f\n",m,a,b);std::exit(2);}}

int main(){
 using namespace biscot;
 constexpr double sr=48000;
 for(int index=0;index<3;++index){
  SidLfoCore core;core.reset(index);
  RefLfo ref;ref.seed=index==0?7654321.:(index==1?8765432.:9876543.);
  check(core.age==2&&core.phase==0&&core.delayAge==1000000,"cold LFO initial state");
  for(int t=0;t<9000;++t){
   const int wave=(t/1300+index)%7;
   const bool oneShot=t>=3200 && t<4900;
   const float len=0.30f;
   const double inc=t<6200?0.0003:0.0008;
   const double delay=t>=2100?0.35:0.;
   const double smooth=t<1800?0.:0.68;
   if(t==2100){core.noteTrigger(false,false,0.6);ref.start(false,false,0.6);check(core.phase==ref.phase,"free mode trigger keeps phase");}
   if(t==3200){core.noteTrigger(false,true,0.3);ref.start(false,true,0.3);}
   if(t==6200){core.noteTrigger(true,false,0.8);ref.start(true,false,0.8);}
   const auto a=core.tick(inc,wave,delay,smooth,oneShot,len,0.2,0.6,sr);
   const auto b=ref.sample(inc,wave,delay,smooth,oneShot,len,0.2,0.6,sr);
   near(a,b,2.5e-6,"LFO JSFX sample parity");
   near(core.phase,ref.phase,1e-10,"LFO phase parity");
   near(core.delayAge,ref.delay,1e-8,"LFO Delay parity");
   check(core.active==ref.active,"LFO One Shot state parity");
  }
 }
 // Verify that a stopped One Shot resumes in free mode with no MIDI trigger.
 SidLfoCore shot;shot.reset(0);shot.noteTrigger(true,true,0.);
 for(int j=0;j<40;++j)shot.tick(0.01,0,0,0,true,0.25,0,0,sr);
 check(!shot.active,"One Shot stops after configured cycle fraction");
 shot.tick(0.01,0,0,0,false,0.25,0,0,sr);
 check(shot.active,"One Shot Off resumes free mode");
 // Filter envelope and LFO trigger events share the Op1 Trigger/Legato policy.
 SidMidiNoteState keys;
 SidAmpEnvelope filter;
 auto a=keys.noteOn(0,1,60,true);
 check(sidFilterLfoShouldTrigger(a.filterRetrigger,true,false,false),"legato first note retrigger");
 filter.noteOn(false);
 for(int i=0;i<480;++i)filter.tick(0.01f,0.2f,0.25f,sr);
 const float prior=filter.level;
 a=keys.noteOn(0,1,64,true);
 check(!sidFilterLfoShouldTrigger(a.filterRetrigger,true,false,false),"legato second key no retrigger");
 check(!sidFilterLfoShouldTrigger(true,false,false,true),"active arp does not reattack filter");
 check(!sidFilterLfoShouldTrigger(true,true,true,false),"pedal-held legato suppresses new attack");
 check(sidFilterLfoShouldTrigger(true,false,true,false),"trigger mode can restart filter");
 near(filter.level,prior,1e-7,"filter gate continuity");
 keys.noteOff(0,1,64);
 auto off=keys.noteOff(0,1,60);
 check(off.filterReleased,"last physical Note Off releases filter unless sustained");
 filter.noteOff(0.2f,sr);
 float last=filter.level;
 for(int i=0;i<140000;++i){const float now=filter.tick(0.01f,0.2f,0.25f,sr);check(now<=last+1e-6f,"filter release monotone");last=now;}
 check(filter.stage==0,"filter reaches idle after natural exponential release");
 std::puts("OK: JSFX v2.38 filter ADSR, Trigger/Legato, 3 LFO waveform/Delay/Smooth/Squash/One Shot parity");
}
