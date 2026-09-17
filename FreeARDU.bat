@echo off
setlocal EnableExtensions EnableDelayedExpansion

rem ==========================================================
rem  Activation des couleurs ANSI dans la console Windows
rem ==========================================================
for /f "tokens=2 delims=[]" %%v in ('ver') do set "WINVER=%%v"
reg add "HKCU\Console" /v VirtualTerminalLevel /t REG_DWORD /d 1 /f >nul 2>&1

for /f %%e in ('echo prompt $E ^| cmd') do set "ESC=%%e"
if not defined ESC for /f %%e in ('"prompt $E$S & echo on & for %%b in (1) do rem"') do set "ESC=%%e"
rem Fallback robuste : caractere ESC via powershell si la methode ci-dessus echoue
if not defined ESC (
    for /f %%e in ('powershell -NoProfile -Command "[char]27"') do set "ESC=%%e"
)

set "C_RESET=%ESC%[0m"
set "C_BOLD=%ESC%[1m"
set "C_DIM=%ESC%[2m"
set "C_RED=%ESC%[31m"
set "C_GREEN=%ESC%[32m"
set "C_YELLOW=%ESC%[33m"
set "C_BLUE=%ESC%[34m"
set "C_MAGENTA=%ESC%[35m"
set "C_CYAN=%ESC%[36m"
set "C_WHITE=%ESC%[97m"
set "C_GRAY=%ESC%[90m"
set "C_BG_BLUE=%ESC%[44m"

set "TAG_OK=%C_GREEN%[OK]%C_RESET%"
set "TAG_ERR=%C_RED%[ERREUR]%C_RESET%"
set "TAG_WARN=%C_YELLOW%[!]%C_RESET%"
set "TAG_RUN=%C_CYAN%[>>]%C_RESET%"
set "TAG_INFO=%C_BLUE%[i]%C_RESET%"

set "ROOT=%~dp0"
set "PROFILE=debug"
set "PIO_ENV=mimxrt1060_evk"
set "PIO_CMD=pio"
set "RENODE_EXE=C:\Users\safouane\Renode\renode_1.16.1-dotnet_portable\renode.exe"

if not "%~1"=="" goto command_line
goto menu

:command_line
if /i "%~1"=="help" goto help
if /i "%~1"=="menu" goto menu
if /i "%~1"=="build" goto command_build
if /i "%~1"=="build-run" goto command_build_run
if /i "%~1"=="run" goto command_run
if /i "%~1"=="clean" goto command_clean
if /i "%~1"=="upload" goto command_upload
if /i "%~1"=="monitor" goto command_monitor
if /i "%~1"=="debug" goto command_debug
echo %TAG_ERR% Commande inconnue : %C_BOLD%%~1%C_RESET%
echo.
goto help

:command_build
call :select_profile "%~2"
if errorlevel 1 exit /b 2
call :build "%~3"
exit /b %errorlevel%

:command_build_run
call :select_profile "%~2"
if errorlevel 1 exit /b 2
call :build
if errorlevel 1 exit /b 1
call :run
exit /b %errorlevel%

:command_run
call :select_profile "%~2"
if errorlevel 1 exit /b 2
call :run
exit /b %errorlevel%

:command_clean
call :select_profile "%~2"
if errorlevel 1 exit /b 2
call :clean
exit /b %errorlevel%

:command_upload
call :select_profile "%~2"
if errorlevel 1 exit /b 2
call :upload
exit /b %errorlevel%

:command_monitor
call :monitor
exit /b %errorlevel%

:command_debug
call :select_profile "%~2"
if errorlevel 1 exit /b 2
call :debug
exit /b %errorlevel%

:menu
cls
call :header
echo  %C_CYAN%[1]%C_RESET% Compiler le firmware %C_DIM%(debug)%C_RESET%
echo  %C_CYAN%[2]%C_RESET% Compiler le firmware %C_DIM%(release)%C_RESET%
echo  %C_CYAN%[3]%C_RESET% Nettoyer puis compiler
echo  %C_CYAN%[4]%C_RESET% Compiler et lancer dans Renode
echo  %C_CYAN%[5]%C_RESET% Flasher le firmware %C_DIM%(J-Link)%C_RESET%
echo  %C_CYAN%[6]%C_RESET% Moniteur serie
echo  %C_CYAN%[7]%C_RESET% Debug avec PlatformIO
echo  %C_CYAN%[8]%C_RESET% Aide %C_DIM%(commandes)%C_RESET%
echo  %C_CYAN%[9]%C_RESET% Quitter
echo.
echo %C_GRAY%--------------------------------------------------------%C_RESET%
choice /c 123456789 /n /m "Selectionne une action : "
if errorlevel 9 goto :eof
if errorlevel 8 goto help_menu
if errorlevel 7 (
    call :select_profile
    call :debug
    pause
    goto menu
)
if errorlevel 6 (
    call :monitor
    pause
    goto menu
)
if errorlevel 5 (
    call :select_profile
    call :upload
    pause
    goto menu
)
if errorlevel 4 (
    call :select_profile
    call :build
    if not errorlevel 1 call :run
    pause
    goto menu
)
if errorlevel 3 (
    call :select_profile
    call :clean
    if not errorlevel 1 call :build
    pause
    goto menu
)
if errorlevel 2 (
    set "PROFILE=release"
    call :set_profile
    call :build
    pause
    goto menu
)
set "PROFILE=debug"
call :set_profile
call :build
pause
goto menu

:help_menu
cls
call :help
pause
goto menu

:select_profile
if /i "%~1"=="release" (
    set "PROFILE=release"
) else if /i "%~1"=="debug" (
    set "PROFILE=debug"
) else if "%~1"=="" (
    choice /c DR /n /m "Profil : [D]ebug ou [R]elease ? "
    if errorlevel 2 (set "PROFILE=release") else (set "PROFILE=debug")
) else (
    echo %TAG_ERR% Profil invalide : %C_BOLD%%~1%C_RESET%
    exit /b 2
)
call :set_profile
exit /b 0

:set_profile
if /i "%PROFILE%"=="release" (
    set "PIO_ENV=mimxrt1060_evk_release"
) else (
    set "PROFILE=debug"
    set "PIO_ENV=mimxrt1060_evk"
)
exit /b 0

:check_tools
cd /d "%ROOT%"
where %PIO_CMD% >nul 2>&1
if errorlevel 1 (
    echo %TAG_ERR% PlatformIO introuvable. Installe PlatformIO ou ajoute %C_BOLD%pio%C_RESET% au PATH.
    exit /b 1
)
exit /b 0

:build
call :check_tools
if errorlevel 1 exit /b 1
if /i "%~1"=="clean" call :clean
echo.
echo %TAG_RUN% %C_BOLD%Compilation%C_RESET% %C_MAGENTA%%PROFILE%%C_RESET% %C_GRAY%(%PIO_ENV%)%C_RESET%
echo %C_GRAY%--------------------------------------------------------%C_RESET%
%PIO_CMD% run -e "%PIO_ENV%"
if errorlevel 1 (
    echo %C_GRAY%--------------------------------------------------------%C_RESET%
    echo %TAG_ERR% La compilation a echoue.
    exit /b 1
)
echo %C_GRAY%--------------------------------------------------------%C_RESET%
echo %TAG_OK% Compilation terminee.
exit /b 0

:clean
call :check_tools
if errorlevel 1 exit /b 1
echo %TAG_RUN% %C_BOLD%Nettoyage%C_RESET% %C_GRAY%(%PIO_ENV%)%C_RESET%
%PIO_CMD% run -e "%PIO_ENV%" -t clean
if errorlevel 1 (
    echo %TAG_ERR% Le nettoyage a echoue.
    exit /b 1
)
echo %TAG_OK% Nettoyage termine.
exit /b 0

:run
cd /d "%ROOT%"
if not exist ".pio\build\%PIO_ENV%\firmware.elf" (
    echo %TAG_ERR% Firmware introuvable. Compile-le d'abord.
    exit /b 1
)
if not exist "%RENODE_EXE%" (
    echo %TAG_ERR% Renode introuvable : %C_WHITE%%RENODE_EXE%%C_RESET%
    echo %TAG_INFO% Verifie/adapte la variable %C_BOLD%RENODE_EXE%C_RESET% en haut du script.
    exit /b 1
)
echo %TAG_RUN% Lancement de Renode avec le firmware %C_MAGENTA%%PROFILE%%C_RESET%.
call "%ROOT%Renode.bat" "%PROFILE%" "%RENODE_EXE%"
exit /b %errorlevel%

:upload
call :check_tools
if errorlevel 1 exit /b 1
echo %TAG_RUN% %C_BOLD%Flashage%C_RESET% %C_GRAY%(%PIO_ENV%)%C_RESET%
%PIO_CMD% run -e "%PIO_ENV%" -t upload
if errorlevel 1 (
    echo %TAG_ERR% Le flashage a echoue.
    exit /b 1
)
echo %TAG_OK% Flashage termine.
exit /b 0

:monitor
call :check_tools
if errorlevel 1 exit /b 1
echo %TAG_RUN% Ouverture du moniteur serie PlatformIO. %C_DIM%(Ctrl+C pour quitter)%C_RESET%
%PIO_CMD% device monitor
exit /b %errorlevel%

:debug
call :check_tools
if errorlevel 1 exit /b 1
echo %TAG_RUN% %C_BOLD%Debug%C_RESET% %C_GRAY%(%PIO_ENV%)%C_RESET%
%PIO_CMD% debug -e "%PIO_ENV%"
exit /b %errorlevel%

:header
echo %C_CYAN%%C_BOLD%============================================%C_RESET%
echo %C_CYAN%%C_BOLD%              FreeARDU CLI%C_RESET%
echo %C_CYAN%%C_BOLD%============================================%C_RESET%
echo %TAG_INFO% Racine   : %C_WHITE%%ROOT%%C_RESET%
echo %TAG_INFO% Profil   : %C_MAGENTA%%PROFILE%%C_RESET% %C_GRAY%(%PIO_ENV%)%C_RESET%
echo.
exit /b 0

:help
call :header
echo %C_BOLD%Interface en ligne de commande FreeARDU%C_RESET%
echo.
echo %C_YELLOW%Usage :%C_RESET%
echo   FreeARDU.bat                          Menu interactif
echo   FreeARDU.bat build [debug^|release]   Compiler le firmware
echo   FreeARDU.bat build-run [debug^|release]
echo                                            Compiler puis lancer dans Renode
echo   FreeARDU.bat run [debug^|release]     Lancer le firmware existant dans Renode
echo   FreeARDU.bat clean [debug^|release]   Supprimer les fichiers de build PlatformIO
echo   FreeARDU.bat upload [debug^|release]
echo                                            Compiler/flasher via J-Link
echo   FreeARDU.bat monitor                 Ouvrir le moniteur serie PlatformIO
echo   FreeARDU.bat debug [debug^|release]   Lancer le debogueur PlatformIO
echo   FreeARDU.bat help                    Afficher cette aide
echo.
exit /b 0