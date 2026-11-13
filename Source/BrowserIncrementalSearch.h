// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include <juce_core/juce_core.h>

namespace biscot {
// Shared prefix search for the Preset, Pattern and Song file browsers.
// Typing "bass" quickly finds a filename starting with "bass"; repeating a
// single letter cycles through matching files, as with the old initial search.
// The editor's own TextEditor handles filenames when the focus is in an edit.
class BrowserIncrementalSearch {
public:
 void reset() noexcept { prefix.clear(); lastKeyTimeMs=-1.; }
 const juce::String& currentPrefix() const noexcept { return prefix; }
 template<typename NameAt>
 int type(juce::juce_wchar key,int current,int count,NameAt nameAt,bool backwards=false){
  if(count<=0 || !juce::CharacterFunctions::isLetterOrDigit(key))return -1;
  const double now=juce::Time::getMillisecondCounterHiRes();
  const bool expired=lastKeyTimeMs<0. || now-lastKeyTimeMs>1200.;
  const juce::String letter=juce::String::charToString(juce::CharacterFunctions::toLowerCase(key));
  const bool cycle=expired || (prefix.length()==1 && prefix.equalsIgnoreCase(letter));
  auto nextPrefix=cycle?letter:prefix+letter;
  lastKeyTimeMs=now;
  const auto locate=[&](const juce::String& wanted,bool includeCurrent)->int{
   const int base=juce::jlimit(0,count-1,current);
   // Prefer an exact filename prefix, then a prefix after a word separator.
   // For example, typing "bass" can find "Electric Bass.biscot".
   for(int phase=0;phase<2;++phase)
    for(int offset=includeCurrent?0:1;offset<count+(includeCurrent?0:1);++offset){
     const int index=(base+(backwards?-offset:offset)+count*(count+1))%count;
     const auto name=nameAt(index);
     if(phase==0){if(name.startsWithIgnoreCase(wanted))return index;}
     else for(int j=1;j<name.length();++j){
      const auto separator=name[j-1];
      if(separator==' ' || separator=='_' || separator=='-' || separator=='.')
       if(name.substring(j).startsWithIgnoreCase(wanted))return index;
     }
    }
   return -1;
  };
  int match=locate(nextPrefix,!cycle);
  if(match<0 && !cycle){
   // A failed prefix is not a dead end: use this last key as a new search.
   nextPrefix=letter;match=locate(letter,false);
  }
  prefix=nextPrefix;
  return match;
 }
private:
 juce::String prefix;
 double lastKeyTimeMs=-1.;
};
}
