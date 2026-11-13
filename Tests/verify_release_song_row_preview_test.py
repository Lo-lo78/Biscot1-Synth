from pathlib import Path
p = Path(__file__).resolve().parent / 'TrackerSongRowAuditionFxTest.cpp'
s = p.read_text(encoding='utf-8')
assert '#define CHECK(expression)' in s
assert 'CHECK(fx.insert(17))' in s
assert 'CHECK(change.insert(20))' in s
assert 'CHECK(step.insert(1))' in s
assert 'CHECK(plainStep.insert(1))' in s
assert '#include <cassert>' not in s
print('OK: Song row preview tests run insertions and checks even when NDEBUG is defined')
