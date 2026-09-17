@echo off
REM Renode launcher script for NXP i.MX RT1060 EVK
echo ============================================
echo   Renode - FreeARDU i.MX RT1060 Emulator
echo ============================================
echo.

REM Get script directory
set "SCRIPT_DIR=%~dp0"

REM Chemin vers l'executable Renode.
REM Passe en 2e argument par FreeARDU.bat ; sinon on retombe sur "renode" (PATH).
set "RENODE_EXE=%~2"
if "%RENODE_EXE%"=="" set "RENODE_EXE=renode"

REM Default firmware path
set "FIRMWARE_PATH=%SCRIPT_DIR%.pio\build\mimxrt1060_evk\firmware.elf"

REM Parse arguments
if /i "%~1"=="release" set "FIRMWARE_PATH=%SCRIPT_DIR%.pio\build\mimxrt1060_evk_release\firmware.elf"
if /i "%~1"=="debug" set "FIRMWARE_PATH=%SCRIPT_DIR%.pio\build\mimxrt1060_evk\firmware.elf"
if /i "%~1"=="firmware=" set "FIRMWARE_PATH=%~2"

REM Check firmware exists
if not exist "%FIRMWARE_PATH%" (
    echo ERROR: Firmware not found at: %FIRMWARE_PATH%
    echo Build first with: platformio run -e mimxrt1060_evk
    pause
    exit /b 1
)

REM Check Renode executable exists (only when a full path was given)
if not "%RENODE_EXE%"=="renode" (
    if not exist "%RENODE_EXE%" (
        echo ERROR: Renode executable not found at: %RENODE_EXE%
        pause
        exit /b 1
    )
)

echo Using firmware: %FIRMWARE_PATH%
echo Using Renode  : %RENODE_EXE%
echo Starting Renode...
echo.

REM Create RESC file in temp
set "RESC_FILE=%TEMP%\renode_%RANDOM%.resc"

(
echo # Renode RESC script for NXP i.MX RT1060 EVK
echo mach create "mimxrt1060"
echo machine LoadPlatformDescription @platforms/boards/mimxrt1060_evk.repl
echo sysbus LoadELF "%FIRMWARE_PATH%"
echo cpu PC `sysbus GetSymbolAddress "reset_handler"`
echo showAnalyzer sysbus.lpuart1
echo start
echo log "Starting emulation..."
) > "%RESC_FILE%"

REM Run Renode
"%RENODE_EXE%" "%RESC_FILE%"

REM Cleanup
del "%RESC_FILE%" 2>nul
echo.
echo Renode session ended.
pause