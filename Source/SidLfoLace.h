#pragma once
// A zero-allocation, deterministic, one-source LFO interlace. This is a
// Biscot1 implementation inspired by the SIDStation Lace concept; it does
// not claim to reproduce Elektron's exact original algorithm.
#include <algorithm>
#include <cmath>
namespace biscot {
inline float sidLfoLace(float own,float source,double phase,int rate,float mix) noexcept {
 if(mix<=0.f)return own;
 const int slices=std::clamp(rate,1,32)*2;
 const double x=phase-std::floor(phase);
 const int slot=std::clamp((int)(x*slices),0,slices-1);
 if((slot&1)==0)return own;
 return own+(source-own)*std::clamp(mix,0.f,1.f);
}
}
