/**
 * @file packet_process.h
 * @brief Định nghĩa các hàm xử lý gói tin và quản lý thread pool
 */

#ifndef PACKET_PROCESS_H
#define PACKET_PROCESS_H

#include "parser_data.h"
#include "tc.h"  // Add this include to access test_result_info_t

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
 * @param capacity Kích thước tối đa của queue
 * @return task_queue_t* Con trỏ đến queue đã khởi tạo hoặc NULL nếu thất bại
 */
task_queue_t* init_task_queue(int capacity);

/**
 * @brief Thêm một task vào queue
 * 
 * @param queue Con trỏ đến queue
 * @param test_case Con trỏ đến test case
 * @return int 0 nếu thành công, -1 nếu thất bại
 */
int enqueue_task(task_queue_t *queue, test_case_t *test);

/**
 * @brief Thêm tất cả test cases vào queue
 * 
 * @param queue Con trỏ đến queue
 * @param test_cases Mảng test cases
 * @param count Số lượng test cases
 * @return int Số lượng test cases đã thêm vào queue
 */
int enqueue_test_cases(task_queue_t *queue, test_case_t *tests, int count);

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
thread_pool_t* init_thread_pool(int threads, task_queue_t *queue);

/**
 * @brief Dừng thread pool và giải phóng tài nguyên
 * 
 * @param thread_pool Con trỏ đến thread pool
 */
void stop_thread_pool(thread_pool_t *pool);

/**
 * @brief Lấy số lượng task đã hoàn thành
 * 
 * @param thread_pool Con trỏ đến thread pool
 * @return int Số lượng task đã hoàn thành
 */
int get_completed_count(thread_pool_t *pool);

/**
 * @brief Lấy số lượng task thành công
 * 
 * @param thread_pool Con trỏ đến thread pool
 * @return int Số lượng task thành công
 */
int get_success_count(thread_pool_t *pool);

/**
 * @brief Lấy số lượng task thất bại
 * 
 * @param thread_pool Con trỏ đến thread pool
 * @return int Số lượng task thất bại
 */
int get_failed_count(thread_pool_t *pool);

/**
 * @brief Lấy mảng kết quả test từ thread pool
 * 
 * @param pool Con trỏ đến thread pool
 * @param count Con trỏ để lưu số lượng kết quả
 * @return test_result_info_t* Mảng kết quả test
 */
test_result_info_t* get_results(thread_pool_t *pool, int *count);

#endif // PACKET_PROCESS_H