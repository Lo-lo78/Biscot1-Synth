// SPDX-License-Identifier: AGPL-3.0-or-later
#include "../Source/TrackerPatternPlayback.h"
#include <iostream>
#include <cstdlib>
using namespace biscot;
static void require(bool v,const char* msg){if(!v){std::cerr<<msg<<'\n';std::exit(1);}}
int main(){
 TrackerPattern p(false);p.length=4;
 p.steps[0].tabs[0]={{1,60},{2,0.92},{2004,2}};
 p.steps[0].tabs[1]={{1108,1200}};
 p.steps[0].tabs[2]={{1123,4},{1233,0.5}};
 p.steps[1].tabs[1]={{1108,2400}}; // Filter-only row is silent on isolated preview.
 p.steps[2].tabs[2]={{1123,8}};   // LFO-only row is also silent.
 p.steps[3].tabs[0]={{1,67}};
 require(trackerCanAuditionStep(p,0,0),"stopped: Note/Gate/Filter/LFO row previews");
 require(trackerCanAuditionStep(p,0,3),"one-step preview can be restarted on edit");
 require(trackerCanAuditionStep(p,0,2),"paused: preview allowed on edit");
 require(!trackerCanAuditionStep(p,0,1),"editing must not interrupt full Pattern playback");
 require(!trackerCanAuditionStep(p,1,0),"Filter-only step must not trigger preview");
 require(!trackerCanAuditionStep(p,2,3),"LFO-only step must not restart previous note");
 require(!trackerCanAuditionStep(p,-1,0),"header has no preview");
 require(!trackerCanAuditionStep(p,4,0),"out-of-range has no preview");
 require(trackerCanAuditionStep(p,3,0),"independent later Note triggers preview");
 // Any modified value in the active row is reflected in the *new* compiled
 // snapshot; old snapshots remain valid until the audio thread releases them.
 auto before=compileTrackerPattern(p);
 p.steps[0].tabs[0][0].value=72;
 p.steps[0].tabs[1][0].value=1800;
 p.steps[0].tabs[2][0].value=12;
 auto after=compileTrackerPattern(p);
 require(before.rows[0].note==60 && after.rows[0].note==72,"Note update reaches snapshot");
 require(before.rows[0].ordered[3].value==1200 && after.rows[0].ordered[3].value==1800,"Filter update reaches snapshot");
 require(before.rows[0].ordered[4].value==4 && after.rows[0].ordered[4].value==12,"LFO update reaches snapshot");
 TrackerPatternTransport audio;
 audio.start(0,after,true);
 require(audio.tick(after,48000,120).startStep==0,"edited one-step preview starts at current step");
 std::cout<<"TrackerPatternEditAuditionTest OK\n";
}
