#include "../Source/TrackerCellPresentation.h"
#include <iostream>
#include <string>
#define CHECK(expr) do { if(!(expr)) { std::cerr << "CHECK failed: " << #expr \
 << " (line " << __LINE__ << ")\n";return 1;} } while(false)
int main(){
 using namespace biscot;
 // Synth enums must be spoken by their actual names, not raw numeric values.
 CHECK(trackerCellLabel({2004,2})=="OP Wave, Pulse");
 CHECK(trackerCellLabel({2000,0})=="OP Mode Trigger, Trigger");
 CHECK(trackerCellLabel({2000,1})=="OP Mode Trigger, Legato");
 CHECK(trackerCellLabel({2022,1})=="OP Arp Mode, SID Seq");
 CHECK(trackerCellLabel({2016,3})=="OP LFO Rate Mod Target, LFO3");
 CHECK(trackerCellLabel({2250,1})=="OP Sustain Pedal, On");
 CHECK(trackerCellLabel({2251,1})=="Pitch Bend Mode, Omni");
 CHECK(trackerCellLabel({2252,1})=="Chord Op1 Controls, Op1 Master");
 CHECK(trackerCellLabel({1110,0})=="Filter Low Pass, Off");
 CHECK(trackerCellLabel({1110,1})=="Filter Low Pass, On");
 CHECK(trackerCellLabel({1122,0})=="LFO1 Mode, Free");
 CHECK(trackerCellLabel({1124,4})=="LFO1 Wave, Square");
 CHECK(trackerCellLabel({1147,6})=="LFO2 Wave, Smooth Random");
 CHECK(trackerCellLabel({1170,5})=="LFO3 Wave, Sample Hold");
 CHECK(trackerCellLabel({9,0})=="Curve Ratio, Off");
 CHECK(trackerCellLabel({9,6})=="Curve Ratio, 1 to 3");
 CHECK(trackerCellLabel({3,1})=="Previous value");
 CHECK(trackerCellLabel({6,1})=="Pause");
 CHECK(trackerCellLabel({1,60})=="Note, 60 C 4");
 CHECK(trackerCellLabel({1231,0.5})=="LFO1 Square PWM, 0.5000");
 // Persistent logical column must survive shorter or empty steps.
 const int logical=7;
 CHECK(trackerVisibleCell(logical,8)==7);
 CHECK(trackerVisibleCell(logical,2)==1);
 CHECK(trackerVisibleCell(logical,0)==0);
 CHECK(trackerVisibleCell(logical,8)==7);
 CHECK(trackerVisibleCell(2,3)==2);
 CHECK(trackerVisibleCell(2,1)==0);
 CHECK(trackerVisibleCell(2,4)==2);
 return 0;
}
