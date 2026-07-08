@echo off
setlocal enabledelayedexpansion

echo [BUILD] Starting build process for FreeARDU...

:: Simple loading bar simulation
set "bar="
set "fill=####################"
set "empty=                    "

for /L %%i in (1,1,20) do (
    set "progress=!fill:~0,%%i!"
    set "remaining=!empty:~%%i!"
    echo | set /p "=[!progress!!remaining!] %%i%%" 
    ping -n 1 127.0.0.1 >nul
    echo | set /p "= "
)
echo.

echo [BUILD] Running PlatformIO build...
cd FreeARDU
pio run
if %ERRORLEVEL% equ 0 (
    echo [SUCCESS] Build finished successfully.
) else (
    echo [ERROR] Build failed.
)
cd ..
pause
