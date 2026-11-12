# Biscot1 Synth VST3 — TEST0.2.25 — Song engine completion pass

This test release continues the first audible Song F2 milestone (TEST0.2.24). It implements the remaining core Song scheduler features, with a new native DSP interpretation for per-operator Sidechain and Delay. The MIDI, SID wave generator, ADSR, filter kernels and existing F1 Pattern transport remain unchanged except where the Song explicitly drives the shared synth resources.

## Song End chaining

- Global F2 `Song End` (Off or a Song index 0..99) now transitions at the end of the three independent lane timelines. If the destination Song does not exist in the bank, transport stops normally.
- A Song may refer to itself to repeat. The next Song restarts its own three independent lanes from Song row 0.
- Existing note release tails, LFO phase, delay history and synthesis state are retained across links. No editable Song or Pattern map is consulted on the audio thread.
- All linked Songs are compiled on the UI/message thread as immutable snapshots and pinned for the performance. Reused Pattern IDs share compiled data inside each full Song compilation.

## Pattern state and Filter/LFO ownership

- When a Song entry sets `Pattern Start` above 0, the motor primes Wave, envelopes, Filter and LFO automation from earlier Pattern steps before the first audible note, without issuing Note On events for skipped steps.
- **Filter Operators controls routing**: bit 1=Op1, bit 2=Op2, bit 4=Op3. Value 7 routes all three operators through the same shared filter; value 1 routes only Op1. The value persists until a subsequent Song event changes it.
- **Filter Owner controls automation only**: the sole active Pattern using shared Filter parameters is the automatic owner. The Song owner preference resolves conflicts when multiple Patterns use the resource simultaneously. Identical arbitration is applied independently to each of the three shared LFO generators.
- LFO waveform/rate changes are controlled by their owner; LFO pitch, PWM, level and pan depths remain individually mapped to each operator, including when another operator owns the source oscillator.
- F2 row effects work even when no Pattern is inserted in that Song row. Events are applied at the start of that row, not incorrectly repeated at every Pattern step.

## Sidechain, Delay and mixer

- Native per-operator Sidechain Source 0..3 and Amount 0..100. Source Off or Amount 0 reset the sidechain state. The processor uses a smoothed ducking envelope based on the source operator's VCA output.
- Native stereo per-operator Delay Speed 0..8 (0=off; 1 is one quarter-note; 2 is an eighth), Feedback 0..100 (internally capped for stability), Volume 0..100 and Ping-Pong Depth 0..100. The delay buffer is prepared outside the audio callback, supports up to eight seconds of history and preserves echo tails across Song transitions and natural release.
- Mute, Solo and Volume modifications made in F2 while the current Song is playing are picked up at the next audio block. No Song bank read occurs inside the audio callback.
- Idle mode is preserved when no sound or delay tail remains. The delay buffer is logically cleared on return to idle to avoid bulk memory writes on the real-time thread.

## Compatibility / remaining checks

- Existing `.biscot` project and preset Song serialization is unchanged; Alt+S remains fully self-contained.
- Ctrl+S Song export and portable Pattern/Song folder packaging remain **phase 4**.
- These native Sidechain/Delay algorithms are new DSP implementations and should not be described as bit-for-bit identical to the old JSFX Core without direct REAPER listening tests. Timing, feedback, ping-pong, filter routing and owner-priority must be auditioned in REAPER.
- JUCE VST3 build under Windows and NVDA/OSARA integration require GitHub Actions and a user test; JUCE-independent Release and static regression tests do not replace this step.
