// SPDX-License-Identifier: AGPL-3.0-or-later
#include "../Source/TrackerSongPlayback.h"
#include <cassert>
#include <cmath>
int main(){
 using namespace biscot;
 TrackerPatternBank patterns;
 patterns.at(0).length=8;
 TrackerSong song;
 song.laneLengths={8,8,8};
 auto& fx=song.atStep(0,0);
 assert(fx.insert(17));fx.values[17]=2.;
 assert(fx.insert(18));fx.values[18]=80.;
 assert(fx.insert(19));fx.values[19]=75.;
 assert(fx.insert(20));fx.values[20]=50.;
 auto& change=song.atStep(0,2);
 // Sparse overrides inherit other delay controls from row 0.
 assert(change.insert(20));change.values[20]=100.;
 auto& step=song.atStep(0,4);
 assert(step.insert(1));step.values[1]=0;step.values[6]=8;
 const auto full=compileTrackerSong(song,0,patterns);
 const auto preview=compileTrackerSongRowPreview(full,0,4,3);
 assert(preview!=nullptr);
 assert(preview->clips[0].size()==1);
 assert(preview->clips[0][0].playLength==3);
 assert(preview->fxRows[0].size()==1);
 const auto& result=preview->fxRows[0].at(0);
 for(int id=17;id<=20;++id)assert(result.present[(size_t)id]);
 assert(result.value[17]==2.);
 assert(result.value[18]==80.);
 assert(result.value[19]==75.);
 assert(result.value[20]==100.);
 // Without a delay event, the Song row preview must not acquire one.
 TrackerSong plain;plain.laneLengths={8,8,8};
 auto& plainStep=plain.atStep(0,4);
 assert(plainStep.insert(1));plainStep.values[1]=0;
 const auto basic=compileTrackerSongRowPreview(compileTrackerSong(plain,0,patterns),0,4,3);
 assert(basic && basic->fxRows[0].empty());
}
