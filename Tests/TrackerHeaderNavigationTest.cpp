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
 CHECK(!trackerHeaderShortcut('x',false,length,repeats,5,0).valid);
 auto single=trackerHeaderShortcut('d',false,length,1,-1,0);
 CHECK(single.row==length+1 && single.cell==0);
 std::cout<<"OK F1 remembered step and L/M/S/R/P D/T cyclic navigation\n";
}
