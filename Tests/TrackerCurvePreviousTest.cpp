// SPDX-License-Identifier: AGPL-3.0-or-later
#include "../Source/TrackerPatternPlayback.h"
#include "../Source/TrackerPatternInsert.h"
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <algorithm>

using namespace biscot;
static void must(bool b, const char* label){ if(!b)throw std::runtime_error(label); }
static bool near(double a,double b,double e=0.00001){return std::abs(a-b)<e;}
static const TrackerAutomatedParameter* find(const TrackerPlayablePattern& p,int st,int sid){
 for(const auto& a:p.rows.at((size_t)st).automation)if(a.synthIndex==sid)return &a;
 return nullptr;
}
static TrackerPattern blank(int n){TrackerPattern p(false);p.length=n;p.steps.clear();return p;}
int main(){
 try {
  {
   auto p=blank(5);
   p.cells(0,1)={{4,100},{1108,200}};
   p.cells(4,1)={{1108,1200}};
   const auto c=compileTrackerPattern(p);
   const auto* a=find(c,2,108);must(a!=nullptr,"Curve must cross empty steps");
   must(a->sourceStep==0 && a->targetStep==4,"Curve source/target");
   must(near(trackerCurveAt(*a,2,0),700),"Curve midpoint start of step 2");
   must(near(trackerCurveEventValue(*a,2,32),1043.75,0.001),"Curve step-local endpoint");
   must(find(c,4,108)->sourceStep==4,"Next real parameter terminates curve");
  }
  {
   auto p=blank(2);
   p.cells(0,1)={{4,-25},{1108,0}};
   p.cells(1,1)={{1108,100}};
   auto c=compileTrackerPattern(p);const auto& a=*find(c,0,108);
   must(near(trackerCurveAt(a,0,0.7),0),"Negative curve stays flat until final quarter");
   must(near(trackerCurveAt(a,0,0.875),50),"Negative curve smooth end");
   p.cells(0,1)={{4,100},{8,8},{9,1},{1108,0}};
   c=compileTrackerPattern(p);const auto& b=*find(c,0,108);
   must(b.resolution==8 && b.ratio==1,"Resolution and ratio are ordered before target");
   must(near(trackerCurveEventValue(b,0,1),0),"Ratio 1:1 first segment straight");
   must(trackerCurveEventValue(b,0,2)>0,"Ratio 1:1 second segment curved");
   p.cells(0,1)={{4,100},{8,8},{1108,0}};
   c=compileTrackerPattern(p);const auto& d=*find(c,0,108);
   must(near(trackerCurveEventValue(d,0,1),0),"Resolution alone alternates odd straight");
   must(trackerCurveEventValue(d,0,2)>0,"Resolution alone alternates even curved");
  }
  {
   auto p=blank(4);
   p.cells(0,0)={{2004,1}};
   p.cells(1,0)={{3,1},{2004,6}}; // inherited, not real 6
   p.cells(2,0)={{3,1},{2004,3}}; // another inherited occurrence
   p.cells(3,0)={{2004,5}};
   auto c=compileTrackerPattern(p);
   must(near(find(c,1,22)->from,1),"Previous value inherits actual step 0 (not stored 6)");
   must(near(find(c,2,22)->from,1),"Chains of Previous value inherit same source");
   p.cells(1,0)={{2004,6},{3,1}};
   c=compileTrackerPattern(p);
   must(near(find(c,1,22)->from,6),"Previous value only to right");
   must(near(find(c,2,22)->from,6),"Next inherited cell follows new real value");
  }
  {
   auto p=blank(4);
   p.cells(0,1)={{4,100},{1108,200}};
   p.cells(1,1)={{1108,400}}; // straight nearest occurrence prevents older curve
   p.cells(3,1)={{1108,1000}};
   const auto c=compileTrackerPattern(p);
   must(find(c,2,108)==nullptr,"Straight nearest value cancels older Curve");
  }
  {
   auto p=blank(3);
   p.cells(0,1)={{1108,500}};
   p.cells(1,1)={{3,1},{1108,999}};
   const auto c=compileTrackerPattern(p);
   must(near(find(c,1,108)->from,500),"Filter Previous value works");
   p.cells(1,1)={{1108,999},{3,1}};
   must(near(find(compileTrackerPattern(p),1,108)->from,999),"Marker after Filter has no effect");
  }
  {
   auto p=blank(3);
   p.cells(0,0)={{1,60},{2,0.5}};
   const TrackerParam* resolution=findTrackerParam(8);
   const TrackerParam* ratio=findTrackerParam(9);
   const TrackerParam* wave=findTrackerParam(2004);
   must(resolution && ratio && wave,"Definitions available");
   auto r=insertTrackerParameter(p,1,0,*ratio);
   must(r.status==TrackerInsertStatus::inserted && p.view(1,0).size()==2,"Auto add Curve with Ratio");
   must(p.view(1,0)[0].id==4 && near(p.view(1,0)[0].value,100) && p.view(1,0)[1].id==9,"Curve precedes Ratio");
   r=insertTrackerParameter(p,1,0,*resolution);
   must(r.status==TrackerInsertStatus::inserted && r.index==1,"Resolution inserts before Ratio");
   must(p.view(1,0)[1].id==8,"Correct order Curve/Resolution/Ratio");
   r=insertTrackerParameter(p,1,0,*resolution);
   must(r.status==TrackerInsertStatus::alreadyPresent,"No duplicate Resolution");
   r=insertTrackerParameter(p,1,0,*findTrackerParam(7));
   must(r.status==TrackerInsertStatus::needsArpeggiator,"Sub Note requires active arpeggiator");
   p.cells(1,0).push_back({2022,1});
   r=insertTrackerParameter(p,1,0,*findTrackerParam(7));
   must(r.status==TrackerInsertStatus::inserted,"Sub Note accepted after Arp On");
   r=insertTrackerParameter(p,1,0,*findTrackerParam(7));
   must(r.status==TrackerInsertStatus::inserted,"Repeated Sub Notes permitted");
   p.cells(2,0)={{3,1}};
   r=insertTrackerParameter(p,2,0,*wave);
   must(r.status==TrackerInsertStatus::inserted && near(p.view(2,0).back().value,wave->initial),"Default value without previous source");
  }
  {
   auto p=blank(4);
   p.cells(0,0)={{4,100},{2004,1}};
   p.cells(3,0)={{2004,5}};
   const auto c=compileTrackerPattern(p);
   must(find(c,1,22) && find(c,2,22),"Generic OP curve works through empty steps");
   must(near(trackerCurveAt(*find(c,2,22),2,0),1+4*trackerCurveProgress(2./3.,100)),"Mid-Pattern starting automation is deterministic");
  }
  {
   // Compare the constant-time audio scheduler with the exact reference
   // floor((stepSamples-1)*segment/resolution) event offsets.
   for(const int segments:{2,8,32,256,1024})for(const double length:{1.,4.,15.5,256.,6000.}){
    for(int sample=0;sample<(int)std::ceil(length);++sample){
     int expected=0;
     for(int seg=1;seg<=segments;++seg)
      if(std::floor((std::max(0.,length-1.)*seg)/segments)<=sample)expected=seg;
     const int actual=trackerCurveDueSegment(length,(double)sample,segments);
     must(actual==expected,"Constant-time due segment equals JSFX sample offsets");
    }
   }
  }
  {
   // Golden parity check between the optimized single-pass snapshot compiler
   // and the directly translated JSFX lookup semantics (nonduplicate cells).
   auto p=blank(48);
   for(int st=0;st<48;++st){
    if(st%3==0)p.cells(st,0).push_back({4,st%6==0?100.:-35.});
    if(st%5==0)p.cells(st,0).push_back({8,st%10==0?8.:32.});
    if(st%7==0)p.cells(st,0).push_back({9,(double)((st/7)%9)});
    if(st%4==0)p.cells(st,0).push_back({2004,(double)((st/4)%8)});
    if(st%6==0)p.cells(st,1).push_back({1108,(double)(200+st*17)});
    if(st%9==0)p.cells(st,2).push_back({1123,1.+st});
   }
   const auto compiled=compileTrackerPattern(p);
   for(int st=0;st<p.length;++st){
    const auto slow=trackerCompileStepAutomation(p,st);
    const auto& fast=compiled.rows[(size_t)st].automation;
    must(slow.size()==fast.size(),"Optimized compiler retains all parameters");
    for(const auto& wanted:slow){
     const auto it=std::find_if(fast.begin(),fast.end(),[&](const auto& x){return x.synthIndex==wanted.synthIndex;});
     must(it!=fast.end(),"Optimized compiler preserves parameter ID");
     must(near(it->from,wanted.from) && near(it->to,wanted.to) &&
          it->sourceStep==wanted.sourceStep && it->targetStep==wanted.targetStep &&
          it->resolution==wanted.resolution && it->ratio==wanted.ratio,
          "Optimized compiler matches JSFX event semantics");
    }
   }
  }
  std::cout<<"Tracker Curve / Previous / Resolution / Ratio / F2 insertion: PASS\n";
  return 0;
 }catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<"\n";return 1;}
}
