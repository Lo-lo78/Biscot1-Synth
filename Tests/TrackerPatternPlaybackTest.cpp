// SPDX-License-Identifier: AGPL-3.0-or-later
#include "../Source/TrackerPatternPlayback.h"
#include <iostream>
#include <cstdlib>
#include <cmath>
using namespace biscot;
static void require(bool ok,const char* msg){if(!ok){std::cerr<<msg<<'\n';std::exit(1);}}
int main(){
 TrackerPattern p; p.length=4;p.repeat=2;p.repeatSpeeds={1.,2.};p.repeatTransposes={0,12};
 p.steps[0].tabs[0]={{1,60},{2,0.92},{2004,1},{2002,0.8}};
 p.steps[1].tabs[0]={{1,62},{2,0.5}};
 p.steps[2].tabs[0]={{6,1}};
 p.steps[3].tabs[0]={{1,65},{2,1}};
 p.steps[3].tabs[2]={{1123,4},{1170,2},{1233,0.75}};
 auto snapshot=compileTrackerPattern(p);
 require(snapshot.rows.size()==4,"four active rows");
 require(snapshot.rows[0].note==60 && snapshot.rows[0].gate>0.91,"note and gate");
 require(snapshot.rows[2].pause,"pause is recognized");
 require(snapshot.rows[3].ordered.size()==5,"F2 LFO1 LFO3 ordered parameters");
 require(snapshot.rows[3].ordered[4].id==1233,"LFO3 Square PWM");
 TrackerPatternTransport clock;clock.start(1,snapshot);
 require(clock.running && clock.nextStep==1,"start at edit cursor");
 auto ev=clock.tick(snapshot,48000,120);
 require(ev.startStep==1,"first step from cursor, not zero");
 require(std::abs(clock.stepSamples(snapshot,48000,120)-6000)<0.01,"16th at 120 BPM");
 clock.gateFor(3000,false);
 int gateCount=0;for(int i=0;i<5999;++i){ev=clock.tick(snapshot,48000,120);if(ev.gateOff)++gateCount;}
 require(gateCount==1,"gate off once");
 ev=clock.tick(snapshot,48000,120);require(ev.startStep==2,"next step remains on grid");
 // Completing one repeat changes speed and transpose without resetting time.
 while(clock.repeatIndex==0){clock.tick(snapshot,48000,120);}
 require(clock.repeatIndex==1,"repeat advanced");
 require(std::abs(clock.stepSamples(snapshot,48000,120)-3000)<0.01,"repeat speed x2");
 require(snapshot.repeatTransposes[1]==12,"transpose repeat 2");
 auto current=clock.currentStep;clock.stop();
 require(!clock.running,"pause/stop command leaves no notes");
 clock.start(current,snapshot);require(clock.tick(snapshot,48000,120).startStep==current,"resume from audible step");
 clock.stop();clock.start(3,snapshot,true);
 require(clock.tick(snapshot,48000,120).startStep==3,"audition requested step");
 bool finished=false;for(int i=0;i<12000;++i){auto t=clock.tick(snapshot,48000,120);if(t.finished){finished=true;break;}}
 require(finished && !clock.running,"audition plays one step only");
 // Library audition must stop after its limited number of steps even when
 // the saved Pattern repeats many times, and it must start at a requested note.
 clock.stop();clock.start(1,snapshot,false,3);
 require(clock.tick(snapshot,48000,120).startStep==1,"library preview starts at requested row");
 int played=1;bool libraryFinished=false;
 for(int i=0;i<100000;++i){
  const auto tick=clock.tick(snapshot,48000,120);
  if(tick.startStep>=0)++played;
  if(tick.finished){libraryFinished=true;break;}
 }
 require(libraryFinished && played==3,"library preview stops after three steps");
 std::cout<<"TrackerPatternPlaybackTest OK\n";
}
