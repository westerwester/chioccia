# 5. Compilazione e licenze

## 5.1 Requisiti comuni

- CMake ≥ 3.22, Git.
- JUCE **9.0.4** (scaricato automaticamente con FetchContent, oppure una copia locale passata con
  `-DCHIOCCIA_JUCE_DIR=/percorso/JUCE`). Requisiti minimi dichiarati da JUCE 9: C++17,
  Visual Studio 2019, Xcode 12.4 (macOS 10.15.4 Intel / 11.0 Apple Silicon), g++ 7 / Clang 6.
- Nessuna dipendenza oltre a JUCE (che include il VST3 SDK 3.8).

## 5.2 Opzioni CMake

| Opzione | Default | Significato |
|---|---|---|
| `CHIOCCIA_BUILD_PLUGIN` | ON | plug-in JUCE (VST3; AU su macOS) |
| `CHIOCCIA_BUILD_TESTS` | ON | test del core, strumento di misura, test del plug-in |
| `CHIOCCIA_BUILD_STANDALONE` | OFF | app standalone (comoda per provare la GUI; su Linux richiede gli header ALSA) |
| `CHIOCCIA_ENABLE_VST2` | OFF | VST2, solo con `CHIOCCIA_VST2_SDK_DIR` (vedi 5.6) |
| `CHIOCCIA_JUCE_DIR` | vuoto | JUCE locale invece di FetchContent |
| `CHIOCCIA_JUCE_TAG` | 9.0.4 | tag JUCE usato da FetchContent |
| `CHIOCCIA_LINUX_MINIMAL_X11` | OFF | Linux: disattiva XInput2/Xrandr/Xinerama/Xcursor se mancano i pacchetti `-dev` |

Solo il core DSP e i test (nessun JUCE, compila in pochi secondi):

```bash
cmake -S . -B build-core -DCHIOCCIA_BUILD_PLUGIN=OFF
cmake --build build-core --config Release
ctest --test-dir build-core -C Release --output-on-failure
./build-core/chioccia_measure docs/misure 48000        # rigenera le misure
python3 tools/plot_measurements.py docs/misure          # grafici (matplotlib)
```

## 5.3 Windows (Visual Studio 2022)

Il file binario per Windows si ottiene in due modi:

- **sul tuo PC Windows**: installa Visual Studio 2022 o 2026 Community (gratuito) con il carico di
  lavoro *Sviluppo di applicazioni desktop con C++*, estrai il progetto in una cartella corta (es.
  `C:\chioccia`) e fai doppio clic su `build-windows.bat`. Lo script trova Visual Studio da solo
  (vswhere + VsDevCmd), scarica JUCE (con Git se c'è, altrimenti come archivio), compila con Ninja,
  esegue i test, mette il plug-in in `dist\Chioccia.vst3` e prova a installarlo in
  `C:\Program Files\Common Files\VST3` (serve "Esegui come amministratore");
- **senza un PC Windows**: carica il progetto in un repository GitHub. Il workflow
  `.github/workflows/build.yml` compila con MSVC su un runner Windows (e anche macOS e Linux),
  esegue `ctest` e pluginval, e pubblica gli zip del plug-in tra gli *Artifacts* della run.

Non è possibile cross-compilare il plug-in per Windows da Linux: JUCE 9 ha rimosso il supporto a
MinGW e dichiara non supportata la cross-compilazione Windows da Linux (`BREAKING_CHANGES.md`).

Comandi manuali equivalenti:

```bat
git clone <repo> chioccia && cd chioccia
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --target Chioccia_VST3
ctest --test-dir build -C Release --output-on-failure
```

- Risultato: `build\Chioccia_artefacts\Release\VST3\Chioccia.vst3`.
- Installazione: copia la cartella `Chioccia.vst3` in `C:\Program Files\Common Files\VST3\`
  (in alternativa imposta `COPY_PLUGIN_AFTER_BUILD TRUE` in `juce_add_plugin`, con permessi di amministratore).
- ARM64: `-A ARM64` (non provato).

## 5.4 macOS (Xcode)

```bash
cmake -S . -B build -G Xcode -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" -DCMAKE_OSX_DEPLOYMENT_TARGET=10.15
cmake --build build --config Release --target Chioccia_VST3 Chioccia_AU
ctest --test-dir build -C Release --output-on-failure
```

- Risultati: `Chioccia.vst3` e `Chioccia.component` in `build/Chioccia_artefacts/Release/`.
- Installazione: `~/Library/Audio/Plug-Ins/VST3/` e `~/Library/Audio/Plug-Ins/Components/`.
- Valida l'AU: `auval -v aufx Chcc Plla` (dopo aver riavviato `AudioComponentRegistrar` o fatto
  logout/login se l'AU non compare).
- Distribuzione: firma con un certificato *Developer ID Application* (`codesign --deep --options runtime`)
  e notarizza con `xcrun notarytool`; senza notarizzazione Gatekeeper blocca il plug-in su altri Mac.

## 5.5 Linux

```bash
sudo apt install build-essential cmake git libasound2-dev libx11-dev libxcomposite-dev \
     libxcursor-dev libxext-dev libxinerama-dev libxrandr-dev libxrender-dev libxi-dev \
     libfreetype-dev libfontconfig1-dev
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure      # i test del plug-in creano la GUI: in headless usa xvfb-run
```

Installazione: copia `Chioccia.vst3` in `~/.vst3/`.

**Compilato in questo ambiente** (Ubuntu 24.04, g++ 13.3, CMake 3.28, JUCE 9.0.4,
`-DCHIOCCIA_LINUX_MINIMAL_X11=ON` perché mancavano alcuni pacchetti `-dev`): core, test, misure,
VST3 Linux x86_64. Windows e macOS **non** sono stati compilati qui.

## 5.6 VST2 (facoltativo)

Steinberg ha smesso di distribuire l'SDK VST2 ai nuovi sviluppatori nel 2018: puoi compilare e
distribuire un VST2 solo se possiedi legalmente l'SDK e l'accordo di licenza VST2. In quel caso:

```bash
cmake -S . -B build -DCHIOCCIA_ENABLE_VST2=ON -DCHIOCCIA_VST2_SDK_DIR=/percorso/vstsdk2.4
```

Il CMake si ferma con un errore esplicito se la cartella non contiene `pluginterfaces/vst2.x/aeffect.h`.
Il resto del progetto non dipende in alcun modo dal VST2.

## 5.7 Licenze (verificate il 20/10/2026, non è consulenza legale)

- **JUCE 9**: moduli in doppia licenza **AGPLv3** oppure **licenza commerciale JUCE 9**. I prezzi e
  le condizioni sono stati mantenuti identici a JUCE 8: *Starter* gratuita fino a 20.000 USD di
  ricavi/finanziamenti annui (senza splash screen), *Indie* fino a 300.000 USD, *Pro* senza
  limite. Per un plug-in closed-source serve una licenza JUCE; con AGPLv3 il codice va
  distribuito secondo l'AGPL. Leggi l'EULA su juce.com prima di distribuire.
- **VST3 SDK 3.8** (incluso in JUCE 9): licenza MIT; il logo/marchio VST richiede le linee guida
  Steinberg (`VST3_Usage_Guidelines`).
- **Lilita One**: SIL Open Font License 1.1, incorporata come dato binario; il file di licenza è in
  `assets/fonts/`.
- **Chioccia**: tutto il codice e l'artwork vettoriale sono originali.
