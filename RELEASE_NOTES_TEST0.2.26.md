# Biscot1 Synth VST3 — TEST0.2.26

## F2 Song End / concatenazione

- La transizione Song End ora usa l'ultima destinazione pubblicata dall'editor, anche se l'utente modifica Song End durante la riproduzione. L'audio non legge mai il banco Song mutabile.
- La mappa delle Song viene aggiornata al confine musicale e la Song successiva parte dalla riga 0; Note Off delle tre corsie, code Delay e stato del sintetizzatore vengono gestiti senza ricreare il motore.
- Se in F2 si assegna Song End a una Song che non è stata ancora visitata, viene creato il relativo slot vuoto. Per sentirla, occorre inserirvi un Pattern.
- La pagina F2 segue automaticamente la Song di destinazione e annuncia «Song N started» quando rileva il passaggio. Questo risolve anche il caso in cui il motore avesse cambiato Song senza che NVDA leggesse il nuovo numero.
- **Attenzione:** la Song termina quando **tutte e tre** le corsie hanno completato la loro **Length** e i rispettivi Repeat, compresi gli step Song vuoti. Un Pattern nello step 0 non significa che la Song finisca subito dopo il Pattern se Length=16 e gli altri 15 step restano vuoti. Per provare Song End rapidamente, portare Length a 1 in *tutte e tre* le corsie delle due Song di prova.

## Anteprima immediata dei Pattern in F2

- Su una riga Song che contiene un Pattern, le frecce e le modifiche ai valori avviano una brevissima anteprima, come nel vecchio Tracker Lua 0.166.
- Viene usato esclusivamente l'operatore selezionato, con gli step del Pattern compresi nella finestra **Pattern Start + Play Length**, senza ripetere l'intera Song.
- Il valore predefinito è **3 step del Pattern**. Aggiungere nell'elenco aperto con Invio il parametro **Preview Steps** (1–1024) per una durata diversa **in quella specifica riga Song**. La durata effettiva non supera Play Length.
- Anteprima e riproduzione Song sono separate: la navigazione **non interrompe** una Song in riproduzione. La preview non segue Song End, non ripete la Song e non richiede nuovi thread DSP.
- Il dato Preview Steps è facoltativo e viene scritto nel formato Song esistente come parametro numero 23, senza cambiare il numero identificativo di alcuno dei 17 parametri storici. Tutti i `.biscot` precedenti restano leggibili e usano 3 step quando il parametro non è presente.

## Accessibilità, F1/F2 e memoria della pagina

- I normali annunci di F2 non ripetono «Operator 1/2/3» ad ogni parametro. Restano il valore e, quando serve, il numero dello step.
- Il tasto Tab continua a leggere **Song, nome, operatore, step e parametro**.
- Se si sceglie esplicitamente un'altra corsia con 1/2/3, il nome dell'operatore viene annunciato una sola volta.
- Quando si apre F1 o F2 viene letto «Pattern N» oppure «Song N»; ogni editor conserva la propria posizione durante i passaggi fra le pagine.
- Riaprendo Alt+Q, viene ripristinata l'ultima pagina F1/F2 usata, anche dopo la ricreazione della finestra VST, tramite `trackerLastMode` nello stato del preset.

## Verifiche e limitazioni

- Test C++ indipendente `TrackerSongEndPreviewTest`: catena reale Song 0→1 con eventi musicali, durata della preview, un solo operatore, esclusione dei Repeat e assenza di anteprima nelle righe Song vuote.
- Regressioni SongBank, SongEditing, SongPlayback, SongAdvanced e regressioni già collaudate su F1, Curve, Sustain, arpeggiatori, filtri e LFO.
- Nessun cambiamento agli algoritmi degli oscillatori SID, ai filtri, agli ADSR o ai normali arpeggiatori. Le nuove anteprime utilizzano il trasporto Song, su snapshot immutabili compilati fuori dal thread audio.
- L'effettiva compilazione Windows VST3 e gli annunci NVDA devono essere confermati tramite GitHub Actions e REAPER.
- Rimangono previsti per la fase 4: Ctrl+S Song, file `.biscot1-song`, cartelle dei Pattern associati e browser di importazione/esportazione.
