#!/usr/bin/env python3
"""Public 0.99.7: release metadata, NVDA About, docs and append-only controls."""
from release_version_guard import ROOT,release_version,release_artifact
src=lambda path:(ROOT/path).read_text(encoding='utf-8')
cmake=src('CMakeLists.txt')
editor=src('Source/PluginEditor.cpp')
workflow=src('.github/workflows/build-windows.yml')
readme=src('README.md')
notes=src('RELEASE_NOTES_0.99.7.md')
en=src('Docs/BISCOT1_0.99.7_GUIDE_EN.md')
it=src('Docs/BISCOT1_0.99.7_GUIDA_IT.md')
body=src('Docs/GITHUB_RELEASE_0.99.7_BODY_EN.md')
params=src('Source/ParameterCatalog.h')
assert release_version=='0.99.7',release_version
assert release_artifact=='Biscot1-0.99.7-Windows-x64'
assert 'project(Biscot1 VERSION 0.99.7 LANGUAGES C CXX)' in cmake
for required in ('constexpr auto biscotVersion="0.99.7";','constexpr auto biscotReleaseDate="21 November 2026";',
                 'Biscot1 Synth - VST3 0.99.7','New in 0.99.7:','AHDSR Trigger','Ping Pong Pan',
                 'Portamento and Arp Glide','Ctrl+Shift+A',', Auto'):
    assert required in editor,required
assert workflow.count('Biscot1-0.99.7-Windows-x64')==3
assert 'python Tests/verify_public_release_0997.py' in workflow
assert 'python Tests/verify_test0252_glide_pan_sync.py' in workflow
for required in ('0.99.7', '21 November 2026', 'TEST0.2.52','RELEASE_NOTES_0.99.6.md'):
    assert required in notes,required
for doc in (en,it,body):
    for required in ('0.99.7','21 ', 'AHDSR Trigger','Ping Pong Pan','Portamento','Glide','0.99.7-Windows-x64'):
        assert required in doc,required
for required in ('RELEASE_NOTES_0.99.7.md', 'Docs/BISCOT1_0.99.7_GUIDE_EN.md',
                 'Docs/BISCOT1_0.99.7_GUIDA_IT.md','Docs/GITHUB_RELEASE_0.99.7_BODY_EN.md'):
    assert required in readme,required
for i in range(3):
    assert f'{{{256+i}, "Op{i+1} AHDSR Trigger"' in params
    assert f'{{{259+i}, "Op{i+1} Ping Pong Pan"' in params
assert src('Source/SidPingPongPan.h') and src('Source/SidPitchGlideMotion.h')
print('OK: 0.99.7 release metadata, About, bilingual guides, CI archive and append-only parameters')
