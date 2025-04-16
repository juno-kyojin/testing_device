#!/bin/bash

# Đường dẫn
SOURCE_FILE="/home/tobie/testing_device/plugins/minimal_ping.c"
OUTPUT_FILE="/home/tobie/testing_device/plugins/minimal_ping.so"
PROJECT_DIR="/home/tobie/testing_device"
INCLUDE_DIR="${PROJECT_DIR}/include"

echo "Compiling minimal plugin..."

# Tạo các thư mục cần thiết
mkdir -p "${PROJECT_DIR}/plugins"
mkdir -p "${PROJECT_DIR}/var/log"

# Xóa file log debug cũ và tạo mới
touch "${PROJECT_DIR}/var/log/minimal_plugin.log"
chmod 666 "${PROJECT_DIR}/var/log/minimal_plugin.log"

# Biên dịch plugin với -g để debug
gcc -g -Wall -fPIC -shared -o "$OUTPUT_FILE" "$SOURCE_FILE" \
    -I"$INCLUDE_DIR" -lcjson

if [ $? -eq 0 ]; then
    echo "Minimal plugin compiled successfully: $OUTPUT_FILE"
    chmod 755 "$OUTPUT_FILE"
else
    echo "Compilation failed!"
    exit 1
fi

echo "Setup complete. Run with:"
echo "USE_PLUGINS=1 ./bin/testing_device ./config/config1.json"
exit 0
