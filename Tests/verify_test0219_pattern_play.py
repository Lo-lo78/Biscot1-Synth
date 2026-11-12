"""Regression checks for first audible Pattern test, without requiring JUCE on Linux."""
from pathlib import Path
root=Path(__file__).resolve().parents[1]
processor=(root/'Source/PluginProcessor.cpp').read_text()
header=(root/'Source/PluginProcessor.h').read_text()
panel=(root/'Source/TrackerPatternPanel.cpp').read_text()
clock=(root/'Source/TrackerPatternPlayback.h').read_text()
workflow=(root/'.github/workflows/build-windows.yml').read_text()
assert 'processBlock(juce::AudioBuffer<float>& buffer,juce::MidiBuffer& midi)' in processor
assert 'trackerTransport.tick(' in processor
assert 'trackerApplyStep(playback->rows[' in processor
assert 'trackerReleaseNote();' in processor
assert 'trackerSetOverride(3,2.f);' in processor
assert 'juce::MidiMessage::noteOn(1,note,' in processor
assert 'const bool coldIdleNoMidi=idleSleep.asleep && midi.isEmpty() && !trackerTransport.running' in processor
assert 'trackerRequested.exchange(0,' in processor
assert 'trackerPlayable.load(std::memory_order_acquire)' in processor
assert 'std::atomic<std::shared_ptr<const TrackerPlayablePattern>>' in header
assert 'juce::Timer::callAfterDelay(65,' in panel
assert 'if(code==32)' in panel
assert 'pauseAtAudibleStep();' in panel
assert 'playFromCursor();' in panel
assert 'auditionCurrentStep();' in panel
assert 'std::max(1.,rate/(' in clock
assert 'TrackerPatternPlaybackTest.exe' in workflow
print('OK: Pattern preview, transport, F2 audio mapping, idle guard and Win workflow')
