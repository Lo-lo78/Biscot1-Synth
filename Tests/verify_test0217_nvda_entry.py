"""Static regression of LB-303's accessible root/focus pattern."""
from pathlib import Path
root=Path(__file__).resolve().parents[1]
source=(root/'Source/PluginEditor.cpp').read_text()
header=(root/'Source/PluginEditor.h').read_text()
workflow=(root/'.github/workflows/build-windows.yml').read_text()
assert 'std::unique_ptr<juce::AccessibilityHandler> createAccessibilityHandler() override;' in header
assert 'std::unique_ptr<juce::AccessibilityHandler> BiscotEditor::createAccessibilityHandler()' in source
assert 'return createIgnoredAccessibilityHandler(*this);' in source
assert 'setFocusContainerType(juce::Component::FocusContainerType::keyboardFocusContainer);' in source
assert 'setWantsKeyboardFocus(true);' in source
assert 'void BiscotEditor::focusGained(FocusChangeType){initialFocus();}' in source
assert 'if(focused!=nullptr && focused!=safe.getComponent())return;' in source
assert 'safe->grid.grabKeyboardFocus();' in source
assert 'if(safe->grid.hasKeyboardFocus(false))handler->grabFocus();' in source
assert 'juce::AccessibilityHandler::clearCurrentlyFocusedHandler();' in source
assert 'announce(grid,labelFor(selected));' in source
assert 'createIgnoredAccessibilityHandler(*this)' not in (root/'Source/PresetBrowser.cpp').read_text()
assert 'verify_test0217_nvda_entry.py' in workflow
assert 'TEST0.2.21.1' in source and 'TEST0.2.21.1' in workflow
print('OK: ignored root like LB-303, native Grid/Value/Edit focus preserved')
