@echo off
rem ============================================================================================
rem  Chioccia - compila il plug-in VST3 per Windows.
rem
rem  Serve: Visual Studio 2022 o 2026 (la versione Community e' gratuita) con il carico di lavoro
rem  "Sviluppo di applicazioni desktop con C++" e una connessione internet (la prima volta lo
rem  script scarica JUCE). Basta un doppio clic su questo file.
rem ============================================================================================
setlocal EnableExtensions
cd /d "%~dp0"

rem Windows limita i percorsi a 260 caratteri: meglio una cartella corta come C:\chioccia
set "HERE=%CD%"
if not "%HERE:~50,1%"=="" (
    echo ATTENZIONE: la cartella e' in un percorso lungo. Se la compilazione fallisce
    echo             sposta il progetto in una cartella corta, ad esempio C:\chioccia
    echo.
)

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
    echo [ERRORE] Visual Studio non trovato.
    echo          Installa Visual Studio Community da https://visualstudio.microsoft.com
    echo          con il carico di lavoro "Sviluppo di applicazioni desktop con C++".
    goto :error
)

set "VSDIR="
for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSDIR=%%i"
if not defined VSDIR (
    echo [ERRORE] Visual Studio e' installato ma manca il compilatore C++.
    echo          Apri "Visual Studio Installer", premi "Modifica" e attiva
    echo          "Sviluppo di applicazioni desktop con C++".
    goto :error
)

echo Visual Studio trovato in: %VSDIR%
call "%VSDIR%\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64 -no_logo
where cl >nul 2>nul
if errorlevel 1 (
    echo [ERRORE] Impossibile attivare il compilatore C++ di Visual Studio.
    goto :error
)

where cmake >nul 2>nul
if errorlevel 1 (
    echo [ERRORE] CMake non trovato. In "Visual Studio Installer" premi "Modifica" e attiva
    echo          il componente "Strumenti CMake C++ per Windows".
    goto :error
)

rem Ninja (incluso negli strumenti CMake di Visual Studio) evita problemi con le versioni di VS.
set "GENERATOR=-A x64"
where ninja >nul 2>nul
if not errorlevel 1 set "GENERATOR=-G Ninja -DCMAKE_BUILD_TYPE=Release"

echo.
echo === 1/4 Configurazione (la prima volta scarica JUCE, qualche minuto) ===
cmake -S . -B build-win %GENERATOR%
if errorlevel 1 goto :error

echo.
echo === 2/4 Compilazione (10-20 minuti la prima volta) ===
cmake --build build-win --config Release --parallel
if errorlevel 1 goto :error

echo.
echo === 3/4 Test automatici ===
set "TESTS_OK=1"
ctest --test-dir build-win -C Release --output-on-failure
if errorlevel 1 set "TESTS_OK=0"

echo.
echo === 4/4 Copia del plug-in ===
set "PLUGIN=build-win\Chioccia_artefacts\Release\VST3\Chioccia.vst3"
if not exist "%PLUGIN%" (
    echo [ERRORE] Il plug-in compilato non e' stato trovato in %PLUGIN%
    goto :error
)
if exist dist\Chioccia.vst3 rmdir /s /q dist\Chioccia.vst3
xcopy /E /I /Y /Q "%PLUGIN%" "dist\Chioccia.vst3" >nul
if errorlevel 1 goto :error

set "VST3DIR=%CommonProgramFiles%\VST3"
set "INSTALLED=0"
if not exist "%VST3DIR%" mkdir "%VST3DIR%" 2>nul
xcopy /E /I /Y /Q "dist\Chioccia.vst3" "%VST3DIR%\Chioccia.vst3" >nul 2>nul
if not errorlevel 1 set "INSTALLED=1"

echo.
echo ============================================================================================
echo  FATTO. Il plug-in e' in: %CD%\dist\Chioccia.vst3
if "%INSTALLED%"=="1" (
    echo  Installato anche in: %VST3DIR%\Chioccia.vst3
) else (
    echo  Per installarlo copia la cartella dist\Chioccia.vst3 in "%VST3DIR%"
    echo  oppure riesegui questo file con clic destro, "Esegui come amministratore".
)
if "%TESTS_OK%"=="0" (
    echo.
    echo  ATTENZIONE: alcuni test automatici sono falliti, vedi i messaggi sopra.
)
echo  Poi nella tua DAW fai una nuova scansione dei plug-in e cerca "Chioccia".
echo ============================================================================================
pause
exit /b 0

:error
echo.
echo  COMPILAZIONE NON RIUSCITA - copia i messaggi qui sopra e mandali a chi ti aiuta.
pause
exit /b 1
