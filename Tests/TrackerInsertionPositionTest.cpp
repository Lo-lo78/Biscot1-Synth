// SPDX-License-Identifier: AGPL-3.0-or-later
#include "../Source/TrackerPatternInsert.h"
#include "../Source/TrackerPatternPlayback.h"
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>
using namespace biscot;
static void require(bool condition,const char* what){if(!condition)throw std::runtime_error(what);}
static TrackerPattern blank(){TrackerPattern p(false);p.length=16;return p;}
static void expectIds(const std::vector<TrackerCell>& cells,std::initializer_list<int> ids){
 require(cells.size()==ids.size(),"Number of cells not preserved");
 size_t i=0;for(const int id:ids)require(cells[i++].id==id,"Unexpected order of cells");
}
int main(){try{
 {
  auto p=blank();
  p.cells(1,0)={{1,60},{2,0.9},{2004,2},{2002,1}};
  // Third position is zero-based index 2. Old third moves to fourth.
  const auto r=pasteTrackerParameter(p,1,0,2,{2005,7});
  require(r.status==TrackerInsertStatus::inserted && r.index==2,"Paste must select third cell");
  expectIds(p.view(1,0),{1,2,2005,2004,2002});
  require(p.view(1,0)[2].value==7,"Paste preserves copied value");
  const auto dup=pasteTrackerParameter(p,1,0,1,{2005,13});
  require(dup.status==TrackerInsertStatus::alreadyPresent && dup.index==2,"Existing param cannot duplicate");
  expectIds(p.view(1,0),{1,2,2005,2004,2002});
  require(p.view(1,0)[2].value==7,"Duplicate paste does not overwrite existing value");
  const auto atFirst=pasteTrackerParameter(p,1,0,0,{2008,0.25});
  require(atFirst.index==0,"Paste can insert at first cell");
  expectIds(p.view(1,0),{2008,1,2,2005,2004,2002});
  const auto atEnd=pasteTrackerParameter(p,1,0,100,{2011,0.25});
  require(atEnd.index==6,"Paste beyond end appends");
 }
 {
  auto p=blank();
  p.cells(2,0)={{1,60},{2,.92},{2004,2}};
  const auto* def=findTrackerParam(2005);
  require(def!=nullptr,"OP Pitch definition exists");
  const auto result=insertTrackerParameter(p,2,0,*def,1);
  require(result.status==TrackerInsertStatus::inserted && result.index==1,"Picker inserts at selected second cell");
  expectIds(p.view(2,0),{1,2005,2,2004});
  const auto old=insertTrackerParameter(p,2,0,*def,2);
  require(old.status==TrackerInsertStatus::alreadyPresent && old.index==1,"Picker reselects existing parameter");
  expectIds(p.view(2,0),{1,2005,2,2004});
  // No positional argument keeps append for legacy callers.
  auto r=insertTrackerParameter(p,2,0,*findTrackerParam(2011));
  require(r.index==4,"Legacy default append preserved");
 }
 {
  auto p=blank();p.cells(3,1)={{1108,200},{1110,1}};
  const auto r=insertTrackerParameter(p,3,1,*findTrackerParam(9),1);
  require(r.status==TrackerInsertStatus::inserted && r.index==2,"Ratio follows auto Curve at insertion point");
  expectIds(p.view(3,1),{1108,4,9,1110});
  auto res=insertTrackerParameter(p,3,1,*findTrackerParam(8),3);
  require(res.status==TrackerInsertStatus::inserted && res.index==2,"Resolution before existing Ratio");
  expectIds(p.view(3,1),{1108,4,8,9,1110});
  require(p.view(3,1)[1].value==100,"Automatically inserted Curve initialized to 100");
  // All three sections use the same insert-at-position rule.
  p.cells(3,2)={{1122,0},{1123,3},{1124,1}};
  const auto lf=insertTrackerParameter(p,3,2,*findTrackerParam(1127),1);
  require(lf.index==1,"LFO insert at selected cell");
  expectIds(p.view(3,2),{1122,1127,1123,1124});
 }
 {
  // Previous Value is a chronological marker, not a global step flag.
  auto p=blank();p.cells(0,0)={{2005,14}};
  p.cells(1,0)={{1,60},{3,1},{2,0.92}};
  const auto* pitch=findTrackerParam(2005);
  auto a=insertTrackerParameter(p,1,0,*pitch,0);
  require(a.index==0 && std::abs(p.view(1,0)[0].value-pitch->initial)<1.e-12,
          "Insertion before Previous Value must not inherit");
  auto p2=blank();p2.cells(0,0)={{2005,14}};p2.cells(1,0)={{1,60},{3,1},{2,0.92}};
  const auto b=insertTrackerParameter(p2,1,0,*pitch,2);
  require(b.index==2 && p2.view(1,0)[2].value==14,
          "Insertion after Previous Value should inherit from preceding step");
 }
 {
  auto p=blank();p.cells(0,0)={{1,60},{2022,1},{2004,2}};
  require(pasteTrackerParameter(p,0,0,1,{7,64}).status==TrackerInsertStatus::needsArpeggiator,
          "Sub Note cannot precede Arp Mode");
  require(pasteTrackerParameter(p,0,0,2,{7,64}).status==TrackerInsertStatus::inserted,
          "Sub Note can follow Arp Mode");
  require(pasteTrackerParameter(p,0,0,3,{7,67}).status==TrackerInsertStatus::inserted,
          "Sub Note can be repeated");
  expectIds(p.view(0,0),{1,2022,7,7,2004});
 }
 {
  auto p=blank();
  for(int i=0;i<128;++i)p.cells(0,0).push_back({1+i,(double)i});
  const auto before=p.view(0,0).size();
  require(pasteTrackerParameter(p,0,1,0,{1108,300}).status==TrackerInsertStatus::full,
          "All sections share a 128-cell limit");
  require(insertTrackerParameter(p,0,1,*findTrackerParam(1108),0).status==TrackerInsertStatus::full,
          "Picker observes 128-cell limit");
  require(p.view(0,0).size()==before,"Rejected insertion cannot change state");
 }
 std::cout<<"F1 parameter paste/picker insert at selected cell: PASS\n";
 return 0;
}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<"\n";return 1;}}
