"""Regression: the JUCE LFO function must not close before return baseWave.

An extra `}` after Lace previously caused MSVC C2059 at the next return
and hundreds of cascading namespace errors in GitHub Actions.
"""
from pathlib import Path

source = (Path(__file__).resolve().parents[1] / 'Source' / 'PluginProcessor.cpp').read_text()
start = source.index('float BiscotProcessor::processLfo(')
end = source.index('float BiscotProcessor::processFilter(', start)
body = source[start:end]
assert body.count('{') == body.count('}'), 'processLfo brace mismatch: premature function closure'
assert body.count('return baseWave;') == 1
assert body.rfind('return baseWave;') < body.rfind('}')
assert 'sidLfoLace(' in body
print('OK 0.99.5: processLfo braces and return remain inside function (MSVC C2059 guard)')
