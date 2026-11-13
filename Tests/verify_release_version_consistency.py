"""Release-number consistency: fail early if any next-version test was missed."""
from pathlib import Path
from release_version_guard import ROOT, release_version, release_test, release_artifact
import re

cmake=(ROOT/'CMakeLists.txt').read_text(encoding='utf-8')
editor=(ROOT/'Source/PluginEditor.cpp').read_text(encoding='utf-8')
workflow=(ROOT/'.github/workflows/build-windows.yml').read_text(encoding='utf-8')
assert f"project(Biscot1 VERSION {release_version} LANGUAGES C CXX)" in cmake
assert f"Biscot1 Synth - VST3 {release_test}" in editor
assert f'{release_artifact}.zip' in workflow
assert f'name: {release_artifact}' in workflow
for script in (ROOT/'Tests').glob('*.py'):
    if script.name in ('release_version_guard.py','verify_release_version_consistency.py'):
        continue
    source=script.read_text(encoding='utf-8')
    # A check must never pin TEST or the CMake version to an older build.
    assert not re.search(r"(?:assert .*(?:['\"]TEST0\.2\.\d+['\"]|['\"]VERSION 0\.2\.\d+['\"]))",source),script.name
    assert not re.search(r"['\"]Biscot1-TEST0\.2\.\d+-Windows-x64",source),script.name
print('OK: CMake, accessible editor, CI artifact and version-independent regression tests are consistent:',release_test)
