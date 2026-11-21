#!/usr/bin/env python3
"""Historic public 0.99.6 docs; no longer asserts the active plugin version."""
from release_version_guard import ROOT
src=lambda p:(ROOT/p).read_text(encoding='utf-8')
notes=src('RELEASE_NOTES_0.99.6.md')
en=src('Docs/BISCOT1_0.99.6_GUIDE_EN.md')
it=src('Docs/BISCOT1_0.99.6_GUIDA_IT.md')
body=src('Docs/GITHUB_RELEASE_0.99.6_BODY_EN.md')
readme=src('README.md')
assert '16 November 2026' in notes
for doc in (en,it):
    for required in ('0.99.6','16 ', 'Ctrl+Shift+A','Op1','Op2','Op3','Mod Env','Filter Hold','249', '255','0.99.6-Windows-x64'):
        assert required in doc,required
assert 'Biscot1-0.99.6-Windows-x64.vst3.zip' in body
assert 'RELEASE_NOTES_0.99.6.md' in readme
assert src('Source/ParameterCatalog.h').count('"Filter Hold", 0.0f, 5.0f, 0.001f, 0.0f')==1
print('OK: archived 0.99.6 release notes, guides and Hold parameter are retained')
