#!/bin/bash
# Renode launcher script for NXP i.MX RT1060 EVK
# This script launches Renode with the FreeARDU firmware

set -e

# Get script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Default firmware path
FIRMWARE_PATH="$SCRIPT_DIR/.pio/build/mimxrt1060_evk/firmware.elf"

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        release)
            FIRMWARE_PATH="$SCRIPT_DIR/.pio/build/mimxrt1060_evk_release/firmware.elf"
            shift
            ;;
        debug)
            FIRMWARE_PATH="$SCRIPT_DIR/.pio/build/mimxrt1060_evk/firmware.elf"
            shift
            ;;
        firmware=*)
            FIRMWARE_PATH="${1#firmware=}"
            shift
            ;;
        *)
            shift
            ;;
    esac
done

echo "============================================"
echo "  Renode - FreeARDU i.MX RT1060 Emulator"
echo "============================================"
echo

# Check if firmware exists
if [[ ! -f "$FIRMWARE_PATH" ]]; then
    echo "ERROR: Firmware not found at: $FIRMWARE_PATH"
    echo "Please build the project first: platformio run -e mimxrt1060_evk"
    exit 1
fi

echo "Using firmware: $FIRMWARE_PATH"
echo "Starting Renode..."
echo

# Check if renode is installed
if ! command -v renode &> /dev/null && ! command -v renode_dotnet &> /dev/null; then
    echo "ERROR: Renode is not installed or not in PATH"
    echo "Please install Renode from: https://renode.io/"
    exit 1
fi

# Use renode_dotnet if available (common on Windows), otherwise use renode
RENODE_CMD="renode"
if command -v renode_dotnet &> /dev/null; then
    RENODE_CMD="renode_dotnet"
fi

# Create temporary RESC file with the correct firmware path
TEMP_RESC=$(mktemp)

cat > "$TEMP_RESC" << RESC
# Renode RESC script for NXP i.MX RT1060 EVK
mach create "mimxrt1060"
machine LoadPlatformDescription @platforms/boards/mimxrt1060_evk.repl
sysbus LoadELF "$FIRMWARE_PATH"
cpu PC \`sysbus GetSymbolAddress "reset_handler"\`
showAnalyzer sysbus.lpuart1
start
log "Starting emulation..."
RESC

# Launch Renode with the RESC file
"$RENODE_CMD" "$TEMP_RESC"

# Cleanup
rm -f "$TEMP_RESC"

echo
echo "Renode session ended."
