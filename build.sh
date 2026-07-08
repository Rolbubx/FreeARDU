#!/bin/bash

echo "[BUILD] Starting build process for FreeARDU..."
echo ""

# Better animated loading bar
for i in {0..100..5}; do
    filled=$((i / 5))
    empty=$((20 - filled))
    bar=$(printf '%*s' $filled | tr ' ' '#')
    spaces=$(printf '%*s' $empty | tr ' ' '-')
    printf "\rInitializing... [%s%s] %d%%" "$bar" "$spaces" "$i"
done
printf "\n"
echo ""

echo "[BUILD] Running PlatformIO build..."

# Navigate to the root directory where platformio.ini is located
cd "$(dirname "$0")"

pio run
if [ $? -eq 0 ]; then
    echo "[SUCCESS] Build finished successfully."
    echo ""
    echo "[UPLOAD] Do you want to upload the firmware? [Y/N]"
    read -p "Enter choice: " UPLOAD_CHOICE
    
    if [[ "$UPLOAD_CHOICE" =~ ^[Yy]$ ]]; then
        echo "[UPLOAD] Starting firmware upload..."
        pio upload
        if [ $? -eq 0 ]; then
            echo "[SUCCESS] Upload finished successfully."
        else
            echo "[ERROR] Upload failed."
        fi
    else
        echo "[UPLOAD] Upload skipped by user."
    fi
else
    echo "[ERROR] Build failed."
fi
