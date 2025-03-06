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
     memcpy(task, &queue->tasks[queue->head], sizeof(task_t)); ▋