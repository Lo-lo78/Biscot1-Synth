# Biscot1 Synth 0.99.6 — Guida alle novità accessibili

**16 novembre 2026 · VST3 per Windows x64**

## Novità 0.99.6 — Hold AHDSR nei sette inviluppi

Tutti gli ADSR hanno il nuovo stadio **Hold** fra Attack e Decay, indipendente:

- **Op1, Op2, Op3 Hold:** inviluppi d'ampiezza dei tre operatori.
- **Op1, Op2, Op3 Mod Env Hold:** inviluppi di modulazione dei tre operatori.
- **Filter Hold:** inviluppo condiviso del filtro.

Ogni Hold va da **0 a 5000 ms**, con passo di **1 ms**, e parte da **0 ms**, preservando il suono dei preset precedenti. Mantiene il livello massimo raggiunto durante Attack e poi avvia Decay. Note Off avvia Release anche mentre Hold è attivo.

Nelle pagine accessibili dei parametri Hold segue Attack. Nel tracker Alt+Q, F1 Pattern, sono disponibili i parametri **OP Hold**, **OP Mod Env Hold** e **Filter Hold**, con la mappatura delle Song già esistente. Gli ID VST3 precedenti restano invariati; i nuovi sono 249–255.

## Novità 0.99.6 — Automazione accessibile REAPER

Scegli un parametro nelle pagine principali e premi **Ctrl+Shift+A** da Grid, Value o Edit: il comando attiva/disattiva la envelope **ACTIVE** di REAPER senza cancellare i punti precedentemente registrati. NVDA annuncia **`, Auto`** quando l'automazione è attiva. Funziona anche per tutti e sette gli Hold. Non modifica il tracker Alt+Q.

## Navigazione principale

- **Alt+A:** apre l'About accessibile. Tab e Shift+Tab scorrono testo, collegamento al progetto, contatto e pulsante Close; Escape chiude.
- **Alt+Q:** apre il tracker integrato. **F1** accede ai Pattern e **F2** alle Song.
- **1/2/3:** cambiano operatore/sezione soltanto nella zona Steps; nelle altre zone non alterano l'operatore selezionato.
- **Spazio in F2:** avvia o ferma la Song. Lo Stop ripristina la posizione da cui era partita la riproduzione manuale anche dopo passaggi fra Song.
- **Ctrl+Freccia su su Song Length:** al limite superiore rimane silenzioso.

## Parametri LFO introdotti nella 0.99.5

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

Dopo il workflow Windows riuscito, utilizza l'artefatto `Biscot1-0.99.6-Windows-x64.zip` per la release GitHub con tag `v0.99.6`. Per i siti che richiedono `*.vst3.zip`, allega una copia identica come `Biscot1-0.99.6-Windows-x64.vst3.zip`. Conserva una copia dei preset e dei progetti prima di aggiornare.
