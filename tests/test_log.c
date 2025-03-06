#include "log.h"
#include "file_process.h"  // Đảm bảo include để sử dụng read_file
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    // Khởi tạo logger
    init_logger();
    printf("Logger initialized. Starting test...\n");

    // Tạo thư mục logs nếu chưa có
    system("mkdir -p logs");

    // Test log_set_level
    log_set_level(LOG_LVL_DEBUG);
    printf("Set log level to DEBUG\n");

    // Test printf_log (đồng bộ)
    printf_log(LOG_LVL_DEBUG, "This is a debug message");
    printf_log(LOG_LVL_ERROR, "This is an error message");
    printf_log(LOG_LVL_WARN, "This is a warning message");
    printf("Logged synchronous messages\n");
    sleep(1); // Chờ ghi

    // Test log_async (không đồng bộ)
    log_async(LOG_LVL_DEBUG, "This is an async debug message");
    sleep(2); // Chờ thread flush
    printf("Logged asynchronous message\n");

    // Test log_with_detailed_timestamp
    log_with_detailed_timestamp(LOG_LVL_DEBUG, "Timestamped debug message");
    printf("Logged with detailed timestamp\n");

    // Test log_by_module
    log_by_module(LOG_LVL_DEBUG, "TestModule", "LAN", "Module test message");
    printf("Logged by module with network type\n");

    // Test log_progress
    log_progress(75, "Test progress message");
    printf("Logged progress 75%%\n");

    // Test log_system_resources
    log_system_resources(60, 70, 1024 * 1024 * 50); // CPU: 60%, RAM: 70%, Disk: 50MB
    printf("Logged system resources\n");

    // Test set_log_batch_size và log_batch_flush
    set_log_batch_size(3);
    for (int i = 0; i < 4; i++) {
        printf_log(LOG_LVL_DEBUG, "Batch test message %d", i);
    }
    log_batch_flush(LOG_FILE_PATH, 1); // Buộc flush
    printf("Set batch size to 3 and forced flush\n");

    // Test rotate_log_file (giả lập bằng cách thêm dữ liệu)
    for (int i = 0; i < 1000; i++) {
        printf_log(LOG_LVL_DEBUG, "Filling log %d", i);
    }
    rotate_log_file(LOG_FILE_PATH, "logs/log_backup.txt");
    printf("Tested log rotation\n");

    // Test rotate_log_by_time
    rotate_log_by_time(LOG_FILE_PATH, "logs/log_%Y-%m-%d.txt");
    printf("Tested log rotation by time\n");

    // Test send_realtime_log
    send_realtime_log("192.168.1.100", "user", LOG_LVL_DEBUG, "Realtime test message");
    printf("Sent realtime log to PC\n");

    // Dọn dẹp và kiểm tra
    cleanup_logger();
    printf("Logger cleanup completed. Check logs/log.txt, logs/log_backup.txt, and logs/log_YYYY-MM-DD.txt for results.\n");

    // Đọc và in nội dung log để kiểm tra
    char *log_content = read_file(LOG_FILE_PATH);
    if (log_content) {
        printf("Log content:\n%s\n", log_content);
        free(log_content);
    } else {
        printf("Failed to read log file\n");
    }

    return 0;
}