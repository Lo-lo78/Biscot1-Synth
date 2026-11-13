// SPDX-License-Identifier: AGPL-3.0-or-later
#include "../Source/TrackerSongPlayback.h"
#include <cstdlib>
#include <iostream>
// CHECK must run in Release too: MSVC defines NDEBUG and removes assert()
// expressions, including the parameter insertions performed by this test.
#define CHECK(expression) do { if (!(expression)) { \
 std::cerr << "TrackerSongRowAuditionFxTest failed at line " << __LINE__ \
           << ": " #expression << "\n"; std::exit(1); \
} } while (false)
#include <cmath>
int main(){
 using namespace biscot;
 TrackerPatternBank patterns;
 patterns.at(0).length=8;
 TrackerSong song;
 song.laneLengths={8,8,8};
 auto& fx=song.atStep(0,0);
 CHECK(fx.insert(17));fx.values[17]=2.;
 CHECK(fx.insert(18));fx.values[18]=80.;
 CHECK(fx.insert(19));fx.values[19]=75.;
 CHECK(fx.insert(20));fx.values[20]=50.;
 auto& change=song.atStep(0,2);
 // Sparse overrides inherit other delay controls from row 0.
 CHECK(change.insert(20));change.values[20]=100.;
 auto& step=song.atStep(0,4);
 CHECK(step.insert(1));step.values[1]=0;step.values[6]=8;
 const auto full=compileTrackerSong(song,0,patterns);
 const auto preview=compileTrackerSongRowPreview(full,0,4,3);
 CHECK(preview!=nullptr);
 CHECK(preview->clips[0].size()==1);
 CHECK(preview->clips[0][0].playLength==3);
 CHECK(preview->fxRows[0].size()==1);
 const auto& result=preview->fxRows[0].at(0);
 for(int id=17;id<=20;++id)CHECK(result.present[(size_t)id]);
 CHECK(result.value[17]==2.);
 CHECK(result.value[18]==80.);
 CHECK(result.value[19]==75.);
 CHECK(result.value[20]==100.);
 // Without a delay event, the Song row preview must not acquire one.
 TrackerSong plain;plain.laneLengths={8,8,8};
 auto& plainStep=plain.atStep(0,4);
 CHECK(plainStep.insert(1));plainStep.values[1]=0;
 const auto basic=compileTrackerSongRowPreview(compileTrackerSong(plain,0,patterns),0,4,3);
 CHECK(basic && basic->fxRows[0].empty());
}
