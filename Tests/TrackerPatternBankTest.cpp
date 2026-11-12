#include "../Source/TrackerPatternBank.h"
#include <cassert>
int main(){
 using namespace biscot;
 TrackerPatternBank b;auto& a=b.at(0);
 assert(a.length==16 && a.view(0,0).size()==8 && a.view(15,0).size()==8);
 assert(a.view(16,0).empty());
 assert(a.insert(4,1,0,{1108,200}) && a.insert(4,2,0,{1123,4}));
 assert(a.totalCells(4)==10 && a.view(4,1)[0].id==1108);
 assert(a.erase(4,1,0) && a.totalCells(4)==9);
 assert(!a.erase(5,1,0));
 for(int i=0;i<119;++i)assert(a.insert(4,0,(int)a.view(4,0).size(),{1,(double)i}));
 assert(a.totalCells(4)==128 && !a.insert(4,0,0,{1,99}));
 assert(a.insert(1023,0,0,{1,60})==false); // length initially 16
 a.length=1024;assert(a.insert(1023,0,0,{1,60}));
 b.at(999).name="end";assert(b.find(999)->name=="end");
 b.reset();assert(b.find(999)==nullptr && b.find(0)!=nullptr);
}
