## Novità TEST0.2.46 — AHDSR Hold

Tutti i sette inviluppi hanno Hold dopo Attack: Op1/Op2/Op3, tre Mod Env e Filter. Intervallo 0–5 s, passo 0,001 s, default 0. Hold resta al picco prima di Decay; Note Off avvia Release anche da Hold. CTRL+MAIUSC+A attiva/disattiva l’automazione del parametro selezionato in REAPER. In F1 Pattern i tre controlli si chiamano OP Hold, OP Mod Env Hold e Filter Hold.

# Biscot1 Synth 0.99.5 — Guida alle novità accessibili

**15 novembre 2026 · VST3 per Windows x64**

## Navigazione principale

- **Alt+A:** apre l'About accessibile. Tab e Shift+Tab scorrono testo, collegamento al progetto, contatto e pulsante Close; Escape chiude.
- **Alt+Q:** apre il tracker integrato. **F1** accede ai Pattern e **F2** alle Song.
- **1/2/3:** cambiano operatore/sezione soltanto nella zona Steps; nelle altre zone non alterano l'operatore selezionato.
- **Spazio in F2:** avvia o ferma la Song. Lo Stop ripristina la posizione da cui era partita la riproduzione manuale anche dopo passaggi fra Song.
- **Ctrl+Freccia su su Song Length:** al limite superiore rimane silenzioso.

## Nuovi parametri LFO nella 0.99.5

Ciascuno degli LFO1, LFO2 e LFO3 aggiunge cinque controlli indipendenti:

1. **Drift Amount (0–1, default 0):** sposta progressivamente la fase a ogni ciclo.
2. **Drift Cycle (1–32):** numero di cicli dopo i quali si ripete la figura di fase.
3. **Lace Source (Off, LFO1, LFO2, LFO3; default Off):** sorgente della modulazione, compreso l'LFO stesso.
4. **Lace Rate (1–32):** suddivisione usata da Lace.
5. **Lace Mix (0–1, default 0):** quantità di modulazione Lace; a zero conserva il comportamento precedente.

I controlli sono disponibili nelle pagine accessibili degli LFO, nelle liste dei parametri LFO dei Pattern e delle Song e nell'automazione della DAW.

## Cambiamenti accumulati dalla 0.99.1

- **0.99.2:** indicatore **Set,** sui parametri inseriti; Ctrl+Invio inserisce senza uscire dalla lista in F2. Delay Ping-Pong con ripetizioni stereo anche a profondità intermedie e anteprima Song con gli effetti della riga corrente.
- **0.99.3:** i tasti 1/2/3 agiscono soltanto nella zona Steps; la riproduzione manuale segue correttamente la Song raggiunta attraverso Song End.
- **0.99.4:** limite superiore di Song Length silenzioso; Play/Stop ricorda e ripristina Song, operatore, riga e cella di partenza.
- **0.99.5:** LFO Phase Drift e Lace, disattivati per impostazione predefinita.

## Esperimento sul rumore SID

Il sorgente allegato contiene anche una variazione sperimentale della frequenza di avanzamento del rumore SID a 23 bit. Non è ancora documentata come modifica sonora validata all'ascolto: i dettagli della prova sono in `NOISE_SID_23BIT_CLOCK_TRIAL.md`.

## Pubblicazione

Dopo l'esito positivo del workflow Windows, utilizza l'artefatto `Biscot1-0.99.5-Windows-x64.zip` per la release stabile GitHub con tag `v0.99.5`. Per il collegamento diretto da siti che riconoscono `*.vst3.zip`, allega una **copia identica** rinominata `Biscot1-0.99.5-Windows-x64.vst3.zip`. Prima di sostituire il VST3 precedente, conserva una copia dei preset e dei progetti importanti.


### TEST0.2.45 — Automazione accessibile in REAPER

Nelle 12 pagine dei parametri principali, premi **Ctrl+Shift+A** su un parametro selezionato (Grid, Value o Edit) per attivare o disattivare la sua envelope in REAPER. NVDA annuncia **«, Auto»** quando l'automazione è attiva. I punti di automazione già registrati vengono conservati. Il comando non modifica il tracker Alt+Q. Questa è una versione di prova; la release pubblica rimane 0.99.5.
