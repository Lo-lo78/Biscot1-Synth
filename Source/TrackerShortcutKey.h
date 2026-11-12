// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
// Small JUCE-independent helper for Windows Control-key translations.
// JUCE can report Ctrl+C as 'C', 'c', or ASCII ETX (3). In the last case
// comparing getTextCharacter() with 'c' fails and clipboard commands die.
namespace biscot {
constexpr bool trackerShortcutLetterMatches(int keyCode,int textCharacter,char lower,bool controlDown) noexcept {
 if(lower<'a'||lower>'z')return false;
 const int upper=lower-'a'+'A';
 return keyCode==lower||keyCode==upper||textCharacter==lower||textCharacter==upper||
   (controlDown&&(keyCode==lower-'a'+1 || textCharacter==lower-'a'+1));
}
}
