@echo off
setlocal EnableExtensions EnableDelayedExpansion

set "PROFILE=debug"

if not "%~1"=="" (
    if /i "%~1"=="debug" set "PROFILE=debug"
    if /i "%~1"=="release" set "PROFILE=release"
    if /i "%~1"=="/h" goto :usage
    if /i "%~1"=="-h" goto :usage
    if /i "%~1"=="help" goto :usage
)

call "%~dp0FreeARDU.bat" build-run "%PROFILE%"
exit /b %ERRORLEVEL%

:usage
echo FreeARDU_AutoRenode.bat
echo.
echo Builds the selected firmware profile and launches Renode automatically.
echo.
echo Usage:
echo   FreeARDU_AutoRenode.bat [debug ^| release]
echo.
exit /b 0
