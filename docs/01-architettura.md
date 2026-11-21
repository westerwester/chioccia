# 1. Architettura e percorso del segnale

## Schema

```
                    ┌──────────────────────────────── PERCORSO DRY ─────────────────────────────────┐
 in ─► sanitize ─► Input Trim ─┤                                                                   │
  │   (NaN/Inf→0,              │  ritardo intero L (= latenza dichiarata) ─────────────────► dry ──┤
  │    limite ±60 dBFS)        │                                                                   │
  │                            └──── PERCORSO WET ─────────────────────────────────────────────┐   │
  │                                                                                            │   │
  │      × g_cella ──► × Gain ──► [ ↑OS ─► tubo + trasformatore ─► ↓OS ] ─► pad (L − L_q) ─►   │   │
  │         ▲          (make-up)    colorazione (solo residuo non lineare)                     │   │
  │         │                                                     + noise/hum (opz.) ─► Wet Trim ─► wet
  │         │ link stereo (dominio del guadagno)                                               │   │
  │   cella ottica ◄── EL panel ◄── drive (Peak Reduction) ◄── HF emphasis ◄── HPF detector ◄──┘   │
  │   (c, m, d)                                          segnale post-cella (retroazione)          │
  │                                                                                                │
  │                    legge di mix (crossfade lineare | dry costante) ──► Listen (Mix/Dry/Wet) ◄──┘
  │                                                │
  │                                       Output Trim ──► Compare a volume compensato (opz.)
  │                                                │
  └──► ritardo L (bypass) ───────────── crossfade bypass ──► out
```

- **L** è costante: è la latenza del percorso 4x (72 campioni con i filtri attuali, a ogni sample
  rate). I modi 1x e 2x vengono riempiti con ritardo puro (`pad`), quindi cambiare qualità non
  cambia mai la latenza dichiarata all'host.
- La **cella ottica** e il **detector** lavorano alla frequenza dell'host; solo lo stadio di
  **colorazione** (l'unico che genera armoniche nel segnale audio) è sovracampionato.
- Il **detector** legge il segnale *dopo* la cella e *prima* del Gain (topologia a retroazione
  rispetto all'attenuatore): il Gain non cambia mai la quantità di compressione.

## Moduli

| Modulo | File | Responsabilità | Dipende da JUCE |
|---|---|---|---|
| `OpticalCell` | `source/dsp/OpticalCell.*` | modello della cella EL + fotoresistenza, attacco/rilascio dipendenti dal programma, memoria a 3 stati | no |
| `Detector` | `source/dsp/Detector.*` | HPF del detector, enfasi HF fissa, pannello EL (soglia + legge di potenza + persistenza) | no |
| `ColourStage` | `source/dsp/ColourStage.*` | saturazione tubo asimmetrica + saturazione LF "trasformatore", solo sul residuo non lineare | no |
| `HalfBandOversampler` | `source/dsp/HalfBandOversampler.*` | FIR half-band polifase a fase lineare, 1x/2x/4x, latenza intera | no |
| `OptoEngine` | `source/dsp/OptoEngine.*` | catena completa, dry/wet, compensazione, mix, listen, diagnostica, bypass, metering | no |
| `Smoothers`, `DelayLine`, `Biquad` | `source/dsp/*.h` | utilità real-time | no |
| `ChiocciaProcessor` | `source/plugin/PluginProcessor.*` | APVTS, stato, preset, bus, latenza, bypass host | sì |
| `ChiocciaEditor` + `gui/` | `source/plugin/PluginEditor.*`, `source/gui/*` | interfaccia vettoriale ridimensionabile | sì |

Ogni modulo DSP ha una struct di configurazione (`OpticalCellConfig`, `DetectorConfig`,
`ColourConfig`) esposta dall'engine: le costanti di calibrazione sono in un solo posto e i test /
lo strumento di misura possono modificarle senza toccare il codice.

## Thread e real-time

- **Thread audio**: `processBlock` legge i parametri da `std::atomic<float>` (APVTS), li passa
  all'engine come struct semplice, elabora campione per campione. Nessuna allocazione, nessun
  lock, nessun accesso a file. `juce::ScopedNoDenormals` + azzeramento esplicito degli stati
  minuscoli (`flushTiny`) per la protezione dai denormali anche fuori da JUCE.
- **Message thread**: GUI, preset, stato. `prepareToPlay` alloca tutto (tre catene di
  oversampling per canale, linee di ritardo, storico per il priming).
- **Metering**: l'engine scrive `std::atomic<float>` (GR, RMS in/out, guadagno di compare) una
  volta per blocco; la GUI li legge a 60 Hz e applica la balistica del VU.
- Buffer di qualsiasi dimensione (anche 0, 1, o maggiore di quella annunciata): l'elaborazione è
  per-campione e senza buffer temporanei; il test `block_size_invariance` verifica che l'uscita
  sia **identica bit a bit** con blocchi da 1, 512 o di dimensione casuale.

## Scelte di framework e formati

- **JUCE 9.0.4 + CMake**: maturo per plug-in multi-formato, CMake nativo, include il VST3 SDK
  3.8 (licenza MIT). Licenza: AGPLv3 oppure licenza commerciale JUCE 9 (dettagli in
  [05-build.md](05-build.md)). Il core DSP non dipende da JUCE e può essere riusato altrove.
- **VST3** (Windows, macOS, Linux): formato principale.
- **AU** (macOS): attivato automaticamente su Apple; da validare con `auval` su un Mac.
- **VST2**: solo con `-DCHIOCCIA_ENABLE_VST2=ON -DCHIOCCIA_VST2_SDK_DIR=...` e un SDK VST2
  ottenuto legalmente (Steinberg non lo distribuisce più ai nuovi sviluppatori). Nessuna parte del
  progetto ne ha bisogno.
- **AAX / CLAP**: non inclusi in questa versione.
