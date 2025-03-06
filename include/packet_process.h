/**
 * @file packet_process.h
 * @brief Thread pool and queue management for parallel test execution
 *
 * This module manages a thread pool and distributed in-memory queue system
 * for parallel execution of test cases, optimized for mass batch processing.
 */

 #ifndef PACKET_PROCESS_H
 #define PACKET_PROCESS_H
 
 #include <stdint.h>
 #include <pthread.h>
 #include "../include/parser_data.h"  // For test_case_t
 
 /**
  * @brief Task status values
  */
 typedef enum {
     TASK_STATUS_PENDING,   /**< Task is waiting to be processed */
     TASK_STATUS_RUNNING,   /**< Task is currently being processed */
     TASK_STATUS_COMPLETED, /**< Task has been successfully completed */
     TASK_STATUS_FAILED,    /**< Task failed during processing */
     TASK_STATUS_TIMEOUT    /**< Task timed out during processing */
 } task_status_t;
 
 /**
  * @brief Task structure for the processing queue
  */
 typedef struct {
     test_case_t test_case;  /**< Test case to process */
     task_status_t status;   /**< Current status of the task */
     uint64_t start_time;    /**< Timestamp when processing started */
     uint64_t end_time;      /**< Timestamp when processing completed */
     int result_code;        /**< Result code after processing */
     char result_message[256]; /**< Result message after processing */
 } task_t;
 
 /**
  * @brief Initialize the thread pool for test processing
  *
  * @param thread_count Number of threads to create
  * @param queue_size Maximum queue size
  * @return int 0 on success, negative value on error
  */
 int init_thread_pool(int thread_count, int queue_size);
 
 /**
  * @brief Clean up thread pool resources
  *
  * @return int 0 on success, negative value on error
  */
 int cleanup_thread_pool(void);
 
 /**
  * @brief Adjust thread pool size at runtime
  *
  * @param new_thread_count New number of threads
  * @return int 0 on success, negative value on error
  */
 int adjust_thread_pool_size(int new_thread_count);
 
 /**
  * @brief Adjust task queue size at runtime
  *
  * @param new_queue_size New maximum queue size
  * @return int 0 on success, negative value on error
  */
 int adjust_task_queue_size(int new_queue_size);
 
 /**
  * @brief Enqueue a task to the processing queue
  *
  * @param task Task to enqueue
  * @return int 0 on success, negative value on error
  */
 int enqueue_task(const task_t* task);
 
 /**
  * @brief Process queued test cases
  *
  * This function is called by worker threads to process test cases
  * from the queue.
  *
  * @param thread_id ID of the worker thread
  * @return void* Always NULL
  */
 void* process_queued_test_cases(void* thread_id);
 
 /**
  * @brief Get the current progress of the queue
  *
  * @param processed Pointer to store number of processed tasks
  * @param total Pointer to store total number of tasks
  * @return int 0 on success, negative value on error
  */
 int get_queue_progress(uint64_t* processed, uint64_t* total);
 
 /**
  * @brief Log the current progress of the queue
  *
  * @param batch_id Batch identifier
  * @return int 0 on success, negative value on error
  */
 int log_queue_progress(const char* batch_id);
 
 /**
  * @brief Wait for all tasks to be processed
  *
  * @param timeout_ms Maximum time to wait in milliseconds (0 for infinite)
  * @return int 0 if all tasks completed, 1 if timeout, negative value on error
  */
 int wait_for_queue_completion(uint64_t timeout_ms);
 
 /**
  * @brief Get current queue statistics
  *
  * @param pending Pointer to store number of pending tasks
  * @param running Pointer to store number of running tasks
  * @param completed Pointer to store number of completed tasks
  * @param failed Pointer to store number of failed tasks
  * @return int 0 on success, negative value on error
  */
 int get_queue_stats(uint64_t* pending, uint64_t* running, 
                    uint64_t* completed, uint64_t* failed);
 
 #endif /* PACKET_PROCESS_H */