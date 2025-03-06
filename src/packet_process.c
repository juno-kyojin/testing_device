/**
 * @file packet_process.c
 * @brief Implementation of thread pool and queue management
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <unistd.h>
 #include <time.h>
 #include <sys/time.h>
 #include <pthread.h>
 
 #include "../include/packet_process.h"
 #include "../include/log.h"
 #include "../include/tc.h"  // For test case execution
 
 // Thread pool and queue configuration
 #define DEFAULT_THREAD_COUNT 4
 #define DEFAULT_QUEUE_SIZE 10000
 #define MAX_THREADS 64
 #define MAX_QUEUE_SIZE 1000000
 
 // Task queue structure
 typedef struct {
     task_t* tasks;          // Array of tasks
     int max_size;           // Maximum queue size
     int head;               // Index of the first task
     int tail;               // Index of the last task
     int count;              // Current number of tasks in queue
     pthread_mutex_t mutex;  // Mutex for thread safety
     pthread_cond_t not_empty; // Condition variable for empty queue
     pthread_cond_t not_full;  // Condition variable for full queue
 } task_queue_t;
 
 // Thread pool structure
 typedef struct {
     pthread_t* threads;      // Array of thread handles
     int thread_count;        // Number of threads
     int running;             // 1 if thread pool is running, 0 otherwise
     task_queue_t queue;      // Task queue
     uint64_t total_tasks;    // Total number of tasks received
     uint64_t completed_tasks; // Number of completed tasks
     uint64_t failed_tasks;    // Number of failed tasks
     pthread_mutex_t stats_mutex; // Mutex for statistics
 } thread_pool_t;
 
 // Global thread pool instance
 static thread_pool_t pool = {0};
 
 // Get current timestamp in milliseconds
 static uint64_t get_timestamp_ms(void) {
     struct timeval tv;
     gettimeofday(&tv, NULL);
     return (uint64_t)(tv.tv_sec) * 1000 + (uint64_t)(tv.tv_usec) / 1000;
 }
 
 // Initialize the task queue
 static int init_task_queue(task_queue_t* queue, int queue_size) {
     if (!queue || queue_size <= 0) {
         return -1;
     }
     
     // Allocate task array
     queue->tasks = (task_t*)calloc(queue_size, sizeof(task_t));
     if (!queue->tasks) {
         return -1;
     }
     
     // Initialize queue parameters
     queue->max_size = queue_size;
     queue->head = 0;
     queue->tail = 0;
     queue->count = 0;
     
     // Initialize synchronization primitives
     if (pthread_mutex_init(&queue->mutex, NULL) != 0) {
         free(queue->tasks);
         return -1;
     }
     
     if (pthread_cond_init(&queue->not_empty, NULL) != 0) {
         pthread_mutex_destroy(&queue->mutex);
         free(queue->tasks);
         return -1;
     }
     
     if (pthread_cond_init(&queue->not_full, NULL) != 0) {
         pthread_cond_destroy(&queue->not_empty);
         pthread_mutex_destroy(&queue->mutex);
         free(queue->tasks);
         return -1;
     }
     
     return 0;
 }
 
 // Clean up the task queue
 static void cleanup_task_queue(task_queue_t* queue) {
     if (!queue) {
         return;
     }
     
     pthread_mutex_lock(&queue->mutex);
     
     // Free task array
     if (queue->tasks) {
         free(queue->tasks);
         queue->tasks = NULL;
     }
     
     queue->max_size = 0;
     queue->head = 0;
     queue->tail = 0;
     queue->count = 0;
     
     pthread_mutex_unlock(&queue->mutex);
     
     // Destroy synchronization primitives
     pthread_cond_destroy(&queue->not_empty);
     pthread_cond_destroy(&queue->not_full);
     pthread_mutex_destroy(&queue->mutex);
 }
 
 // Enqueue a task
 static int enqueue_task_internal(task_queue_t* queue, const task_t* task) {
     if (!queue || !task) {
         return -1;
     }
     
     pthread_mutex_lock(&queue->mutex);
     
     // Wait if the queue is full
     while (queue->count == queue->max_size) {
         pthread_cond_wait(&queue->not_full, &queue->mutex);
     }
     
     // Add the task to the queue
     memcpy(&queue->tasks[queue->tail], task, sizeof(task_t));
     
     // Update queue pointers
     queue->tail = (queue->tail + 1) % queue->max_size;
     queue->count++;
     
     // Signal that the queue is not empty
     pthread_cond_signal(&queue->not_empty);
     
     pthread_mutex_unlock(&queue->mutex);
     
     return 0;
 }
 
 // Dequeue a task
 static int dequeue_task_internal(task_queue_t* queue, task_t* task) {
     if (!queue || !task) {
         return -1;
     }
     
     pthread_mutex_lock(&queue->mutex);
     
     // Wait if the queue is empty
     while (queue->count == 0) {
         // If thread pool is not running, return immediately
         if (!pool.running) {
             pthread_mutex_unlock(&queue->mutex);
             return -1;
         }
         
         pthread_cond_wait(&queue->not_empty, &queue->mutex);
         
         // Check again after waking up
         if (!pool.running && queue->count == 0) {
             pthread_mutex_unlock(&queue->mutex);
             return -1;
         }
     }
     
     // Get the task from the queue
     memcpy(task, &queue->tasks[queue->head], sizeof(task_t));
     
     // Update queue pointers
     queue->head = (queue->head + 1) % queue->max_size;
     queue->count--;
     
     // Signal that the queue is not full
     pthread_cond_signal(&queue->not_full);
     
     pthread_mutex_unlock(&queue->mutex);
     
     return 0;
 }
 
 // Thread worker function
 static void* thread_pool_worker(void* arg) {
     int thread_id = *((int*)arg);
     free(arg);  // Free memory allocated for thread ID
     
     LOG_INFO(LOG_CAT_TEST_ENGINE, "Worker thread %d started", thread_id);
     
     task_t task;
     
     while (pool.running) {
         // Dequeue a task
         if (dequeue_task_internal(&pool.queue, &task) != 0) {
             // If dequeue failed and thread pool is no longer running, exit
             if (!pool.running) {
                 break;
             }
             
             // Otherwise, try again
             usleep(10000);  // Sleep for 10ms to avoid busy waiting
             continue;
         }
         
         // Process the task
         LOG_DEBUG(LOG_CAT_TEST_ENGINE, "Thread %d processing test case ID %lu", 
                  thread_id, (unsigned long)task.test_case.id);
         
         // Update task status and timestamps
         task.status = TASK_STATUS_RUNNING;
         task.start_time = get_timestamp_ms();
         
         // Execute the test case
         int result = execute_test_case_by_network(&task.test_case);
         
         // Update task status based on execution result
         task.end_time = get_timestamp_ms();
         if (result == 0) {
             task.status = TASK_STATUS_COMPLETED;
             snprintf(task.result_message, sizeof(task.result_message), "Test case executed successfully");
         } else {
             task.status = TASK_STATUS_FAILED;
             snprintf(task.result_message, sizeof(task.result_message), "Test case execution failed with code %d", result);
         }
         task.result_code = result;
         
         // Update statistics
         pthread_mutex_lock(&pool.stats_mutex);
         if (task.status == TASK_STATUS_COMPLETED) {
             pool.completed_tasks++;
         } else {
             pool.failed_tasks++;
         }
         pthread_mutex_unlock(&pool.stats_mutex);
         
         // Report progress periodically
         uint64_t processed = pool.completed_tasks + pool.failed_tasks;
         if (processed % 1000 == 0) {
             // Log progress
             log_queue_progress("current");
         }
     }
     
     LOG_INFO(LOG_CAT_TEST_ENGINE, "Worker thread %d exiting", thread_id);
     return NULL;
 }
 
 int init_thread_pool(int thread_count, int queue_size) {
     // Validate parameters
     if (thread_count <= 0) {
         thread_count = DEFAULT_THREAD_COUNT;
     } else if (thread_count > MAX_THREADS) {
         thread_count = MAX_THREADS;
     }
     
     if (queue_size <= 0) {
         queue_size = DEFAULT_QUEUE_SIZE;
     } else if (queue_size > MAX_QUEUE_SIZE) {
         queue_size = MAX_QUEUE_SIZE;
     }
     
     LOG_INFO(LOG_CAT_TEST_ENGINE, "Initializing thread pool with %d threads and queue size %d", 
              thread_count, queue_size);
     
     // Initialize the thread pool structure
     memset(&pool, 0, sizeof(pool));
     
     // Initialize statistics mutex
     if (pthread_mutex_init(&pool.stats_mutex, NULL) != 0) {
         LOG_ERROR(LOG_CAT_TEST_ENGINE, "Failed to initialize statistics mutex");
         return -1;
     }
     
     // Initialize the task queue
     if (init_task_queue(&pool.queue, queue_size) != 0) {
         pthread_mutex_destroy(&pool.stats_mutex);
         LOG_ERROR(LOG_CAT_TEST_ENGINE, "Failed to initialize task queue");
         return -1;
     }
     
     // Allocate thread array
     pool.threads = (pthread_t*)calloc(thread_count, sizeof(pthread_t));
     if (!pool.threads) {
         cleanup_task_queue(&pool.queue);
         pthread_mutex_destroy(&pool.stats_mutex);
         LOG_ERROR(LOG_CAT_TEST_ENGINE, "Failed to allocate thread array");
         return -1;
     }
     
     pool.thread_count = thread_count;
     pool.running = 1;
     
     // Create worker threads
     for (int i = 0; i < thread_count; i++) {
         int* thread_id = (int*)malloc(sizeof(int));
         if (!thread_id) {
             LOG_ERROR(LOG_CAT_TEST_ENGINE, "Failed to allocate thread ID memory");
             //continue;
        }
        
        *thread_id = i;
        
        if (pthread_create(&pool.threads[i], NULL, thread_pool_worker, thread_id) != 0) {
            LOG_ERROR(LOG_CAT_TEST_ENGINE, "Failed to create worker thread %d", i);
            free(thread_id);
            continue;
        }
    }
    
    // Check if at least one thread was created
    int active_threads = 0;
    for (int i = 0; i < thread_count; i++) {
        if (pool.threads[i] != 0) {
            active_threads++;
        }
    }
    
    if (active_threads == 0) {
        cleanup_thread_pool();
        LOG_ERROR(LOG_CAT_TEST_ENGINE, "Failed to create any worker threads");
        return -1;
    }
    
    LOG_INFO(LOG_CAT_TEST_ENGINE, "Thread pool initialized with %d active threads", active_threads);
    return 0;
}

int cleanup_thread_pool(void) {
    if (!pool.running) {
        // Already cleaned up
        return 0;
    }
    
    LOG_INFO(LOG_CAT_TEST_ENGINE, "Cleaning up thread pool resources");
    
    // Signal threads to exit
    pool.running = 0;
    
    // Wake up all waiting threads
    pthread_mutex_lock(&pool.queue.mutex);
    pthread_cond_broadcast(&pool.queue.not_empty);
    pthread_mutex_unlock(&pool.queue.mutex);
    
    // Wait for threads to exit
    if (pool.threads) {
        for (int i = 0; i < pool.thread_count; i++) {
            if (pool.threads[i] != 0) {
                pthread_join(pool.threads[i], NULL);
                LOG_DEBUG(LOG_CAT_TEST_ENGINE, "Worker thread %d has exited", i);
            }
        }
        
        free(pool.threads);
        pool.threads = NULL;
    }
    
    // Clean up queue resources
    cleanup_task_queue(&pool.queue);
    
    // Clean up statistics mutex
    pthread_mutex_destroy(&pool.stats_mutex);
    
    LOG_INFO(LOG_CAT_TEST_ENGINE, "Thread pool cleanup completed");
    return 0;
}

int adjust_thread_pool_size(int new_thread_count) {
    // Validate new thread count
    if (new_thread_count <= 0) {
        new_thread_count = DEFAULT_THREAD_COUNT;
    } else if (new_thread_count > MAX_THREADS) {
        new_thread_count = MAX_THREADS;
    }
    
    if (new_thread_count == pool.thread_count) {
        // No change needed
        return 0;
    }
    
    LOG_INFO(LOG_CAT_TEST_ENGINE, "Adjusting thread pool size from %d to %d threads",
             pool.thread_count, new_thread_count);
    
    // Case 1: Increasing thread count
    if (new_thread_count > pool.thread_count) {
        // Allocate new thread array
        pthread_t* new_threads = (pthread_t*)realloc(pool.threads, 
                                                    new_thread_count * sizeof(pthread_t));
        if (!new_threads) {
            LOG_ERROR(LOG_CAT_TEST_ENGINE, "Failed to reallocate thread array");
            return -1;
        }
        
        pool.threads = new_threads;
        
        // Initialize new thread entries
        for (int i = pool.thread_count; i < new_thread_count; i++) {
            pool.threads[i] = 0;
        }
        
        // Create new worker threads
        for (int i = pool.thread_count; i < new_thread_count; i++) {
            int* thread_id = (int*)malloc(sizeof(int));
            if (!thread_id) {
                LOG_ERROR(LOG_CAT_TEST_ENGINE, "Failed to allocate thread ID memory");
                continue;
            }
            
            *thread_id = i;
            
            if (pthread_create(&pool.threads[i], NULL, thread_pool_worker, thread_id) != 0) {
                LOG_ERROR(LOG_CAT_TEST_ENGINE, "Failed to create worker thread %d", i);
                free(thread_id);
                continue;
            }
            
            LOG_DEBUG(LOG_CAT_TEST_ENGINE, "Created new worker thread %d", i);
        }
        
        // Update thread count
        pool.thread_count = new_thread_count;
        
        LOG_INFO(LOG_CAT_TEST_ENGINE, "Thread pool size increased to %d threads", new_thread_count);
        return 0;
    }
    
    // Case 2: Decreasing thread count
    // Note: We can't easily kill specific threads, so we'll just let the extra ones
    // continue running but not create new ones when they exit
    
    // Keep track of the old thread count
    int old_thread_count = pool.thread_count;
    
    // Update thread count
    pool.thread_count = new_thread_count;
    
    LOG_INFO(LOG_CAT_TEST_ENGINE, "Thread pool size decreased to %d threads (old threads will exit naturally)",
             new_thread_count);
    return 0;
}

int adjust_task_queue_size(int new_queue_size) {
    // Validate new queue size
    if (new_queue_size <= 0) {
        new_queue_size = DEFAULT_QUEUE_SIZE;
    } else if (new_queue_size > MAX_QUEUE_SIZE) {
        new_queue_size = MAX_QUEUE_SIZE;
    }
    
    LOG_INFO(LOG_CAT_TEST_ENGINE, "Adjusting task queue size from %d to %d",
             pool.queue.max_size, new_queue_size);
    
    pthread_mutex_lock(&pool.queue.mutex);
    
    // Check if queue is currently empty, which is required for resizing
    if (pool.queue.count > 0) {
        pthread_mutex_unlock(&pool.queue.mutex);
        LOG_ERROR(LOG_CAT_TEST_ENGINE, "Cannot resize queue while it contains tasks");
        return -1;
    }
    
    // Allocate new task array
    task_t* new_tasks = (task_t*)realloc(pool.queue.tasks, new_queue_size * sizeof(task_t));
    if (!new_tasks) {
        pthread_mutex_unlock(&pool.queue.mutex);
        LOG_ERROR(LOG_CAT_TEST_ENGINE, "Failed to reallocate task queue");
        return -1;
    }
    
    // Update queue
    pool.queue.tasks = new_tasks;
    pool.queue.max_size = new_queue_size;
    pool.queue.head = 0;
    pool.queue.tail = 0;
    
    pthread_mutex_unlock(&pool.queue.mutex);
    
    LOG_INFO(LOG_CAT_TEST_ENGINE, "Task queue size adjusted to %d", new_queue_size);
    return 0;
}

int enqueue_task(const task_t* task) {
    if (!task) {
        return -1;
    }
    
    // Update task count
    pthread_mutex_lock(&pool.stats_mutex);
    pool.total_tasks++;
    pthread_mutex_unlock(&pool.stats_mutex);
    
    // Enqueue the task
    return enqueue_task_internal(&pool.queue, task);
}

void* process_queued_test_cases(void* thread_id) {
    // This function is just a wrapper for thread_pool_worker
    // It allows external code to create worker threads if needed
    return thread_pool_worker(thread_id);
}

int get_queue_progress(uint64_t* processed, uint64_t* total) {
    if (!processed || !total) {
        return -1;
    }
    
    pthread_mutex_lock(&pool.stats_mutex);
    *total = pool.total_tasks;
    *processed = pool.completed_tasks + pool.failed_tasks;
    pthread_mutex_unlock(&pool.stats_mutex);
    
    return 0;
}

int log_queue_progress(const char* batch_id) {
    if (!batch_id) {
        return -1;
    }
    
    uint64_t processed, total;
    get_queue_progress(&processed, &total);
    
    if (total == 0) {
        LOG_INFO(LOG_CAT_TEST_ENGINE, "[PROGRESS] 0.00%% - No tasks received - Batch: %s", batch_id);
        return 0;
    }
    
    double percentage = 100.0 * processed / total;
    
    LOG_INFO(LOG_CAT_TEST_ENGINE, "[PROGRESS] %.2f%% - Processed %lu/%lu tests - Batch: %s",
             percentage, (unsigned long)processed, (unsigned long)total, batch_id);
    return 0;
}

int wait_for_queue_completion(uint64_t timeout_ms) {
    uint64_t start_time = get_timestamp_ms();
    
    while (1) {
        // Check if all tasks have been processed
        uint64_t processed, total;
        get_queue_progress(&processed, &total);
        
        if (processed >= total && total > 0) {
            // All tasks have been processed
            LOG_INFO(LOG_CAT_TEST_ENGINE, "All tasks completed: %lu/%lu",
                    (unsigned long)processed, (unsigned long)total);
            return 0;
        }
        
        // Check for timeout
        if (timeout_ms > 0) {
            uint64_t current_time = get_timestamp_ms();
            if (current_time - start_time >= timeout_ms) {
                // Timeout reached
                LOG_WARN(LOG_CAT_TEST_ENGINE, "Timeout reached waiting for queue completion: %lu/%lu tasks processed",
                        (unsigned long)processed, (unsigned long)total);
                return 1;
            }
        }
        
        // Sleep to avoid busy waiting
        usleep(100000);  // Sleep for 100ms
    }
}

int get_queue_stats(uint64_t* pending, uint64_t* running, uint64_t* completed, uint64_t* failed) {
    if (!pending || !running || !completed || !failed) {
        return -1;
    }
    
    pthread_mutex_lock(&pool.queue.mutex);
    *pending = pool.queue.count;
    pthread_mutex_unlock(&pool.queue.mutex);
    
    pthread_mutex_lock(&pool.stats_mutex);
    *completed = pool.completed_tasks;
    *failed = pool.failed_tasks;
    *running = pool.total_tasks - *pending - *completed - *failed;
    pthread_mutex_unlock(&pool.stats_mutex);
    
    return 0;
}