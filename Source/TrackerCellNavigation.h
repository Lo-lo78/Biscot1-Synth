// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include "TrackerPatternBank.h"
#include <utility>

namespace biscot {
// JUCE ComboBox selection notifications can arrive after the grid has been
// repopulated for another step. Re-selecting the already visible item is not
// an intentional horizontal movement: it must never erase the remembered
// logical column used by the original TrackerControl Lua (0.166).
inline bool trackerCellSelectionWasChanged(int notifiedCell,int visibleCell) noexcept {
 return notifiedCell!=visibleCell;
}

// Search the same parameter ID through subsequent/preceding *steps* in the
// currently selected OP/Filter/LFO section. Skip steps without the parameter;
// retain the stored order of cells and choose the first matching occurrence.
// No allocation and no modification of Pattern data.
inline std::pair<int,int> trackerFindMatchingParameter(
 const TrackerPattern& p,int currentStep,int tab,int parameterId,int direction) noexcept {
 if(direction!=1 && direction!=-1) return {-1,-1};
 if(tab<0 || tab>2 || currentStep<0 || currentStep>=p.length) return {-1,-1};
 for(int step=currentStep+direction;step>=0 && step<p.length;step+=direction){
  const auto& cells=p.view(step,tab);
  for(int index=0;index<(int)cells.size();++index)
   if(cells[(size_t)index].id==parameterId) return {step,index};
 }
 return {-1,-1};
}
} // namespace biscot
