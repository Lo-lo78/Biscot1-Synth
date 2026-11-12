#include "../Source/TrackerPatternBank.h"
#include "../Source/IncrementalParameterSearch.h"
#include <cassert>
#include <string>
#include <vector>
int main(){
 using namespace biscot;
 const std::vector<std::wstring> names={L"Note", L"LFO1 Mode", L"LFO1 Rate",
   L"LFO1 Square PWM", L"LFO2 Mode", L"LFO2 Square PWM", L"LFO3 Wave"};
 IncrementalParameterSearch search;
 const auto label=[&](int i){return names[(size_t)i];};
 const int count=(int)names.size();
 int pos=search.next(L's',0,count,1,1000,label);
 assert(pos==3); // word prefix behind LFO1
 pos=search.next(L'q',pos,count,1,1050,label);
 assert(pos==3); // multiple letters select the same item
 pos=search.next(L's',pos,count,1,2100,label);
 assert(pos==5); // a new single-letter search after timeout
 pos=search.next(L's',pos,count,1,2150,label);
 assert(pos==3); // repeated key cycles, including wrap-around
 pos=search.next(L'r',pos,count,-1,3500,label);
 assert(pos==2); // Shift+R: reverse search from current
 search.reset();
 pos=search.next(L'w',0,count,1,5000,label);
 assert(pos==6); // LFO3 Wave is reachable from the generic LFO section

 TrackerPatternBank bank;
 auto& p=bank.at(0);
 assert(p.insert(0,1,0,{1108,900}));
 assert(p.insert(0,2,0,{1231,0.25}));
 assert(p.insert(0,2,1,{1232,0.50}));
 assert(p.insert(0,2,2,{1233,0.75}));
 assert(p.view(0,2).size()==3); // three independent LFO parameter cells in one step
 const TrackerStep snapshot=p.steps.at(0);
 p.steps[3]=snapshot;
 assert(p.view(3,0).size()==8); // all OP cells copied
 assert(p.view(3,1).size()==1 && p.view(3,2).size()==3);
 p.cells(0,2)[0].value=0.9;
 assert(p.view(3,2)[0].value==0.25); // independent deep clone
 p.steps.erase(0);
 assert(p.view(0,2).empty() && p.view(3,2).size()==3);
 assert(bank.find(0)->view(3,0).size()==8);
 return 0;
}
