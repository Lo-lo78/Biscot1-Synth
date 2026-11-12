// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
// Separate the physical held-key pool from the last complete arpeggio chord.
// The release chord is captured on NOTE ON (or an explicit arp-configuration
// change), never on NOTE OFF. Successive manual key releases must not shrink
// the release memory to the final physically held key. Pedal and no-pedal
// paths must both preserve the same chord for the VCA's natural Release.
#include <algorithm>
#include <array>

namespace biscot {
template <std::size_t N>
inline void sidLatchArpReleasePool(const std::array<int,N>& live,int liveCount,
                                   std::array<int,N>& released,int& releasedCount,
                                   bool latch) noexcept {
 if(!latch || liveCount<=0)return;
 releasedCount=std::clamp(liveCount,0,(int)N);
 std::copy_n(live.begin(),releasedCount,released.begin());
}
} // namespace biscot
