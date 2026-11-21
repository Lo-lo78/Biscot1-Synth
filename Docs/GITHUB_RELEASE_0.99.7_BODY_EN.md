# Biscot1 Synth 0.99.7 — AHDSR Trigger & Pitch-Synchronized Ping Pong Pan

**21 November 2026 | Windows x64 VST3**

## What's new

- **Three independent AHDSR Trigger switches (Op1/2/3):** automatically cycle each operator's amplitude Attack/Hold/Decay/Release while its note gate is held. Off by default, with natural Release after Note Off or CC64 release.
- **Three signed Ping Pong Pan parameters (Op1/2/3):** -1 to +1, default 0. Positive values alternate right/left, negative values left/right; intensity sets width.
- **True arpeggiator-step alternation:** works on SID Seq and classical arp steps, even with 2, 4, 6 or 8 simultaneous held notes. Coincident arp and AHDSR attacks flip just once.
- **Pitch-locked stereo movement:** Pan follows the exact per-sample movement of operator **Portamento** and **Arp Glide**, instead of arriving late behind a fixed pan-smoothing time. Hard pitch jumps move immediately; live glide changes and retriggered steps stay synchronized.
- **Live envelope automation:** the three operator Release, three Mod Env Release and Filter Release respond during an already-sounding tail, including after Note Off. The seven Hold controls adjust during their own active Hold stage. Drift amount and Noise Character also respond on sounding notes.
- **Accessibility:** new parameters are available in the native editor, tracker F1 Pattern and Song lanes. Existing REAPER **Ctrl+Shift+A** toggles active automation envelopes without deleting recorded points; NVDA announces **`, Auto`** when active.

## Backward compatibility

- Six new VST3 controls are appended at **IDs 256–261**. All earlier IDs are retained, including the seven Hold controls from version 0.99.6 at 249–255.
- Legacy `.biscot` presets use **Trigger Off** and **Ping Pong Pan 0**, preserving the original sound unless the new options are enabled.
- LFO Phase Drift/Lace, SID filters, Patterns/Songs and multi-output support are retained. The SID noise-clock trial in the source remains experimental.

**Recommended download for the website:** `Biscot1-0.99.7-Windows-x64.vst3.zip` (identical VST3 bundle to the GitHub Actions artifact `Biscot1-0.99.7-Windows-x64.zip`).

**Project and source:** https://github.com/Lo-lo78/Biscot1-Synth
