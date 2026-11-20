// TEST0.2.52: signed pan must follow the *same per-sample pitch trajectory*
// as Portamento, SID Seq and classic Arp Glide (no independent 3ms lag).
#include "../Source/SidPingPongPan.h"
#include "../Source/SidPitchGlideMotion.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <cmath>
#include <iostream>
using namespace biscot;
static constexpr double sr=48000.;
static void near(double a,double b,double tol=0.00003){assert(std::abs(a-b)<=tol);}
static void glideTo(float start,float target,float amount,float coeff){
 SidPingPongPan p;p.prepare(sr);
 p.advanceAtAttack(amount);
 near(p.tick(amount,1.f),amount,0.);
 p.advance();
 float pitch=start;
 for(int n=0;n<1000;++n){
  SidPitchGlideMotion motion;
  const float before=pitch;
  pitch+=(target-pitch)*coeff;
  motion.add(before,target,pitch);
  const float pan=p.tick(amount,motion.coefficient());
  // Endpoints move in EXACT normalized correspondence with pitch glide.
  const double normalized=(pitch-start)/(target-start);
  const double expected=amount*(1.-2.*normalized);
  near(pan,expected,0.0001);
 }
}
static void testChangingGlide(){
 SidPingPongPan p;p.prepare(sr);p.advanceAtAttack(1.f);p.tick(1.f,1.f);
 float pitch=60.f;
 p.advance();
 for(int n=0;n<2500;++n){
  // Even sample-to-sample modulation of glide speed does not desync panning.
  const float coeff=n%9==0?0.28f:n%4==0?0.0007f:0.013f;
  const float before=pitch;
  pitch+=(80.f-pitch)*coeff;
  SidPitchGlideMotion motion;motion.add(before,80.f,pitch);
  const float pan=p.tick(1.f,motion.coefficient());
  near(pan,1.-2.*(pitch-60.f)/20.f,0.00005);
 }
}
static void testFastClockAndRetrigger(float coefficient){
 SidPingPongPan p;p.prepare(sr);p.advanceAtAttack(1.f);p.tick(1.f,1.f);
 double pitch=60.;
 double prevPan=1.;
 double prevPitch=pitch;
 double target=72.;
 for(int n=0;n<300;++n){
  // New arpeggio steps arrive every 7 samples, long before slower glides end.
  if(n%7==0){
   target=(n/7)%2==0?72.:60.;
   prevPan=p.tick(1.f,0.f);prevPitch=pitch;
   p.advance();
  }
  const double before=pitch;
  pitch+=(target-pitch)*coefficient;
  SidPitchGlideMotion m;m.add(before,target,pitch);
  const double pan=p.tick(1.f,m.coefficient());
  const double desired=(p.firstSide?1.:-1.);
  // For every retarget, the remaining pan movement mirrors the remaining pitch.
  if(std::abs(target-prevPitch)>1.e-7)
   near(pan,prevPan+(desired-prevPan)*(pitch-prevPitch)/(target-prevPitch),0.0001);
  else near(pan,desired,0.0001);
 }
}
static void testCombinedGlide(){
 SidPingPongPan p;p.prepare(sr);p.advanceAtAttack(+1.f);p.tick(+1.f,1.f);
 p.advance();
 double note=60.,arp=0.,start=60.;
 constexpr double noteDestination=65.,arpDestination=7.;
 for(int n=0;n<1000;++n){
  const double beforeN=note,beforeA=arp;
  const double cNote=1.-std::exp(-1./(sr*0.05));
  const double cArp=1./(sr*0.02);
  note+=(noteDestination-note)*cNote;
  arp+=(arpDestination-arp)*cArp;
  SidPitchGlideMotion motion;
  motion.add(beforeN,noteDestination,note);
  motion.add(beforeA,arpDestination,arp);
  const double pan=p.tick(+1.f,motion.coefficient());
  near(pan,1.-2.*((note+arp)-start)/12.,0.00005);
 }
}
static void testSquareAndDepth(){
 for(float signedAmount:{-1.f,1.f}){
  SidPingPongPan p;p.prepare(sr);
  p.advanceAtAttack(signedAmount);near(p.tick(signedAmount,1.f),signedAmount,0.);
  // No pitch glide: instant switch on the SAME audio sample even during sustain.
  p.advance();near(p.tick(signedAmount,1.f),-signedAmount,0.);
  p.advance();near(p.tick(signedAmount,1.f),signedAmount,0.);
  // Automation of amount alone retains its own de-click (not side latency).
  const float before=p.tick(signedAmount,1.f);
  const float after=p.tick(0.f,1.f);
  assert(std::abs(after)<std::abs(before) && std::abs(after)>0.f);
  for(int n=0;n<2500;++n)p.tick(0.f,1.f);
  near(p.tick(0.f,1.f),0.f,0.);
 }
}
int main(){
 const float sidCoef=1.f/(1.f+0.18f*0.18f*2000.f);
 const float classicCoef=1.f/(float)(sr*(0.002+0.08*0.3));
 const float portCoef=1.f-std::exp(-1.f/(float)(sr*0.04));
 for(float amount:{-1.f,1.f}){
  glideTo(60.f,72.f,amount,sidCoef);
  glideTo(60.f,72.f,amount,classicCoef);
  glideTo(60.f,72.f,amount,portCoef);
  glideTo(64.f,56.f,amount,0.65f); // Fast glide: no additional lag.
 }
 testChangingGlide();testFastClockAndRetrigger(0.16f);
 testFastClockAndRetrigger(0.004f);
 testCombinedGlide();testSquareAndDepth();
 std::cout<<"OK: Pan precisely follows square pitch steps, SID Seq, classic arp, portamento, fast/retriggered/combined glides and signed depth\n";
}
