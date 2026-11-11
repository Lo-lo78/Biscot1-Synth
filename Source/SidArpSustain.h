#pragma once
// MIDI CC64's held arp-note pool is separate from physically held-note stacks.
// Fixed arrays: never allocates, locks, or scans for allocations on the audio thread.
#include <array>
#include <cstdint>
#include <cstddef>

namespace biscot {
struct SidArpSustain {
 using Channels=std::array<std::array<std::uint64_t,128>,16>;
 std::array<Channels,3> stored{};
 void clear() noexcept {stored={};}
 void hold(int op,int channel,int note,std::uint64_t order) noexcept {
  if(op>=0 && op<3 && channel>=1 && channel<=16 && note>=0 && note<128)
   stored[(std::size_t)op][(std::size_t)(channel-1)][(std::size_t)note]=order;
 }
 void clearChannel(int channel) noexcept {
  if(channel<1||channel>16)return;
  for(auto& op:stored)op[(std::size_t)(channel-1)].fill(0);
 }
 void clearOperator(int op) noexcept {
  if(op<0||op>=3)return;
  for(auto& channel:stored[(std::size_t)op])channel.fill(0);
 }
 std::uint64_t timestamp(int op,int channel,int note) const noexcept {
  if(op<0||op>=3||channel<1||channel>16||note<0||note>=128)return 0;
  return stored[(std::size_t)op][(std::size_t)(channel-1)][(std::size_t)note];
 }
 bool any(int op) const noexcept {
  if(op<0||op>=3)return false;
  for(const auto& channel:stored[(std::size_t)op])
   for(auto stamp:channel)if(stamp)return true;
  return false;
 }
};
// Existing Chord Allocate behavior is unchanged in Independent mode.
// The master control is relevant ONLY in MIDI Mode=Chord Allocate.
inline bool chordMasterEnabled(int midiMode,bool switchOn) noexcept {
 return midiMode==1 && switchOn;
}
inline int chordVoiceIndex(int midiMode,bool switchOn,int op) noexcept {
 return chordMasterEnabled(midiMode,switchOn)?0:op;
}
inline bool midiAddressesOperator(int midiMode,int op,int channel) noexcept {
 return midiMode!=2 || channel==op+1;
}
}
