# Biscot1 Synth 0.99.4 — Song Length silent boundary and Space transport origin

- F2 Song: Ctrl+Up on the Length header is silent when already at the upper boundary, matching F1 Pattern. No redundant ComboBox refresh or NVDA announcement.
- F2 Song: pressing Space to Play captures the active Song, operator, row, cell and per-operator remembered step. Space to Stop restores that exact starting position, even when Song End has chained through multiple Songs.
- While playing, F2 can still follow Song End transitions; Stop returns to the saved starting Song rather than leaving the editor on the last chained Song.
- No changes to Song Select or Song Transport automation controlled by the host.
