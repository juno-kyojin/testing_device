/**
 * @file main.c
 * @brief Main entry point for the test case execution system
 *
 * This file serves as the main entry point for the test case execution system.
 * It initializes the system, sets up a file system watcher, and monitors the `config`
 * directory for new test case files to process. The system is designed to run continuously
 * as a daemon, automatically started at boot via a systemd service (`testing_device.service`),
 * ensuring uninterrupted test case execution.
 *
 * @author [junokyojin]
 * @date 2025-04-23
 * @see config_watcher.h
 * @see log.h
 */
#include "config_watcher.h"
#include "log.h"

/**
 * @brief Main entry point for the test case execution system
 *
 * This function initializes the system, sets up a file system watcher to monitor the
 * `config` directory, and processes test case files as they arrive. It runs continuously
 * until an error occurs or the system is terminated, then cleans up resources.
 *
 * @param argc Number of command-line arguments (not used).
 * @param argv Array of command-line arguments (not used).
 * @return int Exit code (0 for success, 1 for failure).
 */
int main(int argc, char *argv[]) {
    // Initialize the system
    init_logger();
    init_action_dispatch();

    // Initialize file system watcher
    int fd, wd;
    if (initializeFileWatcher("config", &fd, &wd) != 0) {
        cleanup_logger();
        return 1;
    }

    // Monitor directory and process test case files
    int result = monitorConfigDirectory(fd, wd);

    // Clean up resources
    cleanupFileWatcher(fd, wd);
    return result;
}