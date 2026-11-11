"""Verify the Windows CI generator/toolchain contract and JUCE header strategy."""
from pathlib import Path
root = Path(__file__).resolve().parents[1]
w = (root / '.github/workflows/build-windows.yml').read_text(encoding='utf-8')
assert 'runs-on: windows-2022' in w, 'CI must use VS2022 runner'
assert 'Visual Studio 17 2022' in w, 'CMake generator must match installed VS'
assert not any(x.split('#')[0].strip() == 'runs-on: windows-latest' for x in w.splitlines())
for name in ['PluginProcessor.h', 'PluginEditor.h']:
    assert '#include <JuceHeader.h>' not in (root / 'Source' / name).read_text(encoding='utf-8'), name
assert (root / 'Reference' / 'Biscot1Synth_v2.38.jsfx').is_file()
print('OK: Windows toolchain and JUCE includes consistent (static test)')
