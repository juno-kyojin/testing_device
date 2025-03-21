// #include "packet_process.h"
// #include "tc.h"
// #include "log.h"
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <pthread.h>
// #include <unistd.h>

// // Define the task queue structure - use named struct to match forward declaration
// struct task_queue_t {
//     test_case_t **tests;
//     int capacity;
//     int size;
//     int next_index;
//     pthread_mutex_t mutex;
//     pthread_cond_t cond;
// };

// // Define the thread pool structure - use named struct to match forward declaration
// struct thread_pool_t {
//     pthread_t *threads;          /* Mảng các thread */
//     int thread_count;            /* Số lượng thread */
//     task_queue_t *queue;         /* Con trỏ đến task queue */
//     int running;                 /* Cờ báo thread pool đang chạy */
//     int completed;               /* Số task đã hoàn thành */
//     int successful;              /* Số task thành công */
//     int failed;                  /* Số task thất bại */
//     pthread_mutex_t mutex;       /* Mutex để bảo vệ các bộ đếm */
//     test_result_info_t *results; /* Mảng lưu kết quả test */
//     int result_capacity;         /* Sức chứa của mảng kết quả */
// };

// // Worker thread function
// static void *worker_function(void *arg) {
//     thread_pool_t *pool = (thread_pool_t *)arg;
//     task_queue_t *queue = pool->queue;
    
//     while (pool->running) {
//         // Get task from queue with locking
//         pthread_mutex_lock(&queue->mutex);
        
//         while (queue->size == 0 && pool->running) {
//             pthread_cond_wait(&queue->cond, &queue->mutex);
//         }
        
//         if (!pool->running) {
//             pthread_mutex_unlock(&queue->mutex);
//             break;
//         }
        
//         // Get next test case
//         test_case_t *test = queue->tests[queue->next_index];
//         queue->next_index = (queue->next_index + 1) % queue->capacity;
//         queue->size--;
        
//         pthread_mutex_unlock(&queue->mutex);
        
//         // Execute test case
//         if (test) {
//             test_result_info_t result;
//             execute_test_case(test, &result);
            
//             // Update statistics and store result
//             pthread_mutex_lock(&pool->mutex);
//             int index = pool->completed;
//             if (index < pool->result_capacity) {
//                 memcpy(&pool->results[index], &result, sizeof(test_result_info_t));
//             }
            
//             pool->completed++;
//             if (result.status == TEST_RESULT_SUCCESS) pool->successful++;
//             else pool->failed++;
//             pthread_mutex_unlock(&pool->mutex);
//         }
//     }
    
//     return NULL;
// }

// // Initialize task queue
// task_queue_t* init_task_queue(int capacity) {
//     task_queue_t *queue = malloc(sizeof(task_queue_t));
//     if (!queue) return NULL;
    
//     queue->tests = malloc(capacity * sizeof(test_case_t*));
//     if (!queue->tests) {
//         free(queue);
//         return NULL;
//     }
    
//     queue->capacity = capacity;
//     queue->size = 0;
//     queue->next_index = 0;
//     pthread_mutex_init(&queue->mutex, NULL);
//     pthread_cond_init(&queue->cond, NULL);
    
//     return queue;
// }

// // Add test to queue
// int enqueue_task(task_queue_t *queue, test_case_t *test) {
//     pthread_mutex_lock(&queue->mutex);
    
//     if (queue->size >= queue->capacity) {
//         pthread_mutex_unlock(&queue->mutex);
//         return -1;
//     }
    
//     int index = (queue->next_index + queue->size) % queue->capacity;
//     queue->tests[index] = test;
//     queue->size++;
    
//     pthread_cond_signal(&queue->cond);
//     pthread_mutex_unlock(&queue->mutex);
    
//     return 0;
// }

// // Add multiple tests to queue
// int enqueue_test_cases(task_queue_t *queue, test_case_t *tests, int count) {
//     int added = 0;
    
//     for (int i = 0; i < count; i++) {
//         if (tests[i].enabled) {
//             if (enqueue_task(queue, &tests[i]) == 0) {
//                 added++;
//             }
//         }
//     }
    
//     return added;
// }

// // Initialize thread pool with results storage
// thread_pool_t* init_thread_pool(int threads, task_queue_t *queue) {
//     thread_pool_t *pool = malloc(sizeof(thread_pool_t));
//     if (!pool) return NULL;
    
//     pool->threads = malloc(threads * sizeof(pthread_t));
//     if (!pool->threads) {
//         free(pool);
//         return NULL;
//     }
    
//     // Estimate the number of test cases from queue capacity
//     int result_capacity = queue->capacity;
//     pool->results = calloc(result_capacity, sizeof(test_result_info_t));
//     if (!pool->results) {
//         free(pool->threads);
//         free(pool);
//         return NULL;
//     }
    
//     pool->thread_count = threads;
//     pool->queue = queue;
//     pool->running = 1;
//     pool->completed = 0;
//     pool->successful = 0;
//     pool->failed = 0;
//     pool->result_capacity = result_capacity;
//     pthread_mutex_init(&pool->mutex, NULL);
    
//     for (int i = 0; i < threads; i++) {
//         pthread_create(&pool->threads[i], NULL, worker_function, pool);
//     }
    
//     return pool;
// }

// // Stop thread pool
// void stop_thread_pool(thread_pool_t *pool) {
//     if (!pool) return;
    
//     // Signal threads to stop
//     pool->running = 0;
//     pthread_cond_broadcast(&pool->queue->cond);
    
//     // Wait for threads to complete
//     for (int i = 0; i < pool->thread_count; i++) {
//         pthread_join(pool->threads[i], NULL);  // Fixed: removed the &
//     }
    
//     // Clean up resources
//     pthread_mutex_destroy(&pool->mutex);
//     free(pool->threads);
//     // Don't free results here as they're still needed
//     free(pool);
// }

// // Free task queue
// void free_task_queue(task_queue_t *queue) {
//     if (!queue) return;
    
//     pthread_mutex_destroy(&queue->mutex);
//     pthread_cond_destroy(&queue->cond);
//     free(queue->tests);
//     free(queue);
// }

// // Get statistics
// int get_completed_count(thread_pool_t *pool) {
//     return pool ? pool->completed : 0;
// }

// int get_success_count(thread_pool_t *pool) {
//     return pool ? pool->successful : 0;
// }

// int get_failed_count(thread_pool_t *pool) {
//     return pool ? pool->failed : 0;
// }

// // Get results array from thread pool
// test_result_info_t* get_results(thread_pool_t *pool, int *count) {
//     if (!pool || !count) return NULL;
    
//     *count = pool->completed < pool->result_capacity ? 
//              pool->completed : pool->result_capacity;
    
//     return pool->results;
// }
