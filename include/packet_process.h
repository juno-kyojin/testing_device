#ifndef PACKET_PROCESS_H
#define PACKET_PROCESS_H

#include "parser_data.h"
#include <pthread.h>

#define THREAD_POOL_SIZE 4  // Số thread cố định trong pool
#define TASK_QUEUE_SIZE 100 // Giới hạn hàng đợi task

// Cấu trúc cho task trong task queue
typedef struct {
    test_case_t *test_case;
    int is_valid;  // Đánh dấu task có hợp lệ hay không
} task_t;

// Đọc test case từ FIFO queue và thực thi bằng thread pool
void process_queued_test_cases();

// Khởi tạo thread pool
void init_thread_pool();

// Hàm worker cho thread trong pool
void *thread_pool_worker(void *arg);

#endif // PACKET_PROCESS_H