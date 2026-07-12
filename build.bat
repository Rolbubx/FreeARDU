@echo off
setlocal enabledelayedexpansion

:: Check for arguments
set "BUILD_TYPE=debug"
set "EXTRA_ARGS="
set "CLEAN_FIRST=0"

:parse_args
if "%~1"=="" goto end_parse
if /i "%~1"=="release" (
    set "BUILD_TYPE=release"
    set "EXTRA_ARGS=-e mimxrt1060_evk_release"
)
if /i "%~1"=="debug" (
    set "BUILD_TYPE=debug"
    set "EXTRA_ARGS=-e mimxrt1060_evk"
)
if /i "%~1"=="clean" (
    set "CLEAN_FIRST=1"
)
shift
goto parse_args
:end_parse

:: ANSI Colors
set "C_RESET=[0m"
set "C_CYAN=[1;36m"
set "C_GREEN=[1;32m"
set "C_RED=[1;31m"
set "C_YELLOW=[1;33m"
set "C_MAGENTA=[1;35m"

echo %C_CYAN%[%TIME%] [BUILD] Starting FreeARDU build (%BUILD_TYPE%)%C_RESET%
echo.

:: Faster loading bar (Simpler for compatibility)
for /L %%i in (1,4,20) do (
    echo %C_MAGENTA%Initializing... %%i/20%C_RESET%
    timeout /t 1 /nobreak >nul
)

cls
echo %C_CYAN%[%TIME%] [BUILD] Starting FreeARDU build (%BUILD_TYPE%)%C_RESET%
echo.

:: Navigate to the root directory
cd /d "%~dp0"

if "%CLEAN_FIRST%"=="1" (
    echo %C_YELLOW%[%TIME%] [BUILD] Cleaning previous builds...%C_RESET%
    pio run --target clean %EXTRA_ARGS% >nul 2>&1
)

echo %C_CYAN%[%TIME%] [BUILD] Running PlatformIO build (%BUILD_TYPE%)...%C_RESET%
pio run %EXTRA_ARGS%

if %ERRORLEVEL% equ 0 (
    echo.
    echo %C_GREEN%[%TIME%] [SUCCESS] Build finished successfully.%C_RESET%
    echo.
    echo %C_YELLOW%[UPLOAD] Do you want to upload the firmware? [Y/N]%C_RESET%
    set /p UPLOAD_CHOICE=
    
    if /i "!UPLOAD_CHOICE!"=="Y" (
        echo %C_CYAN%[%TIME%] [UPLOAD] Starting firmware upload...%C_RESET%
        pio upload %EXTRA_ARGS%
        if %ERRORLEVEL% equ 0 (
            echo %C_GREEN%[%TIME%] [SUCCESS] Upload finished successfully.%C_RESET%
        ) else (
            echo %C_RED%[%TIME%] [ERROR] Upload failed.%C_RESET%
        )
    ) else (
        echo %C_MAGENTA%[UPLOAD] Upload skipped by user.%C_RESET%
    )
) else (
    echo.
    echo %C_RED%[%TIME%] [ERROR] Build failed. Check the output above for details.%C_RESET%
)

pause
