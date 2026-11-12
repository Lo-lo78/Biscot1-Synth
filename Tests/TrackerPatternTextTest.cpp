#include "../Source/TrackerPatternText.h"
#include <iostream>
#define CHECK(x) do {if(!(x)){std::cerr<<"FAILED "<<#x<<" at line "<<__LINE__<<'\n';return 1;}}while(0)
int main(){
 using namespace biscot;
 TrackerPattern original(false);
 original.name="SID 6581 - Bass ";original.name += "\xc3\xa8";
 original.length=1024;original.midiNote=42;original.repeat=3;original.releaseArp=true;
 original.repeatSpeeds={1.25,0.5,16};original.repeatTransposes={0,-12,7};
 original.cells(0,0)={{1,60},{2,0.92},{2004,2}};
 original.cells(0,1)={{1108,-200.125},{4,100},{8,1024}};
 original.cells(42,2)={{1123,1.125},{1150,0.002}};
 original.cells(1023,0)={{1,127}};
 const auto text=serializeTrackerPattern(original,4);
 CHECK(text.find("pattern=4\n")!=std::string::npos);
 CHECK(text.find("step\t1023\t1\n")!=std::string::npos);
 CHECK(text.find("speed\t2\t0.5\n")!=std::string::npos);
 CHECK(text.find("name_hex=")!=std::string::npos);
 int slot=-1;TrackerPattern loaded(false);
 CHECK(deserializeTrackerPattern(text,slot,loaded));
 CHECK(slot==4&&loaded.name==original.name&&loaded.length==1024);
 CHECK(loaded.midiNote==42&&loaded.repeat==3&&loaded.releaseArp);
 CHECK(loaded.repeatSpeeds==original.repeatSpeeds);
 CHECK(loaded.repeatTransposes==original.repeatTransposes);
 for(int step:{0,42,1023})for(int tab=0;tab<3;++tab){
  const auto& a=original.view(step,tab);const auto& b=loaded.view(step,tab);
  CHECK(a.size()==b.size());
  for(size_t n=0;n<a.size();++n)CHECK(a[n].id==b[n].id && a[n].value==b[n].value);
 }
 auto invalid=text;invalid.replace(invalid.find("pattern=4"),9,"pattern=9999");
 CHECK(!deserializeTrackerPattern(invalid,slot,loaded));CHECK(slot==4);
 invalid=text+"cell\t1\tnan\n";CHECK(!deserializeTrackerPattern(invalid,slot,loaded));
 CHECK(serializeTrackerPattern(loaded,slot)==text);
 auto windowsText=text;
 for(size_t at=0;(at=windowsText.find("\n",at))!=std::string::npos;at+=2)windowsText.replace(at,1,"\r\n");
 CHECK(deserializeTrackerPattern(windowsText,slot,loaded));
 CHECK(serializeTrackerPattern(loaded,slot)==text);
 std::cout<<"OK native single-pattern library format, all three tabs and full repeat metadata\n";
}
