// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
// Portable, human-readable standalone Pattern export. Deliberately separate
// from the full .biscot preset/REAPER VST3 state serializer.
// Compatible with the legacy Lua field layout (pattern=, step TAB, cell TAB,
// speed TAB, transpose TAB), with an extra UTF-8-safe hexadecimal name field.
#include "TrackerPatternBank.h"
#include <charconv>
#include <cmath>
#include <iomanip>
#include <locale>
#include <map>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>

namespace biscot {
namespace detail {
inline std::string hexBytes(const std::string& bytes){
 constexpr char hex[]="0123456789abcdef";
 std::string out;out.reserve(bytes.size()*2);
 for(unsigned char c: bytes){out.push_back(hex[c>>4]);out.push_back(hex[c&15]);}
 return out;
}
inline bool decodeHex(std::string_view hex, std::string& bytes){
 if(hex.size()%2 || hex.size()>16384)return false;
 auto digit=[](char c)->int{
  if(c>='0'&&c<='9')return c-'0';
  if(c>='a'&&c<='f')return c-'a'+10;
  if(c>='A'&&c<='F')return c-'A'+10;
  return -1;
 };
 bytes.clear();bytes.reserve(hex.size()/2);
 for(size_t i=0;i<hex.size();i+=2){
  const int hi=digit(hex[i]),lo=digit(hex[i+1]);if(hi<0||lo<0)return false;
  bytes.push_back(char((hi<<4)|lo));
 }
 return true;
}
inline bool integer(std::string_view s, int& out){
 if(s.empty())return false;
 const auto [end,err]=std::from_chars(s.data(),s.data()+s.size(),out);
 return err==std::errc{} && end==s.data()+s.size();
}
inline bool real(std::string_view s,double& out){
 std::istringstream stream{std::string(s)};stream.imbue(std::locale::classic());
 stream>>out;
 return !stream.fail() && (stream>>std::ws).eof() && std::isfinite(out);
}
inline bool field(std::string_view line,std::string_view prefix,int& dest){
 return line.starts_with(prefix)&&integer(line.substr(prefix.size()),dest);
}
inline bool pairFields(std::string_view line,std::string_view prefix,int& index,
                       std::string_view& value){
 if(!line.starts_with(prefix))return false;
 line.remove_prefix(prefix.size());const auto tab=line.find('\t');
 if(tab==std::string_view::npos)return false;
 value=line.substr(tab+1);return integer(line.substr(0,tab),index);
}
} // namespace detail

inline std::string serializeTrackerPattern(const TrackerPattern& p,int slot){
 std::ostringstream out;out.imbue(std::locale::classic());
 out<<std::setprecision(17);
 out<<"BISCOT1_TRACKER_PATTERN_V1\n";
 out<<"pattern="<<slot<<'\n';
 out<<"name_hex="<<detail::hexBytes(p.name)<<'\n';
 out<<"length="<<p.length<<'\n';
 out<<"midi_note="<<p.midiNote<<'\n';
 out<<"release_arp="<<(p.releaseArp?1:0)<<'\n';
 out<<"repeat="<<p.repeat<<'\n';
 for(int index=0;index<p.repeat;++index){
  out<<"speed\t"<<index+1<<'\t'<<(index<(int)p.repeatSpeeds.size()?p.repeatSpeeds[(size_t)index]:1.)<<'\n';
  out<<"transpose\t"<<index+1<<'\t'<<(index<(int)p.repeatTransposes.size()?p.repeatTransposes[(size_t)index]:0)<<'\n';
 }
 for(const auto& [step,data]:p.steps){
  if(step<0||step>=TrackerPatternBank::maxSteps)continue;
  for(int tab=0;tab<3;++tab){
   const auto& cells=data.tabs[(size_t)tab];if(cells.empty())continue;
   out<<"step\t"<<step<<'\t'<<tab+1<<'\n';
   for(const auto& c:cells)out<<"cell\t"<<c.id<<'\t'<<c.value<<'\n';
  }
 }
 return out.str();
}
// For the future Song/Pattern library importer, use exactly the same codec.
// A malformed file never mutates an existing Pattern or slot number.
inline bool deserializeTrackerPattern(const std::string& text,int& slot,TrackerPattern& pattern){
 if(text.size()>16*1024*1024)return false;
 std::istringstream in(text);in.imbue(std::locale::classic());
 std::string line;
 if(!std::getline(in,line))return false;
 if(!line.empty()&&line.back()=='\r')line.pop_back();
 if(line!="BISCOT1_TRACKER_PATTERN_V1" && line!="BISCOT1_NATIVE_PATTERN_V1")return false;
 TrackerPattern candidate(false);
 std::map<int,double> speeds;std::map<int,int> transposes;
 bool hasSlot=false,hasLength=false,hasRepeat=false,hasMidi=false,hasRelease=false;
 int proposedSlot=-1,step=-1,tab=-1,rows=0;
 while(std::getline(in,line)){
  if(!line.empty()&&line.back()=='\r')line.pop_back();
  if(line.empty())continue;
  const std::string_view v(line);
  if(v.starts_with("pattern=")){
   if(hasSlot||!detail::field(v,"pattern=",proposedSlot)||proposedSlot<0||proposedSlot>=1000)return false;
   hasSlot=true;
  }else if(v.starts_with("name_hex=")){
   if(!detail::decodeHex(v.substr(9),candidate.name))return false;
  }else if(v.starts_with("length=")){
   if(hasLength||!detail::field(v,"length=",candidate.length)||candidate.length<1||candidate.length>1024)return false;
   hasLength=true;
  }else if(v.starts_with("midi_note=")){
   if(hasMidi||!detail::field(v,"midi_note=",candidate.midiNote)||candidate.midiNote< -1||candidate.midiNote>127)return false;
   hasMidi=true;
  }else if(v.starts_with("release_arp=")){
   int flag=0;if(hasRelease||!detail::field(v,"release_arp=",flag)||(flag!=0&&flag!=1))return false;
   candidate.releaseArp=flag!=0;hasRelease=true;
  }else if(v.starts_with("repeat=")){
   if(hasRepeat||!detail::field(v,"repeat=",candidate.repeat)||candidate.repeat<1||candidate.repeat>1000)return false;
   hasRepeat=true;
  }else if(v.starts_with("speed\t")){
   int index=0;std::string_view value;
   double speed=0;
   if(!detail::pairFields(v,"speed\t",index,value)||!detail::real(value,speed)||
      index<1||index>1000||speed<0.25||speed>16||!speeds.emplace(index,speed).second)return false;
  }else if(v.starts_with("transpose\t")){
   int index=0,transpose=0;std::string_view value;
   if(!detail::pairFields(v,"transpose\t",index,value)||!detail::integer(value,transpose)||
      index<1||index>1000||transpose< -96||transpose>96||!transposes.emplace(index,transpose).second)return false;
  }else if(v.starts_with("step\t")){
   int target=0;std::string_view value;
   if(!detail::pairFields(v,"step\t",target,value)||!detail::integer(value,tab)||
      target<0||target>=1024||tab<1||tab>3)return false;
   step=target;--tab;if(++rows>3072)return false;
  }else if(v.starts_with("cell\t")){
   int id=0;std::string_view value;double number=0;
   if(step<0||tab<0||!detail::pairFields(v,"cell\t",id,value)||
      !detail::real(value,number)||id<0||id>10000)return false;
   auto& item=candidate.steps[step];
   int count=0;for(const auto& cells:item.tabs)count+=(int)cells.size();
   if(count>=128)return false;
   item.tabs[(size_t)tab].push_back({id,number});
  }else return false;
 }
 if(!hasSlot||!hasLength||!hasRepeat||!hasMidi||!hasRelease)return false;
 candidate.repeatSpeeds.assign((size_t)candidate.repeat,1.);
 candidate.repeatTransposes.assign((size_t)candidate.repeat,0);
 for(const auto& [index,value]:speeds){if(index>candidate.repeat)return false;candidate.repeatSpeeds[(size_t)(index-1)]=value;}
 for(const auto& [index,value]:transposes){if(index>candidate.repeat)return false;candidate.repeatTransposes[(size_t)(index-1)]=value;}
 slot=proposedSlot;pattern=std::move(candidate);return true;
}
} // namespace biscot
