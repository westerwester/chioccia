# 4. Compressione parallela, latenza, oversampling e bypass

## 4.1 Percorsi

- **Dry**: `ingresso × Input Trim` → ritardo intero di **L** campioni. Nessuna saturazione, nessun
  filtro, nessuna normalizzazione. Con Mix = 0 % l'uscita è **identica bit a bit** all'ingresso
  ritardato, anche con PR 100, Colour 100 %, Limit e rumore attivo (test `dry_clean`).
- **Wet**: cella ottica → Gain → colorazione sovracampionata → riempimento di ritardo (pad) →
  rumore/ronzio opzionali → Wet Trim.
- **Somma** → Listen → Output Trim → Compare (opzionale) → crossfade di bypass.

## 4.2 Latenza

| Qualità | Latenza propria del percorso wet | Pad aggiunto | Latenza dichiarata all'host |
|---|---|---|---|
| Eco (1x) | 0 | 72 | **72 campioni** |
| High (2x) | 65 | 7 | **72 campioni** |
| Ultra (4x) | 72 | 0 | **72 campioni** |

72 campioni = 1,633 ms a 44,1 kHz, 1,500 ms a 48 kHz, 0,375 ms a 192 kHz (i filtri sono definiti
in campioni, quindi la latenza in ms diminuisce al crescere del sample rate).

Perché costante: se la latenza cambiasse con la qualità, molti host riallineerebbero la PDC solo a
trasporto fermo o non lo farebbero affatto. Con una latenza fissa il dry interno e l'eventuale bus
parallelo esterno restano sempre allineati. Il costo è 1,5 ms di latenza anche in modalità Eco.

`setLatencySamples()` viene chiamato in `prepareToPlay` (e nel costruttore); il test
`latency_processing` verifica che il valore dichiarato coincida con quello dell'engine a 44,1 –
192 kHz, mono e stereo.

## 4.3 Oversampling

- **Stadi coinvolti**: solo la colorazione (tubo + trasformatore). Detector e cella lavorano alla
  frequenza dell'host: la loro non linearità agisce sul controllo, che viene filtrato, non sul
  segnale audio.
- **Filtri**: FIR half-band a fase lineare, finestra di Kaiser, attenuazione progettata 100 dB.
  Stadio 1 (1x↔2x): 131 coefficienti, banda passante fino a ≈ 0,45 fs (19,8 kHz a 44,1 kHz).
  Stadio 2 (2x↔4x): 27 coefficienti. Implementazione polifase (i coefficienti nulli non vengono
  calcolati). Per rendere intera la latenza del 4x si aggiunge 1 campione di ritardo a 2x.
- **Misure** (test automatici, 48 kHz): errore rispetto all'ingresso ritardato a 19,2 kHz −110,6 dB
  (2x) e −96,8 dB (4x); aliasing di una sinusoide da 15 kHz a −3 dBFS con Colour 100 %: −16,6 dB
  (1x), −41,7 dB (2x), −85,4 dB (4x).
- **Limite**: sopra ≈ 0,45 fs i filtri attenuano (−6 dB a fs/2). A 44,1 kHz il contenuto tra
  ~20 e 22 kHz del wet viene ridotto: è una limitazione di banda, non uno sfasamento.

### Cambio di qualità durante la riproduzione

Tutte e tre le catene sono allocate in `prepareToPlay`. Al cambio:

1. la nuova catena viene azzerata e ne vengono copiati gli stati dei filtri lenti (DC e banda bassa);
2. viene "innescata" rielaborando gli ultimi 256 campioni d'ingresso del wet (storico circolare,
   nessuna allocazione), così filtri e pad contengono la stessa storia della catena uscente;
3. crossfade lineare di 20 ms tra le due catene (coerenti in tempo, quindi niente buchi di livello).

Risultato misurato (test `quality_switch`, tre cambi in 4 s): differenza rispetto a un render a
qualità costante −108 dB con Colour 0 % e −75 dB con Colour 60 % (la differenza residua è
l'aliasing diverso tra le modalità). Latenza invariata. Un nuovo cambio richiesto durante un
crossfade viene eseguito al termine di quello in corso.

## 4.4 Cosa si compensa e cosa no

| Effetto | Compensabile? | Come viene gestito |
|---|---|---|
| Ritardo dei filtri di oversampling | **Sì** (ritardo intero) | dry ritardato di L; null test |
| Risposta in ampiezza dei filtri sopra 0,45 fs | No (è banda limitata) | dichiarato; residuo misurabile col null test sopra ~19 kHz |
| Sfasamenti lineari nel wet | Non presenti per progetto | nessun EQ lineare nel wet; il passa-alto a 8 Hz agisce solo sul residuo non lineare |
| Variazione di guadagno (compressione) | No, è l'effetto voluto | — |
| Armoniche della colorazione | No, è l'effetto voluto | Colour 0 % = identità |
| Rumore/ronzio | No | disattivato di default |

**Non si promette cancellazione perfetta** tra segnale processato e originale: il null test verifica
solo l'**allineamento temporale**, rendendo identitario il percorso wet.

## 4.5 Null test (diagnostica)

`Diagnostic = Null Residual`: il wet diventa identitario (cella a guadagno 1, Gain 0 dB, niente
colorazione, rumore, Wet Trim) ma **mantiene l'oversampling e il pad**; l'uscita è `dry − wet`.

| Sample rate | 1x | 2x | 4x |
|---|---|---|---|
| 44,1 kHz | silenzio esatto | −131,0 dBFS | −123,9 dBFS |
| 48 kHz | silenzio esatto | −133,7 dBFS | −121,7 dBFS |
| 88,2 – 192 kHz | silenzio esatto | < −80 dBFS (soglia del test) | < −80 dBFS |

(segnale: 6 sinusoidi 60 Hz – 15 kHz a −6 dBFS). `Wet Identity` permette di ascoltare/misurare il
solo wet identitario. La diagnostica non è automatizzabile e il passaggio avviene con una
dissolvenza di 5 ms.

## 4.6 Legge di miscelazione

- **Crossfade lineare (default)**: `out = (1 − m)·dry + m·wet`. Dry e wet sono la stessa sorgente,
  allineata nel tempo e in fase: si sommano in ampiezza. Un crossfade a potenza costante
  (sen/cos) darebbe +3 dB a metà corsa con segnali correlati: per questo non è usato.
- **Dry constant (alternativa)**: `out = dry + m·wet`. Il dry resta sempre a guadagno unitario e il
  compresso si "aggiunge", come su un bus parallelo del mixer. Utile per la compressione
  parallela "alla New York": il Mix diventa il livello di mandata del compresso. A Mix 100 % la
  somma è più forte dell'ingresso (atteso).
- Nessuna normalizzazione nascosta in nessuna delle due modalità (test `mix_law`: errore < 1e-6
  rispetto alle formule).
- `Listen` permette di ascoltare Dry o Wet da soli (dopo Output Trim) per verificare il blend.

## 4.7 Uso su bus parallelo esterno

1. Crea un bus/aux, inserisci Chioccia con **Mix 100 %** (preset *Parallel Bus (100% Wet)*).
2. Manda la traccia al bus con una mandata **post-fader**; tieni la traccia originale invariata.
3. La DAW compensa i 72 campioni di Chioccia sul percorso del bus (PDC). Verifica: metti
   `Diagnostic = Wet Identity` sul plug-in del bus, inverti la polarità del bus e porta il fader
   allo stesso livello della traccia: la somma deve annullarsi (sotto ~19 kHz). Se non si annulla,
   la PDC della DAW su quel percorso non è attiva: usa il Mix interno.

## 4.8 Bypass

- Il parametro `bypass` è esposto come bypass dell'host (`getBypassParameter`): VST3 e AU lo
  collegano al pulsante di bypass della DAW.
- Con bypass attivo l'uscita è l'ingresso **ritardato di L** (stessa latenza dichiarata → nessuno
  spostamento temporale), con crossfade di 20 ms; a regime è identica bit a bit all'ingresso
  ritardato (test `bypass` e `host_bypass`).
- Se l'host usa il proprio bypass senza il parametro, `processBlockBypassed` esegue l'engine con il
  bypass forzato: stessa latenza, nessun click, stati "caldi" alla riattivazione.
- In bypass il plug-in continua a elaborare (costo CPU invariato) per evitare transitori al ritorno.

## 4.9 Compare a volume compensato

Opzione **esplicita**, disattivata di default, separata dal Gain: misura lentamente l'RMS
d'ingresso e di uscita (τ 1,5 s) e applica dopo l'Output Trim un guadagno (±24 dB, τ 0,3 s) che
porta l'uscita allo stesso livello dell'ingresso. Serve solo per giudicare il suono senza
l'effetto "più forte = migliore"; non va lasciata attiva in stampa. Il valore applicato è mostrato
nel pannello avanzato. In silenzio (< −70 dBFS) il guadagno viene mantenuto.
