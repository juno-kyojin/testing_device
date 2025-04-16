#!/bin/bash

# Đường dẫn
SOURCE_FILE="/home/tobie/testing_device/plugins/debug_ping_plugin.c"
OUTPUT_FILE="/home/tobie/testing_device/plugins/debug_ping_plugin.so"
PROJECT_DIR="/home/tobie/testing_device"
INCLUDE_DIR="${PROJECT_DIR}/include"

echo "Compiling debug plugin..."

# Create output directories if they don't exist
mkdir -p "${PROJECT_DIR}/plugins"
mkdir -p "${PROJECT_DIR}/var/log"

# Remove old plugin if it exists
rm -f "$OUTPUT_FILE"

# Compile with minimal dependencies to avoid issues
gcc -shared -fPIC -g -o "$OUTPUT_FILE" "$SOURCE_FILE" \
    -I"$INCLUDE_DIR" -ldl -lcjson

if [ $? -eq 0 ]; then
    echo "Debug plugin compiled successfully: $OUTPUT_FILE"
    chmod 755 "$OUTPUT_FILE"
else
    echo "Compilation failed!"
    exit 1
fi

touch "${PROJECT_DIR}/var/log/debug_plugin.log"
chmod 666 "${PROJECT_DIR}/var/log/debug_plugin.log"

echo "Debug setup complete."
exit 0
