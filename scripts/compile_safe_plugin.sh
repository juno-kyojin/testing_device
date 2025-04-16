#!/bin/bash

# Đường dẫn
SOURCE_FILE="/home/tobie/testing_device/plugins/safe_ping_plugin.c"
OUTPUT_FILE="/home/tobie/testing_device/plugins/safe_ping_plugin.so"
PROJECT_DIR="/home/tobie/testing_device"
INCLUDE_DIR="${PROJECT_DIR}/include"

echo "Compiling safe plugin..."

# Tạo các thư mục cần thiết
mkdir -p "${PROJECT_DIR}/plugins"
mkdir -p "${PROJECT_DIR}/var/log"

# Xóa file log debug cũ
rm -f "${PROJECT_DIR}/var/log/safe_plugin_debug.log"
touch "${PROJECT_DIR}/var/log/safe_plugin_debug.log"
chmod 666 "${PROJECT_DIR}/var/log/safe_plugin_debug.log"

# Biên dịch plugin
gcc -Wall -fPIC -shared -o "$OUTPUT_FILE" "$SOURCE_FILE" \
    -I"$INCLUDE_DIR" -lcjson

if [ $? -eq 0 ]; then
    echo "Safe plugin compiled successfully: $OUTPUT_FILE"
    chmod 755 "$OUTPUT_FILE"
    
    # Thêm các plugin khác nếu có
    rm -f "${PROJECT_DIR}/plugins/ping_plugin.so"
    rm -f "${PROJECT_DIR}/plugins/minimal_ping_plugin.so"
    rm -f "${PROJECT_DIR}/plugins/debug_ping_plugin.so"
else
    echo "Compilation failed!"
    exit 1
fi

echo "Setup complete. Run with:"
echo "USE_PLUGINS=1 ./bin/testing_device ./config/config1.json"
exit 0
