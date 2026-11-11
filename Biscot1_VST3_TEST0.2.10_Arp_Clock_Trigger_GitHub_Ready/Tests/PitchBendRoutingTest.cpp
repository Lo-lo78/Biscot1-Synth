#include "../Source/PitchBendRouting.h"
#include <array>
#include <cmath>
#include <stdexcept>
#include <iostream>

using namespace biscot;
static void require(bool ok, const char* label) { if (!ok) throw std::runtime_error(label); }
int main() {
 auto route = [](int channel, float wheel, int midiMode, int bendMode,
                 std::array<float,3>& voices) {
  routePitchBend(channel, wheel, midiMode, bendMode,
                 [&voices](int op,float value){ voices[(size_t)op]=value; });
 };
 // Multi is unchanged: 1, 2 and 3 each control their own voice.
 std::array<float,3> bends{};
 route(1,0.25f,2,0,bends);require(bends==std::array<float,3>{0.25f,0.f,0.f},"multi ch1");
 route(2,-0.5f,2,0,bends);require(bends==std::array<float,3>{0.25f,-0.5f,0.f},"multi ch2");
 route(3,1.f,2,0,bends);require(bends==std::array<float,3>{0.25f,-0.5f,1.f},"multi ch3");
 for(int channel=4;channel<=16;++channel){
  auto old=bends;route(channel,0.f,2,0,bends);require(bends==old,"multi ignores 4..16");
 }
 // All Operators Same Note preserves the prior ch1 -> 3 voices behaviour.
 bends={};route(1,0.4f,0,0,bends);
 require(bends==std::array<float,3>{0.4f,0.4f,0.4f},"legacy same-note mode");
 // Omni routes a wheel from any of the 16 channels to all 3 active operators.
 for(int channel=1;channel<=16;++channel){
  bends={0.8f,0.3f,-0.2f};
  const float wheel=channel%2==0?-0.5f:0.625f;
  for(int midiMode=0;midiMode<3;++midiMode){
   route(channel,wheel,midiMode,1,bends);
   require(bends==std::array<float,3>{wheel,wheel,wheel},"omni channel and mode");
  }
 }
 bends={0.1f,0.2f,0.3f};route(0,-1.f,2,1,bends);route(17,-1.f,2,1,bends);
 require(bends==std::array<float,3>{0.1f,0.2f,0.3f},"invalid MIDI channels");
 route(7,0.f,1,1,bends);
 require(bends==std::array<float,3>{0.f,0.f,0.f},"omni center resets all voices");
 std::cout << "OK: Multi legacy routing, Omni channels 1-16, center reset, invalid channels" << std::endl;
}
