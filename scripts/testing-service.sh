#!/bin/bash

export USE_PLUGINS=1
echo "Plugin system enabled" >> "$LOG_DIR/service.log"

INPUT_DIR="/home/tobie/testing_device/var/input"
OUTPUT_DIR="/home/tobie/testing_device/var/results"
LOG_DIR="/home/tobie/testing_device/var/log"
BINARY="/home/tobie/testing_device/bin/testing_device"

# Tạo các thư mục cần thiết
mkdir -p "$INPUT_DIR" "$OUTPUT_DIR" "$LOG_DIR"

# Thiết lập log rotation
echo "Starting testing device service at $(date)" >> "$LOG_DIR/service.log"

# Hàm kiểm tra tính hợp lệ của JSON
validate_json() {
    local file="$1"
    if [ ! -s "$file" ]; then
        echo "Error: File $file is empty" >> "$LOG_DIR/service.log"
        return 1
    fi
    
    # Thử parse với python trước
    python3 -c "import json; json.load(open('$file'))" 2>/dev/null
    local python_result=$?
    
    # Thử parse với jq nếu có
    if command -v jq &> /dev/null; then
        jq . "$file" >/dev/null 2>&1
        local jq_result=$?
        
        # Nếu cả hai đều thất bại, thì file không phải JSON hợp lệ
        if [ $python_result -ne 0 ] && [ $jq_result -ne 0 ]; then
            echo "Error: Invalid JSON in $file (failed both python and jq validation)" >> "$LOG_DIR/service.log"
            return 1
        fi
    else
        # Nếu không có jq, chỉ dựa vào kết quả python
        if [ $python_result -ne 0 ]; then
            echo "Error: Invalid JSON in $file (failed python validation)" >> "$LOG_DIR/service.log"
            return 1
        fi
    fi
    
    return 0
}

# Hàm tạo JSON lỗi hợp lệ
create_error_json() {
    local output_file="$1"
    local error_msg="$2"
    
    # Escape các ký tự đặc biệt trong error_msg
    error_msg=$(echo "$error_msg" | sed 's/\\/\\\\/g' | sed 's/"/\\"/g')
    
    echo "{\"error\": \"$error_msg\"}" > "$output_file"
    
    # Kiểm tra lại file vừa tạo
    if ! validate_json "$output_file"; then
        echo "Critical: Failed to create valid error JSON file" >> "$LOG_DIR/service.log"
        # Thử tạo lại một JSON đơn giản nhất
        echo "{\"error\":\"Unknown error\"}" > "$output_file"
    fi
}

# Vòng lặp vô hạn để giám sát thư mục input
while true; do
    # Tìm các file .json mới trong thư mục input
    for config_file in "$INPUT_DIR"/*.json; do
        if [ -f "$config_file" ]; then
            filename=$(basename "$config_file")
            timestamp=$(date +"%Y%m%d_%H%M%S")
            result_file="$OUTPUT_DIR/${filename%.*}_$timestamp.json"
            
            echo "Processing $filename at $timestamp" >> "$LOG_DIR/service.log"
            
            # Kiểm tra xem input file có phải JSON hợp lệ không
            if ! validate_json "$config_file"; then
                echo "Warning: Input file $filename is not valid JSON" >> "$LOG_DIR/service.log"
                create_error_json "$result_file" "Input file is not valid JSON"
                
                # Di chuyển file lỗi vào thư mục processed
                mkdir -p "$INPUT_DIR/processed"
                mv "$config_file" "$INPUT_DIR/processed/${filename%.*}_invalid_$timestamp.json"
                continue
            fi
            
            # Tạo file tạm để lưu kết quả
            temp_result_file="/tmp/result_$timestamp.json"
            
            # Thực thi test case và lưu kết quả vào file tạm, cũng lưu stderr vào log
            echo "Running: $BINARY '$config_file' > '$temp_result_file'" >> "$LOG_DIR/service.log"
            $BINARY "$config_file" > "$temp_result_file" 2>> "$LOG_DIR/service.log"
            exit_code=$?
            
            echo "Binary exit code: $exit_code" >> "$LOG_DIR/service.log"
            
            # Kiểm tra xem file kết quả có tồn tại và có nội dung không
            if [ ! -s "$temp_result_file" ]; then
                echo "Error: Empty result file" >> "$LOG_DIR/service.log"
                create_error_json "$result_file" "Empty result from test execution"
            elif [ $exit_code -ne 0 ] || ! validate_json "$temp_result_file"; then
                # Log nội dung của file không hợp lệ để debug
                echo "Error: Invalid JSON output for $filename (exit code: $exit_code)" >> "$LOG_DIR/service.log"
                echo "Content of invalid file (first 200 bytes):" >> "$LOG_DIR/service.log"
                head -c 200 "$temp_result_file" >> "$LOG_DIR/service.log"
                echo "" >> "$LOG_DIR/service.log"
                
                # Tạo file JSON lỗi hợp lệ thay thế
                create_error_json "$result_file" "Failed to execute test or invalid JSON output"
            else
                # Nếu hợp lệ, copy kết quả vào thư mục output
                cp "$temp_result_file" "$result_file"
                echo "Valid JSON saved to $result_file" >> "$LOG_DIR/service.log"
            fi
            
            # Xóa file tạm
            rm -f "$temp_result_file"
            
            # Di chuyển file đã xử lý vào thư mục processed
            mkdir -p "$INPUT_DIR/processed"
            mv "$config_file" "$INPUT_DIR/processed/${filename%.*}_$timestamp.json"
        fi
    done
    
    # Nghỉ 5 giây trước khi kiểm tra lại
    sleep 5
done
