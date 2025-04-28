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
 * @brief Initialize the config directory watcher using `inotify`
 *
 * This function initializes an `inotify` instance and sets up a watch on the specified
 * directory to monitor for new files. It logs errors if initialization or setup fails.
 *
 * @param dir_path Path to the directory to monitor (e.g., "config").
 * @param fd Pointer to store the `inotify` file descriptor.
 * @param wd Pointer to store the `inotify` watch descriptor.
 * @return int
 *         - 0 if initialization and setup were successful.
 *         - -1 if an error occurred (e.g., failed to initialize `inotify` or add watch).
 */
int initializeFileWatcher(const char *dir_path, int *fd, int *wd);

/**
 * @brief Monitor the config directory for new test case files and process them
 *
 * This function continuously monitors the config directory using `inotify` for new files
 * with a `.json` extension. When a new file is detected (via `IN_CREATE` or `IN_MOVED_TO`
 * events), it processes the file by parsing, executing test cases, and writing results.
 * The function runs in an infinite loop until an error occurs or the system is terminated.
 *
 * @param fd `inotify` file descriptor.
 * @param wd `inotify` watch descriptor.
 * @return int
 *         - 0 if monitoring completed successfully (e.g., on termination).
 *         - -1 if an error occurred (e.g., failed to read `inotify` events).
 */
int monitorConfigDirectory(int fd, int wd);

/**
 * @brief Clean up config directory watcher resources
 *
 * This function removes the `inotify` watch, closes the file descriptor, and cleans up
 * the logger resources.
 *
 * @param fd `inotify` file descriptor.
 * @param wd `inotify` watch descriptor.
 * @return None
 */
void cleanupFileWatcher(int fd, int wd);

#endif /* CONFIG_WATCHER_H */