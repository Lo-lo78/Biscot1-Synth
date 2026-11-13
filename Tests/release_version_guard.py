"""Canonical TEST and artifact labels derived from the version in CMakeLists.txt.

Imported by release regression checks; avoids stale hardcoded version assertions.
"""
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
CMAKE = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
_match = re.search(r"^project\(Biscot1 VERSION (\d+\.\d+\.\d+) LANGUAGES C CXX\)", CMAKE, re.M)
if not _match:
    raise AssertionError("Missing or malformed Biscot1 CMake project version")
release_version = _match.group(1)
release_test = "TEST" + release_version
release_cmake = "VERSION " + release_version
release_artifact = "Biscot1-" + release_test + "-Windows-x64"
