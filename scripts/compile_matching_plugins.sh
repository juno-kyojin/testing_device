#!/bin/bash

# Đường dẫn thư mục dự án
PROJECT_DIR="/home/tobie/testing_device"
PLUGIN_DIR="${PROJECT_DIR}/plugins"
INCLUDE_DIR="${PROJECT_DIR}/include"
LOG_DIR="${PROJECT_DIR}/var/log"

# Đảm bảo các thư mục tồn tại
mkdir -p "$PLUGIN_DIR"
mkdir -p "$LOG_DIR"

echo "Compiling plugins to match config file names..."

# Xóa tất cả plugin cũ nếu cần
rm -f "$PLUGIN_DIR"/*.so

# Biên dịch Debug Ping Plugin
echo "Compiling Debug Ping Plugin..."
gcc -g -Wall -fPIC -shared -o "$PLUGIN_DIR/debug_ping_plugin.so" "$PLUGIN_DIR/debug_ping_plugin.c" \
    -I"$INCLUDE_DIR" -lcjson

# Tạo log file
touch "$LOG_DIR/debug_ping.log"
chmod 666 "$LOG_DIR/debug_ping.log"

# Biên dịch Speedtest Plugin
echo "Compiling Speedtest Plugin..."
gcc -g -Wall -fPIC -shared -o "$PLUGIN_DIR/speedtest_plugin.so" "$PLUGIN_DIR/speedtest_plugin.c" \
    -I"$INCLUDE_DIR" -lcjson

# Kiểm tra kết quả
if [ -f "$PLUGIN_DIR/debug_ping_plugin.so" ] && [ -f "$PLUGIN_DIR/speedtest_plugin.so" ]; then
    echo "Plugins compiled successfully!"
    echo "- ${PLUGIN_DIR}/debug_ping_plugin.so"
    echo "- ${PLUGIN_DIR}/speedtest_plugin.so"
    
    # Đảm bảo quyền thực thi
    chmod 755 "$PLUGIN_DIR"/*.so
    
    echo "Run with: USE_PLUGINS=1 ./bin/testing_device ./config/config1.json"
else
    echo "Compilation failed!"
    exit 1
fi

exit 0
