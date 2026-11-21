# 3. Modello DSP

Il modello è **comportamentale**: riproduce i comportamenti documentati pubblicamente dei
livellatori ottici classici, non è una simulazione circuitale di un'unità specifica e **non è stato
confrontato con un riferimento reale** (hardware o plug-in). Le costanti sono state tarate solo
contro questi obiettivi pubblici:

| Comportamento documentato | Fonte | Obiettivo usato | Misurato su Chioccia (48 kHz) |
|---|---|---|---|
| Attacco medio ≈ 10 ms, dipendente da frequenza/programma | [Wikipedia](https://en.wikipedia.org/wiki/LA-2A_Leveling_Amplifier), [UA WebZine 2003](https://www.uaudio.com/webzine/2003/june/text/content4.html) | ≈ 10 ms a GR moderata | t63 7,5 – 14,8 ms (vedi [misure](misure/misure.md)) |
| Rilascio ≈ 60 ms al 50 %, poi 1 – 15 s per il rilascio completo, più lento dopo compressione lunga/forte | stesse fonti | due stadi + memoria | 50 % in 63 – 156 ms; < 0,5 dB in 1,7 s (burst 0,1 s) … 14 s (burst 30 s) |
| Compress: ginocchio morbido, nominalmente ≈ 4:1, dipendente dal programma | UA WebZine 2003 | ≈ 3,5 – 4:1 a GR elevata | 3,1 – 3,9:1 sugli ultimi 6 dB |
| Limit: rapporto molto alto (nominalmente ∞:1) | UA WebZine 2003 | ≥ 10:1 a GR elevata | 8,9 – 11,3:1 |
| Gain e Peak Reduction indipendenti | descrizione dei controlli | detector prima del Gain | verificato per costruzione |

## 3.1 Detector e pannello EL (`Detector`)

Ingresso: segnale **dopo la cella e prima del Gain** del campione precedente (retroazione).

1. `HPF`: Butterworth 2° ordine, 10 Hz fissi (solo DC) o 20 – 500 Hz se l'utente attiva il filtro.
2. Enfasi HF fissa: high-shelf +2 dB a 2,5 kHz (approssimazione della maggiore sensibilità alle
   alte frequenze tipica di questi detector; misurata: +0,8 dB di GR a 3 – 8 kHz rispetto a 1 kHz).
3. Drive: `v = |s|·D − v0` con `D = 10^((−10 + 0,6·PR)/20)` e soglia `v0 = 0,02`.
4. Luce istantanea: `L = v^p` (0 se v ≤ 0), saturata a 10⁴. `p = 3,5` (Compress) o `12` (Limit).
5. Persistenza del pannello: salita τ = 0,2 ms, discesa τ = 2 ms.

## 3.2 Cella ottica (`OpticalCell`)

Tre stati in "unità di luce":

- **c** – portatori veloci (determina la conduttanza della fotoresistenza);
- **m** – memoria (trappole), si carica durante la compressione;
- **d** – trappole profonde (storia di lungo periodo).

**Attacco** (quando `L > c`), nel dominio logaritmico:

```
step = ln(L + ε) − ln(c + ε)                      ε = 1e-4
t    = step / (step + 3)                           0…1: gradini grandi → attacco più rapido
τa   = (14 ms + (6 ms − 14 ms)·t) · (1 + 0,12·p·γ) normalizzazione per il guadagno d'anello
ln(c + ε) += step · (1 − e^(−dt/τa))
```

**Rilascio** (quando `L ≤ c`): `c` scende con τ = 40 ms verso `max(L, 0,22·m)` → primo stadio rapido
fino a un "pavimento" fissato dalla memoria.

**Memoria**: se `c > m`, `m` si carica con τ = 1,5 s (max 50 ≈ 30 dB di GR); altrimenti scarica con
`τm = 0,45 s + 2,5 s · clamp(d/m, 0, 1)`: dopo una compressione lunga (`d` carico) il rilascio è
più lento.

**Trappole profonde**: `d` insegue `m` con τ = 6 s in carica e 20 s in scarica.

**Attenuazione** (partitore con fotoresistenza in parallelo):

```
g = 1 / (1 + K · c^γ)          K = 1, γ = 0,85
```

In regime stazionario `c = L` e la curva statica dipende solo dal detector; il rapporto asintotico
della retroazione è `1 + p·γ` (≈ 4:1 in Compress, ≈ 11:1 in Limit) con ginocchio morbido naturale.

**Protezioni**: luce saturata a 10⁴ (≈ 70 dB di GR massima), memoria limitata, reset automatico se
uno stato diventa non finito, azzeramento dei valori < 1e-30.

## 3.3 Link stereo

Ogni canale ha detector e cella propri. Il link agisce sul guadagno:
`g_i' = g_i + link · (min(gL, gR) − g_i)`. Con link = 100 % entrambi i canali ricevono
**esattamente** lo stesso guadagno (verificato: errore del rapporto L/R < 1e-5), quindi l'immagine
stereo non si sposta. Il detector di ciascun canale vede comunque il proprio segnale post-cella.

## 3.4 Colorazione (`ColourStage`, sovracampionata)

Si somma al segnale solo il **residuo non lineare** `r = f(u) − u`, filtrato passa-alto a 8 Hz (per
togliere la componente continua generata dall'asimmetria). Con Colour = 0 % lo stadio è
un'identità esatta e la parte lineare del wet resta in fase con il dry.

- **Tubo**: `f(x) = (tanh(x + b) − tanh b) / (1 − tanh² b)` con `x = u·drive/1,6`, `b = 0,038`
  (pendenza unitaria nell'origine; H2 + H3).
- **Trasformatore**: saturazione `tanh` simmetrica della banda bassa estratta con un passa-basso a
  90 Hz (soprattutto H3, crescente al diminuire della frequenza), ginocchio 1,2, quantità 0,5.
- **Colour**: 0 – 20 % dissolve il residuo da 0 a 1; il drive nelle curve va da −12 dB (0 %) a
  +12 dB (100 %), 0 dB a 50 %.
- **Gain** precede la colorazione: alzarlo spinge di più le curve (come in un amplificatore reale);
  non c'è nessun limiter nascosto e l'uscita in virgola mobile può superare 0 dBFS.

THD misurata a 1 kHz, PR 0, Colour 50 %: 0,04 % a −30 dBFS, 0,16 % a −18 dBFS, 0,98 % a −6 dBFS,
2,6 % a −1 dBFS (picco). Tabelle complete in [misure/misure.md](misure/misure.md).

## 3.5 Gestione dei livelli

- Riferimento: **0 VU = −18 dBFS RMS** (sinusoide). Il VU in modalità Output usa questo riferimento.
- Input Trim agisce su dry e wet; Wet Trim solo sul wet prima della somma; Output Trim dopo.
- Ingresso protetto: NaN/Inf → 0, limite di sicurezza a ±1000 (+60 dBFS). Non è un limitatore
  musicale: nessun segnale reale lo raggiunge; serve a mantenere gli stati finiti e recuperabili
  (test `robustness`: dopo 0,1 s di NaN/Inf/±1e30 l'uscita torna identica a un'istanza nuova).

## 3.6 Approssimazioni dichiarate

1. Il modello della cella è fenomenologico (tre costanti di tempo), non deriva dalla fisica del CdS
   né da misure di una cella T4 reale.
2. La topologia a retroazione e il meccanismo del modo Limit (esponente più ripido) sono scelte di
   progetto plausibili, **non verificate su uno schema elettrico**.
3. L'enfasi HF del detector è fissa e scelta a orecchio/progetto.
4. Nessuna curva di risposta in frequenza lineare (larghezza di banda di trasformatori/valvole) è
   modellata nel wet, per mantenere la coerenza di fase con il dry in parallelo.
5. La distorsione è modellata con due curve statiche e non con la dinamica dell'alimentazione o
   l'isteresi del trasformatore.
6. Il ripple del controllo a basse frequenze (THD 1,3 % a 50 Hz con ~13 dB di GR) è un effetto del
   modello: plausibile per un compressore ottico, ma non confrontato con il riferimento.
7. Rumore/ronzio sono generici (bianco + 50/60 Hz con 3ª armonica), non campionati da un'unità.
