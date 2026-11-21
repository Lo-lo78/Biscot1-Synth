# Biscot1 Synth 0.99.7 — Guida alle novità e all'accessibilità

**21 novembre 2026 · VST3 Windows x64**

Questa versione integra le modifiche dalla **TEST0.2.47 alla TEST0.2.52**. Le sette funzioni Hold e il comando Ctrl+Shift+A della 0.99.6 rimangono disponibili.

## AHDSR Trigger: tre nuovi interruttori

Ogni operatore ha un proprio controllo **Op1 AHDSR Trigger**, **Op2 AHDSR Trigger** o **Op3 AHDSR Trigger**, con valori **Off / On** e valore iniziale **Off**.

- **Off:** normale inviluppo AHDSR di ampiezza, senza modifiche rispetto alla versione precedente.
- **On:** mentre una nota rimane premuta (anche tramite il pedale CC64), l'inviluppo di ampiezza ripete **Attack → Hold → Decay → Sustain istantaneo → Release → nuovo Attack**. Il ciclo continua finché il gate è attivo; rilasciando definitivamente la nota, l'inviluppo termina con il suo Release naturale.
- I tre Trigger funzionano indipendentemente e **non riavviano la fase degli oscillatori**, né l'inviluppo di modulazione o quello del filtro.

Nel tracker **Alt+Q, F1 Pattern**, il parametro si chiama **OP AHDSR Trigger**; la mappatura delle Song usa l'operatore della rispettiva corsia. Il parametro è automatizzabile in REAPER.

## Ping Pong Pan: alternanza stereo per ciascun operatore

Sono disponibili **Op1 Ping Pong Pan**, **Op2 Ping Pong Pan** e **Op3 Ping Pong Pan**, ognuno con intervallo **-1,00 ... +1,00**, passo **0,01** e valore iniziale **0**.

- **0:** nessun Ping Pong aggiuntivo; il Pan originale continua a funzionare.
- **Valore positivo**, ad esempio **+1:** il primo attacco parte verso destra, il successivo verso sinistra.
- **Valore negativo**, ad esempio **-1:** il primo attacco parte verso sinistra, il successivo verso destra.
- Un valore vicino a zero produce un'alternanza meno ampia. La posizione è sommata al Pan normale e alle sue modulazioni.

L'alternanza avviene sui **Note On**, sui nuovi attacchi generati da **AHDSR Trigger** e sugli step udibili degli arpeggiatori **SID Seq** e **classici**. Funziona anche con **2, 4, 6 o 8 note** premute; se un attacco AHDSR e uno step di arpeggio coincidono, c'è **un solo** cambio stereo.

### Ping Pong sincronizzato al pitch

- Con **Portamento e Arp Glide disattivati**, il cambio di lato avviene subito, sullo stesso campione del salto di pitch; non compare un breve frammento del nuovo Attack sul lato precedente.
- Con **Portamento o Arp Glide attivi**, il Pan percorre la transizione seguendo il **movimento reale del pitch**: se il glide è lento, anche il Pan lo è; se è rapido, lo segue senza ritardo autonomo.
- Vale per **SID Seq**, arpeggiatori classici e Portamento degli operatori, anche con glide modificato in tempo reale o uno step nuovo prima che finisca il precedente.
- Cambiando direttamente via automazione la **quantità** di Ping Pong Pan, rimane una breve interpolazione anticlick di circa **3 ms**. Non rallenta i passaggi di lato comandati dal pitch.

Nel tracker **Alt+Q, F1 Pattern**, cerca **OP Ping Pong Pan**. Il parametro funziona anche nelle Song e con l'automazione VST3 di REAPER.

## Automazione in tempo reale degli inviluppi

Dalla TEST0.2.47 i seguenti parametri reagiscono mentre la voce è già attiva:

- **Op1/2/3 Release**, **Op1/2/3 Mod Env Release** e **Filter Release:** se modifichi il Release mentre la coda suona dopo Note Off, la coda si accorcia o si prolunga **senza una nuova nota**. Un valore 0 durante Release la fa terminare.
- I **sette Hold** della 0.99.6: se modifichi Hold *durante* il suo stadio, la durata residua si aggiorna senza riavviare Attack o riavvolgere fasi già concluse.
- **Op1/2/3 Drift:** la quantità può variare su una nota in esecuzione, conservando il sorteggio casuale iniziale della nota. **Noise Character:** la variazione interviene senza attendere il successivo impulso del clock rumore.

L'automazione non genera retroattivamente note dopo che tutte le voci sono finite.

## Comandi da tastiera per NVDA e REAPER

- **Alt+A:** About accessibile. **Tab / Shift+Tab** navigano testo in sola lettura, link al progetto, contatto e Close. **Esc** chiude.
- **Alt+Q:** tracker nativo; **F1** Pattern, **F2** Song. Nell'area Steps, **1/2/3** cambiano la sezione/operatore; fuori dagli Steps non alterano l'operatore.
- **Ctrl+Shift+A:** nelle pagine dei parametri, da **Grid, Value o Edit**, attiva o disattiva lo stato **ACTIVE** dell'inviluppo REAPER per il parametro selezionato, **senza cancellarne i punti**. NVDA aggiunge **`, Auto`** al nome del parametro se attivo. Non intercetta il tracker Alt+Q.
- **Spazio in F2:** Play/Stop della Song; lo Stop ripristina la posizione iniziale anche dopo Song collegate.

## Compatibilità e aggiornamento

La versione **0.99.6** aveva introdotto sette Hold (ampiezza Op1/2/3, Mod Env Op1/2/3 e Filter) con intervallo **0–5 secondi**, passo **1 ms**, valore predefinito **0**, e ID VST3 **249–255**. La **0.99.7 aggiunge solo sei nuovi ID, 256–261**; nessun vecchio ID è stato rinumerato. Nei preset `.biscot` precedenti, i nuovi Trigger partono da **Off** e i Ping Pong Pan da **0**.

Sono conservati gli LFO Phase Drift/Lace della 0.99.5 e i Pattern/Song. La variazione sperimentale del clock del rumore SID a 23 bit resta una prova da ascoltare, non un miglioramento sonoro già certificato (`NOISE_SID_23BIT_CLOCK_TRIAL.md`).

**Per pubblicare:** verifica la build Windows, poi crea la release GitHub **`v0.99.7`** con l'archivio **`Biscot1-0.99.7-Windows-x64.zip`**. Per il download diretto dal sito aggiungi, con contenuto identico, **`Biscot1-0.99.7-Windows-x64.vst3.zip`**. Prima dell'aggiornamento conserva una copia di preset e progetti REAPER.
