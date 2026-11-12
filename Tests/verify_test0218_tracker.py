"""TEST0.2.19: first native F1/F2 tracker; preserve trusted synth/audio code."""
from pathlib import Path
import hashlib,re
root=Path(__file__).resolve().parents[1]
source=(root/'Source/PluginProcessor.cpp').read_text()
editor=(root/'Source/PluginEditor.cpp').read_text()
header=(root/'Source/PluginProcessor.h').read_text()
panel=(root/'Source/TrackerPatternPanel.cpp').read_text()
model=(root/'Source/TrackerPatternBank.h').read_text()
workflow=(root/'.github/workflows/build-windows.yml').read_text()
assert 'TEST0.2.19' in editor and 'TEST0.2.19' in workflow
assert 'if(letter(\'q\')){openTracker();return true;}' in editor
assert 'trackerPanel->open();' in editor
assert 'Alt+Q' in (root/'Source/PluginEditor.cpp').read_text()
assert 'std::map<int,TrackerPattern> patterns;' in model
assert 'maxPatterns=1000,maxSteps=1024,maxCells=128' in model
assert 'return createIgnoredAccessibilityHandler(*this);' in panel
assert 'trackerPatternsV1' in source
assert 'const juce::Identifier trackerStateKey{"trackerPatternsV1"};' in source
assert 'constexpr juce::Identifier' not in source # MSVC C2131: Identifier ctor is not constexpr
assert 'void BiscotProcessor::commitTrackerState()' in source
assert 'state.replaceState(tree)' in source and 'trackerPatterns=std::move(restored)' in source
assert 'onPresetBrowser' in editor
assert 'Tests/TrackerPatternBankTest.cpp' in (root/'CMakeLists.txt').read_text()
assert 'TrackerPatternBankTest.exe' in workflow
assert (root/'Reference/LegacyTracker/Biscot1_Tracker_0.166.jsfx').exists()
assert (root/'Reference/LegacyTracker/Biscot1_TrackerControl_0.166.lua').exists()
assert (root/'Reference/LegacyTracker/Biscot1_Tracker_Project_Startup_Loader_0.166.lua').exists()
assert len(re.findall(r'^ \{\d+,"', (root/'Source/TrackerParameterCatalog.h').read_text(), re.M))>=106
assert 'paramHeight' in panel and 'code==juce::KeyPress::F1Key' in panel
# Keep the exact audio process and original MIDI handling, since this test
# version deliberately does not connect the tracker to the live synth.
for begin,end,expected in [
 ('void BiscotProcessor::renderOne(','void BiscotProcessor::advanceIdleLfos(', 'd6e631be7980f9840d4c44e30b3dd6bd02c632f8cb5ea6df40297c0aa6e27914'),
 ('void BiscotProcessor::onMidi(','float BiscotProcessor::polyBlep(','bf9fef8fccb1ea742bf1b01e719cbb7e27fbe100f2aec2b0e608f9e197ffa8e8'),
]:
 fragment=source[source.index(begin):source.index(end)]
 assert hashlib.sha256(fragment.encode()).hexdigest()==expected
print('OK: native tracker F1/F2, persistent Pattern data, original MIDI and DSP preserved')
