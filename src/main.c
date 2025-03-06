#include "log.h"
#include "parser_option.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
    // Khởi tạo logger
    init_logger();

    // Phân tích tùy chọn dòng lệnh
    parsers_option(argc, argv);

    // Ghi log khởi động
    log_with_detailed_timestamp(LOG_LVL_DEBUG, "Device started successfully");

    // Chạy hệ thống (gọi các module khác, ví dụ: Test Case Manager)
    // (Triển khai thêm logic ở đây dựa trên pipeline)

    // Dọn dẹp khi kết thúc
    cleanup_logger();
    return 0;
}