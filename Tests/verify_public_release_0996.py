#!/usr/bin/env python3
"""Public release 0.99.6: About, metadata, guides and archive naming."""
from pathlib import Path
from release_version_guard import ROOT, release_version, release_artifact
src=lambda p:(ROOT/p).read_text(encoding='utf-8')
editor=src('Source/PluginEditor.cpp')
cmake=src('CMakeLists.txt')
workflow=src('.github/workflows/build-windows.yml')
readme=src('README.md')
notes=src('RELEASE_NOTES_0.99.6.md')
en=src('Docs/BISCOT1_0.99.6_GUIDE_EN.md')
it=src('Docs/BISCOT1_0.99.6_GUIDA_IT.md')
body=src('Docs/GITHUB_RELEASE_0.99.6_BODY_EN.md')
assert release_version=='0.99.6',release_version
assert 'project(Biscot1 VERSION 0.99.6 LANGUAGES C CXX)' in cmake
assert 'constexpr auto biscotVersion="0.99.6";' in editor
assert 'constexpr auto biscotReleaseDate="16 November 2026";' in editor
assert 'Biscot1 Synth - VST3 0.99.6' in editor
assert 'New in 0.99.6: AHDSR Hold for all seven envelopes' in editor
assert 'Ctrl+Shift+A' in editor and ', Auto' in editor
assert workflow.count('Biscot1-0.99.6-Windows-x64')==3
assert 'python Tests/verify_public_release_0996.py' in workflow
assert 'python Tests/verify_test0246_ahdsr_hold.py' in workflow
assert 'python Tests/verify_test0245_reaper_envelope_automation.py' in workflow
assert 'RELEASE_NOTES_0.99.6.md' in readme and '16 November 2026' in notes
for doc in (en,it):
    for required in ('0.99.6','16 ', 'Ctrl+Shift+A','Op1','Op2','Op3','Mod Env','Filter Hold','249', '255','0.99.6-Windows-x64'):
        assert required in doc,required
assert 'Biscot1-0.99.6-Windows-x64.vst3.zip' in body
assert '0.99.6' in body
assert src('Source/ParameterCatalog.h').count('"Filter Hold", 0.0f, 5.0f, 0.001f, 0.0f')==1
print('OK: 0.99.6 metadata, About, English/Italian guides, 7 Hold controls, automation, artifact')
