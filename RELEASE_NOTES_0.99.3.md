# Biscot1 Synth 0.99.3 — Context-sensitive operators and live Song position

- In Alt+Q F1 Pattern and F2 Song, unmodified 1/2/3 switch sections/operators only inside Steps. In Length, Repeat, Speed, Transpose and other shared regions these keys do not change the selected operator.
- F2 Song Space now captures the currently playing Song after Song End chaining, even if the GUI timer has not yet refreshed, and remembers the last audible row before stopping. The next manual Space can start from the reached Song/row rather than the original Song/row.
- Does not change DAW Song Select or Song Transport automation, DSP or Song End chaining.
