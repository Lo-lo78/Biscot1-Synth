#pragma once
#include <array>

namespace biscot {
// Snapshots only the parameters that change the live classical-arp note
// lists. MIDI NOTE/CC64 events rebuild at their original sample offsets;
// this avoids a 3 x 16 x 128 channel/note scan on every audio block.
struct SidArpPoolSettings {
 int midiMode=0;
 std::array<int,3> arpMode{};
 std::array<int,3> octaveUp{};
 std::array<int,3> octaveDown{};
 std::array<bool,3> pedalEnabled{};
 bool operator==(const SidArpPoolSettings&) const noexcept = default;
};
struct SidArpPoolCache {
 SidArpPoolSettings last{};
 bool initialized=false;
 void invalidate() noexcept { initialized=false; }
 bool needsRefresh(const SidArpPoolSettings& current) const noexcept {
  return !initialized || !(last==current);
 }
 void didRefresh(const SidArpPoolSettings& current) noexcept {
  last=current;
  initialized=true;
 }
};
} // namespace biscot
