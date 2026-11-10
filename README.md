# Biscot1 Synth VST3 — TEST0.1.1 (build configuration fix)

**Important:** This is a **partial native audio port**, not a faithful 1:1 conversion of Biscot1 Synth v2.38. Do not replace the JSFX yet.

## Source of truth
`Reference/Biscot1Synth_v2.38.jsfx` is an unchanged copy of the user's November 10, 2026 source. The project includes all **224 native JSFX sliders** as automatable VST3 parameters with their original names, ranges and defaults. Plugin IDs are `s002` to `s225`; the first JSFX slider is 2 and no reindexing is performed.

## Implemented in TEST0.1
- JUCE 8 VST3 instrument / GitHub Actions Windows x64 packaging.
- Basic native three-oscillator synth: independent MIDI note modes, per-operator waveform, pitch, fine pitch, pitch bend, pulse width, level and pan; simple amp ADSR.
- Very basic shared low-pass and DC blocker; stereo mix output; optional additional output buses reserved for subsequent builds.
- Keyboard-accessible JUCE parameter list and value editor. Native host parameters are exposed for accessibility and automation.
- DAW project state recalls the full parameter catalogue.

## **Not implemented / not equivalent yet**
- The faithful SID oscillator math, AA/mipmaps/oversampling and wave-combination details.
- Original 6581/8580 nonlinear multi-mode filters and filter envelopes.
- Exact chord voice allocation/legato/arp logic, oscillator hard sync and ring modulation, three LFOs, cross-mod matrix, modulation envelopes and LFO rate cross modulation.
- Full independent output buses and sophisticated audio sleep behavior.
- The accessible LB-303-style main editor and Alt+Q Pattern/Song interface (placeholder only).
- Exact silence/click behavior at pulse endpoints: native approximation is **not yet a parity test**.
- `Reference/*.jsfx` cannot be executed by the VST3 as-is. This is not a JSFX wrapper.

The plugin deliberately exposes parameters before all DSP connections are implemented. An automated catalogue check does not prove DSP/audio equivalence. Windows compilation still requires GitHub Actions verification; the local environment cannot execute the Windows runner.

## Building on GitHub
Create a **new** repository, e.g. `Lo-lo78/Biscot1-Synth`, upload contents from this archive at repository root, or push using the Git command below. On push, Actions builds the VST3 and publishes the ZIP under the workflow Artifacts (not Releases).

```cmd
git add . && git commit -m "Add Biscot1 VST3 native migration foundation TEST0.1" && git push
```

For a local build with Visual Studio 2022: `cmake -S . -B build -G "Visual Studio 17 2022" -A x64` followed by `cmake --build build --config Release`. GitHub Actions is pinned to `windows-2022` to ensure the matching generator is present.

## Next steps for true DSP parity
Port the oscillator accumulator + wave blending/AA and 6581/8580 filter directly from the JSFX, then per-operator envelopes, MIDI, modulation and arpeggiation, testing rendered audio against REAPER at each milestone. Only after the synth is equivalent, integrate the old tracker into Alt+Q. Do **not** confuse this initial proof of build structure with a finished replacement.

## Build fix — TEST0.1.1 (10 November 2026)
The initial GitHub Actions log (`logs_116141940146.zip`) reported:
`Generator Visual Studio 17 2022 could not find any instance of Visual Studio`.
This occurred because `windows-latest` moved to a Visual Studio 2026 runner.
The workflow now explicitly uses `windows-2022` (matching generator `Visual Studio 17 2022`).
Also replaced `#include <JuceHeader.h>` with JUCE module headers. The project does not invoke `juce_generate_juce_header`, so relying on that generated header would otherwise fail at C++ compilation.
The ZIP staging is explicit to retain the `.vst3` directory at the archive root.
These are build-only changes: the synthesizer DSP, source JSFX, parameter catalog and MIDI behavior remain unchanged. This build has **not** been compiled on a GitHub Windows runner from this environment.

Do **not** run `git init` again. Copy the extracted project contents into the existing local `Biscot1-Synth` repository (preserving its `.git` directory), then run:

```cmd
git add . && git commit -m "Fix Biscot1 VST3 TEST0.1.1 GitHub Actions build configuration" && git push
```

Build artifact name after a successful workflow run: `Biscot1-TEST0.1.1-Windows-x64`.
