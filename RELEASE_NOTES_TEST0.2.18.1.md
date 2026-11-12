# Biscot1 Synth VST3 — TEST0.2.18.1 (Windows compile fix)

## Cause and correction

- GitHub Actions failed in `Source/PluginProcessor.cpp` line 763, MSVC error `C2131`, because `juce::Identifier::Identifier` is not a constant-expression constructor.
- Changed exactly one identifier declaration from `constexpr juce::Identifier trackerStateKey{"trackerPatternsV1"};` to `const juce::Identifier trackerStateKey{"trackerPatternsV1"};`.
- The property key string is unchanged. Pattern bank serialization, saved projects, and `.biscot` files remain compatible.
- Updated VST3 version, visible editor version and workflow artifact to `TEST0.2.18.1`.
- Added a regression assertion to prevent reintroducing `constexpr` for `juce::Identifier`.

## Scope and verification

No MIDI/audio DSP, oscillators, SID filters, LFOs, CPU idle code or original tracker F1/F2 behavior has been modified. Run GitHub Actions to verify the full Windows VST3 compile; the Linux standalone tests do not compile JUCE.

## Git (Windows CMD)

    git add . && git commit -m "Biscot1 VST3 TEST0.2.18.1 fix MSVC C2131 tracker Identifier" && git push
