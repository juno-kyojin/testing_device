/**
 * @file config_watcher.c
 * @brief Implementation of config directory watcher utilities
 *
 * This file implements utilities for monitoring the config directory using `inotify`.
 * It initializes an `inotify` instance, watches the `config` directory for new test case files,
 * processes them, and cleans up resources when done. These utilities are used to detect
 * new test case files in the `config` directory and process them in the test case execution
 * system.
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

int initializeFileWatcher(const char *dir_path, int *fd, int *wd) {
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

    log_message(LOG_LVL_DEBUG, "Started monitoring directory %s for new test case files...", dir_path);
    return 0;
}

int monitorConfigDirectory(int fd, int wd) {
    char buffer[BUF_LEN];
    while (1) {
        int length = read(fd, buffer, BUF_LEN);
        if (length < 0) {
            log_message(LOG_LVL_ERROR, "Failed to read inotify events");
            return -1;
        }

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

                        // Process the test case file
                        processTestCaseFile(filepath);
                    }
                }
            }
            i += EVENT_SIZE + event->len;
        }
    }
    return 0;
}

void cleanupFileWatcher(int fd, int wd) {
    inotify_rm_watch(fd, wd);
    close(fd);
    cleanup_logger();
}