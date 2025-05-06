/**
 * @file config_watcher.c
 * @brief Implementation of config directory watcher utilities
 *
 * This file implements utilities for monitoring the config directory using `inotify`.
 * It initializes an `inotify` instance, watches the `config` directory for new test case files,
 * adds them to a queue, processes files from the queue in order, and cleans up resources
 * when done. These utilities are used to detect new test case files in the `config` directory
 * and process them in the test case execution system.
 *
 * @author [junokyojin]
 * @date 2025-04-23
 * @see config_watcher.h
 * @see test_case_handler.h
 * @see log.h
 */
#include "config_watcher.h"
#include "test_case_handler.h"
#include "log.h"
#include <string.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

/**
 * @def EVENT_SIZE
 * @brief Size of an inotify event structure
 *
 * Defines the size of the `inotify_event` structure used for monitoring file system
 * events. It is used to calculate the buffer size for reading events.
 */
#define EVENT_SIZE (sizeof(struct inotify_event))

/**
 * @def BUF_LEN
 * @brief Buffer length for reading inotify events
 *
 * Defines the buffer size for reading inotify events. It is set to accommodate multiple
 * events, ensuring the system can handle bursts of file system activity.
 */
#define BUF_LEN (1024 * (EVENT_SIZE + 16))

/**
 * @brief Initialize the file queue
 *
 * This function initializes the file queue by setting its head and tail pointers to NULL.
 *
 * @param queue Pointer to the file queue to initialize.
 * @return None
 */
static void initFileQueue(FileQueue *queue) {
    queue->head = NULL;
    queue->tail = NULL;
}

/**
 * @brief Add a file path to the queue (enqueue)
 *
 * This function creates a new queue node with the given file path and adds it to the
 * end of the queue.
 *
 * @param queue Pointer to the file queue.
 * @param filepath File path to add to the queue.
 * @return int
 *         - 0 if the file path was successfully added.
 *         - -1 if memory allocation failed.
 */
static int enqueueFile(FileQueue *queue, const char *filepath) {
    // Allocate memory for the new node
    QueueNode *node = (QueueNode *)malloc(sizeof(QueueNode));
    if (!node) {
        log_message(LOG_LVL_ERROR, "Failed to allocate memory for queue node");
        return -1;
    }

    // Allocate memory for the file path and copy it
    node->filepath = strdup(filepath);
    if (!node->filepath) {
        log_message(LOG_LVL_ERROR, "Failed to allocate memory for file path");
        free(node);
        return -1;
    }

    node->next = NULL;

    // Add the node to the queue
    if (queue->head == NULL) {
        // Queue is empty
        queue->head = node;
        queue->tail = node;
    } else {
        // Add to the end of the queue
        queue->tail->next = node;
        queue->tail = node;
    }

    log_message(LOG_LVL_DEBUG, "Added file to queue: %s", filepath);
    return 0;
}

/**
 * @brief Remove and return a file path from the queue (dequeue)
 *
 * This function removes the first file path from the queue and returns it. The caller
 * is responsible for freeing the returned file path.
 *
 * @param queue Pointer to the file queue.
 * @return char* File path removed from the queue, or NULL if the queue is empty.
 */
static char *dequeueFile(FileQueue *queue) {
    if (queue->head == NULL) {
        return NULL; // Queue is empty
    }

    // Remove the head node
    QueueNode *node = queue->head;
    char *filepath = node->filepath;
    queue->head = node->next;

    // If the queue is now empty, update the tail
    if (queue->head == NULL) {
        queue->tail = NULL;
    }

    // Free the node (but not the filepath, which is returned)
    free(node);
    return filepath;
}

/**
 * @brief Check if the queue is empty
 *
 * This function checks if the file queue is empty.
 *
 * @param queue Pointer to the file queue.
 * @return int
 *         - 1 if the queue is empty.
 *         - 0 if the queue is not empty.
 */
static int isQueueEmpty(FileQueue *queue) {
    return queue->head == NULL;
}

/**
 * @brief Clean up the file queue
 *
 * This function frees all nodes in the file queue, including the file paths they store.
 *
 * @param queue Pointer to the file queue to clean up.
 * @return None
 */
static void cleanupFileQueue(FileQueue *queue) {
    while (!isQueueEmpty(queue)) {
        char *filepath = dequeueFile(queue);
        if (filepath) {
            free(filepath);
        }
    }
}

int initializeFileWatcher(const char *dir_path, int *fd, int *wd, FileQueue *queue) {
    // Create an inotify instance
    *fd = inotify_init();
    if (*fd < 0) {
        log_message(LOG_LVL_ERROR, "Failed to initialize inotify");
        return -1;
    }

    // Monitor the specified directory
    *wd = inotify_add_watch(*fd, dir_path, IN_CREATE | IN_MOVED_TO);
    if (*wd < 0) {
        log_message(LOG_LVL_ERROR, "Failed to add watch for directory %s", dir_path);
        close(*fd);
        return -1;
    }

    // Initialize the file queue
    initFileQueue(queue);

    log_message(LOG_LVL_DEBUG, "Started monitoring directory %s for new test case files...", dir_path);
    return 0;
}

int monitorConfigDirectory(int fd, int wd, FileQueue *queue) {
    char buffer[BUF_LEN];
    while (1) {
        // Read inotify events
        int length = read(fd, buffer, BUF_LEN);
        if (length < 0) {
            log_message(LOG_LVL_ERROR, "Failed to read inotify events");
            return -1;
        }

        // Process inotify events
        int i = 0;
        while (i < length) {
            struct inotify_event *event = (struct inotify_event *)&buffer[i];
            if (event->len) {
                if (event->mask & (IN_CREATE | IN_MOVED_TO)) {
                    // Check if the file has a .json extension
                    if (strstr(event->name, ".json") != NULL) {
                        char filepath[512];
                        snprintf(filepath, sizeof(filepath), "config/%s", event->name);
                        log_message(LOG_LVL_DEBUG, "Detected new test case file: %s", filepath);

                        // Add the file to the queue instead of processing immediately
                        if (enqueueFile(queue, filepath) != 0) {
                            log_message(LOG_LVL_ERROR, "Failed to add file to queue: %s", filepath);
                        }
                    }
                }
            }
            i += EVENT_SIZE + event->len;
        }

        // Process files from the queue
        while (!isQueueEmpty(queue)) {
            char *filepath = dequeueFile(queue);
            if (filepath) {
                log_message(LOG_LVL_DEBUG, "Processing file from queue: %s", filepath);
                processTestCaseFile(filepath);
                free(filepath); // Free the file path after processing
            }
        }
    }
    return 0;
}

void cleanupFileWatcher(int fd, int wd, FileQueue *queue) {
    // Clean up the file queue
    cleanupFileQueue(queue);

    // Clean up inotify resources
    inotify_rm_watch(fd, wd);
    close(fd);
    cleanup_logger();
}