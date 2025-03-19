#include "packet_process.h"
#include "tc.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>

/**
 * @brief Node trong priority queue
 */
typedef struct task_node {
    task_t task;                 /* Thông tin task */
    struct task_node *next;      /* Con trỏ đến node tiếp theo */
} task_node_t;

/**
 * @brief Cấu trúc dữ liệu cho task queue
 */
struct task_queue_t {
    task_node_t *head;           /* Con trỏ đến task có độ ưu tiên cao nhất */
    int size;                    /* Kích thước hiện tại của queue */
    int max_size;                /* Kích thước tối đa của queue */
    pthread_mutex_t mutex;       /* Mutex để bảo vệ truy cập vào queue */
    pthread_cond_t not_empty;    /* Condition variable cho queue không rỗng */
    pthread_cond_t not_full;     /* Condition variable cho queue không đầy */
};

/**
 * @brief Cấu trúc dữ liệu cho thread pool
 */
struct thread_pool_t {
    pthread_t *threads;          /* Mảng các thread */
    int thread_count;            /* Số lượng thread */
    task_queue_t *queue;         /* Con trỏ đến task queue */
    int running;                 /* Cờ báo thread pool đang chạy */
    int completed_tasks;         /* Số task đã hoàn thành */
    int success_tasks;           /* Số task thành công */
    int failed_tasks;            /* Số task thất bại */
    pthread_mutex_t counter_mutex; /* Mutex để bảo vệ các bộ đếm */
};

/**
 * @brief Hàm xử lý được chạy bởi mỗi thread trong pool
 */
static void *worker_function(void *arg);

/**
 * @brief Tăng bộ đếm task đã hoàn thành
 */
static void increment_completed_counter(thread_pool_t *pool, int success);

task_queue_t* init_task_queue(int max_size) {
    if (max_size <= 0) {
        log_message(LOG_LVL_ERROR, "Invalid task queue size: %d", max_size);
        return NULL;
    }

    task_queue_t *queue = (task_queue_t *)malloc(sizeof(task_queue_t));
    if (!queue) {
        log_message(LOG_LVL_ERROR, "Failed to allocate memory for task queue");
        return NULL;
    }

    queue->head = NULL;
    queue->size = 0;
    queue->max_size = max_size;

    if (pthread_mutex_init(&queue->mutex, NULL) != 0) {
        log_message(LOG_LVL_ERROR, "Failed to initialize queue mutex");
        free(queue);
        return NULL;
    }

    if (pthread_cond_init(&queue->not_empty, NULL) != 0) {
        log_message(LOG_LVL_ERROR, "Failed to initialize queue not_empty condition");
        pthread_mutex_destroy(&queue->mutex);
        free(queue);
        return NULL;
    }

    if (pthread_cond_init(&queue->not_full, NULL) != 0) {
        log_message(LOG_LVL_ERROR, "Failed to initialize queue not_full condition");
        pthread_cond_destroy(&queue->not_empty);
        pthread_mutex_destroy(&queue->mutex);
        free(queue);
        return NULL;
    }

    log_message(LOG_LVL_DEBUG, "Task queue initialized with max size: %d", max_size);
    return queue;
}

int enqueue_task(task_queue_t *queue, test_case_t *test_case, int priority) {
    if (!queue || !test_case) {
        log_message(LOG_LVL_ERROR, "Invalid queue or test case");
        return -1;
    }

    task_node_t *new_node = (task_node_t *)malloc(sizeof(task_node_t));
    if (!new_node) {
        log_message(LOG_LVL_ERROR, "Failed to allocate memory for task node");
        return -1;
    }

    new_node->task.test_case = test_case;
    new_node->task.priority = priority;
    new_node->task.extra_data = NULL;
    new_node->task.extra_data_size = 0;
    new_node->next = NULL;

    pthread_mutex_lock(&queue->mutex);

    // Đợi nếu queue đầy
    while (queue->size >= queue->max_size) {
        log_message(LOG_LVL_DEBUG, "Task queue full, waiting...");
        pthread_cond_wait(&queue->not_full, &queue->mutex);
    }

    // Chèn task vào vị trí phù hợp dựa trên độ ưu tiên
    if (queue->head == NULL || priority > queue->head->task.priority) {
        // Chèn vào đầu queue
        new_node->next = queue->head;
        queue->head = new_node;
    } else {
        // Tìm vị trí thích hợp
        task_node_t *current = queue->head;
        while (current->next != NULL && current->next->task.priority >= priority) {
            current = current->next;
        }
        new_node->next = current->next;
        current->next = new_node;
    }

    queue->size++;
    
    // Thông báo có task mới trong queue
    pthread_cond_signal(&queue->not_empty);
    
    pthread_mutex_unlock(&queue->mutex);
    
    log_message(LOG_LVL_DEBUG, "Task for test case %s enqueued with priority %d", 
                test_case->id, priority);
    return 0;
}

task_t* dequeue_task(task_queue_t *queue) {
    if (!queue) {
        log_message(LOG_LVL_ERROR, "Invalid queue");
        return NULL;
    }

    pthread_mutex_lock(&queue->mutex);

    // Đợi nếu queue rỗng
    while (queue->size == 0 && queue->head == NULL) {
        log_message(LOG_LVL_DEBUG, "Task queue empty, waiting...");
        pthread_cond_wait(&queue->not_empty, &queue->mutex);
    }

    if (queue->head == NULL) {
        pthread_mutex_unlock(&queue->mutex);
        return NULL;
    }

    // Lấy task từ đầu queue
    task_node_t *node = queue->head;
    task_t *task = (task_t *)malloc(sizeof(task_t));
    
    if (!task) {
        log_message(LOG_LVL_ERROR, "Failed to allocate memory for task");
        pthread_mutex_unlock(&queue->mutex);
        return NULL;
    }

    // Sao chép dữ liệu task
    memcpy(task, &node->task, sizeof(task_t));
    
    // Cập nhật queue
    queue->head = node->next;
    queue->size--;
    
    // Thông báo có chỗ trống trong queue
    pthread_cond_signal(&queue->not_full);
    
    pthread_mutex_unlock(&queue->mutex);
    
    if (task->test_case) {
        log_message(LOG_LVL_DEBUG, "Task for test case %s dequeued", task->test_case->id);
    }
    
    free(node);
    return task;
}

void free_task_queue(task_queue_t *queue) {
    if (!queue) return;

    pthread_mutex_lock(&queue->mutex);

    // Giải phóng tất cả các node trong queue
    task_node_t *current = queue->head;
    task_node_t *next;
    while (current) {
        next = current->next;
        // Không giải phóng test_case vì nó được quản lý bởi caller
        if (current->task.extra_data) {
            free(current->task.extra_data);
        }
        free(current);
        current = next;
    }

    pthread_mutex_unlock(&queue->mutex);

    // Hủy các đối tượng đồng bộ hóa
    pthread_cond_destroy(&queue->not_empty);
    pthread_cond_destroy(&queue->not_full);
    pthread_mutex_destroy(&queue->mutex);

    free(queue);
    log_message(LOG_LVL_DEBUG, "Task queue freed");
}

thread_pool_t* init_thread_pool(int thread_count, task_queue_t *queue) {
    if (thread_count <= 0 || !queue) {
        log_message(LOG_LVL_ERROR, "Invalid thread count or queue");
        return NULL;
    }

    thread_pool_t *pool = (thread_pool_t *)malloc(sizeof(thread_pool_t));
    if (!pool) {
        log_message(LOG_LVL_ERROR, "Failed to allocate memory for thread pool");
        return NULL;
    }

    pool->threads = (pthread_t *)malloc(thread_count * sizeof(pthread_t));
    if (!pool->threads) {
        log_message(LOG_LVL_ERROR, "Failed to allocate memory for threads");
        free(pool);
        return NULL;
    }

    pool->thread_count = thread_count;
    pool->queue = queue;
    pool->running = 1;
    pool->completed_tasks = 0;
    pool->success_tasks = 0;
    pool->failed_tasks = 0;

    if (pthread_mutex_init(&pool->counter_mutex, NULL) != 0) {
        log_message(LOG_LVL_ERROR, "Failed to initialize counter mutex");
        free(pool->threads);
        free(pool);
        return NULL;
    }

    // Tạo các thread worker
    for (int i = 0; i < thread_count; i++) {
        if (pthread_create(&pool->threads[i], NULL, worker_function, (void *)pool) != 0) {
            log_message(LOG_LVL_ERROR, "Failed to create worker thread %d", i);
            // Dừng các thread đã tạo
            pool->running = 0;
            pthread_cond_broadcast(&queue->not_empty);
            pthread_cond_broadcast(&queue->not_full);
            
            // Đợi các thread đang chạy
            for (int j = 0; j < i; j++) {
                pthread_join(pool->threads[j], NULL);
            }
            
            pthread_mutex_destroy(&pool->counter_mutex);
            free(pool->threads);
            free(pool);
            return NULL;
        }
    }

    log_message(LOG_LVL_DEBUG, "Thread pool initialized with %d threads", thread_count);
    return pool;
}

int enqueue_test_cases_to_queue(task_queue_t *queue, test_case_t *test_cases, int count) {
    if (!queue || !test_cases || count <= 0) {
        log_message(LOG_LVL_ERROR, "Invalid queue, test cases, or count");
        return -1;
    }

    int enqueued = 0;
    for (int i = 0; i < count; i++) {
        // Chỉ thêm vào queue nếu test case được bật
        if (test_cases[i].enabled) {
            int priority = 0;
            
            // Thiết lập độ ưu tiên dựa trên loại test
            switch (test_cases[i].type) {
                case TEST_PING:
                    priority = 10;  // Độ ưu tiên cao nhất cho ping test
                    break;
                case TEST_THROUGHPUT:
                    priority = 5;   // Độ ưu tiên trung bình cho throughput test
                    break;
                case TEST_VLAN:
                    priority = 7;   // Độ ưu tiên khá cao cho VLAN test
                    break;
                case TEST_SECURITY:
                    priority = 3;   // Độ ưu tiên thấp cho security test (có thể tốn nhiều thời gian)
                    break;
                case TEST_OTHER:
                default:
                    priority = 1;   // Độ ưu tiên thấp nhất cho các test khác
                    break;
            }
            
            if (enqueue_task(queue, &test_cases[i], priority) == 0) {
                enqueued++;
            }
        }
    }

    log_message(LOG_LVL_DEBUG, "Enqueued %d out of %d test cases", enqueued, count);
    return enqueued;
}

static void *worker_function(void *arg) {
    thread_pool_t *pool = (thread_pool_t *)arg;
    task_queue_t *queue = pool->queue;
    test_result_info_t result;

    log_message(LOG_LVL_DEBUG, "Worker thread started");

    while (pool->running) {
        // Lấy task từ queue
        task_t *task = dequeue_task(queue);
        if (!task) {
            continue;
        }

        // Thực thi test case
        if (task->test_case) {
            log_message(LOG_LVL_DEBUG, "Executing test case: %s", task->test_case->id);
            
            struct timeval start_time, end_time;
            gettimeofday(&start_time, NULL);
            
            int exec_result = execute_test_case(task->test_case, &result);
            
            gettimeofday(&end_time, NULL);
            float elapsed = (end_time.tv_sec - start_time.tv_sec) * 1000.0f + 
                           (end_time.tv_usec - start_time.tv_usec) / 1000.0f;
            
            result.execution_time = elapsed;
            
            if (exec_result == 0 && result.status == TEST_RESULT_SUCCESS) {
                log_message(LOG_LVL_DEBUG, "Test case %s completed successfully in %.2f ms", 
                           task->test_case->id, elapsed);
                increment_completed_counter(pool, 1);
            } else {
                log_message(LOG_LVL_ERROR, "Test case %s failed in %.2f ms, status: %d", 
                           task->test_case->id, elapsed, result.status);
                increment_completed_counter(pool, 0);
            }
        }

        // Giải phóng task
        if (task->extra_data) {
            free(task->extra_data);
        }
        free(task);
    }

    log_message(LOG_LVL_DEBUG, "Worker thread exiting");
    return NULL;
}

int process_queued_test_cases(thread_pool_t *pool, task_queue_t *queue, int total_count) {
    if (!pool || !queue) {
        log_message(LOG_LVL_ERROR, "Invalid pool or queue");
        return -1;
    }

    // Không cần làm gì đặc biệt ở đây vì các thread worker đã tự động xử lý các task
    // Chúng ta chỉ cần đợi tất cả các task hoàn thành

    log_message(LOG_LVL_DEBUG, "Waiting for all test cases to complete...");

    // Đợi cho đến khi tất cả các task được xử lý
    while (1) {
        pthread_mutex_lock(&pool->counter_mutex);
        int completed = pool->completed_tasks;
        pthread_mutex_unlock(&pool->counter_mutex);

        if (completed >= total_count) {
            break;
        }

        // Log tiến trình
        log_message(LOG_LVL_DEBUG, "Progress: %d/%d completed", completed, total_count);
        
        // Tránh CPU spin bằng cách sleep
        usleep(500000);  // Sleep 500ms
    }

    log_message(LOG_LVL_DEBUG, "All test cases completed: %d success, %d failed", 
               pool->success_tasks, pool->failed_tasks);
    return 0;
}

int stop_thread_pool(thread_pool_t *pool) {
    if (!pool) {
        log_message(LOG_LVL_ERROR, "Invalid pool");
        return -1;
    }

    // Đánh dấu pool đang dừng
    pool->running = 0;

    // Đánh thức tất cả các thread đang đợi
    pthread_cond_broadcast(&pool->queue->not_empty);
    pthread_cond_broadcast(&pool->queue->not_full);

    // Đợi tất cả các thread kết thúc
    for (int i = 0; i < pool->thread_count; i++) {
        pthread_join(pool->threads[i], NULL);
    }

    // Giải phóng tài nguyên
    pthread_mutex_destroy(&pool->counter_mutex);
    free(pool->threads);
    free(pool);

    log_message(LOG_LVL_DEBUG, "Thread pool stopped and freed");
    return 0;
}

static void increment_completed_counter(thread_pool_t *pool, int success) {
    pthread_mutex_lock(&pool->counter_mutex);
    pool->completed_tasks++;
    if (success) {
        pool->success_tasks++;
    } else {
        pool->failed_tasks++;
    }
    pthread_mutex_unlock(&pool->counter_mutex);
}

int get_completed_task_count(thread_pool_t *pool) {
    if (!pool) return 0;
    
    pthread_mutex_lock(&pool->counter_mutex);
    int count = pool->completed_tasks;
    pthread_mutex_unlock(&pool->counter_mutex);
    
    return count;
}

int get_success_task_count(thread_pool_t *pool) {
    if (!pool) return 0;
    
    pthread_mutex_lock(&pool->counter_mutex);
    int count = pool->success_tasks;
    pthread_mutex_unlock(&pool->counter_mutex);
    
    return count;
}

int get_failed_task_count(thread_pool_t *pool) {
    if (!pool) return 0;
    
    pthread_mutex_lock(&pool->counter_mutex);
    int count = pool->failed_tasks;
    pthread_mutex_unlock(&pool->counter_mutex);
    
    return count;
}
