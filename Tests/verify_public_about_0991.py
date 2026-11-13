from pathlib import Path
r=Path(__file__).resolve().parents[1]
editor=(r/'Source/PluginEditor.cpp').read_text()
header=(r/'Source/PluginEditor.h').read_text()
cmake=(r/'CMakeLists.txt').read_text()
workflow=(r/'.github/workflows/build-windows.yml').read_text()
for phrase in ('Biscot1 Synth\\nVersion: ', 'Release date: ', 'License: ', 'Project: ', 'Contact: ', 'setReadOnly(true)', 'aboutInfo.grabKeyboardFocus()', 'aboutReturnFocus', 'handleAboutKey'):
    assert phrase in editor,phrase
for phrase in ('aboutInfo', 'aboutProject', 'aboutContact', 'aboutClose'):
    assert phrase in header,phrase
for fname in ('TrackerPatternPanel','TrackerSongPanel'):
    assert 'onAbout' in (r/f'Source/{fname}.h').read_text()
    assert 'onAbout()' in (r/f'Source/{fname}.cpp').read_text()
assert 'VERSION 0.99.1' in cmake
assert 'Biscot1-0.99.1-Windows-x64.zip' in workflow
print('OK: public About, Alt+A routing and 0.99.1 version')
