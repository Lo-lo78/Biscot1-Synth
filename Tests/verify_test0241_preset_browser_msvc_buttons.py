"""Prevent MSVC C3535/C2440/C2665 in JUCE preset-browser button registration.

ConfirmationButton is derived from juce::TextButton, but auto* deduced from
an initializer list of addresses of both types is ill-formed with MSVC.
"""
from pathlib import Path

root=Path(__file__).resolve().parents[1]
source=(root/'Source/PresetBrowser.cpp').read_text(encoding='utf-8')
header=(root/'Source/PresetBrowser.h').read_text(encoding='utf-8')
assert 'ConfirmationButton accept' in header
assert 'juce::TextButton back' in header
assert 'for (auto* b : { &accept, &cancel, &back })' not in source
for name in ('accept','cancel','back'):
    assert f'addAndMakeVisible ({name});' in source, name
assert 'back.onClick = [this] { parentFolder(); };' in source
print('OK: mixed-type JUCE buttons are registered separately for MSVC')
