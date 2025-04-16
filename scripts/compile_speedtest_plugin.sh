#!/bin/bash

# Đường dẫn
SOURCE_FILE="/home/tobie/testing_device/plugins/speedtest_plugin.c"
OUTPUT_FILE="/home/tobie/testing_device/plugins/speedtest_plugin.so"
PROJECT_DIR="/home/tobie/testing_device"
INCLUDE_DIR="${PROJECT_DIR}/include"
LOG_DIR="${PROJECT_DIR}/var/log"

echo "Compiling Speedtest Plugin..."

# Đảm bảo thư mục log tồn tại
mkdir -p "$LOG_DIR"

# Xóa file cũ nếu có
rm -f "$OUTPUT_FILE"

# Tạo log file và đặt quyền
touch "$LOG_DIR/speedtest_plugin.log"
chmod 666 "$LOG_DIR/speedtest_plugin.log"

# Biên dịch plugin
gcc -g -Wall -fPIC -shared -o "$OUTPUT_FILE" "$SOURCE_FILE" \
    -I"$INCLUDE_DIR" -lcjson

if [ $? -eq 0 ]; then
    echo "Speedtest plugin compiled successfully: $OUTPUT_FILE"
    chmod 755 "$OUTPUT_FILE"
    
    # Kiểm tra xem plugin có đầy đủ symbols cần thiết không
    echo "Checking exported symbols..."
    nm -D "$OUTPUT_FILE" | grep "register_plugin"
    
    # Kiểm tra các dependencies
    echo "Checking dependencies..."
    ldd "$OUTPUT_FILE"
    
    echo "Setup complete."
else
    echo "Compilation failed!"
    exit 1
fi

echo "Run with: USE_PLUGINS=1 ./bin/testing_device ./config/config1.json"
exit 0
