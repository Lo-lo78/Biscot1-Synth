"""TEST0.2.25: release-latch MIDI integration and F1 direct navigation wiring."""
from pathlib import Path
root=Path(__file__).resolve().parents[1]
processor=(root/'Source/PluginProcessor.cpp').read_text(encoding='utf-8')
header=(root/'Source/PluginProcessor.h').read_text(encoding='utf-8')
ui=(root/'Source/TrackerPatternPanel.cpp').read_text(encoding='utf-8')
cmake=(root/'CMakeLists.txt').read_text(encoding='utf-8')
workflow=(root/'.github/workflows/build-windows.yml').read_text(encoding='utf-8')
assert 'void rebuildArpPools(bool captureRelease=false)' in header
assert 'sidLatchArpReleasePool(pool,count,arpReleasePools[(size_t)i],' in processor
assert 'arpReleaseCounts[(size_t)i],captureRelease);' in processor
note_on=processor.split('if(m.isNoteOn()){',1)[1].split('if(m.isNoteOff()){',1)[0]
note_off=processor.split('if(m.isNoteOff()){',1)[1].split('float BiscotProcessor::polyBlep(',1)[0]
assert 'rebuildArpPools(true);return;' in note_on
assert 'noteOff(ch,note);' in note_off and 'rebuildArpPools();return;' in note_off
pedal=processor.split('if(m.getControllerNumber()==64){',1)[1].split('if(m.getControllerNumber()==123)',1)[0]
assert 'std::copy_n(arpPools' not in pedal
assert 'trackerMoveWithinF1Region(row,delta,pat.length,pat.repeat)' in ui
assert 'trackerF1HomeTarget(row,pat.length,lastRealStep)' in ui
assert 'trackerHeaderShortcut(command,m.isShiftDown()' in ui
assert "{'l','m','s','r','p','d','t'}" in ui
assert 'SidArpReleaseLatchTest.exe' in workflow and 'TrackerHeaderNavigationTest.exe' in workflow
assert 'VERSION 0.2.25' in cmake and 'TEST0.2.25' in workflow
print('OK release chord retained until ADSR idle, F1 direct header / D/T cycle wired')
