# Biscot1 Synth VST3 — TEST0.2.9 (MIDI held notes, legato, retrigger)

**Base:** TEST0.2.5.1, whose audible DSP and NVDA Grid navigation are preserved.

This is phase 1 of MIDI parity with the reference JSFX v2.38. Changes are
limited to note-event state and its envelope/LFO trigger decisions.

- Three independent **16-note, newest-note priority** held stacks: duplicated
  Note On events are retained and newest matching Note Off is removed.
  Releasing an overlapped higher note returns to the previous note without
  restarting the envelope.
- `All Operators Same Note`: all operators follow the same stack.
- `Chord Allocate 3 Mono Ops`: first unused operator takes the incoming note,
  otherwise Op1 stacks the new note. **Op1 Mode Trigger** controls legato on
  Op2 and Op3 while in Chord mode, as in the JSFX.
- `MIDI Channels 1-3`: each operator has its own stack; other MIDI channels
  still feed the original global filter-note tracking but do not play an operator.
- Arpeggiated voices retrigger only on the first Note On of a phrase. The
  arpeggiator's clock/steps and SID Seq memory are scheduled for phase 2 and
  are **not** yet claimed to match the JSFX.
- A separate global filter held stack uses Op1 Mode Trigger to determine
  when the filter ADSR and LFO note-trigger event fires; a note event no longer
  resets the three LFOs once per synthesized operator.
- Physical MIDI Note On duplicates on the same channel no longer make an
  arpeggiator's held-note pool prematurely forget a still-held pitch after
  one Note Off.
- Pitch Bend Omni/Multi and all existing parameter IDs are unchanged. The
  patch browser, saved editor position, accessible keyboard shortcuts and
  audio oscillator/filter/output code are unchanged.

Test coverage: `SidMidiNoteStateTest` is a JUCE-free C++ regression executable
compiled and executed by GitHub Actions. Local `g++` test passed; JUCE VST3 build
and REAPER/NVDA audio/MIDI tests still require the Windows Actions runner.

To test inside REAPER: hold C4, then E4, release E4 (expect return to C4);
try this with Legato on/off, repeat the same note twice before releasing it,
then test 4 notes in Chord Allocate, releasing the newest Op1 note first.

## Git command for this version

```cmd
git add . && git commit -m "Biscot1 VST3 TEST0.2.9 restore JSFX held-note stacks and legato routing" && git push
```

---

# Biscot1 Synth VST3 – TEST0.2.5.1 (Grid speech fix)

## TEST0.2.5.1: NVDA Grid speech regression fix

The TEST0.2.5 Grid keyboard navigation accidentally became silent when
`select()` changed the ComboBox ID with `dontSendNotification` without
replacing the suppressed screen reader announcement. This release sends one
concise UI Automation notification containing **parameter name and current
value** after an actual selection change. Applies to Up/Down, Left/Right,
Home/End, Page Up/Down, Ctrl+Home/End and incremental search. Silent borders
remain silent, and repeated search on the same entry is still announced.

No title/description mutation, no redundant accessibility focus requests,
and no extra speech on initial plugin open. The preset browser, parameter
persistence and DSP are unchanged from TEST0.2.5.

**Not yet verified inside REAPER/NVDA:** static tests cannot guarantee how a
particular NVDA/Windows UIA configuration speaks the result. The next build
needs an explicit keyboard navigation test on Windows.


Based on **TEST0.2.4**, retaining the exact same synthesis code from the sound-matched TEST0.2.3. **No oscillator, filter, MIDI or output-stage DSP changes.**

## Accessibility changes

- **Silent initial editor construction**: static title, section and instructions are visual-only (`setAccessible(false)`); removed duplicate accessibility focus-grabs and mutating titles/descriptions. REAPER may still announce its own FX window, which a plugin cannot suppress.
- Grid/Value/Edit now have stable names without extra `Tab` or `Alt+L` descriptions. No redundant UIA descriptions are injected during normal navigation.
- Typing letters/digits searches the **current section's parameter names** incrementally (first word or another word); repeated letter cycles; Shift reverses search. Alt+Shift+Left/Right changes section.
- The **last selected parameter and section** are stored in the plugin's host snapshot (`editorLastParameter`) and restored when the editor is reopened and when the REAPER project is reloaded.
- Alt+B opens a keyboard-driven preset browser; Alt+S saves the entire current patch as a `.biscot` file in `Documents/Biscot1-Synth`.
- Browser: Up/Down preview; Enter confirms load; Escape cancels and restores the original sound. Backspace/Left parent folder, Ctrl+N new folder, F2 rename, Delete confirmation. Saves use an explicit name and overwrite confirmation. Focus returns to Grid, Value, or Edit.
- Browser is **integrated into the same plug-in window**, reusing the LB-303 accessibility/browse workflow, without Lua, gmem, or a separate process. `Alt+Q` still reserves the future tracker.

**Testing:** Static Python tests run in GitHub Actions. Full Windows JUCE build and REAPER/NVDA behavior still require user verification. Preset file format deliberately differs from LB-303: `.biscot`, not `.lb`.

## Git command

```cmd
git add . && git commit -m "Biscot1 VST3 TEST0.2.5.1 restore NVDA Grid navigation speech" && git push
```

---

# Biscot1 Synth VST3 — TEST0.2.4 (Pitch Bend Mode: Multi / Omni)

This is a focused MIDI routing update on top of the **sound-matched TEST0.2.3**.
No oscillator, filter, envelope, analog output-stage, LFO, or keyboard-navigation DSP changes.

- New accessible **Pitch Bend Mode** choice beside **MIDI Mode** on the first editor page.
- **Multi (default):** the existing pitch wheel routing for MIDI channels 1, 2, and 3 is unchanged; the legacy "All Operators Same Note" MIDI mode still bends all three operators from channel 1.
- **Omni:** pitch wheel from **any MIDI channel 1–16** bends the three operators together. Each operator retains its own `Op1/2/3 Pitch Bend` semitone range.
- This changes **pitch bend reception only**. It does NOT make MIDI notes on channel 7 audible in `MIDI Channels 1-3` mode; use a suitable note-routing MIDI mode for the input MIDI file.
- The new VST3-specific parameter has stable ID `s226`. The original 224 JSFX-derived parameter IDs `s002`–`s225` are unchanged, and the reference JSFX remains unmodified. User presets default to Multi unless changed.
- JUCE-independent routing tests cover all 16 channels, legacy Multi behaviour, and center/reset. Python static tests protect parameter order and editor placement.

## Git command (existing local repository, do not re-initialize Git)

```cmd
git add . && git commit -m "Biscot1 VST3 TEST0.2.4 add Omni and Multi pitch bend modes" && git push
```

**Build:** GitHub Actions compiles on `windows-2022` and stores `Biscot1-TEST0.2.4-Windows-x64.zip` under Artifacts. Windows/REAPER runtime playback remains to be verified.

---

# Biscot1 Synth VST3 — TEST0.2.3 (JSFX filter and wavetable migration)

## Changes compared with TEST0.2.2

This release targets the **large tonal mismatch** between JSFX v2.38 and the VST3.
Earlier builds used a generic trapezoidal filter and different oscillator waveforms. Those were different synthesis algorithms, not just different filter presets.

**Ported directly from the supplied JSFX 2.38 source:**

- The 8580 and 6581 state-variable filter's original update order, register-to-cutoff curves, damping, internal loss, resonance presence, 6581 low-frequency body leakage, model-specific colour/makeup and -12 dB R4 filtered trim. These replace the generic TPT SVF.
- Original 2048-point, ten-mip Fourier lookup tables for Triangle and Saw, the exact 50% Square table and the two-Saw variable Pulse approximation.
- The same antialiased Triangle+Saw, Triangle+Pulse, Saw+Pulse and triple-combination formulas, with model-dependent coefficients and analytical DC compensation.
- Adaptive 2x/4x/8x/16x oscillator substeps for active Ring/Sync and high-register energy compensation (as in JSFX). The manual and modulated Ring/Sync switches remain available.
- Noise clocked four times per oscillator cycle and the 23-bit SID colour path, replacing the old once-per-cycle noise.
- Exponential Release using the current envelope level, instead of an unrelated linear release.
- Removed the VST3-only velocity-to-amplitude multiplier: the original JSFX gates the notes on velocity but does not apply that multiplication to the signal.
- Corrected Filter Key Follow (32 register units per semitone) and bipolar OSC/ENV Cross Mod source, preserving parameter IDs and keyboard editor controls.

**Verification:** the included standalone C++ test checks the filter sample-by-sample against numerical reference outputs independently calculated from the JSFX formulas (for both 6581/8580), plus oscillator mip integrity. GitHub Actions builds and runs that test. A Windows VST3 compilation and listening test are still necessary.

**Important limits:** this is **not yet a fully 1:1 migration**. The remaining differences include exact MIDI/ARP SID Seq memory and note allocation, LFO state and timing corner cases, oscillator cross-mod ordering, note reset/idle behaviours, and possible discrepancies in noise random-number sequences and dynamics. The JSFX should remain the sound reference. No blanket claim of sample-identical audio is made.

### Git command (in the existing repository)

```cmd
git add . && git commit -m "Biscot1 VST3 TEST0.2.3 port SID filter and bandlimited oscillators from JSFX" && git push
```

---

## Previous release: TEST0.2.2 (output stage and accessibility)

# Biscot1 Synth VST3 — TEST0.2.2 (JSFX output stage and accessible navigation)

## Changes from TEST0.2.1

**Audio output chain ported from the supplied Biscot1 JSFX v2.38:**
- Restored the model-dependent 6581/8580 asymmetric SID output amplifier, bias, parallel wet/dry rounding and makeup constants.
- Restored the original approx. 16 kHz external analogue RC low-pass and 1.6 Hz DC blocker, **before** Master Volume. Removed the VST-only generic terminal saturator.
- Restored JSFX center-pan law (unity on both channels), the 3-voice chord normalization, and the 6581 final post-mix parallel coloration.
- In multichannel mode, the four stereo stems now pass through independent final output stages, and operators routed to the filter are not also duplicated into dry stems.
- Source comparison: `Reference/Biscot1Synth_v2.38.jsfx`, functions `sid_output_amp`, `@slider` output coefficients, and the final `mix_l/mix_r` stage.

**Accessible keyboard improvements:**
- `Alt+Shift+Right` = next page; `Alt+Shift+Left` = previous page. Silent at boundaries; handled before the existing Alt+Right/Left step shortcuts; Ctrl+Tab aliases retained.
- Incremental name search from LB-303: type letters/digits in Grid or Value, build up a prefix for 1 second, search starts of names or words, repeat a character to cycle, hold Shift to search backward (including digits), wrap within the current page. The search announces selected parameter and value; the numeric Edit stays a normal edit box.
- Arrow keys at page boundaries continue to be handled inside the plugin, not by REAPER.

**Limitations:** the FINAL amplifier/RC/DC chain now uses the JSFX equations, but upstream oscillator AA, combined waves, SID filters and some ARP logic remain approximate. This is **not** complete DSP parity. Windows JUCE/MSVC and NVDA runtime tests have not been performed locally. Use GitHub Actions to build and compare audio against the original JSFX.

## Git command for the existing repository

```cmd
git add . && git commit -m "Biscot1 VST3 TEST0.2.2 port JSFX output and improve accessible search" && git push
```

---

## Historical notes: TEST0.2.1 (JUCE 8 build fix; accessible editor + extended DSP)

## What TEST0.2.1 fixes from the GitHub Actions failure

The log `logs_116170501978.zip` demonstrated three Windows C++ compilation errors:

- `PluginProcessor.cpp`: `AudioPlayHead::PositionInfo::getIsPlaying()` is
  a plain `bool` in JUCE 8, not an optional value to dereference. The
  stopped-to-playing transition still resets free-running LFO phases.
- `PluginEditor.cpp`: `juce::StringArray` has no `indexOfIgnoreCase` member.
  The Edit field now finds choice labels using `equalsIgnoreCase`, retaining
  case-insensitive option selection and numeric fallback.

No changes to the 224 parameter IDs, keyboard mapping, MIDI engine, or DSP
behaviour are intended in this fix. The previous TEST0.2 features are retained.
Other compiler *warnings* in the log (deprecated MIDI iterator and Font
constructor, signed/unsigned comparison) are non-fatal and left for a separate
cleanup.

**The static checks pass locally; this archive has not been built with MSVC.**

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
| Alt+Shift+Right / Alt+Shift+Left | Next / previous context (preferred) |
| Ctrl+Tab / Ctrl+Shift+Tab | Next / previous context (legacy aliases) |
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
| Letters and digits in Grid/Value | Incremental name search; Shift searches backward |
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

## GitHub Actions / installation (current TEST0.2.3)

Open a CMD in the existing Git clone of
`https://github.com/Lo-lo78/Biscot1-Synth`, extract this archive at the repo
root **without deleting `.git`**, then run:

```cmd
git add . && git commit -m "Biscot1 VST3 TEST0.2.3 port SID filter and bandlimited oscillators from JSFX" && git push
```

`.github/workflows/build-windows.yml` runs source-level checks and a compiled standalone filter/wavetable regression, builds
the VST3 on `windows-2022` / Visual Studio 2022 with JUCE 8.0.15, and uploads
`Biscot1-TEST0.2.3-Windows-x64.zip` to GitHub Actions Artifacts. The VST3 is
not automatically published as a GitHub Release.

**Validation:** static source checks passed in the preparation environment. A
Windows build and NVDA/REAPER audio test have NOT been performed here. On a
GitHub compiler error, share the full Actions log archive to correct the next
test build.

## License

See LICENSE.md and LICENSE_NOTE.md. The reference JSFX is preserved unchanged.


## TEST0.2.9 – SID Seq recorder (point 2)

The `SID Seq` arpeggiator mode (mode 1) no longer reuses the live-key arpeggio pool. It now uses the JSFX v2.38 clock-insertion memory, independently for each operator: 128 active positions plus mirrored shadow, per-operator position and clock, first step on the first audio sample, and timing based on the host BPM. A note enters **after the currently sounding clock step**; additional keys do not retune the oscillator immediately.

`Chord Allocate` broadcasts every incoming note to each enabled SID Seq, independently from which operator was assigned the note. The per-operator note snapshot stays available throughout the envelope release in All-Operators and MIDI-Channels modes. At the end of the release it is cleared as in the JSFX. The previous DSP, accessibility editor, preset browser and normal arpeggio modes are preserved.

Standalone C++ tests cover insertion, the 128-step limit, clock and release-latch persistence. The VST3 and real-time audio must still be verified in REAPER after GitHub Actions compilation.

## TEST0.2.9: per-operator Sustain Pedal (CC64)

Op1 / Op2 / Op3 each have an independent Sustain Pedal Off/On choice next to the amp ADSR Sustain control. When On, releasing a MIDI key while CC64 is down holds the envelope until CC64 is released. Off is the backward-compatible default. Test the Op3 envelope with Op3 Level above zero. See RELEASE_NOTES_TEST0.2.9.md.


## TEST0.2.9 — CC64 arpeggios / Chord Op1 Controls

In Chord Allocate, `Chord Op1 Controls` (Global/MIDI, adjacent to Pitch Bend Mode) can switch from default `Independent` to `Op1 Master`. Only Portamento, amplitude ADSR, Sustain Pedal CC64 enable, Level and PWM are shared. Wave, pitch, pan, arpeggiator, LFO and other modulation settings stay separate. This affects the *effective sound*; individual stored Op2/3 values are unchanged.

All arpeggiators now retain released notes while CC64 is down if the corresponding operator has Sustain Pedal On. SID Seq adds notes to its current memory without clearing the sequence. Release of CC64 clears pedal-held notes while the arpeggiator can finish over the VCA's normal release. See `RELEASE_NOTES_TEST0.2.9.md`.
