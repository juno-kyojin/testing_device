#!/bin/bash

# Hiển thị kết quả của tất cả các action từ file JSON

RESULT_DIR="/home/tobie/testing_device/var/results"

# Lấy file kết quả mới nhất nếu không có đối số
if [ $# -eq 0 ]; then
    RESULT_FILE=$(ls -t $RESULT_DIR/*.json | head -1)
else
    RESULT_FILE="$1"
fi

# Kiểm tra file tồn tại
if [ ! -f "$RESULT_FILE" ]; then
    echo "Error: Result file not found: $RESULT_FILE"
    exit 1
fi

echo "=== Test Results from: $(basename $RESULT_FILE) ==="
echo ""

# Kiểm tra xem jq có được cài đặt không
if command -v jq &> /dev/null; then
    # Duyệt qua tất cả các action
    actions=$(jq -r '.test_results[].action' "$RESULT_FILE" | sort -u)
    
    for action in $actions; do
        echo "--- ${action^^} RESULTS ---"
        if [ "$action" = "ping" ]; then
            jq -r --arg act "$action" '.test_results[] | select(.action==$act) | .results | "Status: \(.pingCode)\nHost: \(.host)\nAvg Response Time: \(.averageResponseTime) ms\nDetails: \(.details)"' "$RESULT_FILE"
        elif [ "$action" = "speedtest" ]; then
            jq -r --arg act "$action" '.test_results[] | select(.action==$act) | .results | "Status: \(.speedtestCode)\nDownload: \(.downloadSpeed) Mbps\nUpload: \(.uploadSpeed) Mbps\nLatency: \(.latency) ms\nDetails: \(.details)"' "$RESULT_FILE"
        else
            # Xử lý các action khác với định dạng chung
            jq -r --arg act "$action" '.test_results[] | select(.action==$act) | .results | "Status: \(.status // .code // "N/A")\nDetails: \(.details // "No details available")\nRaw Results: \(tostring)"' "$RESULT_FILE"
        fi
        echo ""
    done
else
    echo "jq not installed. Please install with: sudo apt-get install jq"
    echo "Showing raw JSON content:"
    cat "$RESULT_FILE"
    exit 1
fi