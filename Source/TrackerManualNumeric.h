// SPDX-License-Identifier: AGPL-3.0-or-later
// Locale-independent numeric entry helper for the native Biscot1 tracker.
#pragma once
#include <algorithm>
#include <charconv>
#include <cmath>
#include <string>
#include <string_view>
#include <system_error>

namespace biscot {
inline bool parseTrackerManualNumeric(std::string_view input,double& result){
 const auto left=input.find_first_not_of(" \t\r\n");
 if(left==std::string_view::npos)return false;
 const auto right=input.find_last_not_of(" \t\r\n");
 std::string token(input.substr(left,right-left+1));
 std::replace(token.begin(),token.end(),',','.');
 // std::from_chars accepts a leading minus but not a leading plus.
 if(!token.empty() && token.front()=='+')token.erase(token.begin());
 if(token.empty())return false;
 double value=0.0;
 const auto converted=std::from_chars(token.data(),token.data()+token.size(),value,
                                       std::chars_format::general);
 if(converted.ec!=std::errc{} || converted.ptr!=token.data()+token.size() ||
    !std::isfinite(value))return false;
 result=value;
 return true;
}
}
