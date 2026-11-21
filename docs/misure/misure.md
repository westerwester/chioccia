# Misure del modello Chioccia (generate automaticamente)

Generato da `chioccia_measure` a 48000 Hz. Questi valori descrivono questa implementazione; non sono un confronto con hardware o con plug-in di terze parti.

## Curve statiche (sinusoide 1 kHz, Gain 50, Colour 0, Mix 100 %)

GR in dB misurata sull'RMS dell'ultimo secondo di 3 s di segnale. 0 VU = −18 dBFS RMS.

| Modalità | PR | GR @ −30 | GR @ −18 (0 VU) | GR @ −12 | GR @ −6 | GR @ 0 dBFS | rapporto effettivo (ultimi 6 dB) |
|---|---|---|---|---|---|---|---|
| Compress | 20 | 0.00 | 0.06 | 0.47 | 2.07 | 5.03 | 2.0:1 |
| Compress | 40 | 0.06 | 2.07 | 5.03 | 8.75 | 12.83 | 3.1:1 |
| Compress | 50 | 0.47 | 5.03 | 8.75 | 12.83 | 17.10 | 3.5:1 |
| Compress | 60 | 2.07 | 8.75 | 12.83 | 17.10 | 21.47 | 3.7:1 |
| Compress | 80 | 8.75 | 17.10 | 21.47 | 25.89 | 30.34 | 3.9:1 |
| Compress | 100 | 17.10 | 25.89 | 30.34 | 34.81 | 39.29 | 3.9:1 |
| Limit | 20 | 0.00 | 0.00 | 0.00 | 0.71 | 4.89 | 3.3:1 |
| Limit | 40 | 0.00 | 0.71 | 4.89 | 10.01 | 15.34 | 8.9:1 |
| Limit | 50 | 0.00 | 4.89 | 10.01 | 15.34 | 20.74 | 10.0:1 |
| Limit | 60 | 0.71 | 10.01 | 15.34 | 20.74 | 26.17 | 10.6:1 |
| Limit | 80 | 10.01 | 20.74 | 26.17 | 31.63 | 37.09 | 11.1:1 |
| Limit | 100 | 20.74 | 31.63 | 37.09 | 42.55 | 48.02 | 11.3:1 |

## Attacco (gradino da −40 dBFS a livello L, sinusoide 1 kHz)

Tempo perché la GR (in dB) raggiunga il 63 % e il 90 % del valore finale.

| Modalità | PR | L (dBFS RMS) | GR finale | t63 | t90 |
|---|---|---|---|---|---|
| Compress | 60 | -18 | 8.75 dB | 12.1 ms | 19.5 ms |
| Compress | 60 | -12 | 12.83 dB | 9.3 ms | 12.5 ms |
| Compress | 60 | -6 | 17.10 dB | 7.5 ms | 9.4 ms |
| Limit | 60 | -18 | 10.01 dB | 12.0 ms | 13.5 ms |
| Limit | 60 | -12 | 15.34 dB | 13.4 ms | 15.3 ms |
| Limit | 60 | -6 | 20.74 dB | 14.8 ms | 17.2 ms |

## Rilascio in funzione della storia (PR 60, Compress, burst a −8 dBFS RMS poi −80 dBFS)

| Durata burst | GR al rilascio | t al 50 % | t al 10 % | t a GR < 0,5 dB |
|---|---|---|---|---|
| 0.1 s | 15.67 dB | 63 ms | 0.22 s | 1.71 s |
| 0.5 s | 15.66 dB | 70 ms | 1.53 s | 6.32 s |
| 2.0 s | 15.66 dB | 99 ms | 5.84 s | 11.22 s |
| 10.0 s | 15.66 dB | 153 ms | 8.71 s | 14.08 s |
| 30.0 s | 15.66 dB | 156 ms | 8.82 s | 14.19 s |

## Distorsione armonica (PR 0 = nessuna compressione, Gain 50, qualità 2x)

THD = H2…H9 / H1. Livelli in dBFS di picco.

| f | Colour | −30 dBFS | −18 dBFS | −12 dBFS | −6 dBFS | −1 dBFS | H2/H3 a −6 dBFS |
|---|---|---|---|---|---|---|---|
| 1000 Hz | 0 % | < 0.001 % | < 0.001 % | < 0.001 % | < 0.001 % | < 0.001 % | -303 / -159 dB |
| 1000 Hz | 25 % | 0.019 % | 0.076 % | 0.158 % | 0.358 % | 0.816 % | -51 / -54 dB |
| 1000 Hz | 50 % | 0.038 % | 0.158 % | 0.358 % | 0.979 % | 2.568 % | -45 / -42 dB |
| 1000 Hz | 75 % | 0.076 % | 0.358 % | 0.979 % | 3.128 % | 8.028 % | -40 / -31 dB |
| 1000 Hz | 100 % | 0.158 % | 0.979 % | 3.128 % | 9.524 % | 19.314 % | -37 / -21 dB |
| 100 Hz | 0 % | < 0.001 % | < 0.001 % | < 0.001 % | < 0.001 % | < 0.001 % | -319 / -163 dB |
| 100 Hz | 25 % | 0.019 % | 0.075 % | 0.154 % | 0.337 % | 0.724 % | -51 / -56 dB |
| 100 Hz | 50 % | 0.038 % | 0.154 % | 0.337 % | 0.858 % | 2.153 % | -45 / -44 dB |
| 100 Hz | 75 % | 0.075 % | 0.337 % | 0.858 % | 2.610 % | 6.743 % | -40 / -32 dB |
| 100 Hz | 100 % | 0.154 % | 0.858 % | 2.610 % | 8.074 % | 18.500 % | -36 / -22 dB |
| 50 Hz | 0 % | < 0.001 % | < 0.001 % | < 0.001 % | < 0.001 % | < 0.001 % | -320 / -168 dB |
| 50 Hz | 25 % | 0.019 % | 0.076 % | 0.161 % | 0.382 % | 0.919 % | -51 / -52 dB |
| 50 Hz | 50 % | 0.038 % | 0.161 % | 0.382 % | 1.114 % | 3.078 % | -45 / -40 dB |
| 50 Hz | 75 % | 0.076 % | 0.382 % | 1.114 % | 3.790 % | 10.561 % | -39 / -29 dB |
| 50 Hz | 100 % | 0.161 % | 1.114 % | 3.790 % | 12.885 % | 35.125 % | -35 / -18 dB |

### Distorsione da modulazione del guadagno (PR 60, Colour 0, −12 dBFS RMS)

Misura il ripple del controllo a basse frequenze (comportamento tipico dei compressori ottici).

| f | Compress THD | Limit THD |
|---|---|---|
| 50 Hz | 1.271 % | 1.714 % |
| 100 Hz | 0.460 % | 0.575 % |
| 1000 Hz | 0.007 % | 0.007 % |

## Risposta in frequenza del percorso wet (PR 0, −30 dBFS, relativa a 1 kHz)

| Qualità | Colour | 20 Hz | 50 Hz | 100 Hz | 10 kHz | 18 kHz | 20 kHz | 21 kHz |
|---|---|---|---|---|---|---|---|---|
| x1 | 0 % | 0.000 | 0.000 | 0.000 | -0.000 | -0.000 | -0.000 | -0.000 |
| x1 | 100 % | -0.008 | -0.007 | -0.003 | 0.000 | 0.000 | 0.000 | 0.000 |
| x2 | 0 % | 0.000 | 0.000 | 0.000 | 0.000 | 0.000 | -0.000 | -0.000 |
| x2 | 100 % | -0.008 | -0.007 | -0.003 | 0.000 | 0.000 | -0.000 | -0.000 |
| x4 | 0 % | 0.000 | 0.000 | 0.000 | -0.000 | 0.000 | -0.000 | -0.000 |
| x4 | 100 % | -0.008 | -0.007 | -0.003 | -0.000 | 0.000 | -0.000 | -0.000 |

## Dipendenza in frequenza del detector (PR 60, Compress, −12 dBFS RMS)

| f | GR HPF off | GR HPF 100 Hz |
|---|---|---|
| 40 Hz | 12.40 dB | 2.67 dB |
| 80 Hz | 12.56 dB | 8.93 dB |
| 150 Hz | 12.68 dB | 12.15 dB |
| 500 Hz | 12.76 dB | 12.76 dB |
| 1000 Hz | 12.83 dB | 12.83 dB |
| 3000 Hz | 13.62 dB | 13.62 dB |
| 8000 Hz | 13.67 dB | 13.71 dB |

## Aliasing (sinusoide 15 kHz a −3 dBFS, Colour 100 %, PR 0)

Somma della potenza dei prodotti ripiegati sotto 20 kHz (armoniche 2…12 di 15 kHz), relativa alla fondamentale.

| Qualità | aliasing |
|---|---|
| x1 | -16.6 dB |
| x2 | -41.7 dB |
| x4 | -85.4 dB |
