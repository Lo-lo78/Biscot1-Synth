#pragma once
// Filter ADSR / LFO gate sharing the JSFX Op1 Trigger/Legato musical policy.
// An already sounding ARP can restart its step sequence without reattacking
// either the filter envelope or the LFO phases.
namespace biscot {
inline bool sidFilterLfoShouldTrigger(bool filterRequested, bool op1Legato,
                                     bool filterPedalLatched, bool continuingArp) noexcept {
 return filterRequested && !(op1Legato && filterPedalLatched) && !continuingArp;
}
}
