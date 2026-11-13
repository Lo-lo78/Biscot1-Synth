"""Regression: edits in F1 retrigger Lua-compatible one-step preview."""
from pathlib import Path
r=Path(__file__).resolve().parents[1]
panel=(r/'Source/TrackerPatternPanel.cpp').read_text()
clock=(r/'Source/TrackerPatternPlayback.h').read_text()
workflow=(r/'.github/workflows/build-windows.yml').read_text()
assert 'const double adjusted=juce::jlimit(d->min,d->max,v);' in panel
assert 'if(std::abs(c->value-adjusted)<1.0e-12)return;' in panel
assert 'recordUndo();c->value=adjusted;finishEdit();' in panel
# Ensure the audition happens after persistence & immutable snapshot publication.
assert 'recordUndo();c->value=adjusted;finishEdit();' in panel
assert panel.index('recordUndo();c->value=adjusted;finishEdit();') < panel.index('auditionCurrentStep();', panel.index('recordUndo();c->value=adjusted;finishEdit();'))
assert 'auditionCurrentStep();' in panel.split('void TrackerPatternPanel::changeValue(double v)',1)[1].split('void TrackerPatternPanel::adjustValue',1)[0]
assert 'recordUndo();c->value=def->initial;finishEdit();' in panel
assert 'if(trackerCanAuditionStep(currentPattern(),row,status))' in panel
assert 'trackerStepHasAuditionNote' in clock and 'status!=1' in clock
assert 'TrackerPatternEditAuditionTest.exe' in workflow
assert 'VERSION 0.2.30' in (r/'CMakeLists.txt').read_text()
print('OK F1 live-value audition, no-op boundaries, note-only preview, full-playback protection')
