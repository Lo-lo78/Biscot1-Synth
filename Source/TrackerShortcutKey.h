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

// F1's 1/2/3 shortcuts are digits, NOT letters.  Calling
// trackerShortcutLetterMatches() for '1' would always fail because that helper
// intentionally rejects non-letters (a regression introduced in TEST0.2.18.4).
// The keys are accepted via the JUCE virtual keyCode OR text character, so
// keyboard layouts that return a zero text character still work.
constexpr int trackerShortcutSectionIndex(int keyCode,int textCharacter) noexcept {
 for(int digit='1';digit<='3';++digit)
  if(keyCode==digit || textCharacter==digit) return digit-'1';
 return -1;
}
