#pragma once
#include <array>
#include <cmath>
namespace biscot {
// Mirrors Biscot1 JSFX v2.38 LFO-usage gates, including all three
// independent modulation routes and cross-rate sources. Snapshot once per
// audio block; strips with no destination do zero waveform work per sample.
template<class GetParameter>
std::array<bool,3> determineUsedLfos(GetParameter value) noexcept {
 std::array<bool,3> used{};
 constexpr float epsilon=0.0000001f;
 for(int i=0;i<3;++i){
  const int base=132+i*23;
  for(int k=0;k<13;++k)
   used[(std::size_t)i]|=std::abs(value(base+k))>epsilon;
  for(int j=0;j<3;++j){
   used[(std::size_t)i]|=(int(value(191+j))==i+1) || (int(value(197+j))==i+1);
   const int route=211+j*3;
   used[(std::size_t)i]|=int(value(route))==i+1 && value(route+1)>0.f &&
                    std::abs(value(route+2))>epsilon;
   const int rate=220+j*2;
   const bool rateActive=value(rate)>0.f && std::abs(value(rate+1))>epsilon;
   used[(std::size_t)i]|=rateActive && (j==i || int(value(rate))==i+1);
  }
 }
 // Resolve chains deterministically; a source may itself use another source.
 for(int pass=0;pass<3;++pass){
  bool changed=false;
  for(int j=0;j<3;++j){
   if(!used[(std::size_t)j])continue;
   const int ex=234+j*5;
   const int src=(int)value(ex+2);
   if(src>=1 && src<=3 && value(ex+4)>epsilon && !used[(std::size_t)(src-1)]){
    used[(std::size_t)(src-1)]=true;changed=true;
   }
  }
  if(!changed)break;
 }
 return used;
}
}
