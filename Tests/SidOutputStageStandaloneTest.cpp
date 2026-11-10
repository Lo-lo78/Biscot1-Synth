#include "../Source/SidOutputStage.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>

static bool close(double a,double b) { return std::abs(a-b)<1.e-12; }
static void require(bool ok, const char* message) {
    if(!ok) { std::fprintf(stderr,"FAIL: %s\n",message); std::exit(1); }
}
int main() {
    for(bool model : {false,true}) {
        biscot::SidOutputStage out;
        out.prepare(44100.);
        const double alpha=1.-std::exp(-2.*biscot::SidOutputStage::pi*16000./44100.);
        const double hp=std::exp(-2.*biscot::SidOutputStage::pi*1.6/44100.);
        require(close(out.lowpassAlpha,alpha),"LP coefficient");
        require(close(out.highpassR,hp),"HP coefficient");
        const double bias=model?0.012:0.003;
        const double wet=model?0.085:0.022;
        const double dry=1.-wet;
        const double makeup=model?1.022:1.004;
        double lp=0.,xp=0.,yp=0.;
        for(double sample : {0.5,-0.7,0.0,0.25,-0.4,0.0,0.0}) {
            const double z=sample+bias;
            const double drive=z>=0.? (model?1.10:1.025) : (model?0.96:0.99);
            const double amp=(z*dry+(z*drive)/(1.+std::abs(z*drive))/drive*wet)*makeup;
            lp+=(amp-lp)*alpha;
            const double y=lp-xp+hp*yp;xp=lp;yp=y;
            require(close(out.process(sample,model),y),"sample-by-sample JSFX equation parity");
        }
        out.reset();require(close(out.lowpass,0.) && close(out.previousHighpass,0.),"reset");
    }
    std::puts("OK: exact JSFX v2.38 amp/RC/DC equation checked on 8580 and 6581 sequences");
}
