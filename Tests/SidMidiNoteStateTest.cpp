#include "../Source/SidMidiNoteState.h"
#include <cassert>
#include <iostream>
using biscot::SidMidiNoteState;
using biscot::SidHeldStack;
using biscot::sidShouldRetrigger;
int main(){
 // Single voice, Legato: last-note priority, return without a new Note On.
 SidMidiNoteState s;
 auto a=s.noteOn(2,1,60,true);
 assert(a.filterRetrigger && a.trigger[0] && !a.wasHeld[0]);
 a=s.noteOn(2,1,64,true);
 assert(!a.filterRetrigger && a.trigger[0] && a.wasHeld[0]);
 assert(s.voices[0].top()==64);
 auto b=s.noteOff(2,1,64);
 assert(b.affected[0] && b.voiceTop[0]==60 && b.filterTop==60 && !b.filterReleased);
 b=s.noteOff(2,1,60);
 assert(b.voiceTop[0]==-1 && b.filterReleased);
 // In MIDI Channels 1-3, Op2 retains its own stack; channel 7 only feeds filter.
 s.clear();
 a=s.noteOn(2,2,55,false);
 assert(a.trigger[1]&&!a.trigger[0]&&!a.trigger[2]);
 a=s.noteOn(2,7,79,false);
 assert(!a.trigger[0]&&!a.trigger[1]&&!a.trigger[2]);
 b=s.noteOff(2,7,79);
 assert(b.voiceTop[1]==-1 && s.voices[1].top()==55 && b.filterTop==55);
 // Chord allocate: first free Op, then Op1 when all three are occupied.
 s.clear();
 a=s.noteOn(1,1,60,true);assert(a.trigger[0]&&!a.wasHeld[0]);
 a=s.noteOn(1,5,64,true);assert(a.trigger[1]&&!a.wasHeld[1]);
 a=s.noteOn(1,9,67,true);assert(a.trigger[2]&&!a.wasHeld[2]);
 a=s.noteOn(1,12,72,true);assert(a.trigger[0]&&a.wasHeld[0]);
 assert(s.voices[0].top()==72 && s.voices[1].top()==64 && s.voices[2].top()==67);
 b=s.noteOff(1,12,72);
 assert(b.voiceTop[0]==60 && b.voiceTop[1]==64 && b.voiceTop[2]==67);
 b=s.noteOff(1,9,67);assert(b.voiceTop[2]==-1);
 a=s.noteOn(1,1,76,true);assert(a.trigger[2]&&!a.wasHeld[2]);
 // Duplicate Note Ons MUST remain held until both Note Off events.
 s.clear();
 s.noteOn(2,1,60,true);
 a=s.noteOn(2,1,60,true);
 assert(a.wasHeld[0] && s.voices[0].count==2);
 b=s.noteOff(2,1,60);
 assert(s.voices[0].count==1 && b.voiceTop[0]==60 && !b.filterReleased);
 b=s.noteOff(2,1,60);
 assert(s.voices[0].empty() && b.filterReleased);
 assert(sidShouldRetrigger(false,false,true));
 assert(!sidShouldRetrigger(false,true,true));
 assert(sidShouldRetrigger(false,true,false));
 assert(sidShouldRetrigger(true,false,true));
 assert(!sidShouldRetrigger(true,true,false));
 // JSFX removes newest matching occurrence, preserving earlier order.
 SidHeldStack stack;
 stack.add(60); stack.add(62); stack.add(60);
 assert(stack.removeNewest(60) && stack.count==2 && stack.top()==62);
 assert(stack.removeNewest(60) && stack.count==1 && stack.top()==62);
 assert(!stack.removeNewest(60) && stack.top()==62);
 // JSFX overflow: newest 16, oldest evicted.
 for(int i=0;i<20;++i)stack.add(i);
 assert(stack.count==16 && stack.notes[0]==4 && stack.top()==19);
 // Op1's legato setting also controls filter triggering for non-Op1 MIDI.
 s.clear();
 a=s.noteOn(2,1,60,true);assert(a.filterRetrigger);
 a=s.noteOn(2,2,64,true);assert(!a.filterRetrigger);
 a=s.noteOn(2,3,67,false);assert(a.filterRetrigger);
 // Unison (all same note) is one stack per operator; one off returns all.
 s.clear();
 s.noteOn(0,1,60,true);s.noteOn(0,1,62,true);
 b=s.noteOff(0,1,62);
 for(int i=0;i<3;++i)assert(b.voiceTop[i]==60);
 // All Notes Off reset clears every MIDI stack.
 s.clear();
 assert(s.filter.empty()&&s.chord.empty());
 for(const auto& op:s.voices)assert(op.empty());
 std::cout << "SidMidiNoteStateTest: passed\n";
}
