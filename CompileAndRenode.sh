#!/bin/bash

# ANSI Colors
C_RESET="\033[0m"
C_CYAN="\033[1;36m"
C_GREEN="\033[1;32m"
C_RED="\033[1;31m"
C_YELLOW="\033[1;33m"

echo -e "${C_CYAN}[$(date +%T)] [INFO] Starting Compile and Renode process...${C_RESET}"

# 1. Compile
echo -e "${C_CYAN}[$(date +%T)] [BUILD] Calling build.sh debug...${C_RESET}"
chmod +x build.sh
./build.sh debug
if [ $? -ne 0 ]; then
    echo -e "${C_RED}[$(date +%T)] [ERROR] Build failed. Aborting Renode.${C_RESET}"
    exit 1
fi

# 2. Find Renode
RENODE_BIN=$(which renode 2>/dev/null)

if [ -z "$RENODE_BIN" ]; then
    # Check common Linux/macOS paths
    if [ -f "/opt/renode/bin/renode" ]; then
        RENODE_BIN="/opt/renode/bin/renode"
    elif [ -f "/usr/bin/renode" ]; then
        RENODE_BIN="/usr/bin/renode"
    elif [ -f "/usr/local/bin/renode" ]; then
        RENODE_BIN="/usr/local/bin/renode"
    elif [ -d "/Applications/Renode.app" ]; then
        RENODE_BIN="/Applications/Renode.app/Contents/MacOS/Renode"
    else
        echo -e "${C_RED}[$(date +%T)] [ERROR] Renode not found in PATH or default locations.${C_RESET}"
        read -p "Please enter the full path to renode: " RENODE_BIN
    fi
fi

if [ ! -f "$RENODE_BIN" ] && [ ! -x "$RENODE_BIN" ]; then
    echo -e "${C_RED}[$(date +%T)] [ERROR] Renode binary not found or not executable: $RENODE_BIN${C_RESET}"
    exit 1
fi

echo -e "${C_GREEN}[$(date +%T)] [INFO] Found Renode: $RENODE_BIN${C_RESET}"

# 3. Run Renode with commands
ELF_PATH="$(pwd)/.pio/build/mimxrt1060_evk/firmware.elf"

echo -e "${C_CYAN}[$(date +%T)] [RENODE] Launching Renode with firmware: $ELF_PATH${C_RESET}"

"$RENODE_BIN" -e "mach create; machine LoadPlatformDescription @platforms/boards/mimxrt1060_evk.repl; sysbus LoadELF @$ELF_PATH; cpu PC \`sysbus GetSymbolAddress \"reset_handler\"\`; showAnalyzer sysbus.lpuart1; start"
