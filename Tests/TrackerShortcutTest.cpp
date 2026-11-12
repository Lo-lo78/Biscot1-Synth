#include "../Source/TrackerShortcutKey.h"
#include "../Source/TrackerPatternBank.h"
#include <iostream>
#define CHECK(x) do { if(!(x)){ std::cerr << "CHECK failed " << #x << " at line " << __LINE__ << "\n";return 1;} } while(false)
int main(){using namespace biscot;
 for(char c:{'c','x','v','j','z'}){
  CHECK(trackerShortcutLetterMatches(c-'a'+'A',c-'a'+1,c,true));
  CHECK(trackerShortcutLetterMatches(c-'a'+1,c-'a'+1,c,true));
  CHECK(trackerShortcutLetterMatches(c,c,c,false));
  CHECK(!trackerShortcutLetterMatches('D',4,c,false));
 }
 // F1 section numbers must remain functional on both JUCE key paths.
 // Previously a letters-only helper rejected '1', '2' and '3'.
 for(int i=0;i<3;++i){
  const int digit='1'+i;
  CHECK(trackerShortcutSectionIndex(digit,0)==i);
  CHECK(trackerShortcutSectionIndex(0,digit)==i);
  CHECK(trackerShortcutSectionIndex(digit,digit)==i);
  CHECK(!trackerShortcutLetterMatches(digit,digit,'c',false));
 }
 CHECK(trackerShortcutSectionIndex('0','0')==-1);
 CHECK(trackerShortcutSectionIndex('4','4')==-1);
 CHECK(trackerShortcutSectionIndex('c','c')==-1);
 TrackerPatternBank bank;auto& source=bank.at(0);
 CHECK(source.insert(3,2,0,{1231,0.7}));
 const TrackerPattern copy=source;
 auto& target=bank.at(99);target=copy;
 CHECK(target.view(3,2).size()==1);
 source.cells(3,2)[0].value=0.5;
 CHECK(target.view(3,2)[0].value==0.7);
 CHECK(bank.at(999).length==16);
 return 0;
}
