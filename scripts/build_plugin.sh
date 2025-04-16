#!/bin/bash

# Script biên dịch plugin mới

# Kiểm tra tham số
if [ "$#" -lt 1 ]; then
    echo "Usage: $0 <plugin_source_file.c> [output_name]"
    echo "Example: $0 my_plugin.c"
    exit 1
fi

SOURCE_FILE="$1"
OUTPUT_NAME="${2:-${SOURCE_FILE%.c}.so}"

# Kiểm tra file nguồn
if [ ! -f "$SOURCE_FILE" ]; then
    echo "Error: Source file '$SOURCE_FILE' not found."
    exit 1
fi

# Thư mục include và lib
INCLUDE_DIR="/home/tobie/testing_device/include"
LIB_DIR="/home/tobie/testing_device/lib"

# Tạo thư mục plugins nếu chưa có
mkdir -p /home/tobie/testing_device/plugins

# Biên dịch plugin
echo "Compiling plugin: $SOURCE_FILE -> $OUTPUT_NAME"
gcc -shared -fPIC -o "/home/tobie/testing_device/plugins/$OUTPUT_NAME" "$SOURCE_FILE" \
    -I"$INCLUDE_DIR" -L"$LIB_DIR" -ldl -lcjson

# Kiểm tra kết quả
if [ $? -eq 0 ]; then
    echo "Plugin compiled successfully: /home/tobie/testing_device/plugins/$OUTPUT_NAME"
else
    echo "Error: Failed to compile plugin."
    exit 1
fi

exit 0
