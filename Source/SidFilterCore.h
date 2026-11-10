#pragma once
#include <algorithm>
#include <cmath>
namespace biscot {
// Direct transliteration of Biscot1Synth v2.38.jsfx lines 3009-3157.
// NOT a generic TPT SVF: this includes the actual JSFX model coefficients,
// Euler state order, 6581 low-frequency leakage, and post-filter compensation.
struct SidFilterCore {
    double lp = 0, bp = 0, hp = 0, body = 0;
    static double softclip(double x) noexcept { return x/(1.+std::abs(x)); }
    static double clamp(double x,double lo,double hi) noexcept {return std::max(lo,std::min(hi,x));}
    void reset() noexcept {lp=bp=hp=body=0;}
    double process(double input, double reg, double resonance, double sampleRate,
                   bool model6581, bool onLP,bool onBP,bool onHP) noexcept {
        static constexpr double pi=3.1415926535897932384626433832795;
        const double sr=std::max(1.,sampleRate);
        const double cutNorm=clamp(reg,0.,2047.)/2047.;
        const double resNorm=clamp(resonance,0.,15.)/15.;
        const double cutoff=std::min((model6581?28.+std::pow(cutNorm,2.12)*8170.
                                                  :4.+std::pow(cutNorm,1.38)*12500.),sr*0.42);
        const double f=clamp(2.*std::sin(pi*cutoff/sr),0.00015,0.98);
        const double damping=clamp(model6581?1.30-resNorm*0.36:1.16-resNorm*0.90,
                                   model6581?1.06:0.62,1.55);
        const double stateLoss=model6581?0.99780-resNorm*0.00055:0.99980;
        const double bodyF=clamp(2.*std::sin(pi*135./sr),0.0001,0.20);
        const double bodyAmount=model6581?0.22*std::pow(1.-cutNorm,2.35):0.;
        // Update order is essential: hp from previous lp/bp, then bp, then lp.
        hp=input-lp-damping*bp;
        bp+=f*hp;
        lp+=f*bp;
        bp*=stateLoss;lp*=stateLoss;
        bp=clamp(bp,-2.5,2.5);lp=clamp(lp,-2.5,2.5);hp=clamp(hp,-2.5,2.5);
        body+=bodyF*(input-body);
        if(model6581){
            bp*=0.9992-resNorm*0.00035;
            hp*=0.9994-resNorm*0.00025;
            lp*=0.9995-resNorm*0.00020;
        }else{
            if(std::abs(bp)>5.)bp*=0.5;
            if(std::abs(lp)>5.)lp*=0.5;
        }
        const int count=int(onLP)+int(onBP)+int(onHP);
        if(count==0)return input*(model6581?1.122018454:1.);
        double out=0.;
        if(onLP)out+=lp+(model6581?body*bodyAmount:0.);
        if(onBP)out+=bp*(model6581?1.:0.94);
        if(onHP)out+=hp*(model6581?0.98:0.99);
        const double cream=1.+resNorm*0.36;
        if(model6581){
            out*=cream;
            const double invCount=1./count;
            const double modeMix=(double(onLP)+0.70*double(onBP)+0.36*double(onHP))*invCount;
            out+=bp*resNorm*(1.04+resNorm*1.56)*modeMix;
            if(onLP)out+=bp*resNorm*(0.36+resNorm*0.64)*invCount;
            const double postCol=1.06+0.020*resNorm;
            const double postMix=0.23+0.10*resNorm;
            const double dirty=softclip(out*postCol)/postCol;
            out=out*(1.-postMix)+dirty*postMix;
            if(onLP&&onBP&&onHP)out*=1.052;
            out*=(3.45+resNorm*0.16)*1.995262315;
            out*=0.2511886432;
        }else{
            out*=cream;
            const double modeMix=(double(onLP)+0.72*double(onBP)+0.38*double(onHP))/count;
            out+=bp*resNorm*(0.24+resNorm*0.32)*modeMix;
        }
        return out;
    }
};
} // namespace biscot
