// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include <juce_core/juce_core.h>

namespace biscot {
inline juce::String trackerLibraryFileStem(juce::String name){
 name=name.trim();
 for(auto c:juce::String("\\/:*?\"<>|\r\n\t")) name=name.replaceCharacter(c,'_');
 while(name.contains(".."))name=name.replace("..","_");
 name=name.trimCharactersAtEnd(" .").trim();
 name=name.substring(0,90).trimCharactersAtEnd(" .");
 if(name.isEmpty())return {};
 const auto upper=name.upToFirstOccurrenceOf(".",false,false).toUpperCase();
 if(upper=="CON"||upper=="PRN"||upper=="AUX"||upper=="NUL" ||
    (upper.length()==4&&(upper.startsWith("COM")||upper.startsWith("LPT"))&&
     upper[3]>='1'&&upper[3]<='9'))name="_"+name;
 return name;
}
}
