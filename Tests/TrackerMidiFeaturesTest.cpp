// SPDX-License-Identifier: AGPL-3.0-or-later
#include "../Source/TrackerMidiRouting.h"
#include "../Source/TrackerStepRecording.h"
#include <iostream>
#include <cstdlib>
#include <cmath>
using namespace biscot;
static void require(bool ok,const char* msg){if(!ok){std::cerr<<msg<<'\n';std::exit(1);}}
int main(){
 TrackerPatternBank bank;
 auto& p=bank.at(0);p.midiNote=49;p.releaseArp=true;
 auto& a=p.cells(0,0);
 a.clear();a={{1,60},{2,0.92},{2022,2},{7,64},{7,67},{7,72}};
 auto& b=p.cells(1,0);b={{6,1}};
 auto& c=p.cells(2,0);c={{1,65}};
 auto play=compileTrackerPattern(p);
 require(play.releaseArp,"Release Arpeggio did not compile");
 require(play.rows[0].subNotes.size()==3,"Sub Note pool lost chord tones");
 auto disabled=p;p.cells(0,0)[2].value=0;auto disabledPlay=compileTrackerPattern(p);
 require(disabledPlay.rows[0].subNotes.empty(),"Disabled arpeggiator must not launch Sub Notes");
 p.cells(0,0)[2].value=2;
 require(play.rows[0].subNotes[0]==64 && play.rows[0].subNotes[2]==72,"Sub Note order changed");
 require(play.rows[1].pause,"Pause lost during compilation");
 require(trackerReleaseArpNote(play,0,-1)==60,"Release arp wrong first note");
 require(trackerReleaseArpNote(play,1,60)==60,"Release arp must carry note through Pause");
 require(trackerReleaseArpNote(play,2,60)==65,"Release arp must adopt following Note");
 auto& other=bank.at(8);other.midiNote=51;other.cells(0,0).clear();other.cells(0,0)={{1,72}};
 auto midi=compileTrackerMidiMap(bank);
 require(midi.patternIndex[49]==0 && midi.patternIndex[51]==8,"MIDI Pattern mapping invalid");
 require(midi.patternIndex[50]==-1,"Unassigned notes should pass through");
 require(midi.atNote[51] && midi.atNote[51]->rows[0].note==72,"MIDI mapped pattern contents invalid");
 // Duplicate notes: active Pattern wins, preserving original Lua priority.
 other.midiNote=49;bank.activePattern=8;
 auto priority=compileTrackerMidiMap(bank);
 require(priority.patternIndex[49]==8,"Active Pattern priority was lost");
 TrackerMidiInputQueue q;TrackerRecordedMidiEvent event;
 require(q.push({0,71}) && q.push({1,0}) && q.push({2,0}),"MIDI push failed");
 require(q.pop(event) && event.type==0 && event.note==71,"Note On order failed");
 require(q.pop(event) && event.type==1,"Pedal down order failed");
 require(q.pop(event) && event.type==2,"Pedal up order failed");
 require(!q.pop(event),"Empty MIDI queue must not return stale event");
 for(int i=0;i<255;++i)require(q.push({0,i%128}),"Bounded queue filled too early");
 require(!q.push({0,50}),"Bounded queue must not overwrite unread MIDI");
 for(int i=0;i<255;++i)require(q.pop(event) && event.note==i%128,"Bounded queue FIFO failed");
 // Note recording and pedal rest: replace only Note/Pause; preserve parameter commands.
 TrackerPattern record(false);record.length=4;
 record.cells(0,0)={{6,1},{2004,2}};
 require(trackerRecordStep(record,0,63,false),"Failed recording note");
 require(record.view(0,0).size()==2 && record.view(0,0)[0].id==1 && record.view(0,0)[0].value==63,"Recording note removed unrelated OP cell");
 require(trackerRecordStep(record,0,0,true),"Failed recording pedal pause");
 require(record.view(0,0)[0].id==6 && record.view(0,0)[1].id==2004,"Pedal pause did not replace note");
 require(!trackerRecordStep(record,9,72,false),"Cannot write beyond Pattern length");
 std::cout<<"OK MIDI Pattern mapping, Sub Note pools, pitch-only release and step recording\n";
}
