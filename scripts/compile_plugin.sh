#!/bin/bash

# Kiểm tra tham số
if [ "$#" -lt 1 ]; then
    echo "Usage: $0 <plugin_source_file.c> [output_name]"
    exit 1
fi

SOURCE_FILE="$1"
BASE_NAME=$(basename "$SOURCE_FILE" .c)
OUTPUT_NAME="${2:-${BASE_NAME}.so}"

# Thư mục dự án
PROJECT_DIR="/home/tobie/testing_device"
PLUGIN_DIR="${PROJECT_DIR}/plugins"
INCLUDE_DIR="${PROJECT_DIR}/include"
LIB_DIR="${PROJECT_DIR}/lib"
OBJ_DIR="${PROJECT_DIR}/obj"

# Tạo thư mục plugins nếu chưa có
mkdir -p "$PLUGIN_DIR"

echo "Compiling plugin: $SOURCE_FILE -> $OUTPUT_NAME"

# Bước 1: Biên dịch thành object file trước
gcc -I"$INCLUDE_DIR" -fPIC -c "$SOURCE_FILE" -o "${OBJ_DIR}/${BASE_NAME}.o"

if [ $? -ne 0 ]; then
    echo "Error: Compilation failed"
    exit 1
fi

# Bước 2: Liên kết với các thư viện cần thiết
gcc -shared -o "${PLUGIN_DIR}/${OUTPUT_NAME}" "${OBJ_DIR}/${BASE_NAME}.o" \
    -L"$LIB_DIR" \
    "$OBJ_DIR/log.o" \
    "$OBJ_DIR/tc.o" \
    -ldl -lcjson -lm

if [ $? -ne 0 ]; then
    echo "Error: Linking failed"
    exit 1
fi

echo "Plugin compiled successfully: ${PLUGIN_DIR}/${OUTPUT_NAME}"

# Kiểm tra các symbols được xuất
echo "Checking exported symbols:"
nm -D "${PLUGIN_DIR}/${OUTPUT_NAME}" | grep "register_plugin"

exit 0
