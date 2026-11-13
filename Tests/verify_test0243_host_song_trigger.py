from pathlib import Path
r=Path(__file__).resolve().parents[1]
cpp=(r/'Source/PluginProcessor.cpp').read_text()
h=(r/'Source/PluginProcessor.h').read_text()
# Original identifier is intentionally kept so existing host automation IDs resolve.
assert 'juce::ParameterID{"trackerSongTrigger",1},"Song Select",0,99,0' in cpp
assert 'juce::ParameterID{"trackerSongTransport",1},"Song Transport"' in cpp
assert 'juce::StringArray{"Stop","Pause","Play"},0' in cpp
assert 'hostSongTransportParameter=state.getRawParameterValue("trackerSongTransport")' in cpp
assert 'hostSongTriggerParameter=state.getRawParameterValue("trackerSongTrigger")' in cpp
assert 'hostSongPending=hostSelection;' in cpp
assert 'transportStarted && hostTransport==2' in cpp
assert 'hostCommandChanged' in cpp
assert 'hostSongPaused=true;' in cpp
assert 'hostSongPaused=false;' in cpp
assert 'songPlaybackSnapshot && !hostSongPaused' in cpp
assert 'songTransport.stop();' in cpp
assert 'hostSongLastTransport=-1' in cpp
assert 'hostSongTransportParameter' in h
assert 'bank->byNumber[(size_t)selected]' in cpp
assert 'transport->setValueNotifyingHost(transport->convertTo0to1(0.f));' in cpp
assert '"Song Trigger",0,100,0' not in cpp
print('OK: independent Song Select and Stop/Pause/Play host automation')
