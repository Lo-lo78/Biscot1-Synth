# Biscot1 Synth VST3 — TEST0.2.37 (GitHub Actions release-test repair)

The TEST0.2.36 GitHub Actions run failed before compilation because several
Python regression scripts still asserted the old TEST0.2.35 version string.
This release reads its version from CMake for all regression release checks and
adds an early consistency guard for the editor title and GitHub artifact.
All 53 Python structural checks and 33 standalone C++ tests passed locally.
See `RELEASE_NOTES_TEST0.2.37.md`. The full JUCE/MSVC VST3 build must be
verified on GitHub Actions. The separately reported Song library file-loading
issue has not yet been confirmed resolved without an actual saved Song file.

---

# Biscot1 Synth VST3 — TEST0.2.35 (faithful file-browser playback)

**Current tracker library:** in Alt+Q, F1 Ctrl+S opens an accessible Pattern
file-name edit; F2 Ctrl+S opens a single Song name edit, and F2 Alt+S opens
an all-100-Songs file-name edit. The initial name is selected for easy typing;
Enter writes the file and Esc cancels. In these name fields, pressing Space
inserts a real space and explicitly announces `spazio` to NVDA. The same
accessibility fix also applies to the synth's existing `.biscot` preset name,
rename and folder fields.

Library files are under `Documents/Biscot1-Synth/patterns` and
`Documents/Biscot1-Synth/songs`. Single Song `.biscot1-song` may be imported
into any selected Song slot; `.biscot1-all-song` restores all 100 fixed slots.
Both embed referenced Patterns; synth timbre is not overwritten. Colliding
filenames receive a numeric suffix rather than overwriting unrelated exports.
F1 Alt+S still opens the independent complete `.biscot` preset browser.

**Remaining migration priority:** advanced browser management and archive verification;
Windows/REAPER/NVDA user acceptance tests. The following older release
sections remain as historical records, not the current implementation.

---

# Biscot1 Synth VST3 — TEST0.2.25 (Song End, resource arbitration and native Song effects)

Alt+Q F2 Song can now play three operator lanes at once. **Space** starts/stops,
**Ctrl+Space** pauses at the actual Song row. Separate Pattern references,
repeat/speed/transpose, note routing, Song mixer and MIDI Song triggers are
wired into the original SID DSP. The Lua Song migration is **not yet complete**:
Sidechain/Delay, Song End links and the independent Song export still need work.
See `RELEASE_NOTES_TEST0.2.25.md` for the Song End, automatic Filter/LFO ownership, Sidechain and Delay additions. The detailed migration plan is in `SONG_MIGRATION_PLAN.md`.

---


# Biscot1 Synth VST3 — TEST0.2.20

Fixes F1 numeric section shortcuts **1 = OP**, **2 = Filter**, **3 = LFO**. The Windows Ctrl+letter fix mistakenly reused an alphabet-only key matcher for digits; the new helper uses JUCE keyCode or text character and is covered by a Release regression test. F2 stays as in TrackerControl 0.166: Ctrl+Tab changes section; digits remain available to incremental search. F3 Song, the three concurrent operator lanes and routing of patterns to Op1/Op2/Op3 are **not yet migrated**.

Native F1/F2 parameter automation (Previous value, Curve, Resolution, Ratio) plus Lua-compatible F2 insertion rules. The core SID DSP and `.biscot` save format remain unchanged. See `RELEASE_NOTES_TEST0.2.20.md`.

# Biscot1 Synth VST3 — TEST0.2.19.3

**Lua 0.166 audition parity for F1 value edits.** When you change the value
of Note, Gate, OP, Filter or LFO in a step containing a Note, that step now
previews again immediately after the edited data are published to the immutable
audio snapshot. Works with Alt+Up/Down, Alt+PageUp/PageDown, Alt+E and
Alt+Home/End. Backspace reset also previews the restored value. Reaching a
minimum/maximum without a real value change does not retrigger. A step without
Note remains silent on isolated preview, as in Lua; editing does **not**
interrupt a Pattern already playing via Space. No changes to the DSP,
MIDI, preset/REAPER bank format or Song features. See
`RELEASE_NOTES_TEST0.2.19.3.md`.

---

# Biscot1 Synth VST3 — TEST0.2.19.2

First audible native Pattern test (Alt+Q, F1/F2): **Space** starts/stops from the edit cursor, **Ctrl+Space** pauses on the currently sounding step, and vertical F1 movement auditions one step as in TrackerControl Lua 0.166. Note/Gate/Pause, per-repeat speed/transpose and direct F2 OP1/Filter/LFO parameter changes are implemented using the existing SID sound engine. REAPER host and `.biscot` Pattern persistence are maintained.

**This is not the complete tracker:** Curve/Curve Resolution, Sub Notes, full Previous Value semantics, Release Arpeggio, MIDI step recording, F1 Ctrl+S file export and F3–F6 Song/browser are still being migrated. See [`RELEASE_NOTES_TEST0.2.19.2.md`](RELEASE_NOTES_TEST0.2.19.2.md). Complete Windows VST3 build and NVDA audio tests have not yet been performed.

---

# Biscot1 Synth VST3 — TEST0.2.18.4

Fixes F1 Ctrl+C / Ctrl+X / Ctrl+V on Windows, adds the Lua F1 Tab current-position
reading, Enter Pattern number jump (0–999), Ctrl+J step number jump,
Ctrl+PageUp/Down nonempty step search, Alt+Left/Right configurable jumps,
Alt+Home/End min/max, Backspace reset and whole-Pattern copy/initialize.

Synth DSP and MIDI are unchanged. See `RELEASE_NOTES_TEST0.2.18.4.md`.

# Biscot1 Synth VST3 — TEST0.2.18.3

Hotfix for the TEST0.2.18.2 Windows GitHub Actions failure AFTER a successful VST3 compilation.
The MSVC Release build disables <cassert> assertions, so the new TrackerSearchStepTest
skipped operations embedded in assert calls, leaving the LFO test vectors empty and
exiting with code 1. The two Pattern tracker regression tests now use always-on CHECK
that evaluates expressions and reports failures even under -DNDEBUG.

No changes to the synthesizer DSP, MIDI, CPU idle, tracker navigation, preset memory,
serialisation format, parameter IDs or NVDA controls. F1/F2 are as in TEST0.2.18.2.
See RELEASE_NOTES_TEST0.2.18.3.md.

# Biscot1 Synth VST3 — TEST0.2.18.2

**Tracker Alt+Q navigation and F1 editing enhancements:** F1 Home/End
reversed as requested; F2 Home/End unchanged, with Ctrl+Home/End added for
first/last parameter. F2 now has multi-character, word-prefix and reverse
incremental search. F1 gains Shift+arrows step selection, multi-step
copy/cut/paste, step reset, and section clearing. All three LFOs are listed
consecutively in the F2 LFO section, exactly as in the supplied Lua tracker.
See `RELEASE_NOTES_TEST0.2.18.2.md`. No native playback or Song editor yet.

# Biscot1 Synth VST3 — TEST0.2.18.2

Compiler-only hotfix for TEST0.2.18: JUCE `juce::Identifier` cannot be initialized as `constexpr` with MSVC 19.44 / JUCE 8.0.15. The tracker state key is now an immutable ordinary `const juce::Identifier`, with unchanged text `trackerPatternsV1`. The VST3 project/Pattern serialization format is unchanged. This fixes GitHub Actions error C2131 in Source/PluginProcessor.cpp. No changes to MIDI, DSP, CPU idle, accessible editor behavior, or tracker data.

# Biscot1 Synth VST3 — TEST0.2.18

**Initial selection hotfix:** A brand-new instance starts on **Audio Mode, Stereo** (parameter s2) instead of **MIDI Mode** (s3). The previous bug came from the fallback in `restoreLocation()`: it used index 3 even if no last selection had been saved. Saved editor locations in existing projects remain respected. NVDA root/focus behaviour from TEST0.2.17 and the DSP, MIDI, arpeggiators, CPU optimizations, browser and presets are unchanged.

See `RELEASE_NOTES_TEST0.2.17.1.md` for the test instructions.

# Biscot1 Synth VST3 — TEST0.2.17

**NVDA editor-entry focus correction only.** The main `AudioProcessorEditor`
now uses the same ignored accessibility root as the tested LB-303 reference.
JUCE still uses that root as a keyboard focus container and transfers focus
only after REAPER enters the plugin; NVDA should then encounter the native
Grid ComboBox directly instead of announcing an intermediate "unknown".
The native Grid, Value and Edit roles, announcements and keyboard shortcuts
are unchanged. See `RELEASE_NOTES_TEST0.2.17.md`.

No audio/DSP/MIDI or preset changes; TEST0.2.16's idle CPU reduction remains.

# Biscot1 Synth VST3 — TEST0.2.16

**TEST0.2.16 CPU idle pass**: after the last audible release, silent blocks
skip the full 16-channel arpeggiator scans, LFO usage analysis, audio bus view
construction, and per-sample DSP. The host transport is still queried so that
Free LFO phase and BPM restart semantics remain deterministic. During sounding,
classical arpeggio pools are rebuilt at MIDI note/CC64 events (sample accurate),
and only when relevant automated parameters change between blocks. All DSP
formulas for active sound are unchanged.

This is an optimization candidate, **not** a guaranteed REAPER CPU reading:
REAPER's reported `c` includes host/plugin overhead and changes with hardware,
buffer size and enabled output buses. Evaluate unloaded and after a full
Release with equal host and buffer settings. See RELEASE_NOTES_TEST0.2.16.md.

# Biscot1 Synth VST3 — TEST0.2.15

The JSFX v2.38-based native VST3 with accessible Grid/Value/Edit, MIDI and
clocked arpeggiators. **Current work: point 4, CPU idle efficiency, LFO Square
PWM and editor-entry focus.** See `RELEASE_NOTES_TEST0.2.15.md` for implementation
and validation scope. Point 5 is the full audio/MIDI comparison before Alt+Q.

# Biscot1 Synth VST3 — TEST0.2.12 (ARP Release restoration)

**Fix after TEST0.2.11:** In the original JSFX, the arpeggiator continues changing pitch throughout the natural VCA Release. The 0.2.11 test mistakenly required `gate=true` to advance an arp, freezing the pitch after the final key/pedal release. This regression is corrected in 0.2.12 for SID Seq and classical arps.

- **No pedal:** final Note Off triggers the ADSR Release but the arp keeps advancing on its BPM clock using its last latched notes.
- **With CC64:** pedal sustains the current phrase; after CC64-up the arp keeps stepping while the ADSR releases.
- **At ADSR idle:** clear the release latch/sequence and stop the clock. There is no infinite hanging note.
- **Trigger / Legato:** Trigger restarts the sequence on its next time step, Legato keeps running; no per-step ADSR retrigger.
- No changes to the SID oscillator/filter/output DSP, preset interface, parameter IDs, or piano key handling.

Git: `git add . && git commit -m "Biscot1 VST3 TEST0.2.12 restore arp during ADSR Release" && git push`

---

# Biscot1 Synth VST3 — TEST0.2.11 (ARP clock / Trigger / Legato / note release)

## TEST0.2.11 — ARP clock, Trigger, Legato, Note Off

- `OpN Mode Trigger = Trigger`: on a new MIDI key while a phrase is active, the arp **pattern** returns to its first step at the next beat; the underlying BPM clock is not reset. No audible pitch change happens at Note On.
- `OpN Mode Trigger = Legato`: the arp position and phase keep running. A new note is added to the next step pool update.
- Neither mode retriggers Attack, Decay, Sustain or Release **on individual arpeggio steps**; ARP is frequency sequencing, not repeated envelope gates.
- When the last key is released without CC64, the arp stops advancing and holds its last pitch while the original ADSR Release fades. When CC64 is down and the Sustain Pedal parameter is On, the arp keeps running; CC64 up initiates natural Release and stops the arp.
- In Chord Allocate, the common Op1 Trigger/Legato setting controls these transport decisions for all three operators. All other MIDI modes retain individual settings.
- No DSP algorithm, plugin parameter ID, browser, keyboard or accessibility navigation changes.


**Migration notes from TEST0.2.10:** DSP, oscillators, filter and output amp are unchanged.

- Classical ARP modes (Up, Down, Pendulum, Held Order and Random) quantize incoming notes to the existing clock.
- Trigger restarts the arpeggio **pattern** on its next clock edge, preserving BPM phase, without reattacking the per-step ADSR.
- Legato retains the current arp position and clock. Neither mode plays the newly pressed note immediately.
- SID Seq keeps its independent recorded 128-position note memory and shares the same Trigger / Legato transport semantics.
- The final Note Off without CC64 stops arp motion but allows a natural VCA Release tail. Pedal held keeps the clock running until CC64 up.
- The first MIDI note of a phrase starts the envelope normally; pitch-only arpeggio steps never force another Attack.
- In Chord Allocate, Op1 Trigger / Legato determines the behaviour for all three arpeggiators.
- All pre-existing controls, preset format and accessible-editor commands remain intact.

All standalone tests can be run by GitHub Actions; full Windows VST3 + NVDA/REAPER audio checks remain pending.

**Git command:**

```cmd
git add . && git commit -m "Biscot1 VST3 TEST0.2.11 correct arp Trigger Legato and final note release" && git push
```

---

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


## TEST0.2.13 — CC64 release without arp pitch discontinuities

The TEST0.2.12 SID sound, VCA ADSR curve, accessible editor, preset browser,
MIDI modes, arpeggiator clocks and musical Trigger/Legato modes remain unchanged.
The CC64-up event now preserves SID Seq's live chord pitch root, rather than
replacing it with an old per-operator latch root. Classical arpeggiators retain
an exact snapshot of their current note pool when CC64 is lifted. This is the
same release-note material used for a manual key release. Only the envelope
passes into Release; the arpeggio continues until the envelope reaches idle.

GitHub Actions runs `SidPedalReleaseContinuityTest` plus all prior regressions.

## TEST0.2.14 — Point 3: Filter ADSR, Trigger / Legato and LFO fidelity

- Based on TEST0.2.13. Oscillator, SID filter core, amplifier and output stages are unchanged.
- LFO phases, 1,000,000-second initial Delay clocks, One Shot ages and deterministic S&H seeds now match the JSFX v2.38 initialization.
- On allowed MIDI note triggers, LFO Delay restarts for all three LFOs, while Phase/age restart only for Trigger-mode or One Shot LFOs. Op1 Mode Trigger governs the shared filter/LFO gate; repeated arp notes do not retrigger filter/LFO phases.
- LFO processing now matches JSFX ordering: waveform -> Delay fade -> Smooth -> upper/lower half-wave Squash. Smooth Random uses minimum smoothing, and S&H/Smooth Random update on both half-cycle and cycle wrap.
- One Shot LFOs resume free-running when One Shot is disabled after reaching the end, without another MIDI note. All existing modulation-rate cross-connections remain.
- Filter key follow updates with every incoming MIDI note, including one which joins an already-running arpeggio without resetting its envelope. The existing filter ADSR release remains latched through CC64.
- No new slider or VST3 parameter ID. Tests include standalone C++ numerics plus static integration checks. Build/REAPER/NVDA test still required on GitHub Actions.


## Native Tracker TEST0.2.18 (first stage: editor and persistent Pattern bank)

The original Biscot1 Tracker 0.166 Lua, JSFX and project Startup Loader are retained
under `Reference/LegacyTracker/` as the exact behavioural references.
The native tracker has no need for `gmem` or a Lua startup loader to retain Pattern data.

### Keyboard in Alt+Q

- `Alt+Q`: open the tracker from the synth; inside tracker, return to synth.
- `Esc`: return to synth without discarding edits.
- `F1`: Pattern Editor; `Enter`: parameter picker on an actual Pattern step. `F2` is reserved for a future Song editor.
- In F1, `1`, `2`, `3`: OP, Filter, LFO section; `Up/Down`: step;
  `Left/Right`: parameter cell; `PageUp/PageDown`: five steps.
- `Ctrl+Up/Down`: previous/next Pattern (0–999), `Ctrl+Left/Right`: move a
  parameter one cell chronologically within its section.
- `Alt+Up/Down`: change value; `Alt+PageUp/PageDown`: change by ten steps;
  `Alt+E`: exact numeric entry, Enter to accept, Esc to cancel. `Alt+Enter`: jump to a Pattern number.
- `Delete`: remove selected cell; `Ctrl+C` and `Ctrl+V`: copy/paste one cell;
  `Ctrl+Z` / `Ctrl+Shift+Z`: Pattern undo/redo.
- In the Enter-opened parameter picker, arrows navigate parameters; `Ctrl+Tab` cycles OP,
  Filter and LFO; `Enter` inserts and returns to F1; `Ctrl+Enter` inserts
  and remains in the picker. `Ctrl+Left/Right` changes the picker column height. `Esc` returns to F1.
- `Alt+S` and `Alt+B` retain the existing preset save/browser behaviour;
  `.biscot` and REAPER host state now also include all edited Patterns.

The Pattern bank supports up to 1000 Patterns, 1024 steps and 128 ordered cells
per step, with dynamic (sparse) storage. Pattern values are independent from
live synth parameters until the future native tracker playback engine is added.
Original JSFX and plugin MIDI/DSP paths have not been modified by this stage.
The parameter picker also includes VST3-native Sustain Pedal, Pitch Bend Mode,
Chord Op1 Controls and Square PWM with append-only logical tracker IDs.

**Not yet implemented in TEST0.2.18**: playing Patterns or Songs, F3–F6,
step selection/recording, import/export to the agreed library directories,
Ctrl+S standalone Pattern/Song export, Song editor and all Song routing.
Those are subsequent stages; this build is intended to validate the native
F1/F2 UI and project/preset persistence first.

### TEST0.2.19.1: F1/F2 cell navigation and readable choice values
- F1 announces the named options used in the synth for Mode, Wave, Arp, LFO and Off/On controls (not raw enum indices). Curve Ratio displays its original Lua labels.
- F1 remembers the preferred horizontal cell separately from the actual visible cell. Moving through a step with fewer parameters no longer resets the chosen column; when the destination step has enough cells, the cursor returns to that column.
- The three F1 sections retain independent preferred columns when switching. No change to synthesis, playback or preset data format.

### TEST0.2.19.2: F1 true logical cursor, matching-parameter jumps

- Keep the original Lua 0.166 `logical_cell` when crossing a step with fewer cells. A programmatic JUCE ComboBox selection of the currently visible item is NOT a horizontal move and must not override it. Intentional left/right movement (or a genuinely different direct ComboBox selection) does update the logical position.
- In F1, **Ctrl+Alt+Down/Up** visits the next/previous step containing the exact same parameter ID in the current OP/Filter/LFO section. It skips steps without the parameter, focuses the matching cell, announces it and previews the step. No match leaves cursor unchanged. This shortcut is a native extension (not a pre-existing binding in Lua 0.166).
- Existing **Ctrl+PageUp/PageDown** continues to navigate any nonempty step, as before. The DSP, transport, preset format and F2 remain unchanged.

## TEST0.2.19.5 — F1 Enter and Alt+Enter

In Alt+Q F1, Enter edits the current value (including Length, MIDI Note, Repeat, Speed, Transpose and populated step cells). Alt+Enter selects Pattern 0..999 from F1 or F2. Alt+E remains a legacy alias. F2 Enter/Ctrl+Enter preserve their existing insertion behavior.


## TEST0.2.20 — Previous Value and Curve automation

This is the first native Pattern automation pass, ported from `Biscot1 TrackerControl 0.166` and `Biscot1 Tracker.jsfx`. Previous Value only affects later parameter cells in the row, and traces the nearest actual value through consecutive inherited steps. Curve runs toward the next explicit occurrence of the same parameter, including across empty steps. Bipolar Curve -100..100, Resolution 2..1024 and Ratio Off / 1:1 ... 2:2 produce sample-clock automation during Pattern playback. Curve metadata belongs in front of the affected parameter, and Ctrl+Left/Right in F1 can reorder cells. Filter/LFO F2 now expose shared Previous Value, Curve and Pause commands.

F2 Enter inserts (or moves the cursor to an existing parameter), Ctrl+Enter inserts and remains in F2, and Sub Note can be inserted more than once only after an active Arp Mode. Adding Resolution or Ratio without Curve automatically inserts Curve at +100. Playback uses a single-pass, immutable message-thread compiled automation snapshot; no file-format changes, no per-sample allocations.

**Still pending:** live native Sub Note sound, Step Recording MIDI, MIDI Pattern triggers, Release Arpeggio and individual Pattern library exports. F3 Song editor has not yet been started. This test version is not a final parity claim.


## TEST0.2.21 — Cleaner Alt+Q keyboard layout

From F1, **Enter** now opens the OP / Filter / LFO parameter picker on the
selected Pattern step. **Alt+E** opens exact numeric value entry, including
Length, MIDI Note, Repeat, Speed, Transpose and all populated cells. The old
F2 parameter-picker shortcut has been removed: **F2 is reserved for the future
Song editor** and currently announces that the Song page is not implemented.
In the picker, Enter inserts and returns to F1; Ctrl+Enter inserts and stays;
Ctrl+Tab cycles sections; Esc returns to F1. Alt+Enter still jumps to the
specified Pattern 0–999. The F1 Alt+Home/End **value** boundaries have been
reversed at the user's request: **Alt+Home = maximum; Alt+End = minimum**.
Unmodified Home/End and the parameter picker grid navigation are unchanged.
No DSP, MIDI transport, Curve, Pattern persistence or `.biscot` format changes.

### Native tracker TEST0.2.21

F1 announces the concise section/step/value on arrows or 1/2/3; Tab gives the full Pattern number. Header items include Length, MIDI Note, Step Recording and Release Arpeggio. Step Recording captures MIDI Note On and sustain pedal-down as Pause, without passing recorded input to the synth. MIDI Note assignment launches a Pattern while consuming its trigger note. Sub Note and first pitch-only Release Arpeggio execution are integrated; see `RELEASE_NOTES_TEST0.2.21.md`. F2 is reserved for the future Song page.


## TEST0.2.21.1 — Native arp Release chord and F1 direct keys

- A normal, manually released three-note chord now preserves the **complete** arp Release pool. Previous builds shrank it with each MIDI Note Off, yielding only the last note. CC64-held phrases retain their behavior; the VCA Release and audio DSP are unchanged.
- F1: L=Length, M=MIDI Note, S=Step Recording, R=Release Arpeggio, P=Repeat. D/T move to successive Speed/Transpose values; Shift+D/T cycle backwards, including across repeat rows.
- Going Up or Home from Repeat remembers the last visited step, rather than jumping to the final step. Alt+S preset saving and Enter's parameter picker are unaffected.
- Tested with `SidArpReleaseLatchTest`, `TrackerHeaderNavigationTest`, and existing C++ and Python suites. Windows JUCE compilation and NVDA remain to be checked in REAPER.

To push this source from an existing repository (preserve its `.git` folder):

```cmd
git add . && git commit -m "Biscot1 VST3 TEST0.2.21.1 fix manual arp release chord and add F1 header shortcuts" && git push
```

## TEST0.2.21.2 — Correct selected-cell insertion

- F1 Ctrl+V inserts a copied parameter at the **current cell** (before the
  previously selected parameter), not one position to its right. Subsequent
  cells are shifted right and the new cell stays selected.
- F1 Enter opens the parameter picker; Enter or Ctrl+Enter inserts the chosen
  parameter at the same current cell, not at the end of the step.
- Ordinary parameter IDs cannot be duplicated by pasting; Sub Note can be
  repeated after an active OP Arp Mode. Curve Resolution and Ratio retain
  their required relative order, with Curve auto-added when missing.
- Release Arpeggio is retained, OFF by default. In the original tracker, this
  option uses the Pattern's written notes for pitch-only legato movement in
  the final envelope tail. It is distinct from the synthesizer's normal
  arpeggiator, which now continues throughout the normal Release.
- No changes to the SID DSP, normal arpeggiator release, preset format, or
  Pattern playback scheduler.

Git: `git add . && git commit -m "Biscot1 TEST0.2.21.2 fix tracker parameter insertion position" && git push`

## TEST0.2.21.3 — F1 fixed navigation regions and Release Arpeggio analysis

Plain Up/Down/PageUp/PageDown stay strictly within Header, Steps or Repeat/Speed/Transpose. Only Home/End move between regions. **Release Arpeggio** is not an arp sustain toggle: Off leaves the ordinary chord arpeggiator cycling during Release; On tries to replace it with explicit Pattern Note cells (not Sub Notes). The user reports On can sound static; its audible Windows behavior remains to be diagnosed. This update leaves the audio code untouched. See `RELEASE_NOTES_TEST0.2.21.3.md`.

## TEST0.2.21.4 — F1 local Home/End and independent Pattern library

F1 navigation is now three coherent regions (Header, Steps, Repeat/Speed/Transpose):

- **Home/End**: first/last item **within the current region**. On Header neither key moves vertically; on Steps they go to step 0 or Length-1, preserving the OP/Filter/LFO logical cell; on footer they go to Repeat or the last Transpose.
- **Ctrl+Home / Ctrl+End**: navigate to the previous/next region. Footer -> Steps returns to the **last visited Step**, while Header -> Steps also resumes there. They no longer treat Repeat and Speed as separate regions.
- Alt+Home/End still set maximum/minimum parameter values; in the Enter-opened parameter picker, Ctrl+Home/End still reach first/last parameter and Home/End still navigate the selected column.

**Ctrl+S in F1** (including the parameter picker) now saves **only the active Pattern**, such as Pattern 4, as a UTF-8 text `.biscot1-pattern` in `Documents/Biscot1-Synth/Patterns/Singles/Pattern_004.biscot1-pattern`. This standalone library file includes all three OP/Filter/LFO step tables, order of cells, repeat speeds/transposes, MIDI Note, Release Arpeggio, name and length. It does not change the full `.biscot` preset or the REAPER project. The `Patterns/Singles` and `Songs` folders are created on the first export. A file of the same name from another preset is not silently overwritten; a numeric suffix is used when necessary. Re-saving the same Pattern within the same tracker session updates the file originally exported.

**F2 Song is still reserved**. Planned export: `Songs/<SongName>.biscot1-song` plus the exact referenced Patterns in `Patterns/<SongName>/*.biscot1-pattern`, using **relative paths** and stable Pattern IDs. This will be added together with the F2 Song editor, not before it. The audio/sequence in the song package uses the currently loaded synth sound; only the `.biscot` preset includes the synth configuration. A portable Song package must include both the Songs file and the corresponding Patterns subfolder.

Native `.biscot` format, tracker playback and DSP have **not changed** in this release.


## TEST0.2.23 — Song F2 edit controls

Song F2 now has step-range selection (Shift+arrows/Page), cell and step clipboard (Ctrl+C/X/V), independent Undo/Redo (Ctrl+Z/Ctrl+Shift+Z), Song name (Ctrl+R), ordering (Ctrl+Left/Right), Mute/Solo (M/S) and Song volume (+/-). Mixer changes are stored but will take audible effect only after the Phase 3 Song audio engine. F2 Ctrl+S standalone Song export remains Phase 4; Alt+S saves the complete preset including the Song bank. See RELEASE_NOTES_TEST0.2.23.md and SONG_MIGRATION_PLAN.md.

## TEST0.2.26 — Song End, F2 auto-preview and page memory

Song End now supports a live-updated next-Song target and the F2 display follows the active Song, announcing transitions for NVDA. **Every lane must reach the end of its own Length and Repeat before Song End happens**, including blank Song rows. For a quick test, set Length=1 in Op1/2/3 of Song 0 and Song 1, assign Song End=1 in Song 0's Global page (key 4), and start Song 0 with Space.

F2 auditions only the selected operator's Pattern from Pattern Start when navigating Song steps or editing a Pattern-related value. The original Lua default is three Pattern steps. Optionally insert **Preview Steps (1–1024)** from the Enter parameter picker on a given Song row, then edit its value with Alt+E or Alt+arrows. This is a Song editor audition length; it does not modify actual Song playback, Repeat, or the `.biscot` synth sound. Previews never interrupt an already-running full Song.

On F1/F2 opening, NVDA reads Pattern or Song explicitly. F2 value navigation no longer repeats «Operator 1» on every cell; Tab still announces the full location. Alt+Q remembers the last F1/F2 editor in the VST state (`trackerLastMode`), and the two editor instances retain their own cursor position across page switches. All old Song parameters retain their original numeric IDs; new Preview Steps is appended as optional ID 23. See `RELEASE_NOTES_TEST0.2.26.md`.

## TEST0.2.33: Native keyboard-first tracker library browser (previous behavior)

In Alt+Q, press Ctrl+O from F1 for `Documents/Biscot1-Synth/patterns`,
or from F2 for `Documents/Biscot1-Synth/songs`.
The browser uses an accessible JUCE ComboBox (no system file chooser).

- Up/Down: choose file or folder. PageUp/PageDown jump by 5, Home/End jump to boundaries.
- Enter on a folder: enter it. Backspace: parent. Escape: cancel pending import, parent, or close at root.
- Space or P: preview saved file **without loading**. Ctrl+P: toggle automatic preview while navigating (off by default).
- + / - : audition length (1, 3, 8, 16, 32 Pattern steps). F5 refresh. Tab announces the browser context.
- Left/Right on a `.biscot1-all-song` file: select which Song slot (0–99) to hear; importing the file still restores all 100 static slots.
- Enter on a Pattern: import into its original number, or the first free slot if already occupied.
- Enter on a Song: require another Enter to confirm replacement of the current Song slot or all 100 Song slots.
- Imported Pattern/Song dependencies are embedded as in TEST0.2.32. Library previews are immutable, audio-thread safe, bounded and never overwrite project banks. When the browser closes, the tracker regains keyboard focus.

Existing named Ctrl+S / Alt+S exports, NVDA "spazio" typing feedback, Song End,
preview on Song rows, and the synth DSP are unchanged.


## TEST0.2.34: Incremental filename search, three-operator Song preview

This section supersedes the old keyboard controls in the TEST0.2.33 browser section above.

- In ALL Biscot1 file browsers (Preset, Pattern, Song) type a filename prefix progressively. A single `p` jumps to the next filename beginning with P; rapid `pa`, `pat` narrow the search. Prefixes of words also match (e.g. `bass` finds `Electric Bass`). Repeating one letter cycles, and after around 1.2 s idle the next key begins a new search. Folders can also match.
- **P does not trigger preview.** In the Pattern/Song libraries, **Space** plays a temporary preview without loading. **Ctrl+P** toggles automatic preview, persisted separately for Pattern and Song libraries across sessions. `+/-` still chooses 1/3/8/16/32 Pattern steps, 3 by default.
- **Right Arrow** enters a highlighted folder, **Left Arrow** and **Backspace** go to its parent in all three browsers. Enter on a file loads/imports it (with the existing confirmation for Song replacement). In `.biscot1-all-song`, use **Ctrl+Left / Ctrl+Right** to select the preview Song slot 0–99; plain arrows are reserved for folders.
- Library Song audition now combines the first usable Pattern from each of the **three operators**. Each operator retains its Pattern Start, transpose and speed; Song mixer settings are honored. The three short clips start together for instant recognition even when their first Song rows differ. This is not full-Song playback and does not follow Song End. F2's regular step-navigation audition remains operator-only, matching Lua.
- No changes to synth DSP, full Song/Pattern playback, or the `.biscot1-pattern`, `.biscot1-song`, `.biscot1-all-song` formats.

See `RELEASE_NOTES_TEST0.2.34.md` for details and testing limitations.


## TEST0.2.35: faithful browser playback (supersedes 0.2.34 audition)

In Alt+Q, **Pattern browser** (`F1`, `Ctrl+O`) and **Song browser** (`F2`, `Ctrl+O`) now play the exact saved musical timeline, not a reconstruction. Space starts/stops preview. Ctrl+P is remembered independently by browser. Both file formats start at time zero and include initial silence. `+` extends and `-` shortens the preview window: `1`, `3`, `8`, `16`, `32`, `64` 16th-note divisions, or **Full file** (default). The window is **not** an edit of file contents. For Pattern playback, divisions follow actual Pattern step progression, including repeats; for Song playback, the cutoff uses 16th-note real-time divisions at the host tempo and all three lanes continue at their own stored speeds.

- **Pattern**: respects real notes, pauses, parameter automation, gate, per-repeat speed/transpose and all Pattern repeats. No jumping to the first note.
- **Song**: same compiled and sample-clocked three-operator arrangement as normal playback, including lane-specific lengths/repeats/speed, original Song rows, Pattern Start, Play Length, Start Offset, Filter/LFO ownership, FX rows, mixer and Song End chaining **only if the target is in the opened file**. Missing Song End targets simply stop.
- **Full Song archive**: Ctrl+Left / Ctrl+Right chooses the Song slot to preview, while Left/Right without Ctrl navigate folders.
- **Synth presets (`.biscot`)**: the preset browser loads/previews the patch's settings; it does not invent notes or start playing the embedded Song.
- The browser does not import the file during preview and does not modify the active Song/Pattern banks. The sound uses the synth patch currently loaded in the VST3 because Song and Pattern files do not contain a synth patch.
- The short, selected-operator preview used while moving through F2 cells is separate and unchanged.

See `RELEASE_NOTES_TEST0.2.35.md`.


TEST0.2.36: See RELEASE_NOTES_TEST0.2.36.md for cursor, shortcut and library diagnostics.
