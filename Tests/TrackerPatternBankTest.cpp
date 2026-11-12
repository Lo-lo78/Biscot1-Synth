#include "../Source/TrackerPatternBank.h"
#include <iostream>
// Keep regression checks active in GitHub Actions Release builds (-DNDEBUG).
// CHECK always evaluates its argument, unlike assert, and prints the failure.
#define CHECK(expr) do { if (!(expr)) { \
    std::cerr << "CHECK failed: " << #expr << " at " << __FILE__ << ":" << __LINE__ << "\n"; \
    return 1; \
} } while (false)
int main(){
 using namespace biscot;
 TrackerPatternBank b;auto& a=b.at(0);
 CHECK(a.length==16 && a.view(0,0).size()==8 && a.view(15,0).size()==8);
 CHECK(a.view(16,0).empty());
 CHECK(a.insert(4,1,0,{1108,200}) && a.insert(4,2,0,{1123,4}));
 CHECK(a.totalCells(4)==10 && a.view(4,1)[0].id==1108);
 CHECK(a.erase(4,1,0) && a.totalCells(4)==9);
 CHECK(!a.erase(5,1,0));
 for(int i=0;i<119;++i)CHECK(a.insert(4,0,(int)a.view(4,0).size(),{1,(double)i}));
 CHECK(a.totalCells(4)==128 && !a.insert(4,0,0,{1,99}));
 CHECK(a.insert(1023,0,0,{1,60})==false); // length initially 16
 a.length=1024;CHECK(a.insert(1023,0,0,{1,60}));
 b.at(999).name="end";CHECK(b.find(999)->name=="end");
 b.reset();CHECK(b.find(999)==nullptr && b.find(0)!=nullptr);
}
