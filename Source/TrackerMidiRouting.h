// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include <array>
#include <atomic>
#include <memory>
#include "TrackerPatternPlayback.h"
namespace biscot {
struct TrackerRecordedMidiEvent {int type=0; int note=0;}; // 0 Note On, 1 sustain down, 2 sustain up
// Audio-thread producer, message-thread consumer; no memory allocation or locks.
class TrackerMidiInputQueue {
 std::array<TrackerRecordedMidiEvent,256> data{};
 std::atomic<unsigned> write{0},read{0};
public:
 bool push(TrackerRecordedMidiEvent e) noexcept {
  const auto w=write.load(std::memory_order_relaxed);
  const auto next=(w+1u)%data.size();
  if(next==read.load(std::memory_order_acquire))return false;
  data[w]=e;write.store(next,std::memory_order_release);return true;
 }
 bool pop(TrackerRecordedMidiEvent& e) noexcept {
  const auto r=read.load(std::memory_order_relaxed);
  if(r==write.load(std::memory_order_acquire))return false;
  e=data[r];read.store((r+1u)%data.size(),std::memory_order_release);return true;
 }
};
struct TrackerMidiMap {
 std::array<std::shared_ptr<const TrackerPlayablePattern>,128> atNote{};
 std::array<int,128> patternIndex{};
 TrackerMidiMap(){patternIndex.fill(-1);}
};
inline TrackerMidiMap compileTrackerMidiMap(const TrackerPatternBank& bank){
 TrackerMidiMap result;
 for(const auto& entry:bank.patterns){
  const int note=entry.second.midiNote;
  if(note>=0&&note<128){
   if(result.patternIndex[(size_t)note]==-1||entry.first==bank.activePattern){
    result.atNote[(size_t)note]=std::make_shared<const TrackerPlayablePattern>(compileTrackerPattern(entry.second));
    result.patternIndex[(size_t)note]=entry.first;
   }
  }
 }
 return result;
}
}
