# Biscot1 Synth VST3 — TEST0.2 (accessible editor + extended native DSP)

**Development build; the VST3 has NOT reached sonic parity with the original JSFX.**
This milestone starts from the known working TEST0.1.1 and the exact user's
`Reference/Biscot1Synth_v2.38.jsfx`, and follows the workflow and native
accessibility design of the user-supplied LB-303 project.

## Accessible main editor

- 12 contexts: Global/MIDI, Op1, Op2, Op3, Filter, LFO1, LFO2, LFO3,
  Arpeggiators, Ring/Sync, OSC/ENV Cross, LFO Cross/Rate.
- All 224 original source sliders included exactly once, with stable plugin parameter IDs
  `s002` through `s225`. No parameter renaming or reindexing.
- Grid / Value / Edit are three keyboard-focusable native JUCE controls, modelled
  on LB-303. Grid is a native ComboBox: selected item reads 'name, value'.
- Categorical sliders are native VST3 **AudioParameterChoice** with named options
  (rather than anonymous numeric floats); integer sliders use native parameter integers.
  Values and changes propagate through JUCE/APVTS to the host.
- The focused Grid captures all four arrow keys including RIGHT at the last column.
  At column/page borders it remains in the plugin and produces no movement.
- Windows UI Automation notification speech follows the LB-303 approach.
  NVDA/REAPER actual focus behavior must still be verified in the user's Windows setup.

### Keyboard commands

| Key | Action |
| --- | --- |
| Ctrl+Tab / Ctrl+Shift+Tab | Next / previous context |
| Grid arrows | Navigate the 8-row multi-column grid |
| Grid Home / End | First / last row of current column |
| Grid PageUp / PageDown | Move five rows within the column |
| Ctrl+Home / Ctrl+End | First / last item in context |
| Tab and Shift+Tab | Cycle Grid / Value / Edit |
| Enter | Grid ↔ Value; Edit accepts numeric value |
| Value arrows Up/Down | Edit selected value |
| Alt+Up / Alt+Down | Change value in ANY of the three controls |
| Alt+Shift+Up / Down | Change value by 10 steps |
| Alt+Left / Alt+Right | Change step precision |
| Alt+PageUp / PageDown | Change value by 40 steps |
| Alt+Home / Alt+End | Maximum / minimum |
| Alt+E / Alt+L | Focus numeric Edit / Grid |
| Backspace | Reset selected parameter to default |
| Esc in Edit | Cancel and return to Grid |
| Letters in Grid/Value | Incremental name search |
| Alt+Q | Future tracker placeholder only; does not open a sequencer |

## Extended native sound implementation

TEST0.2 adds substantial sound connections compared with TEST0.1.1:

- Amp ADSR and individual Mod ENV, trigger/legato, portamento, drift, pitch bend,
  wave selection, PWM, noise color, level and pan on all three operators.
- Ring modulation and hard sync in SID operator topology, Test Bit, plus LFO/ENV
  bit gating with independent thresholds.
- Three BPM-aware LFOs: shapes, phase, delay, smoothing, one-shot, squash,
  pitch/PWM/level/pan/cutoff depths and mutual rate cross-modulation.
- Six oscillator/envelope cross routes and three assignable LFO cross routes,
  including positive and negative depths, cutoff and resonance targets.
- Approximate 6581/8580 SVF with low/band/high combinations, cutoff, resonance,
  filter ENV, key follow, input/output levels; Op3 mute semantics.
- Per-operator ARP with BPM-aware Up/Down/Pendulum/Held Order/Random, octave ranges and glide;
  note pools refreshed at MIDI events or block boundaries, not at every sample.
- Multichannel bus support when hosts enable the optional output buses and Audio
  Mode is Multichannel; idle modes/presets remain stable across saves/loads.
- Natural PWM endpoints: neither -64 nor +64 hard-mutes the oscillator.

### Not equivalent to JSFX yet

The three oscillators are **not** a sample-identical DSP translation. In
particular, original oversampling / spectral wavetable AA, 6581/8580 nonlinear
filter math, combined SID wave DAC models, exact noise chip behavior, perfect
SID Seq ARP and its recorded memory, full multi-note latch allocation, and some control
corner cases are NOT yet ported faithfully. Arp SID Seq currently plays the held material without a faithful
recorded SID sequence memory; the final tracker tables are not implemented. Cross-modulation and filters are currently C++ approximations.

All 224 parameter IDs are exposed and their major destinations are connected
to the native engine, but this does not mean every sonic edge case is identical
to the original JSFX. Keep the JSFX installed for A/B listening.

## GitHub Actions / installation

Open a CMD in the existing Git clone of
`https://github.com/Lo-lo78/Biscot1-Synth`, extract this archive at the repo
root **without deleting `.git`**, then run:

```cmd
git add . && git commit -m "Biscot1 VST3 TEST0.2 accessible editor and native DSP expansion" && git push
```

`.github/workflows/build-windows.yml` runs three source-level checks, builds
the VST3 on `windows-2022` / Visual Studio 2022 with JUCE 8.0.15, and uploads
`Biscot1-TEST0.2-Windows-x64.zip` to GitHub Actions Artifacts. The VST3 is
not automatically published as a GitHub Release.

**Validation:** static source checks passed in the preparation environment. A
Windows build and NVDA/REAPER audio test have NOT been performed here. On a
GitHub compiler error, share the full Actions log archive to correct the next
test build.

## License

See LICENSE.md and LICENSE_NOTE.md. The reference JSFX is preserved unchanged.
