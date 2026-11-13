// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include "TrackerSongBank.h"
#include "TrackerSongParameters.h"
#include "TrackerPatternText.h"
#include "TrackerLibraryFileNames.h"
#include <juce_core/juce_core.h>
#include <set>
#include <sstream>
#include <iomanip>
#include <map>

namespace biscot {
namespace songpkg {
inline std::set<int> references(const TrackerSong& s){
 std::set<int> ids;
 for(const auto& lane:s.lanes)for(const auto& [row,step]:lane)
  if(step.contains(1) && step.values[1]>=0 && step.values[1]<1000)
   ids.insert((int)step.values[1]);
 return ids;
}
inline juce::String safeName(juce::String name){
 const auto clean=trackerLibraryFileStem(name);
 return clean.isEmpty()?juce::String("Song"):clean;
}
inline bool atomicWrite(const juce::File& target,const juce::String& text){
 juce::TemporaryFile tmp(target);
 return tmp.getFile().replaceWithText(text) && tmp.overwriteTargetFileWithTemporary();
}
inline std::string encode(const TrackerSong& s,int original,const std::map<int,juce::String>& paths){
 std::ostringstream o;o.imbue(std::locale::classic());o<<std::setprecision(17);
 o<<"BISCOT1_SONG_PACKAGE_V1\n"<<original<<'\n'<<detail::hexBytes(s.name)<<'\n';
 o<<s.midiNote<<' '<<s.songEnd<<' '<<s.loopTarget<<' '<<(s.chainLoop?1:0)<<' '<<s.filterOwner<<' '<<s.filterMask<<' ';
 for(int n:s.lfoOwners)o<<n<<' ';for(int n:s.mixerVolumes)o<<n<<' ';
 o<<s.mixerMuteMask<<' '<<s.mixerSoloLane<<'\n';
 o<<paths.size()<<'\n';
 for(auto& [id,path]:paths)o<<id<<' '<<detail::hexBytes(path.toStdString())<<'\n';
 for(int l=0;l<3;++l){
  o<<s.laneLengths[l]<<' '<<s.laneRepeats[l]<<'\n';
  for(int r=0;r<s.laneRepeats[l];++r)o<<(r<(int)s.laneSpeeds[l].size()?s.laneSpeeds[l][r]:1.)<<' ';
  o<<'\n';
  int count=0;for(auto& [row,step]:s.lanes[l])if(!step.empty())++count;
  o<<count<<'\n';
  for(auto& [row,step]:s.lanes[l]){
   if(step.empty())continue;
   o<<row<<' '<<step.order.size();
   for(int id:step.order)o<<' '<<id<<' '<<step.values[id];
   if(step.contains(1))o<<' '<<step.values[5]<<' '<<step.values[6];
   o<<'\n';
  }
 }
 return o.str();
}
inline bool decode(const std::string& text,TrackerSong& song,int& original,std::map<int,juce::String>& paths){
 if(text.size()>16*1024*1024)return false;
 std::istringstream in(text);in.imbue(std::locale::classic());
 std::string magic,hex,numberLine;
 if(!std::getline(in,magic)||magic!="BISCOT1_SONG_PACKAGE_V1"||!std::getline(in,numberLine)||!detail::integer(numberLine,original)||!std::getline(in,hex))return false;
 if(original<0||original>=100||!detail::decodeHex(hex,song.name))return false;
 int loop=0;
 if(!(in>>song.midiNote>>song.songEnd>>song.loopTarget>>loop>>song.filterOwner>>song.filterMask))return false;
 song.chainLoop=loop!=0;
 for(int& n:song.lfoOwners)if(!(in>>n))return false;
 for(int& n:song.mixerVolumes)if(!(in>>n))return false;
 if(!(in>>song.mixerMuteMask>>song.mixerSoloLane))return false;
 if(song.midiNote< -1||song.midiNote>127||song.songEnd< -1||song.songEnd>99||song.loopTarget< -1||song.loopTarget>99||
    loop<0||loop>1||song.filterOwner<1||song.filterOwner>3||song.filterMask<0||song.filterMask>7||
    song.mixerMuteMask<0||song.mixerMuteMask>7||song.mixerSoloLane<0||song.mixerSoloLane>3)return false;
 for(int n:song.lfoOwners)if(n<1||n>3)return false;
 for(int n:song.mixerVolumes)if(n<0||n>100)return false;
 int npaths=0;if(!(in>>npaths)||npaths<0||npaths>1000)return false;
 for(int i=0;i<npaths;++i){
  int id=-1;std::string encoded,decoded;
  if(!(in>>id>>encoded)||id<0||id>=1000||!detail::decodeHex(encoded,decoded)||!paths.emplace(id,juce::String::fromUTF8(decoded.c_str())).second)return false;
 }
 for(int l=0;l<3;++l){
  int len=0,reps=0;if(!(in>>len>>reps)||len<1||len>4096||reps<1||reps>1000)return false;
  song.laneLengths[l]=len;song.laneRepeats[l]=reps;song.laneSpeeds[l].clear();
  for(int r=0;r<reps;++r){double speed=0;if(!(in>>speed)||!std::isfinite(speed)||speed<0.25||speed>16)return false;song.laneSpeeds[l].push_back(speed);}
  int rows=0;if(!(in>>rows)||rows<0||rows>4096)return false;
  for(int r=0;r<rows;++r){
   int row=-1,n=0;if(!(in>>row>>n)||row<0||row>=4096||n<1||n>18||song.lanes[l].count(row))return false;
   TrackerSongStep step;
   for(int k=0;k<n;++k){
    int id=0;double value=0;
    if(!(in>>id>>value)||id<1||id>23||!std::isfinite(value)||!step.insert(id))return false;
    if(const auto* param=songParam(id))if(value<param->min-1.e-7||value>param->max+1.e-7)return false;
    step.values[id]=value;
   }
   if(step.contains(1)){
    if(!(in>>step.values[5]>>step.values[6])||!std::isfinite(step.values[5])||!std::isfinite(step.values[6])||
       step.values[5]<0||step.values[5]>1023||step.values[6]<1||step.values[6]>1024)return false;
    const int id=(int)step.values[1];if(step.values[1]!=id || id>=1000 || id< -1 || (id>=0&&!paths.count(id)))return false;
   }
   song.lanes[l].emplace(row,std::move(step));
  }
 }
 in>>std::ws;return in.eof();
}
// V2 is a single, self-contained text container. Embedded payloads are
// hexadecimal to preserve arbitrary UTF-8 names and the complete V1 codecs.
// No dependency files are written or read when loading a Song.
inline bool decodePayload(const std::string& hex,std::string& out,size_t maxBytes){
 if(hex.size()%2 || hex.size()/2>maxBytes)return false;
 auto digit=[](char c)->int {if(c>='0'&&c<='9')return c-'0';if(c>='a'&&c<='f')return c-'a'+10;if(c>='A'&&c<='F')return c-'A'+10;return -1;};
 out.clear();out.reserve(hex.size()/2);
 for(size_t i=0;i<hex.size();i+=2){int a=digit(hex[i]),b=digit(hex[i+1]);if(a<0||b<0)return false;out.push_back((char)((a<<4)|b));}
 return true;
}
// Two intentionally different library formats:
// .biscot1-song: the selected Song, insertable at any chosen slot.
// .biscot1-all-song: a complete 100-slot Song bank, restored by number.
// Both embed their referenced Patterns. Song End / Loop Target in a single
// Song remain numeric: a missing destination is never silently bundled.
inline bool exportPackage(const TrackerSongBank& songs,const TrackerPatternBank& patterns,
                          const juce::File& root,bool all,juce::String& message,
                          const juce::String& requestedFileName={}){
 const int active=songs.activeSong;
 if(active<0||active>=100||!songs.find(active)){message="Active Song is missing";return false;}
 std::set<int> songIds;
 if(all){for(int i=0;i<100;++i)songIds.insert(i);}
 else songIds.insert(active);
 std::set<int> patternIds;
 for(int id:songIds){
  if(const auto* song=songs.find(id)){
   const auto refs=references(*song);patternIds.insert(refs.begin(),refs.end());
  }
 }
 for(int id:patternIds)if(!patterns.find(id)){message="Missing referenced Pattern "+juce::String(id);return false;}
 const auto library=root.getChildFile("Biscot1-Synth");
 const auto directory=library.getChildFile("songs");
 if(directory.createDirectory().failed()||library.getChildFile("patterns").createDirectory().failed()){
  message="Cannot create Biscot1 library folders";return false;
 }
 const auto* first=songs.find(active);
 const juce::String ext=all?".biscot1-all-song":".biscot1-song";
 auto chosenName=requestedFileName.trim();
 if(chosenName.endsWithIgnoreCase(ext))chosenName=chosenName.dropLastCharacters(ext.length());
 const juce::String filename=chosenName.isNotEmpty()?safeName(chosenName):
  all?juce::String("All_Songs"):safeName(juce::String::fromUTF8(first->name.c_str()).isEmpty()?"Song_"+juce::String(active):juce::String::fromUTF8(first->name.c_str()));
 juce::File target;
 for(int n=1;n<=9999;++n){
  const auto candidate=directory.getChildFile(filename+(n==1?juce::String{}:"_"+juce::String(n))+ext);
  if(!candidate.exists()){target=candidate;break;}
 }
 if(target==juce::File{}){message="Too many Song filenames";return false;}
 std::ostringstream out;out.imbue(std::locale::classic());
 out<<(all?"BISCOT1_ALL_SONG_BUNDLE_V3":"BISCOT1_SINGLE_SONG_BUNDLE_V3")<<'\n';
 out<<active<<' '<<songIds.size()<<' '<<patternIds.size()<<'\n';
 for(int id:songIds){
  const TrackerSong empty;
  const auto* s=songs.find(id);if(!s)s=&empty;
  std::map<int,juce::String> embedded;
  for(int pat:references(*s))embedded.emplace(pat,"embedded");
  const auto payload=encode(*s,id,embedded);
  out<<id<<' '<<detail::hexBytes(payload)<<'\n';
 }
 for(int id:patternIds){
  const auto payload=serializeTrackerPattern(*patterns.find(id),id);
  out<<id<<' '<<detail::hexBytes(payload)<<'\n';
 }
 const auto bytes=out.str();
 if(bytes.size()>256u*1024u*1024u){message="Song package exceeds 256 MB";return false;}
 if(!atomicWrite(target,juce::String::fromUTF8(bytes.data(),(int)bytes.size()))){message="Cannot save Song file";return false;}
 message=(all?"All 100 Song slots saved":"Song "+juce::String(active)+" saved")+
  ". "+juce::String((int)patternIds.size())+" embedded Patterns. "+target.getFileName();
 return true;
}
inline bool exportSong(const TrackerSongBank& songs,const TrackerPatternBank& patterns,const juce::File& root,juce::String& message,const juce::String& name={}){
 return exportPackage(songs,patterns,root,false,message,name);
}
inline bool exportAllSongs(const TrackerSongBank& songs,const TrackerPatternBank& patterns,const juce::File& root,juce::String& message,const juce::String& name={}){
 return exportPackage(songs,patterns,root,true,message,name);
}
inline bool importSong(const juce::File& file,TrackerSongBank& songs,TrackerPatternBank& patterns,juce::String& message){
 if(!file.existsAsFile()||file.getSize()>256ll*1024ll*1024ll){message="Invalid Song file";return false;}
 const auto raw=file.loadFileAsString().toStdString();
 std::istringstream in(raw);in.imbue(std::locale::classic());std::string magic;
 if(!std::getline(in,magic)){message="Invalid Song format";return false;}
 if(magic.size()>=3 && (unsigned char)magic[0]==0xef && (unsigned char)magic[1]==0xbb && (unsigned char)magic[2]==0xbf)magic.erase(0,3);
 if(!magic.empty()&&magic.back()=='\r')magic.pop_back();
 const bool single=magic=="BISCOT1_SINGLE_SONG_BUNDLE_V3";
 const bool all=magic=="BISCOT1_ALL_SONG_BUNDLE_V3";
 const bool legacy=magic=="BISCOT1_SONG_BUNDLE_V2";
 if(!single&&!all&&!legacy){
  message="Unsupported Song library format. Header: "+juce::String::fromUTF8(magic.substr(0,100).c_str());return false;
 }
 if((single&&!file.hasFileExtension(".biscot1-song")) || (all&&!file.hasFileExtension(".biscot1-all-song"))){
  message="File extension does not match Song library format";return false;
 }
 int active=-1,ns=0,np=0;
 if(!(in>>active>>ns>>np)||active<0||active>=100||ns<1||ns>100||np<0||np>1000){message="Invalid Song package header";return false;}
 if((single&&ns!=1)||(all&&ns!=100)){message="Wrong Song count for file format";return false;}
 std::map<int,TrackerSong> loadedSongs;std::map<int,TrackerPattern> loadedPatterns;
 for(int i=0;i<ns;++i){
  int id=-1;std::string hex,payload;TrackerSong song;int decodedId=-1;std::map<int,juce::String> embedded;
  if(!(in>>id>>hex)||id<0||id>=100||!decodePayload(hex,payload,16u*1024u*1024u)||!decode(payload,song,decodedId,embedded)||decodedId!=id||!loadedSongs.emplace(id,std::move(song)).second){message="Invalid embedded Song";return false;}
 }
 for(int i=0;i<np;++i){
  int id=-1,decodedId=-1;std::string hex,payload;TrackerPattern pat(false);
  if(!(in>>id>>hex)||id<0||id>=1000||!decodePayload(hex,payload,16u*1024u*1024u)||!deserializeTrackerPattern(payload,decodedId,pat)||decodedId!=id||!loadedPatterns.emplace(id,std::move(pat)).second){message="Invalid embedded Pattern";return false;}
 }
 in>>std::ws;
 if(!in.eof()||!loadedSongs.count(active)){message="Incomplete Song package";return false;}
 if(single && loadedSongs.size()!=1){message="Single Song file has multiple entries";return false;}
 if(all){for(int i=0;i<100;++i)if(!loadedSongs.count(i)){message="All-Songs file is missing Song slot "+juce::String(i);return false;}}
 for(const auto& [id,s]:loadedSongs){
  // A single Song is intentionally allowed to reference Song End destinations
  // that are not included in its own portable file.
  if(!single)for(int ref:{s.songEnd,s.loopTarget})if(ref>=0&&!loadedSongs.count(ref)){
   message="Missing chained Song in package";return false;
  }
  for(int ref:references(s))if(!loadedPatterns.count(ref)){message="Missing embedded Pattern";return false;}
 }
 // Transactional import: commit neither bank until every slot is available.
 auto newPatterns=patterns;auto newSongs=songs;
 std::map<int,int> patternMap,songMap;
 for(const auto& [id,pat]:loadedPatterns){
  int dest=id;if(newPatterns.patterns.count(dest)){dest=0;while(dest<1000&&newPatterns.patterns.count(dest))++dest;}
  if(dest>=1000){message="Pattern bank is full";return false;}
  newPatterns.patterns.emplace(dest,pat);patternMap[id]=dest;
 }
 if(all){
  newSongs.songs=loadedSongs; // exact 0..99: replace static bank, including empty slots
  for(int i=0;i<100;++i)songMap[i]=i;
  newSongs.activeSong=active;
 }else if(single){
  // The selected destination is explicit, not the original file's Song number.
  const int dest=juce::jlimit(0,99,newSongs.activeSong);
  newSongs.songs[dest]=loadedSongs.at(active);
  songMap[active]=dest;
  // Song End and Loop Target deliberately retain their numeric addresses.
 }else{
  // Compatibility with the older V2 composition-bundle importer.
  for(const auto& [id,s]:loadedSongs){
   int dest=id;if(newSongs.songs.count(dest)){dest=0;while(dest<100&&newSongs.songs.count(dest))++dest;}
   if(dest>=100){message="Song bank is full";return false;}
   newSongs.songs.emplace(dest,s);songMap[id]=dest;
  }
  newSongs.activeSong=songMap.at(active);
 }
 for(const auto& [id,_]:loadedSongs){
  const int dest=single?newSongs.activeSong:songMap.at(id);
  auto& s=newSongs.songs.at(dest);
  if(legacy){
   if(s.songEnd>=0)s.songEnd=songMap.at(s.songEnd);
   if(s.loopTarget>=0)s.loopTarget=songMap.at(s.loopTarget);
  }
  for(auto& lane:s.lanes)for(auto& [row,step]:lane)
   if(step.contains(1)&&step.values[1]>=0)step.values[1]=patternMap.at((int)step.values[1]);
 }
 patterns=std::move(newPatterns);songs=std::move(newSongs);
 message=(single?"Single Song loaded into slot "+juce::String(songs.activeSong):
          all?"All 100 Song slots restored":"Legacy Song package loaded")+
         ". "+juce::String(np)+" embedded Patterns";
 return true;
}
} // namespace songpkg
} // namespace biscot
