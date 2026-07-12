#!/bin/bash

# ANSI Colors
C_RESET='\033[0m'
C_CYAN='\033[1;36m'
C_GREEN='\033[1;32m'
C_RED='\033[1;31m'
C_YELLOW='\033[1;33m'
C_MAGENTA='\033[1;35m'

get_time() {
    date +"%H:%M:%S"
}

BUILD_TYPE="debug"
EXTRA_ARGS=""
CLEAN_FIRST=0

# Parse arguments
for arg in "$@"; do
    case $arg in
        release)
            BUILD_TYPE="release"
            EXTRA_ARGS="-e mimxrt1060_evk_release"
            ;;
        debug)
            BUILD_TYPE="debug"
            EXTRA_ARGS="-e mimxrt1060_evk"
            ;;
        clean)
            CLEAN_FIRST=1
            ;;
    esac
done

echo -e "${C_CYAN}[$(get_time)] [BUILD] Starting FreeARDU build ($BUILD_TYPE)...${C_RESET}"
echo ""

# Better animated loading bar
for i in {0..100..10}; do
    filled=$((i / 5))
    empty=$((20 - filled))
    bar=$(printf "%${filled}s" | tr ' ' '#')
    spaces=$(printf "%${empty}s" | tr ' ' '-')
    printf "\r${C_MAGENTA}Initializing...${C_RESET} ${C_YELLOW}[%s%s] %d%%${C_RESET}" "$bar" "$spaces" "$i"
    sleep 0.1
done
echo -e "\n"

# Navigate to the root directory
cd "$(dirname "$0")"

if [ $CLEAN_FIRST -eq 1 ]; then
    echo -e "${C_YELLOW}[$(get_time)] [BUILD] Cleaning previous builds...${C_RESET}"
    pio run --target clean $EXTRA_ARGS > /dev/null 2>&1
fi

echo -e "${C_CYAN}[$(get_time)] [BUILD] Running PlatformIO build ($BUILD_TYPE)...${C_RESET}"

pio run $EXTRA_ARGS
if [ $? -eq 0 ]; then
    echo -e "\n${C_GREEN}[$(get_time)] [SUCCESS] Build finished successfully.${C_RESET}"
    echo ""
    echo -ne "${C_YELLOW}[UPLOAD] Do you want to upload the firmware? [y/N] ${C_RESET}"
    read -r UPLOAD_CHOICE
    
    if [[ "$UPLOAD_CHOICE" =~ ^[Yy]$ ]]; then
        echo -e "${C_CYAN}[$(get_time)] [UPLOAD] Starting firmware upload...${C_RESET}"
        pio upload $EXTRA_ARGS
        if [ $? -eq 0 ]; then
            echo -e "${C_GREEN}[$(get_time)] [SUCCESS] Upload finished successfully.${C_RESET}"
        else
            echo -e "${C_RED}[$(get_time)] [ERROR] Upload failed.${C_RESET}"
        fi
    else
        echo -e "${C_MAGENTA}[$(get_time)] [UPLOAD] Upload skipped by user.${C_RESET}"
    fi
else
    echo -e "\n${C_RED}[$(get_time)] [ERROR] Build failed.${C_RESET}"
fi
