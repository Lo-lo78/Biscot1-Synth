// Song editor test: selection, ordered clipboard, sparse clearing and undo/redo.
#include "../Source/TrackerSongEditing.h"
#include "../Source/TrackerSongParameters.h"
#include <cstdlib>
using namespace biscot;
static void check(bool b){if(!b)std::abort();}
int main(){
 TrackerSongBank bank;auto& a=bank.at(0);auto& e=a.atStep(0,3);
 check(e.insert(1));e.values[1]=12;e.values[5]=4;e.values[6]=8;
 check(e.insert(3));e.values[3]=-6;
 TrackerSongClipboard cb;cb.copyParameter(0,0,3,1,e);
 check(cb.pasteParameter(a,1,5,0));
 const auto* pasted=a.findStep(1,5);
 check(pasted&&pasted->contains(1)&&pasted->values[1]==12&&pasted->values[5]==4&&pasted->values[6]==8);
 check(!cb.pasteParameter(a,1,5,0)); // no duplicated groups
 check(moveSongParameter(e,3,-1));
 check((e.order==std::vector<int>{3,1}));
 check(moveSongParameter(e,5,-1));
 check((e.order==std::vector<int>{1,3}));
 TrackerSongHistory history;TrackerSong initial=a;
 cb.copySteps(0,0,2,5,a);check(cb.steps.size()==4);
 check(!cb.steps[0].has_value()&&cb.steps[1].has_value());
 check(cb.pasteSteps(a,2,8)==4);
 check(a.findStep(2,8)==nullptr&&a.findStep(2,9)!=nullptr);
 history.record(0,initial);
 a.mixerVolumes[0]=64;a.mixerMuteMask=1;
 check(history.restore(0,a,false));check(a.mixerVolumes[0]==100&&a.findStep(2,9)==nullptr);
 check(history.restore(0,a,true));check(a.mixerVolumes[0]==64&&a.findStep(2,9)!=nullptr);
 cb.copySong(0,a);TrackerSong target;target.name="My destination";
 check(cb.pasteSong(target)&&target.name=="My destination"&&target.mixerMuteMask==1);
 check(eraseSongSteps(target,0,2,4)==3&&target.findStep(0,3)==nullptr);
 check(target.findStep(1,5)!=nullptr);
 check(songClampValue(19,100)==100);
 return 0;
}
