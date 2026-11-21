# 7. Stato del progetto (versione 0.1.0)

Ambiente in cui il progetto è stato sviluppato e verificato: Ubuntu 24.04 x86_64 (VM), g++ 13.3,
CMake 3.28, Ninja, JUCE 9.0.4, pluginval 1.0.4, validator del VST3 SDK 3.8.

## 7.1 Implementato e verificato qui

- Core DSP indipendente da JUCE: cella ottica a tre stati con attacco dipendente dal programma,
  rilascio a due stadi con memoria; detector a retroazione con HPF opzionale ed enfasi HF;
  Compress/Limit; colorazione tubo + trasformatore sul solo residuo non lineare; oversampling
  1x/2x/4x a fase lineare con latenza intera; link stereo nel dominio del guadagno.
- Percorso parallelo: dry pulito bit a bit, compensazione di latenza costante (72 campioni),
  crossfade lineare + modalità Dry Constant, Wet Trim / Output Trim, ascolto Dry/Wet, null test e
  wet identitario, cambio di qualità senza click e senza cambio di latenza.
- Robustezza: nessuna allocazione/lock nel callback, NaN/Inf, denormali, silenzio lungo, livelli
  estremi, buffer da 0 a 4096 campioni e variabili, 44,1 – 192 kHz, mono/stereo.
- Plug-in: 20 parametri con ID stabili, stato con default per parametri mancanti, 6 preset,
  bypass dell'host compensato in latenza, compare a volume compensato opzionale, noise/hum
  opzionali disattivati di default.
- GUI originale vettoriale: due manopole grandi, VU con balistica e selezione GR/Output, Mix
  dorato in evidenza, Output Trim, interruttori Compress/Limit e Power, menu preset, pannello
  avanzato; ridimensionabile 700 – 2000 px a proporzioni fisse, HiDPI (snapshot a 2x).
- VST3 Linux compilato e validato: pluginval strictness 10 *SUCCESS*, validator Steinberg 47/47.
- Test: 102 controlli core + 140 controlli plug-in, tutti superati.

Screenshot: [1000 px](screenshots/gui_1000.png) · [pannello avanzato](screenshots/gui_advanced.png)
· [700 px](screenshots/gui_700_min.png) · [1600 px](screenshots/gui_1600_hidpi.png) · [2x](screenshots/gui_1000_at_2x.png)

## 7.2 Configurato ma non verificato

- **VST3 Windows e macOS, AU macOS**: il CMake li prevede ma qui non è stato possibile compilarli.
  Non c'è un compilatore MSVC/Xcode; JUCE 9 non supporta MinGW né la cross-compilazione da Linux;
  i download degli strumenti Microsoft sono bloccati in questo ambiente. Per produrli ci sono
  `build-windows.bat` e il workflow GitHub Actions `.github/workflows/build.yml` (Windows MSVC,
  macOS universal + AU, Linux, con `ctest`, pluginval e `auval`). Il workflow non è ancora stato
  eseguito: la prima run è anche la prima compilazione con MSVC e Xcode.
- **VST2**: opzionale, richiede SDK e licenza propri.
- **Compatibilità DAW**: nessuna DAW è stata provata. Formati supportati: VST3 (Win/macOS/Linux),
  AU (macOS). Non si dichiara compatibilità con applicazioni non ancora testate.
- **Accessibilità**: titoli e tooltip impostati, non provati con uno screen reader.

## 7.3 Limiti noti

1. Il modello sonoro **non è stato confrontato** con hardware né con il plug-in di riferimento: è
   tarato su valori pubblici (vedi [03](03-modello-dsp.md)). Non è "identico" né "indistinguibile".
2. La scala delle manopole non è calibrata su quella del riferimento (vedi serie A in [06](06-validazione.md)).
3. Latenza fissa di 72 campioni anche in modalità Eco (scelta per l'allineamento costante).
4. Banda del wet limitata a ~0,45 fs dai filtri di oversampling (a 44,1 kHz: attenuazione sopra ~20 kHz).
5. Il bypass continua a consumare CPU (per transizioni pulite).
6. L'oversampling riguarda solo la colorazione; la non linearità del detector non è sovracampionata
   (influisce solo sul controllo).
7. Il compare a volume compensato è un aiuto all'ascolto con costanti di tempo lente: non segue
   variazioni rapide e non va usato in stampa.
8. Su Linux, in questo ambiente, alcune estensioni X11 sono state disattivate per mancanza dei
   pacchetti `-dev` (`CHIOCCIA_LINUX_MINIMAL_X11`): il plug-in funziona, ma su un sistema completo
   conviene compilarlo senza questa opzione.
9. I codici plug-in e il nome azienda sono segnaposto.

## 7.4 Verifiche ancora necessarie (in ordine di priorità)

1. Build Windows/macOS + `ctest` + pluginval (+ `auval` per l'AU).
2. Procedura DAW della sezione 6.3 in almeno: Reaper, Ableton Live, Cubase/Nuendo, Logic Pro (AU),
   Studio One, Bitwig, FL Studio (in base a quelle che usi).
3. Confronto con il riferimento (6.4) e, solo dopo, ricalibrazione delle costanti.
4. Ascolto critico su materiale reale (voce, basso, batteria in parallelo, mix bus).
5. Profilazione CPU nelle DAW con molte istanze e buffer piccoli.
6. Firma del codice e notarizzazione per la distribuzione su macOS; installer per Windows.
