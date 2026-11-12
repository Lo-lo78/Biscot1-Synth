"""Prevent use of assert-only side effects in Release-mode tracker tests."""
from pathlib import Path
root = Path(__file__).resolve().parents[1]
for file in ("TrackerPatternBankTest.cpp", "TrackerSearchStepTest.cpp"):
    source = (root / "Tests" / file).read_text()
    assert "#define CHECK(expr)" in source
    assert "if (!(expr))" in source
    assert "std::cerr" in source
    assert "CHECK(" in source
    assert "#include <cassert>" not in source
    assert "assert(" not in source.replace("unlike assert,", "")
assert "TEST0.2.21" in (root / "Source/PluginEditor.cpp").read_text()
assert "VERSION 0.2.21" in (root / "CMakeLists.txt").read_text()
print("OK: tracker checks run in Release and Debug, without assert side effects")
