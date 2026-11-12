// F1 header direct-key navigation and the remembered edit cursor.
#include "../Source/TrackerHeaderNavigation.h"
#include <iostream>
#define CHECK(x) do {if(!(x)){std::cerr<<"FAILED "<<#x<<" line "<<__LINE__<<"\n";return 1;}}while(0)
int main(){
 using namespace biscot;
 constexpr int length=16,repeats=13;
 CHECK(trackerReturnToLastStep(length,length,5)==5);
 CHECK(trackerReturnToLastStep(length,length,0)==0);
 CHECK(trackerReturnToLastStep(length,length,32)==15);
 CHECK(trackerReturnToLastStep(length+1,length,5)==length);
 const auto one=trackerHeaderShortcut('l',false,length,repeats,5,0);
 CHECK(one.valid && one.row==-1 && one.cell==0);
 for(auto [key,pos]: {std::pair{'m',1},std::pair{'s',2},std::pair{'r',3}}){
  auto target=trackerHeaderShortcut(key,false,length,repeats,5,3);
  CHECK(target.row==-1 && target.cell==pos);
 }
 auto p=trackerHeaderShortcut('p',false,length,repeats,5,0);
 CHECK(p.valid && p.row==length && p.cell==0);
 // Forward D and T visit each of 13 positions across three actual rows.
 for(char ch: {'d','t'}){
  auto cursor=trackerHeaderShortcut(ch,false,length,repeats,-1,0);
  CHECK(cursor.valid && cursor.row==length+1 && cursor.cell==(ch=='t'?1:0));
  for(int idx=1;idx<repeats;++idx){
   cursor=trackerHeaderShortcut(ch,false,length,repeats,cursor.row,cursor.cell);
   CHECK(cursor.row==length+1+idx/5 && cursor.cell==(idx%5)*2+(ch=='t'?1:0));
  }
  cursor=trackerHeaderShortcut(ch,false,length,repeats,cursor.row,cursor.cell);
  CHECK(cursor.row==length+1 && cursor.cell==(ch=='t'?1:0));
  cursor=trackerHeaderShortcut(ch,true,length,repeats,cursor.row,cursor.cell);
  CHECK(cursor.row==length+3 && cursor.cell==2*2+(ch=='t'?1:0));
  // Reverse from anywhere that is not the same kind starts at the last item.
  cursor=trackerHeaderShortcut(ch,true,length,repeats,-1,0);
  CHECK(cursor.row==length+3 && cursor.cell==2*2+(ch=='t'?1:0));
 }
 // Vertical arrows (and PageUp/PageDown) cannot cross the three areas.
 CHECK(trackerMoveWithinF1Region(-1,1,length,repeats)==-1);
 CHECK(trackerMoveWithinF1Region(-1,-20,length,repeats)==-1);
 CHECK(trackerMoveWithinF1Region(0,-1,length,repeats)==0);
 CHECK(trackerMoveWithinF1Region(0,-20,length,repeats)==0);
 CHECK(trackerMoveWithinF1Region(length-1,1,length,repeats)==length-1);
 CHECK(trackerMoveWithinF1Region(length-1,20,length,repeats)==length-1);
 CHECK(trackerMoveWithinF1Region(5,5,length,repeats)==10);
 CHECK(trackerMoveWithinF1Region(length,-1,length,repeats)==length);
 CHECK(trackerMoveWithinF1Region(length,-25,length,repeats)==length);
 CHECK(trackerMoveWithinF1Region(length,1,length,repeats)==length+1);
 CHECK(trackerMoveWithinF1Region(length+3,-1,length,repeats)==length+2);
 CHECK(trackerMoveWithinF1Region(length+3,1,length,repeats)==length+3);
 CHECK(trackerMoveWithinF1Region(length+3,25,length,repeats)==length+3);
 // Ctrl+Home/End cross areas; the remembered step survives footer navigation.
 CHECK(trackerF1HomeTarget(5,length,5)==-1);
 CHECK(trackerF1HomeTarget(length,length,5)==5);
 CHECK(trackerF1HomeTarget(length+3,length,5)==5);
 CHECK(trackerF1HomeTarget(-1,length,5)==-1);
 CHECK(trackerF1EndTarget(-1,length,repeats,5)==5);
 CHECK(trackerF1EndTarget(5,length,repeats,5)==length);
 CHECK(trackerF1EndTarget(length,length,repeats,5)==length);
 CHECK(trackerF1EndTarget(length+2,length,repeats,5)==length+2);
 // Plain Home and End remain in the current area, even if footer
 // spans several rows and its last row has fewer than five repeats.
 for(int headerCell=0;headerCell<4;++headerCell){
  const auto h=trackerF1AreaEdge(-1,headerCell,length,repeats,false);
  const auto e=trackerF1AreaEdge(-1,headerCell,length,repeats,true);
  CHECK(h.row==-1 && h.cell==headerCell);
  CHECK(e.row==-1 && e.cell==headerCell);
 }
 CHECK(trackerF1AreaEdge(5,3,length,repeats,false).row==0);
 CHECK(trackerF1AreaEdge(5,3,length,repeats,true).row==15);
 CHECK(trackerF1AreaEdge(5,3,length,repeats,false).cell==3);
 CHECK(trackerF1AreaEdge(length+2,2,length,repeats,false).row==length);
 CHECK(trackerF1AreaEdge(length+2,2,length,repeats,false).cell==0);
 const auto last=trackerF1AreaEdge(length,0,length,repeats,true);
 CHECK(last.row==length+3 && last.cell==5);  // Transpose 13.
 CHECK(trackerF1AreaEdge(length+3,5,length,repeats,true).cell==5);
 CHECK(trackerF1EndTarget(-1,length,repeats,5)==5);
 CHECK(trackerF1HomeTarget(length+3,length,5)==5);
 CHECK(trackerF1AreaEdge(1,0,1,1,false).row==1);
 CHECK(trackerF1AreaEdge(0,0,1,1,true).row==0);
 CHECK(trackerF1AreaEdge(1,0,1,1,true).row==2);
 CHECK(trackerF1AreaEdge(1,0,1,1,true).cell==1);
 // Dynamic Pattern bounds / one repeat and one step.
 CHECK(trackerMoveWithinF1Region(0,1,1,1)==0);
 CHECK(trackerMoveWithinF1Region(1,-1,1,1)==1);
 CHECK(trackerMoveWithinF1Region(2,3,1,1)==2);
 CHECK(trackerF1HomeTarget(2,1,500)==0);
 CHECK(!trackerHeaderShortcut('x',false,length,repeats,5,0).valid);
 auto single=trackerHeaderShortcut('d',false,length,1,-1,0);
 CHECK(single.row==length+1 && single.cell==0);
 std::cout<<"OK F1 region boundaries, remembered step and direct header shortcuts\n";
}
