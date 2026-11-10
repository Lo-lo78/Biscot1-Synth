#pragma once
#include <algorithm>
#include <array>
#include <cmath>

namespace biscot {
// A source-level port of the 1x wavetable/AA oscillator in Biscot1 JSFX v2.38.
// Tables use exactly the same Fourier series and 2048+1 / 10-mip layout.
// Cross-oscillator Ring/Sync need the JSFX's separate adaptive oversampling pass:
// these are still approximated by the standalone single-sample legacy path below.
class SidWaveCore {
public:
 static constexpr int tableSize=2048, tableStride=tableSize+1, levels=10;
 static constexpr double pi=3.1415926535897932384626433832795;
 std::array<float,tableStride*levels> saw{}, tri{}, square{};
 void prepare(){
  // The original JSFX accumulates the Fourier series through 512 harmonics
  // and snapshots mip levels at harmonic counts 1,2,4,...512.
  for(int i=0;i<=tableSize;++i){
   const double ph=(double)i/tableSize;
   double sumSaw=0,sumTri=0;int nextLevelAt=1,lev=0;
   for(int h=1;h<=512;++h){
    const double a=2*pi*h*ph;
    sumSaw+=std::sin(a)/h;
    if(h&1)sumTri+=std::cos(a)/(double(h)*h);
    if(h==nextLevelAt){
     saw[(size_t)lev*tableStride+i]=(float)((-2/pi)*sumSaw);
     tri[(size_t)lev*tableStride+i]=(float)((-8/(pi*pi))*sumTri);
     ++lev;nextLevelAt*=2;
    }
   }
  }
  for(int lev=0;lev<levels;++lev)for(int i=0;i<=tableSize;++i){
   int shifted=i+tableSize/2;
   if(shifted>=tableSize)shifted-=tableSize;
   square[(size_t)lev*tableStride+i]=saw[(size_t)lev*tableStride+shifted]-saw[(size_t)lev*tableStride+i];
  }
 }
 static double clamp(double v,double lo,double hi){return std::max(lo,std::min(hi,v));}
 static int mip(double increment){
  if(increment<=0.00087890625)return 9;
  if(increment<=0.00175781250)return 8;
  if(increment<=0.00351562500)return 7;
  if(increment<=0.00703125000)return 6;
  if(increment<=0.01406250000)return 5;
  if(increment<=0.02812500000)return 4;
  if(increment<=0.05625000000)return 3;
  if(increment<=0.11250000000)return 2;
  if(increment<=0.22500000000)return 1;
  return 0;
 }
 static double wrap(double ph){ph-=std::floor(ph);return ph<0?ph+1:ph;}
 double read(const std::array<float,tableStride*levels>& data,int lev,double ph)const{
  ph=wrap(ph);const double pos=ph*tableSize;
  const int i=(int)pos;const double f=pos-i;
  const size_t ix=(size_t)lev*tableStride+(size_t)i;
  return (double)data[ix]+((double)data[ix+1]-data[ix])*f;
 }
 double sawMip(int lev,double ph)const{return read(saw,lev,ph);}
 double triMip(int lev,double ph)const{return read(tri,lev,ph);}
 double pulseMip(int lev,double ph,double width)const{
  const double pw=clamp(width,0.0002,0.9998);
  if(pw==0.5)return read(square,lev,ph);
  return sawMip(lev,wrap(ph-pw))-sawMip(lev,ph)+2*pw-1;
 }
 static double combo5dc(double d,bool r4){
  d=clamp(d,0.0002,0.9998);
  if(r4)return d<=0.5?(1.831168*d-0.030912)*d-0.884672:(-1.831168*d+3.631424)*d-1.800256;
  return d<=0.5?(1.969408*d-0.00677376)*d-0.97793024:(-1.969408*d+3.93204224)*d-1.96263424;
 }
 static double combo6dc(double d,bool r4){
  d=clamp(d,0.0002,0.9998);
  return r4?(1.062864*d-0.200304)*d-0.86256:(0.984704*d-0.00620928)*d-0.97849472;
 }
 static double combo7dc(double d,bool r4){
  d=clamp(d,0.0002,0.9998);
  if(r4)return d<=0.5?((1.0602368*d+0.1295952)*d+0.0545664)*d-0.8361024:
                        ((-1.0602368*d+1.5835344)*d+0.1909824)*d-1.002736;
  return d<=0.5?((1.17512533333333*d+0.095316)*d-0.0063544)*d-0.9385336:
                ((-1.17512533333333*d+1.730916)*d+0.1207336)*d-1.11719626666667;
 }
 static double and2(double a,double b){return 2*((a+1)*0.5)*((b+1)*0.5)-1;}
 static double and3(double a,double b,double c){return 2*((a+1)*0.5)*((b+1)*0.5)*((c+1)*0.5)-1;}
 static double dac8(double x){return std::floor(clamp((x+1)*127.5,0,255)+0.5);}
 static double clip(double x){return x/(1+std::abs(x));}
 static double blep(double t,double dt){
  dt=clamp(dt,1e-7,0.5);
  if(t<dt){double x=t/dt;return x+x-x*x-1;}
  if(t>1-dt){double x=(t-1)/dt;return x*x+x+x+1;}
  return 0;
 }
 double render(int wave,double ph,double inc,double pw,double noise,bool r4,
               bool ring=false,bool sync=false,double ringPhase=0,double ringIncrement=0)const{
  wave=std::clamp(wave,0,7);ph=wrap(ph);
  const bool tablePath=!ring&&!sync;
  const int lev=mip(std::max(1e-7,inc*(wave==7?4:wave>=4?2:1)));
  const bool nt=wave==0||wave==4||wave==5||wave==7;
  const bool ns=wave==1||wave==4||wave==6||wave==7;
  const bool np=wave==2||wave==5||wave==6||wave==7;
  double t=0,s=0,p=0;
  if(nt){
   if(tablePath)t=triMip(lev,ph);
   else{
    t=1-4*std::abs(ph-0.5);
    if(ring){
     double sign=ringPhase<0.5?1:-1;
     double rdt=std::max(1e-7,ringIncrement);
     sign+=blep(ringPhase,rdt)-blep(wrap(ringPhase-0.5),rdt);
     t*=clamp(sign,-1,1);
    }
   }
  }
  if(ns)s=tablePath?sawMip(lev,ph):clamp(2*ph-1-blep(ph,inc),-1,1);
  if(np){
   if(tablePath)p=pulseMip(lev,ph,pw);
   else{
    double d=clamp(pw,0.0002,0.9998);
    p=clamp((ph<d?1.:-1.)+blep(ph,inc)-blep(wrap(ph-d),inc),-1.,1.);
   }
  }
  if(wave==0)return t;
  if(wave==1)return s;
  if(wave==2)return p;
  if(wave==3)return noise;
  double v=0,product=0,analog=0,pull=0,gate=(p+1)*0.5;
  if(tablePath){
   if(wave==4)v=r4?(0.45*t+0.55*s+0.34*t*s):(0.40*t+0.60*s+0.28*t*s);
   if(wave==5){
    product=and2(t,p);
    if(r4){pull=gate*t+(1-gate)*(-0.72+0.16*t);analog=0.52*product+0.56*pull;v=(product+analog)*0.50*0.92;}
    else{pull=gate*t+(1-gate)*-0.88;analog=0.78*product+0.24*pull;v=(product*0.76+analog*0.24)*0.98;}
    v-=combo5dc(pw,r4);
   }
   if(wave==6){
    product=and2(s,p);
    if(r4){pull=gate*s+(1-gate)*(-0.70+0.12*s);analog=0.50*product+0.60*pull+0.16*s*p;v=(product*0.48+analog*0.52)*0.90;}
    else{pull=gate*s+(1-gate)*-0.88;analog=0.80*product+0.22*pull;v=(product*0.76+analog*0.24)*0.98;}
    v-=combo6dc(pw,r4);
   }
   if(wave==7){
    product=and3(t,s,p);
    if(r4){pull=gate*(0.50*t+0.50*s+0.32*t*s)+(1-gate)*(-0.76+0.08*t);analog=0.46*product+0.70*pull;v=(product*0.42+analog*0.58)*0.84;}
    else{pull=gate*(0.50*t+0.50*s)+(1-gate)*-0.90;analog=0.76*product+0.26*pull;v=(product*0.74+analog*0.26)*0.94;}
    v-=combo7dc(pw,r4);
   }
  }else{
   const int a=(int)dac8(t),b=(int)dac8(s),c=(int)dac8(p);
   product=((wave==4?(a&b):wave==5?(a&c):wave==6?(b&c):(a&b&c))/127.5)-1;
   if(wave==4){
    if(r4){analog=clip(0.58*product+0.30*t+0.18*s+0.40*t*s);v=(0.55*product+0.45*analog)*0.88;}
    else{analog=clip(0.82*product+0.14*t+0.10*s+0.18*t*s);v=(0.78*product+0.22*analog)*0.96;}
   }else if(wave==5){
    if(r4){pull=gate*t+(1-gate)*(-0.72+0.16*t);analog=clip(0.52*product+0.56*pull);v=(0.50*product+0.50*analog)*0.92;}
    else{pull=gate*t+(1-gate)*-0.88;analog=clip(0.78*product+0.24*pull);v=(0.76*product+0.24*analog)*0.98;}
   }else if(wave==6){
    if(r4){pull=gate*s+(1-gate)*(-0.70+0.12*s);analog=clip(0.50*product+0.60*pull+0.16*s*p);v=(0.48*product+0.52*analog)*0.90;}
    else{pull=gate*s+(1-gate)*-0.88;analog=clip(0.80*product+0.22*pull);v=(0.76*product+0.24*analog)*0.98;}
   }else{
    if(r4){pull=gate*(0.50*t+0.50*s+0.32*t*s)+(1-gate)*(-0.76+0.08*t);analog=clip(0.46*product+0.70*pull);v=(0.42*product+0.58*analog)*0.84;}
    else{pull=gate*(0.50*t+0.50*s)+(1-gate)*-0.90;analog=clip(0.76*product+0.26*pull);v=(0.74*product+0.26*analog)*0.94;}
   }
  }
  v*=1.412537545;
  if(wave==4)v*=1.258925412;
  if(wave==6||wave==7)v*=1.584893192;
  return tablePath?v:clamp(v,-1,1);
 }
};
} // namespace biscot
