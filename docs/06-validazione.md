# 6. Piano di validazione

Regola: si riportano solo risultati effettivamente ottenuti. Le sezioni "Da eseguire" contengono
procedure e moduli da compilare, non risultati.

## 6.1 Test automatici (eseguiti)

`ctest` esegue `core_tests` (102 controlli, nessuna dipendenza) e `plugin_tests` (140 controlli,
JUCE). Output completi: [misure/core_tests_output.txt](misure/core_tests_output.txt),
[misure/plugin_tests_output.txt](misure/plugin_tests_output.txt).

| Test | Cosa verifica |
|---|---|
| `halfband_design` | simmetria (fase lineare), zeri half-band esatti, guadagno DC 1 |
| `oversampler` | picco dell'impulso esattamente alla latenza dichiarata; errore in banda < −80 dB fino a 19,2 kHz @ 48 kHz |
| `null_and_latency` | null test 44,1 / 48 / 88,2 / 96 / 176,4 / 192 kHz × 1x/2x/4x; latenza costante |
| `impulse_alignment` | impulso a Mix 0/50/100 % sullo stesso campione |
| `dry_clean` | Mix 0 % = ingresso ritardato bit a bit con tutto al massimo |
| `mix_law` | crossfade lineare e dry costante conformi alle formule (< 1e-6) |
| `block_size_invariance` | uscita identica bit a bit con blocchi 512, 1, casuali 1–2048 |
| `robustness` | NaN/Inf/±1e30/denormali → uscita finita; ritorno al comportamento normale; silenzio senza denormali |
| `stereo_link` | link 100 % mantiene il rapporto L/R; canali indipendenti con link 0 % |
| `quality_switch` | cambi 2x→4x→1x→2x senza click e senza cambio di latenza |
| `bypass`, `host_bypass` | bypass = ingresso ritardato di L, bit a bit |
| `pr_zero`, `smoothing`, `compare` | nessuna GR a PR 0; nessun salto con cambi di Gain/Trim; compare allinea il livello (±1 dB) |
| `gr_sanity` | GR indipendente dal sample rate (8,75 dB ± 0,01 a 44,1 – 192 kHz) |
| `parameter_ids` | ID e version hint invariati, bypass collegato all'host |
| `bus_layouts` | mono e stereo accettati; mono→stereo e 5.1 rifiutati |
| `latency_processing` | latenza dichiarata = engine; blocchi 0, 1, 13, 511, 512, 2048, 4096 (oltre il massimo annunciato) |
| `state` | salvataggio/ripristino di tutti i parametri, default per parametri mancanti, dati corrotti ignorati |
| `presets` | applicazione di ogni preset + misura della GR |
| `editor` | creazione, ridimensionamento 700 – 1600 px, snapshot 1x e 2x (HiDPI) |

**Validatori esterni (eseguiti su Linux)**: pluginval 1.0.4 strictness 10 → *SUCCESS*
([log](misure/pluginval_linux_strictness10.txt)); validator Steinberg VST3 SDK 3.8 → *47 test
superati, 0 falliti* ([log](misure/steinberg_validator_linux.txt)).

## 6.2 Misure del modello (eseguite, solo su Chioccia)

`chioccia_measure` genera [misure/misure.md](misure/misure.md) e i CSV; i grafici con
`tools/plot_measurements.py`:

- sinusoidi a livelli/frequenze diverse → curve ingresso/uscita (PR 20–100, Compress/Limit);
- attacco con gradini da −40 dBFS a −18/−12/−6 dBFS;
- rilascio dopo burst da 0,1 / 0,5 / 2 / 10 / 30 s (effetto memoria);
- THD e H2/H3 a 50 Hz / 100 Hz / 1 kHz, Colour 0–100 %, da −30 a −1 dBFS;
- distorsione da modulazione del guadagno (ripple) a 50/100/1000 Hz;
- risposta in frequenza del wet a 1x/2x/4x; dipendenza in frequenza del detector con/senza HPF;
- aliasing a 1x/2x/4x.

## 6.3 Prove nelle DAW (da eseguire)

Nessuna DAW era disponibile in questo ambiente. Per ogni DAW compilare la tabella:

| DAW / versione / OS | Formato | Scansione | Mono | Stereo | Automazione | Riapertura sessione | Cambio SR | PDC insert | PDC bus | Bypass host | Note |
|---|---|---|---|---|---|---|---|---|---|---|---|
| | | | | | | | | | | | |

Procedura minima per ciascuna:

1. **Scansione**: il plug-in compare come *Chioccia* (Pollaio Audio), categoria Fx/Dynamics.
2. **Insert mono e stereo**: sorgente vocale e drum loop, VU in GR si muove, nessun click
   muovendo velocemente Gain, Peak Reduction, Mix, Output Trim.
3. **Allineamento**: traccia duplicata con polarità invertita; su una copia Chioccia con
   `Diagnostic = Wet Identity`: la somma deve annullarsi (sotto ~19 kHz). Ripetere con Mix 0 %.
4. **Bus parallelo esterno**: procedura in [04](04-parallelo-latenza.md#47-uso-su-bus-parallelo-esterno).
5. **Automazione**: scrivere automazione su PR, Gain, Mix, Compress/Limit, Quality; rileggerla e
   renderizzare due volte → i due render devono essere identici (null).
6. **Sessione**: impostare valori non di default, salvare, chiudere, riaprire → valori, pannello
   avanzato e dimensione finestra ripristinati.
7. **Preset**: selezione dal menu del plug-in e dal browser dei programmi della DAW.
8. **Sample rate**: 44,1 → 96 → 192 kHz con la sessione aperta; nessun disallineamento.
9. **Bypass dell'host**: attivare/disattivare in riproduzione → nessun click, nessuno spostamento
   temporale (null con la traccia invertita durante il bypass).
10. **Buffer**: 32, 64, 1024, 2048 campioni e, se la DAW lo permette, buffer variabili.
11. **GUI**: ridimensionamento, schermo HiDPI/Retina, apertura/chiusura ripetuta durante la riproduzione.
12. **CPU**: carico con 50 istanze in qualità 4x (riferimento misurato qui: ~2,5 % di un core per
    istanza stereo a 48 kHz in 4x, ~1,1 % in 1x, su una VM; valore indicativo).

## 6.4 Confronto con il plug-in di riferimento (da eseguire)

Solo se si possiede legalmente il plug-in di riferimento. Si confrontano esclusivamente le
**uscite audio** con gli stessi ingressi: niente disassemblaggio, niente accesso al codice, niente
aggiramento di protezioni o autorizzazioni.

**Preparazione**

- Stesso sample rate (48 kHz e 96 kHz), stessa DAW, rendering offline a 32 bit float.
- Allineare la latenza: rendere entrambi con PDC attiva e verificare con un impulso.
- Riferimento di livello comune: sinusoide 1 kHz a −18 dBFS RMS = 0 VU per entrambi (verificare
  dove il riferimento posiziona il proprio 0 VU e, se diverso, documentarlo).

**Segnali** (file WAV generati, stesso file per entrambi)

1. sinusoidi 50 Hz, 100 Hz, 1 kHz, 5 kHz a −30 … 0 dBFS RMS a passi di 3 dB (3 s ciascuna);
2. burst 1 kHz: −40 dBFS → L per 0,1 / 0,5 / 2 / 10 s, poi −80 dBFS per 20 s;
3. sweep logaritmico 20 Hz – 20 kHz a −30 dBFS (risposta in frequenza, senza compressione);
4. two-tone 19 + 20 kHz a −6 dBFS (intermodulazione/aliasing);
5. materiale musicale: voce, basso DI, batteria, mix completo (30 s ciascuno).

**Due serie separate di confronti**

- **A. Parità di posizione delle manopole**: stessi valori numerici di Peak Reduction e Gain,
  stesso modo Compress/Limit. Misura le differenze di GR, livello e timbro così come sono. Le
  scale delle manopole dei due prodotti **non sono calibrate tra loro**: le differenze qui
  descrivono la mappatura dei controlli, non solo il suono.
- **B. Volume e compressione equivalenti**: per ogni sorgente regola PR di Chioccia finché la GR
  media (o la curva statica a 0 VU) coincide entro 0,5 dB con quella del riferimento, poi allinea il
  livello d'uscita (LUFS integrato o RMS) entro 0,1 dB. Solo in questa serie hanno senso ascolti
  comparativi (preferibilmente ABX in cieco).

**Analisi**

- curve statiche e tempi t63/t90, 50 %, 10 %, < 0,5 dB con le stesse definizioni di
  `chioccia_measure` (lo strumento può analizzare file esterni con poche modifiche);
- THD e spettro armonico ai livelli indicati;
- differenza RMS dei render della serie B (null parziale) e spettrogramma della differenza;
- registrare **tutte** le differenze nella tabella sotto, anche quelle a sfavore.

| Misura | Riferimento | Chioccia | Differenza | Note |
|---|---|---|---|---|
| GR @ 0 VU, PR __ | | | | |
| Rapporto effettivo (Compress) | | | | |
| t63 attacco @ −12 dBFS | | | | |
| 50 % rilascio dopo 2 s | | | | |
| Rilascio < 0,5 dB dopo 10 s | | | | |
| THD 1 kHz @ −6 dBFS | | | | |
| Ripple a 50 Hz con 10 dB GR | | | | |

Solo dopo queste misure si potrà modificare la calibrazione (`OpticalCellConfig`,
`DetectorConfig`, `ColourConfig`, mappature in `EngineParameters.h`) per avvicinarsi al
riferimento, e rieseguire `chioccia_measure` per documentare il risultato.
