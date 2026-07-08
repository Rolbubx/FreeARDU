@echo off
setlocal enabledelayedexpansion

echo [BUILD] Starting build process for FreeARDU...
echo.

:: Better loading bar with clear animation
for /L %%i in (1,1,20) do (
    cls
    echo [BUILD] Starting build process for FreeARDU...
    echo.
    echo Initializing...
    set "bar="
    for /L %%j in (1,1,%%i) do set "bar=!bar!#"
    set "remaining="
    for /L %%j in (1,1,20) do set /a "k=%%i+%%j" & if !k! leq 20 set "remaining=!remaining! "
    echo [!bar!!remaining!] %%i%%
    timeout /t 1 /nobreak >nul
)

cls
echo [BUILD] Starting build process for FreeARDU...
echo.

:: Navigate to the root directory where platformio.ini is located
cd /d "%~dp0"

echo [BUILD] Cleaning previous builds...
pio run --target clean >nul 2>&1

echo [BUILD] Running PlatformIO build...
pio run -v
if %ERRORLEVEL% equ 0 (
    echo.
    echo [SUCCESS] Build finished successfully.
    echo.
    echo [UPLOAD] Do you want to upload the firmware? [Y/N]
    set /p UPLOAD_CHOICE=
    
    if /i "!UPLOAD_CHOICE!"=="Y" (
        echo [UPLOAD] Starting firmware upload...
        pio upload
        if %ERRORLEVEL% equ 0 (
            echo [SUCCESS] Upload finished successfully.
        ) else (
            echo [ERROR] Upload failed.
        )
    ) else (
        echo [UPLOAD] Upload skipped by user.
    )
) else (
    echo.
    echo [ERROR] Build failed. Check the output above for details.
)

pause
