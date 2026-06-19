#!/bin/bash
set -e
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$SCRIPT_DIR/../firmware"

echo "Uploading photos to ESP32 LittleFS..."
cd "$PROJECT_DIR"
~/.platformio/penv/bin/pio run -t uploadfs
echo "Done. Disconnect and reconnect power if photos don't appear."
