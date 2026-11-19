// SPDX-License-Identifier: AGPL-3.0-or-later
// TEST0.2.47: REAPER automation while no MIDI gate is held.
// This core is the one used by all three VCAs, Mod ENVs and the shared filter.
#include "../Source/SidAmpEnvelope.h"
#include "../Source/ParameterCatalog.h"
#ifdef NDEBUG
 #undef NDEBUG
#endif
#include <cassert>
#include <cmath>
#include <iostream>
#include <array>
using namespace biscot;

int main() {
 constexpr double sr=1000.;
 // The precise reported failure: Release=0.320, 480 output frames after
 // Note Off, then Release=0. All seven envelopes must stop WITHOUT Note On.
 for(int env=0;env<7;++env){
  SidAmpEnvelope e;
  e.noteOn(true);
  e.tick(0.f,0.f,.1f,.8f,.320f,sr);
  e.noteOff(.320f,sr);
  assert(e.stage==4);
  for(int n=0;n<480;++n)e.tick(0.f,0.f,.1f,.8f,.320f,sr);
  assert(e.stage==4 && e.level>.15f);
  e.tick(0.f,0.f,.1f,.8f,0.f,sr);
  assert(e.stage==0 && e.level==0.f);
  // Changing Release again after the envelope has genuinely ended must not
  // create a ghost note or restart the VCA or modulation envelope.
  for(int n=0;n<100;++n)
   assert(e.tick(0.f,0.f,.1f,.8f,5.f,sr)==0.f && e.stage==0);
 }
 // A very long tail may be curtailed after two seconds at any point.
 SidAmpEnvelope longTail;
 longTail.noteOn(true);longTail.tick(0.f,0.f,0.f,1.f,5.f,sr);
 longTail.noteOff(5.f,sr);
 for(int n=0;n<2000;++n)longTail.tick(0.f,0.f,0.f,1.f,5.f,sr);
 assert(longTail.level>.6f && longTail.stage==4);
 longTail.tick(0.f,0.f,0.f,1.f,0.f,sr);
 assert(longTail.stage==0 && longTail.level==0.f);

 // It also works in the opposite direction (extend the existing Release).
 SidAmpEnvelope extended;
 extended.noteOn(true);extended.tick(0.f,0.f,0.f,1.f,0.2f,sr);
 extended.noteOff(0.2f,sr);
 for(int n=0;n<100;++n)extended.tick(0.f,0.f,0.f,1.f,0.2f,sr);
 const float before=extended.level;
 extended.tick(0.f,0.f,0.f,1.f,2.f,sr);
 assert(extended.stage==4 && extended.level<before);
 assert(std::abs(extended.level-before*(1.f-1.f/2000.f))<1.e-6f);

 // Mid-Hold automation edits the CURRENT plateau, not just the next note.
 SidAmpEnvelope shorten;
 shorten.noteOn(true);shorten.tick(0.f,1.f,.4f,.1f,0.f,sr);
 assert(shorten.stage==5);
 for(int n=0;n<300;++n)shorten.tick(0.f,1.f,.4f,.1f,0.f,sr);
 shorten.tick(0.f,.2f,.4f,.1f,0.f,sr);
 assert(shorten.stage==2 && shorten.level==1.f);
 assert(shorten.tick(0.f,.2f,.4f,.1f,0.f,sr)<1.f);
 SidAmpEnvelope lengthen;
 lengthen.noteOn(true);lengthen.tick(0.f,.1f,.4f,.1f,0.f,sr);
 for(int n=0;n<70;++n)lengthen.tick(0.f,.1f,.4f,.1f,0.f,sr);
 lengthen.tick(0.f,.3f,.4f,.1f,0.f,sr);
 assert(lengthen.stage==5 && lengthen.holdSamplesRemaining==229);
 for(int n=0;n<229;++n)lengthen.tick(0.f,.3f,.4f,.1f,0.f,sr);
 assert(lengthen.stage==2);

 // Unautomated voice output retains exact compatibility with the old tick.
 SidAmpEnvelope reference, live;
 reference.noteOn(true);live.noteOn(true);
 for(int n=0;n<5000;++n){
  if(n==700){reference.noteOff(.5f,sr);live.noteOff(.5f,sr);}
  const float a=reference.tick(.1f,.2f,.4f,.6f,sr);
  const float b=live.tick(.1f,.2f,.4f,.6f,.5f,sr);
  assert(a==b && reference.stage==live.stage);
 }

 // 3 VCAs + 3 Mod ENVs + filter share the same live-automation core.
 for(int op=0;op<3;++op){
  assert(catalog[(size_t)(41+op)].index==43+op);
  assert(catalog[(size_t)(71+op)].index==73+op);
 }
 assert(catalog[119].index==121);
 std::cout << "OK: seven live Release tails, early cut, extend, Hold, parity, no ghost notes\n";
}
