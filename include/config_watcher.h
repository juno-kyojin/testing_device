/**
 * @file config_watcher.h
 * @brief Header file for config directory watcher utilities
 *
 * This header file defines the interface for config directory watcher utilities using `inotify`.
 * It provides functions to initialize the watcher, monitor the `config` directory for new files,
 * and clean up resources. These utilities are used to detect new test case files in the
 * `config` directory and process them accordingly.
 *
 * @author [junokyojin]
 * @date 2025-04-23
 * @see config_watcher.c
 */
#ifndef CONFIG_WATCHER_H
#define CONFIG_WATCHER_H

/**
 * @brief Structure for a queue node to store file paths
 *
 * This structure represents a node in the queue, holding the file path of a test case file
 * and a pointer to the next node in the queue.
 */
typedef struct QueueNode {
    char *filepath;            // File path of the test case file
    struct QueueNode *next;    // Pointer to the next node in the queue
} QueueNode;

/**
 * @brief Structure for the file queue
 *
 * This structure represents the queue used to store file paths of test case files.
 * It maintains pointers to the head and tail of the queue.
 */
typedef struct FileQueue {
    QueueNode *head;           // Pointer to the head of the queue
    QueueNode *tail;           // Pointer to the tail of the queue
} FileQueue;

/**
 * @brief Initialize the config directory watcher using `inotify`
 *
 * This function initializes an `inotify` instance, sets up a watch on the specified
 * directory to monitor for new files, and initializes the file queue. It logs errors
 * if initialization or setup fails.
 *
 * @param dir_path Path to the directory to monitor (e.g., "config").
 * @param fd Pointer to store the `inotify` file descriptor.
 * @param wd Pointer to store the `inotify` watch descriptor.
 * @param queue Pointer to the file queue to initialize.
 * @return int
 *         - 0 if initialization and setup were successful.
 *         - -1 if an error occurred (e.g., failed to initialize `inotify` or add watch).
 */
int initializeFileWatcher(const char *dir_path, int *fd, int *wd, FileQueue *queue);

/**
 * @brief Monitor the config directory for new test case files and process them
 *
 * This function continuously monitors the config directory using `inotify` for new files
 * with a `.json` extension. When a new file is detected (via `IN_CREATE` or `IN_MOVED_TO`
 * events), it adds the file path to the queue. It then processes files from the queue
 * in a first-come, first-served order. The function runs in an infinite loop until an
 * error occurs or the system is terminated.
 *
 * @param fd `inotify` file descriptor.
 * @param wd `inotify` watch descriptor.
 * @param queue Pointer to the file queue containing file paths to process.
 * @return int
 *         - 0 if monitoring completed successfully (e.g., on termination).
 *         - -1 if an error occurred (e.g., failed to read `inotify` events).
 */
int monitorConfigDirectory(int fd, int wd, FileQueue *queue);

/**
 * @brief Clean up config directory watcher resources
 *
 * This function removes the `inotify` watch, closes the file descriptor, cleans up the
 * file queue, and frees logger resources.
 *
 * @param fd `inotify` file descriptor.
 * @param wd `inotify` watch descriptor.
 * @param queue Pointer to the file queue to clean up.
 * @return None
 */
void cleanupFileWatcher(int fd, int wd, FileQueue *queue);

#endif /* CONFIG_WATCHER_H */