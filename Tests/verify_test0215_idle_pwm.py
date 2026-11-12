from pathlib import Path
import json,re
r=Path(__file__).resolve().parents[1]
cat=(r/'Source/ParameterCatalog.h').read_text()
proc=(r/'Source/PluginProcessor.cpp').read_text()
head=(r/'Source/PluginProcessor.h').read_text()
editor=(r/'Source/PluginEditor.cpp').read_text()
lfo=(r/'Source/SidLfoCore.h').read_text()
workflow=(r/'.github/workflows/build-windows.yml').read_text()
assert len(re.findall(r'^  \{\d+, "',cat,re.M))==232
assert 'std::array<ParameterDef,232>' in cat
assert 'std::array<std::atomic<float>*,234>' in head
for i in range(1,4):
 assert f'{{{230+i}, "LFO{i} Square PWM", 0.05f, 0.95f, 0.001f, 0.5f, ""}}' in cat
 assert f'const int pwm={231}+i, wave=124+i*23;' in editor
assert 'case 4:raw=x<std::clamp(squarePwm,0.05,0.95)?1.:-1.;break;' in lfo
assert 'sampleRateHz,param(231+i));' in proc
assert 'if(idleSleep.asleep)' in proc
assert 'lfoUsedCached=determineUsedLfos' in proc
assert 'lfoUsedCached[(size_t)i]?processLfo(i,previous):0.f' in proc
assert 'advanceIdleLfos(next-sample)' in proc
assert 'if(idleSleep.observe(noGate&&!fxTail,vcaIdle&&!fxTail,peak))enterColdIdle();' in proc
assert 'if(multichannel)for(const float x:stems)' in proc
assert 'for(auto& stage:outputStages)stage.reset();' in proc
assert 'juce::AccessibilityHandler::clearCurrentlyFocusedHandler();' in editor
assert 'handler->getNativeImplementation()==nullptr' in editor
assert 'SidIdleAndSquarePwmTest.exe' in workflow
assert 'TEST0.2.25' in editor
assert 'VERSION 0.2.25' in (r/'CMakeLists.txt').read_text()
assert len(json.loads((r/'Reference/ParameterCatalog.json').read_text()))==224
native=json.loads((r/'Reference/NativeParameters.json').read_text())
assert [x['index'] for x in native[-3:]]==[231,232,233]
print('OK: 224 unchanged JSFX controls, 8 append-only native controls; cold idle, LFO PWM, UIA focus')
