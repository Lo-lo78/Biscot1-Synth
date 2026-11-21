# Biscot1 Synth 0.99.7 — Accessible update guide

**21 November 2026 · Windows x64 VST3**

This version brings together changes from **TEST0.2.47 through TEST0.2.52**. The seven AHDSR Hold stages and REAPER Ctrl+Shift+A shortcut from version 0.99.6 remain available.

## AHDSR Trigger: one switch per operator

Three new accessible controls, **Op1 AHDSR Trigger**, **Op2 AHDSR Trigger**, **Op3 AHDSR Trigger**, each have **Off / On**, with **Off by default**.

- **Off:** standard amplitude AHDSR behavior is unchanged.
- **On:** while an operator's MIDI/CC64 gate remains held, its amplitude envelope repeats **Attack → Hold → Decay → one-sample Sustain → Release → Attack**. Once the gate ends, the envelope completes its natural Release without restarting.
- The operators are independent. Trigger does **not** reset oscillator phase, the operator Mod Env or the shared Filter envelope.

In **Alt+Q, F1 Pattern**, find **OP AHDSR Trigger**. Existing Song lane mapping directs it to the corresponding operator. REAPER can automate the parameter.

## Signed Ping Pong Pan per operator

New controls **Op1 Ping Pong Pan**, **Op2 Ping Pong Pan** and **Op3 Ping Pong Pan** are independent; each ranges from **-1.00 to +1.00** in **0.01** steps, with **0** by default.

- At **0**, classic Pan and its modulation remain unchanged.
- At a **positive** depth, e.g. **+1**, the first Attack alternates from the **right**, then left.
- At a **negative** depth, e.g. **-1**, the first Attack alternates from the **left**, then right.
- Smaller magnitudes reduce stereo width. The offset combines with ordinary Pan, envelope modulation and LFO Pan modulation.

Alternation responds to normal **MIDI Note On**, repeating **AHDSR Trigger** attacks and **actual audible steps** in both **SID Seq** and **classical arpeggiators**. It works with **2, 4, 6 and 8 held notes**; coincident arp/Trigger events cause only one side switch.

### Pan follows Portamento and Arp Glide

- With **no active glide**, the stereo side switches **on the same sample as the hard pitch change**, including a new Attack.
- During operator **Portamento**, native **SID Seq Arp Glide** or classical **arpeggiator Glide**, Pan follows the **real per-sample pitch motion**, not a separate fixed-duration pan ramp. Fast glide stays fast; slow glide stays smooth.
- A new arp step may retarget Pan mid-glide; live glide-speed automation and simultaneous glide sources are followed without a separate lag.
- Only **automation of the Ping Pong Pan amount itself** keeps a short **~3 ms de-click**. This does not delay pitch-driven alternating sides.

Find **OP Ping Pong Pan** in the Alt+Q/F1 Pattern parameter picker; it is also available to Song lanes and REAPER VST3 automation.

## Automation during sounding or releasing notes

The TEST0.2.47 fixes allow these parameters to respond while the envelope is already running:

- **Op1/2/3 Release**, **Op1/2/3 Mod Env Release** and **Filter Release** can be automated *after MIDI Note Off*, shortening or extending the currently audible Release tail without another MIDI Note On. Automating Release to zero ends the tail; an envelope that has fully finished is not restarted.
- All **seven Hold** durations update while their own Hold plateau is active. A completed stage is not rewound.
- **Op1/2/3 Drift** amounts adjust on existing notes while the note's original random draw stays latched; **Noise Character** reacts before the next noise clock edge.

Event-driven modes do not retroactively generate Note On events after voices have ended.

## Keyboard accessibility

- **Alt+A:** accessible About; **Tab/Shift+Tab** moves among read-only description, Project, Contact and Close. **Escape** closes.
- **Alt+Q:** open the tracker; **F1** for Pattern, **F2** for Song. **1/2/3** switch the operator/section only inside Steps.
- **Ctrl+Shift+A** on a main parameter page, from **Grid, Value or Edit**, toggles the selected REAPER parameter envelope's **ACTIVE** state without deleting existing automation points. NVDA announces **`, Auto`** when active. This shortcut does not affect Alt+Q tracker navigation.
- **Space in F2:** starts/stops Song playback and restores the starting edit position, including after chained Songs.

## Presets, compatibility and release

Version **0.99.6** introduced seven AHDSR Hold controls (Op1–3 amplitude, Op1–3 modulation and shared Filter), range **0–5 seconds**, **1 ms** steps, default **0**, at host IDs **249–255**. Version **0.99.7 appends six IDs 256–261** without changing earlier parameters. Older `.biscot` presets start with the new **Trigger switches Off** and new **Ping Pong Pan amounts at 0**.

LFO Phase Drift/Lace from 0.99.5 and Pattern/Song support remain available. The **23-bit SID noise-clock listening trial** remains experimental (`NOISE_SID_23BIT_CLOCK_TRIAL.md`), not a verified tonal improvement.

After the successful Windows build, publish GitHub tag **`v0.99.7`** with **`Biscot1-0.99.7-Windows-x64.zip`**. For a website requiring `*.vst3.zip`, attach an **identical copy** named **`Biscot1-0.99.7-Windows-x64.vst3.zip`**. Back up presets and DAW projects before updating.
