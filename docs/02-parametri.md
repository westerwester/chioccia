# 2. Parametri

Gli **ID** sono parte del formato delle sessioni e dell'automazione: non vanno mai rinominati
(il test `parameter_ids` fallisce se cambiano). Tutti usano `ParameterID { id, 1 }`.
I nuovi parametri futuri useranno un nuovo ID e version hint 2.

Lo smoothing è eseguito nell'engine, per campione, con rampe lineari (in dB per i guadagni).

## Pannello principale

| ID | Nome | Unità | Intervallo | Default | Curva / mappatura | Smoothing | Automazione |
|---|---|---|---|---|---|---|---|
| `peakReduction` | Peak Reduction | — | 0 … 100 (passo 0,1) | 30 | guadagno del side-chain = −10 + 0,6·PR dB; sotto PR 2 il side-chain sfuma a zero (PR 0 = nessuna compressione) | 30 ms (dB) | sì |
| `gain` | Gain | — | 0 … 100 | 50 | make-up = (G − 50)·0,6 dB → −30 … +30 dB; 50 = 0 dB | 20 ms (dB) | sì |
| `compLimit` | Compress/Limit | scelta | Compress, Limit | Compress | esponente del pannello EL 3,5 → 12 | 50 ms (morph dell'esponente) | sì |
| `mix` | Mix | % | 0 … 100 | 100 | lineare | 30 ms | sì |
| `outputTrim` | Output Trim | dB | −24 … +24 | 0 | lineare in dB, dopo la somma | 20 ms | sì |
| `vuMode` | VU Meter | scelta | Gain Reduction, Output | Gain Reduction | solo visualizzazione (0 VU = −18 dBFS) | — | no |
| `bypass` | Bypass | bool | off/on | off | collegato al bypass dell'host (`getBypassParameter`) | crossfade 20 ms | sì |

## Pannello avanzato

| ID | Nome | Unità | Intervallo | Default | Curva / mappatura | Smoothing | Automazione |
|---|---|---|---|---|---|---|---|
| `inputTrim` | Input Trim | dB | −24 … +24 | 0 | prima dello split dry/wet (agisce su entrambi) | 20 ms | sì |
| `wetTrim` | Wet Trim | dB | −24 … +24 | 0 | sul wet, prima della somma | 20 ms | sì |
| `stereoLink` | Stereo Link | % | 0 … 100 | 100 | nel dominio del guadagno: g = g_i + link·(min(gL,gR) − g_i); a 100 % guadagno identico sui due canali | 50 ms | sì |
| `scHpfOn` | Detector HPF | bool | off/on | off | quando off resta un DC-block a 10 Hz nel solo detector | 50 ms (frequenza in log2) | sì |
| `scHpfFreq` | Detector HPF Freq | Hz | 20 … 500 | 90 | manopola logaritmica (centro 100 Hz); Butterworth 12 dB/oct sul solo side-chain | 50 ms (log2) | sì |
| `colour` | Colour | % | 0 … 100 | 50 | quantità = min(1, C/20); drive nelle curve = (C − 50)·0,24 dB → −12 … +12 dB; 0 % = identità esatta | 50 ms | sì |
| `quality` | Quality | scelta | Eco (1x), High (2x), Ultra (4x) | High (2x) | oversampling del solo stadio di colorazione; latenza invariata | crossfade 20 ms con priming | sì |
| `compare` | Level-Matched Compare | bool | off/on | off | guadagno lento (τ 1,5 s potenze, 0,3 s guadagno) che allinea l'uscita all'RMS d'ingresso, ±24 dB | 50 ms | sì |
| `noise` | Noise & Hum | % | 0 … 100 | **0** | al 100 %: rumore bianco −80 dBFS RMS + ronzio −88 dBFS RMS (fond.) + 3ª armonica −6 dB; solo sul wet | 50 ms | sì |
| `humFreq` | Hum Frequency | scelta | 50 Hz, 60 Hz | 50 Hz | — | — | sì |
| `mixLaw` | Mix Law | scelta | Linear Crossfade, Dry Constant | Linear | lineare: (1−m)·dry + m·wet; dry costante: dry + m·wet | 30 ms | sì |
| `listen` | Listen | scelta | Mix, Dry Only, Wet Only | Mix | ascolto di verifica | crossfade 20 ms | sì |
| `diagnostic` | Diagnostic | scelta | Off, Null Residual, Wet Identity | Off | Null = dry − wet identitario; vedi [04](04-parallelo-latenza.md) | dissolvenza 5 ms | **no** |

## Stato salvato

`getStateInformation` salva l'albero APVTS (XML binario) con `stateVersion`, `pluginVersion`,
programma corrente, apertura del pannello avanzato e larghezza della finestra.
`setStateInformation`:

- ignora dati corrotti o di altro plug-in senza modificare nulla;
- assegna il **valore di default** ai parametri mancanti (sessioni salvate da versioni più vecchie);
- forza il valore normalizzato canonico di ogni parametro (corregge un caso limite di APVTS con i
  booleani, emerso con pluginval).
