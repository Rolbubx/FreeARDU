#!/bin/bash

echo "[BUILD] Starting build process for FreeARDU..."

# Simple loading bar simulation
draw_bar() {
    local progress=$1
    local total=20
    local filled=$((progress * total / 100))
    local empty=$((total - filled))
    
    printf "\r["
    printf "%${filled}s" | tr ' ' '#'
    printf "%${empty}s" | tr ' ' ' '
    printf "] %d%%" "$progress"
}

for i in {0..100..5}; do
    draw_bar $i
    sleep 0.1
done
echo ""

echo "[BUILD] Running PlatformIO build..."
cd FreeARDU
pio run
if [ $? -eq 0 ]; then
    echo "[SUCCESS] Build finished successfully."
else
    echo "[ERROR] Build failed."
fi
cd ..
