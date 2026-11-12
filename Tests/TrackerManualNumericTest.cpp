// SPDX-License-Identifier: AGPL-3.0-or-later
// Standalone Release-compatible numeric dialog input regression.
#include "../Source/TrackerManualNumeric.h"
#include <cmath>
#include <iostream>
#include <string_view>
using namespace biscot;
int main(){
 const auto check=[](std::string_view text,bool expected,double v=0.0){
  double found=999.0;
  const bool ok=parseTrackerManualNumeric(text,found);
  if(ok!=expected || (ok && std::abs(found-v)>1e-9)){
   std::cerr<<"BAD numeric input: "<<text<<"\n";
   return false;
  }
  return true;
 };
 bool all=true;
 all &= check("16",true,16);
 all &= check("-1",true,-1);
 all &= check("1000",true,1000);
 all &= check(".125",true,.125);
 all &= check("0,75",true,.75);
 all &= check("  +1,25  ",true,1.25);
 all &= check("-96.25",true,-96.25);
 all &= check("",false);
 all &= check("Speed",false);
 all &= check("1..5",false);
 all &= check("NaN",false);
 all &= check("infinity",false);
 all &= check("1 2",false);
 all &= check("99999999999999999999999999999999999999e+999",false);
 if(!all)return 1;
 std::cout<<"TrackerManualNumericTest passed\n";
 return 0;
}
