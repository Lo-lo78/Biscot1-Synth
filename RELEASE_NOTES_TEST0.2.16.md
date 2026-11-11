# Biscot1 Synth VST3 TEST0.2.16 — Deep idle CPU optimization

Base: TEST0.2.15. Objective: reduce idle CPU without modifying audible DSP.

- Silence fast path: after `buffer.clear()`, skip LFO usage scans, arp pool
  reconstruction, output bus view setup, MIDI iterator, and sample-by-sample
  processing when `idleSleep.asleep && midi.isEmpty()`.
- Free-mode LFO clocks continue at block rate, even for presently unused
  destinations, matching the JSFX v2.38 cold-idle clock rule. Host transport
  start/stop and BPM updates are retained.
- Classical arp pool refresh becomes event-driven and parameter-driven:
  note/CC64 events rebuild at their original MIDI sample offsets, while a
  changed mode, octave range, or pedal enable refreshes on the next block.
- No unnecessary classical arp pool work for Arp Off or SID Seq, which has its
  own memory. The last release pools are preserved while Release is sounding.
- No changes to SID oscillator tables, filter, output, ADSR, ring/sync,
  musical arp transport, or accessibility editor. No parameter IDs changed.
- Added JUCE-independent regression for dirty cache, mode/octave/pedal
  changes, and idle Free-clock continuation.

Validation limitations: neither the complete Windows VST3 build nor REAPER
performance can be run here. The target <0.01c, ideally ~0.003c, is not a
promise: measure under identical REAPER conditions and report observations.
