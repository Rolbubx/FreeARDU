# Renode launcher script for NXP i.MX RT1060 EVK (PowerShell)
# This script launches Renode with the FreeARDU firmware

Write-Host "============================================"
Write-Host "  Renode - FreeARDU i.MX RT1060 Emulator"
Write-Host "============================================"
Write-Host ""

# Get script directory
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path

# Default firmware path
$FirmwarePath = Join-Path $ScriptDir ".pio\build\mimxrt1060_evk\firmware.elf"

# Parse command line arguments
param(
    [string]$Firmware = $null
)

if ($Firmware) {
    $FirmwarePath = $Firmware
}
elseif ($args[0] -eq "release") {
    $FirmwarePath = Join-Path $ScriptDir ".pio\build\mimxrt1060_evk_release\firmware.elf"
}
elseif ($args[0] -eq "debug") {
    $FirmwarePath = Join-Path $ScriptDir ".pio\build\mimxrt1060_evk\firmware.elf"
}

# Check if firmware exists
if (-not (Test-Path $FirmwarePath)) {
    Write-Host "ERROR: Firmware not found at: $FirmwarePath" -ForegroundColor Red
    Write-Host "Please build the project first: platformio run -e mimxrt1060_evk" -ForegroundColor Yellow
    Read-Host "Press Enter to exit"
    exit 1
}

Write-Host "Using firmware: $FirmwarePath"
Write-Host "Starting Renode..."
Write-Host ""

# Create temporary RESC file
$TempResc = [System.IO.Path]::GetTempFileName()

@"
# Renode RESC script for NXP i.MX RT1060 EVK
mach create "mimxrt1060"
machine LoadPlatformDescription @platforms/boards/mimxrt1060_evk.repl
sysbus LoadELF "$FirmwarePath"
cpu PC `sysbus GetSymbolAddress "reset_handler"`
showAnalyzer sysbus.lpuart1
start
print "Starting emulation..."
"@ | Out-File -FilePath $TempResc -Encoding ASCII

# Launch Renode
renode $TempResc

# Cleanup
Remove-Item -Path $TempResc -ErrorAction SilentlyContinue

Write-Host ""
Write-Host "Renode session ended."
Read-Host "Press Enter to exit"
