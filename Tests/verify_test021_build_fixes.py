"""Regression guard for the three errors found in GitHub Actions TEST0.2.

This is a source-level test and does not replace an MSVC/JUCE build.
"""
from pathlib import Path
root=Path(__file__).resolve().parents[1]
processor=(root/'Source/PluginProcessor.cpp').read_text(encoding='utf-8')
editor=(root/'Source/PluginEditor.cpp').read_text(encoding='utf-8')
assert 'const bool running=position->getIsPlaying();' in processor
assert 'if(running && !lastPlaying)' in processor
assert 'lastPlaying=running;' in processor
assert 'if(*running' not in processor and 'lastPlaying=*running' not in processor
assert 'labels[i].equalsIgnoreCase(entered)' in editor
assert 'int n=-1;' in editor
assert 'if(n<0)n=entered.getIntValue();' in editor
assert 'labels.indexOfIgnoreCase(' not in editor
assert 'TEST0.2.24' in editor
print('OK: JUCE 8 build fixes for transport bool and choice-label lookup (static check)')
