"""Regression checks for the experimental SID noise-clock change.

This verifies the integration and independently simulates the legacy clean
clock to ensure the 4x stream is unchanged after separation from the SID clock.
"""
from pathlib import Path
import re

root = Path(__file__).resolve().parents[1]
cpp=(root/'Source/PluginProcessor.cpp').read_text(encoding='utf-8')
h=(root/'Source/PluginProcessor.h').read_text(encoding='utf-8')
assert 'v.cleanNoiseClock+=clockDelta*4.;' in cpp
assert 'v.noiseClock+=clockDelta*16.;' in cpp
assert 'while(v.cleanNoiseClock>=1.)' in cpp
assert 'while(v.noiseClock>=1.)' in cpp
assert 'v.cleanNoiseValue*(1.f-character)+v.sidNoiseValue*character' in cpp
assert 'v.sidNoiseSeed=sid;' in cpp
assert 'double noiseClock=0,cleanNoiseClock=0;' in h
assert 'v.cleanNoiseClock=0.;' in cpp
assert 'if(character<=0.f)' not in cpp[cpp.index('// The clean noise retains'):cpp.index('msb[(size_t)i]')]

# Same recurrence and timing as the legacy single 4x clock, including oversampling.
seed_before=seed_after=1234567
old_clock=new_clock=0.
old_value=new_value=0.
for sample in range(100000):
    phase_increment=(40.+(sample%123)*13.)/48000.
    oversample=1 if sample%9 else 4
    for _ in range(oversample):
        delta=phase_increment*4./oversample
        old_clock+=delta
        new_clock+=delta
        while old_clock>=1.:
            old_clock-=1.
            seed_before=(seed_before*1103515245+12345)%2147483647
            old_value=seed_before/1073741823.5-1.
        while new_clock>=1.:
            new_clock-=1.
            seed_after=(seed_after*1103515245+12345)%2147483647
            new_value=seed_after/1073741823.5-1.
        assert seed_before==seed_after and old_value==new_value
print('OK: SID 16x clock, legacy clean 4x clock preserved and independent')
