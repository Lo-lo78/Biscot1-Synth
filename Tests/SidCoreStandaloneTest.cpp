#include "../Source/SidWaveCore.h"
#include "../Source/SidFilterCore.h"
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <limits>

void require(bool ok,const char* message){if(!ok){std::fprintf(stderr,"FAIL: %s\n",message);std::exit(1);}}
static bool close(double x,double y,double delta=1.e-9){return std::abs(x-y)<delta;}
int main(){
 using namespace biscot;
 SidWaveCore waves;waves.prepare();
 require(close(waves.sawMip(9,0.),0.),"mip9 saw first sample");
 require(close(waves.triMip(9,0.),-1.,0.002),"mip9 triangle first sample");
 require(SidWaveCore::mip(.00087890625)==9,"top mip boundary");
 require(SidWaveCore::mip(.225)==1,"first mip boundary");
 for(int wave=0;wave<=7;++wave)for(bool model : {false,true})
   for(double pwm : {.00001,.25,.5,.75,.99999})
     for(int i=0;i<300;++i){
      const double phase=i/300.;
      const double v=waves.render(wave,phase,440./44100.,pwm,0.,model);
      require(std::isfinite(v),"bandlimited wave non-finite");
     }
 // Values independently calculated by translating the JSFX v2.38 @slider/@sample
 // filter equations with the same input sequence (reg 1200, resonance 7.5, LP).
 constexpr std::array<double,10> input{.2,-.4,.1,0.,.7,-.25,0.,0.,0.,0.};
 constexpr std::array<double,10> expected8580{
   .19454767509231874,-.2804998650657974,-.1210879808274098,.01099106309347348,
   .7388695810699597,.17456750440580154,-.13631775616541475,-.17127717903865938,
   -.06849446624824947,.02077369056712828};
 constexpr std::array<double,10> expected6581{
   .2179145736727023,-.2815667132977439,-.10419243041827671,-.06508207689654749,
   .6894945381684204,.2479336271083588,.13889698098133996,.05491314662484066,
   -.0021861485494651294,-.03469352236051407};
 for(bool model : {false,true}){
   SidFilterCore filter;
   const auto& expected=model?expected6581:expected8580;
   for(size_t i=0;i<input.size();++i){
     const double y=filter.process(input[i],1200.,7.5,44100.,model,true,false,false);
     require(close(y,expected[i],1.e-10),"JSFX filter sample parity");
   }
   filter.reset();
   require(filter.lp==0. && filter.bp==0. && filter.body==0.,"filter reset");
 }
 std::puts("OK: JSFX 8580/6581 filter sample parity, table/mip integrity and wave finite bounds");
}
