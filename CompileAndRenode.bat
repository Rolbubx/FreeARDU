@echo off
setlocal enabledelayedexpansion

:: ANSI Colors
set "C_RESET=[0m"
set "C_CYAN=[1;36m"
set "C_GREEN=[1;32m"
set "C_RED=[1;31m"
set "C_YELLOW=[1;33m"

echo %C_CYAN%[%TIME%] [INFO] Starting Compile and Renode process...%C_RESET%

:: 1. Compile
echo %C_CYAN%[%TIME%] [BUILD] Calling build.bat debug...%C_RESET%
call build.bat debug
if %ERRORLEVEL% neq 0 (
    echo %C_RED%[%TIME%] [ERROR] Build failed. Aborting Renode.%C_RESET%
    exit /b %ERRORLEVEL%
)

:: 2. Find Renode
set "RENODE_PATH="
where renode >nul 2>&1
if %ERRORLEVEL% equ 0 (
    for /f "delims=" %%i in ('where renode') do set "RENODE_PATH=%%i"
) else (
    if exist "C:\Program Files\Renode\bin\Renode.exe" (
        set "RENODE_PATH=C:\Program Files\Renode\bin\Renode.exe"
    ) else if exist "C:\Program Files\Renode\Renode.exe" (
        set "RENODE_PATH=C:\Program Files\Renode\Renode.exe"
    ) else (
        echo %C_RED%[%TIME%] [ERROR] Renode not found in PATH or default locations.%C_RESET%
        set /p "RENODE_PATH=Please enter the full path to Renode.exe: "
    )
)

if not exist "!RENODE_PATH!" (
    echo %C_RED%[%TIME%] [ERROR] Renode.exe not found at "!RENODE_PATH!"%C_RESET%
    exit /b 1
)

echo %C_GREEN%[%TIME%] [INFO] Found Renode: !RENODE_PATH!%C_RESET%

:: 3. Run Renode with commands
:: Note: Using relative paths where possible or absolute as requested.
:: The user specifically asked for @F:/FreeARDUrep/.pio/build/mimxrt1060_evk/firmware.elf

set "ELF_PATH=%~dp0.pio\build\mimxrt1060_evk\firmware.elf"
set "ELF_PATH=!ELF_PATH:\=/!"

echo %C_CYAN%[%TIME%] [RENODE] Launching Renode with firmware: !ELF_PATH!%C_RESET%

"!RENODE_PATH!" -e "mach create; machine LoadPlatformDescription @platforms/boards/mimxrt1060_evk.repl; sysbus LoadELF @!ELF_PATH!; cpu PC ``sysbus GetSymbolAddress \"reset_handler\"``; showAnalyzer sysbus.lpuart1; start"

exit /b %ERRORLEVEL%
