/**
 * @file packet_process.h
 * @brief Định nghĩa các hàm xử lý gói tin và quản lý thread pool
 */

 #ifndef PACKET_PROCESS_H
 #define PACKET_PROCESS_H
 
 #include <stdbool.h>
 #include "parser_data.h"
 
 /**
  * @brief Cấu trúc dữ liệu cho task trong queue
  */
 typedef struct {
     test_case_t *test_case;       /**< Con trỏ đến test case cần thực thi */
     int priority;                 /**< Độ ưu tiên của task */
     void *extra_data;             /**< Dữ liệu bổ sung */
     size_t extra_data_size;       /**< Kích thước dữ liệu bổ sung */
 } task_t;
 
 /**
  * @brief Cấu trúc dữ liệu cho queue
  */
 typedef struct task_queue_t task_queue_t;
 
 /**
  * @brief Cấu trúc dữ liệu cho thread pool
  */
 typedef struct thread_pool_t thread_pool_t;
 
 /**
  * @brief Khởi tạo queue
  * 
  * @param max_size Kích thước tối đa của queue
  * @return task_queue_t* Con trỏ đến queue đã khởi tạo hoặc NULL nếu thất bại
  */
 task_queue_t* init_task_queue(int max_size);
 
 /**
  * @brief Thêm một task vào queue
  * 
  * @param queue Con trỏ đến queue
  * @param test_case Con trỏ đến test case
  * @param priority Độ ưu tiên của task
  * @return int 0 nếu thành công, -1 nếu thất bại
  */
 int enqueue_task(task_queue_t *queue, test_case_t *test_case, int priority);
 
 /**
  * @brief Lấy một task từ queue
  * 
  * @param queue Con trỏ đến queue
  * @return task_t* Con trỏ đến task đã lấy ra hoặc NULL nếu queue rỗng
  */
 task_t* dequeue_task(task_queue_t *queue);
 
 /**
  * @brief Giải phóng queue
  * 
  * @param queue Con trỏ đến queue cần giải phóng
  */
 void free_task_queue(task_queue_t *queue);
 
 /**
  * @brief Khởi tạo thread pool
  * 
  * @param thread_count Số lượng thread trong pool
  * @param queue Con trỏ đến task queue
  * @return thread_pool_t* Con trỏ đến thread pool đã khởi tạo hoặc NULL nếu thất bại
  */
 thread_pool_t* init_thread_pool(int thread_count, task_queue_t *queue);
 
 /**
  * @brief Thêm tất cả test cases vào queue
  * 
  * @param queue Con trỏ đến queue
  * @param test_cases Mảng test cases
  * @param count Số lượng test cases
  * @return int Số lượng test cases đã thêm vào queue
  */
 int enqueue_test_cases_to_queue(task_queue_t *queue, test_case_t *test_cases, int count);
 
 /**
  * @brief Xử lý các test cases trong queue
  * 
  * @param thread_pool Con trỏ đến thread pool
  * @param queue Con trỏ đến queue
  * @param total_count Số lượng test cases tổng cộng
  * @return int 0 nếu thành công, -1 nếu thất bại
  */
 int process_queued_test_cases(thread_pool_t *thread_pool, task_queue_t *queue, int total_count);
 
 /**
  * @brief Dừng thread pool và giải phóng tài nguyên
  * 
  * @param thread_pool Con trỏ đến thread pool
  * @return int 0 nếu thành công, -1 nếu thất bại
  */
 int stop_thread_pool(thread_pool_t *thread_pool);
 
 /**
  * @brief Lấy số lượng task đã hoàn thành
  * 
  * @param thread_pool Con trỏ đến thread pool
  * @return int Số lượng task đã hoàn thành
  */
 int get_completed_task_count(thread_pool_t *thread_pool);
 
 /**
  * @brief Lấy số lượng task thành công
  * 
  * @param thread_pool Con trỏ đến thread pool
  * @return int Số lượng task thành công
  */
 int get_success_task_count(thread_pool_t *thread_pool);
 
 /**
  * @brief Lấy số lượng task thất bại
  * 
  * @param thread_pool Con trỏ đến thread pool
  * @return int Số lượng task thất bại
  */
 int get_failed_task_count(thread_pool_t *thread_pool);
 
 #endif /* PACKET_PROCESS_H */