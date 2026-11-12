// TEST0.2.22 sparse Song lanes and Lua parameter groups, Release safe.
#include "../Source/TrackerSongBank.h"
#include "../Source/TrackerSongParameters.h"
#include <cstdlib>
using namespace biscot;
static void check(bool result){if(!result)std::abort();}
int main(){
 TrackerSongBank b;
 check(b.songs.size()==1&&b.activeSong==0);
 auto& s=b.at(23);s.laneLengths={16,48,1024};s.laneRepeats={1,3,5};
 check(s.length()==1024&&s.lanes[0].empty()&&s.lanes[1].empty()&&s.lanes[2].empty());
 auto& op1=s.atStep(0,5);check(op1.insert(1));op1.values[1]=4;
 check(op1.insert(3));op1.values[3]=-12;
 const auto visible=op1.visible();
 check((visible==std::vector<int>{1,5,6,21,7,8,3}));
 check(op1.contains(5)&&op1.contains(21)&&!op1.insert(5));
 check(!op1.insert(1)&&op1.erase(1)&&!op1.contains(5));
 auto& op3=s.atStep(2,4095);check(op3.insert(2));
 check(s.lanes[0].size()==1&&s.lanes[1].empty()&&s.lanes[2].size()==1);
 check(b.at(23).laneRepeats[2]==5&&b.at(24).lanes[0].empty());
 check(songParameters.size()==17&&songParam(1)!=nullptr&&songParam(21)==nullptr);
 check(songClampValue(1,9999)==999&&songClampValue(22,38)==31);
 check(songClampValue(3,-120)==-96&&songClampValue(2,1.17)==1.25);
 b.reset();check(b.songs.size()==1&&b.find(23)==nullptr);
 return 0;
}
