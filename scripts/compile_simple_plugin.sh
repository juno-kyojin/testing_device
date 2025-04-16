#!/bin/bash

# Đường dẫn tới source file
SOURCE_FILE="/home/tobie/testing_device/plugins/simple_ping_plugin.c"
OUTPUT_FILE="/home/tobie/testing_device/plugins/simple_ping_plugin.so"

# Thư mục dự án
PROJECT_DIR="/home/tobie/testing_device"
INCLUDE_DIR="${PROJECT_DIR}/include"

echo "Compiling simple plugin for debugging..."

# Biên dịch plugin đơn giản với -g để debug
gcc -shared -fPIC -g -o "$OUTPUT_FILE" "$SOURCE_FILE" \
    -I"$INCLUDE_DIR" -ldl -lcjson

if [ $? -eq 0 ]; then
    echo "Plugin compiled successfully: $OUTPUT_FILE"
    # Kiểm tra xem shared library có hàm register_plugin không
    if nm -D "$OUTPUT_FILE" | grep -q register_plugin; then
        echo "register_plugin function found in the shared library."
    else
        echo "WARNING: register_plugin function NOT found in the shared library!"
    fi
else
    echo "Compilation failed!"
    exit 1
fi

exit 0
