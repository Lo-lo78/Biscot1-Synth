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
assert 'VERSION '+__import__('release_version_guard').release_version in cmake
assert 'Biscot1-'+__import__('release_version_guard').release_version+'-Windows-x64.zip' in workflow
print('OK: public About, Alt+A routing and 0.99.1 version')

# JUCE TextEditor does not support setKeyHandler: use a typed subclass.
assert 'setKeyHandler' not in editor
assert 'struct AboutReadOnlyEditor final : juce::TextEditor' in header
assert 'owner.handleAboutKey(key)' in header
assert 'juce::TextEditor::keyPressed(key)' in header

# LJuno-116 parity: arrows, word movement, selection, Home/End, page, copy and select all.
for phrase in ('navigateAboutText(key)', 'moveCaretLeft(byWordOrDocument,selecting)',
               'moveCaretRight(byWordOrDocument,selecting)', 'moveCaretUp(selecting)',
               'moveCaretDown(selecting)', 'moveCaretToStartOfLine(selecting)',
               'moveCaretToEndOfLine(selecting)', 'moveCaretToTop(selecting)',
               'moveCaretToEnd(selecting)', 'pageUp(selecting)', 'pageDown(selecting)',
               'aboutInfo.selectAll()', 'aboutInfo.copyToClipboard()',
               'AccessibilityHandler::clearCurrentlyFocusedHandler()'):
    assert phrase in editor,phrase
assert 'navigateAboutText(const juce::KeyPress&)' in header
assert 'setKeyHandler' not in editor
print('OK: About arrow navigation and native text selection match LJuno-116')
