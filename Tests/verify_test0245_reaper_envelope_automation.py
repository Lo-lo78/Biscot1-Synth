from pathlib import Path
root = Path(__file__).resolve().parents[1]
editor = (root / "Source/PluginEditor.cpp").read_text(encoding="utf-8")
header = (root / "Source/PluginEditor.h").read_text(encoding="utf-8")
processor = (root / "Source/PluginProcessor.cpp").read_text(encoding="utf-8")
proc_header = (root / "Source/PluginProcessor.h").read_text(encoding="utf-8")
for needle in ["getVST3ClientExtensions()", "GetFXEnvelope", '"ACTIVE"',
               '"ARM"', '"VISIBLE"', '"GetTouchedOrFocusedFX"',
               '"TrackFX_GetParamName"', "getParameters().indexOf(param)"]:
    assert needle in processor, needle
for needle in ["getReaperEnvelopeActive", "toggleReaperEnvelopeActive"]:
    assert needle in proc_header, needle
for needle in ["selectedAutomationParameter", "syncAutomationIndicator(true)",
               "handleAutomationShortcut", "toggleHostAutomationEnvelope", 
               'observedAutomationState==1?', "updateAutomationAccessibilityNames",
               "addKeyListener(this)", "removeKeyListener(this)",
               "mods.isCtrlDown() && mods.isShiftDown() && !mods.isAltDown()",
               "return handleAutomationShortcut(key,*source)",
               "pollAutomationIndicator()", "param->beginChangeGesture()"]:
    assert needle in editor, needle
assert "!mods.isCommandDown()" not in editor[editor.index("bool BiscotEditor::handleAutomationShortcut"):editor.index("bool BiscotEditor::keyPressed(const juce::KeyPress& key,juce::Component*")]
assert "private juce::KeyListener" in header
assert 'if(!chord)return false;' in editor
assert 'UiaRaiseNotificationEvent' in editor
print("Biscot1 TEST0.2.45 REAPER automation and NVDA shortcut checks: OK")
