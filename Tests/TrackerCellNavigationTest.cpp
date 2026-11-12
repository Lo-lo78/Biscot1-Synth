// F1 navigation regression: Lua 0.166 remembered horizontal position and
// non-contiguous same-parameter step navigation, independent of JUCE.
#include "../Source/TrackerCellNavigation.h"
#include "../Source/TrackerCellPresentation.h"
#include <iostream>
#define CHECK(x) do {if(!(x)){std::cerr<<"FAILED: "<<#x<<" line "<<__LINE__<<"\n";return 1;}}while(0)
int main(){using namespace biscot;
 // Navigation through 8, 2, 8 cells must return to 8, not stick at 2.
 int logical=7;
 int visible=trackerVisibleCell(logical,8);CHECK(visible==7);
 visible=trackerVisibleCell(logical,2);CHECK(visible==1);
 // ComboBox refresh selected the second item, but did not move horizontally.
 CHECK(!trackerCellSelectionWasChanged(1,visible));
 if(trackerCellSelectionWasChanged(1,visible))logical=1;
 visible=trackerVisibleCell(logical,8);CHECK(visible==7);
 // Explicit horizontal move DOES reset the logical position.
 CHECK(trackerCellSelectionWasChanged(0,visible));
 logical=0;visible=trackerVisibleCell(logical,8);CHECK(visible==0);
 TrackerPattern pat(false);pat.length=8;
 pat.cells(0,0)={{1,60},{2004,2},{2002,1}};
 pat.cells(1,0)={{1,62}};
 pat.cells(2,0)={{2004,3},{1,64}};
 pat.cells(4,0)={{2002,0},{2004,1}};
 pat.cells(6,1)={{1108,1200}};pat.cells(7,0)={{1,72}};
 CHECK((trackerFindMatchingParameter(pat,0,0,2004,1)==std::pair<int,int>(2,0)));
 CHECK((trackerFindMatchingParameter(pat,2,0,2004,1)==std::pair<int,int>(4,1)));
 CHECK((trackerFindMatchingParameter(pat,4,0,2004,-1)==std::pair<int,int>(2,0)));
 CHECK((trackerFindMatchingParameter(pat,0,0,2004,-1)==std::pair<int,int>(-1,-1)));
 CHECK((trackerFindMatchingParameter(pat,0,0,1,1)==std::pair<int,int>(1,0)));
 CHECK((trackerFindMatchingParameter(pat,1,0,1,1)==std::pair<int,int>(2,1)));
 CHECK((trackerFindMatchingParameter(pat,0,0,1108,1)==std::pair<int,int>(-1,-1)));
 CHECK((trackerFindMatchingParameter(pat,0,3,2004,1)==std::pair<int,int>(-1,-1)));
 CHECK((trackerFindMatchingParameter(pat,0,0,2004,0)==std::pair<int,int>(-1,-1)));
 CHECK((trackerFindMatchingParameter(pat,6,1,1108,1)==std::pair<int,int>(-1,-1)));
 std::cout<<"OK F1 persistent Lua cursor and matching-parameter step navigation\n";
 return 0;
}
