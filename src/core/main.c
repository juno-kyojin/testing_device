/**
 * @file main.c
 * @brief Main entry point for the test case execution system
 *
 * This file serves as the main entry point for the test case execution system.
 * It initializes the system, sets up a file system watcher using `inotify` to monitor
 * the `config` directory for new test case files in JSON format. When a new test case
 * file is detected, the system processes the file by parsing its test cases, executing
 * the corresponding actions, and writing the results to a JSON file in the `result`
 * directory. Processed test case files are then moved to the `processed` directory to
 * avoid reprocessing.
 *
 * The system is designed to run continuously as a daemon, automatically started at
 * boot via a systemd service (`testing_device.service`), ensuring uninterrupted test
 * case execution.
 *
 * @author [Your Name]
 * @date 2025-04-23
 * @see action.h
 * @see parser.h
 * @see file_process.h
 * @see log.h
 * @see types.h
 */
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/inotify.h>  
#include <unistd.h>
#include <time.h>
#include "file_process.h"
#include "action.h"
#include "action_registry.h"
#include "parser.h"
#include "log.h"
#include "types.h"
#include "cjson/cJSON.h"

/**
 * @def MAX_TEST_CASES
 * @brief Maximum number of test cases that can be processed from a single file
 *
 * Defines the maximum number of test cases that the system can handle from a single
 * test case file. Currently set to 100 to prevent excessive memory usage and ensure
 * system stability.
 */
#define MAX_TEST_CASES 100

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
 * @brief Write test case results to a JSON file with a timestamp
 *
 * This function writes the test case results to a JSON file in the `result` directory.
 * The file name includes a timestamp to prevent overwriting previous results. If the
 * `result` directory does not exist, it is created automatically with appropriate
 * permissions.
 *
 * @param filepath Path to the original test case file (e.g., "config/ping.json").
 * @param results Pointer to the `cJSON` object containing the test case results.
 * @return None
 */
void write_results_to_file(const char *filepath, cJSON *results) {
    // Create the result directory if it does not exist
    struct stat st = {0};
    if (stat("result", &st) == -1) {
        mkdir("result", 0755);
        log_message(LOG_LVL_DEBUG, "Created directory: result");
    }

    // Get the current time
    time_t rawtime;
    struct tm *timeinfo;
    char timestamp[20];
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(timestamp, sizeof(timestamp), "%Y%m%d%H%M%S", timeinfo); // Format: YYYYMMDDHHMMSS

    // Extract the filename from the path
    char *filename = strrchr(filepath, '/');
    if (filename) {
        filename++; // Skip the '/' character
    } else {
        filename = (char *)filepath;
    }

    // Create the result file name: result/<filename>_<timestamp>_result.json
    char result_filepath[512];
    snprintf(result_filepath, sizeof(result_filepath), "result/%s_%s_result.json", filename, timestamp);

    // Write results to the file
    char *json_str = cJSON_Print(results);
    if (!json_str) {
        log_message(LOG_LVL_ERROR, "Failed to print results to JSON string");
        return;
    }

    FILE *fp = fopen(result_filepath, "w");
    if (!fp) {
        log_message(LOG_LVL_ERROR, "Failed to open result file %s for writing", result_filepath);
        free(json_str);
        return;
    }

    fprintf(fp, "%s", json_str);
    fclose(fp);
    free(json_str);
    log_message(LOG_LVL_DEBUG, "Results written to %s", result_filepath);
}

/**
 * @brief Process a single test case file
 *
 * This function processes a test case file by parsing its contents, executing each
 * test case, and writing the results to a JSON file. After processing, the file is
 * moved to the `processed` directory to prevent reprocessing. If the `processed`
 * directory does not exist, it is created automatically with appropriate permissions.
 *
 * @param filepath Path to the test case file to process (e.g., "config/ping.json").
 * @return None
 */
void process_test_case_file(const char *filepath) {
    TestCase test_cases[MAX_TEST_CASES];
    int test_case_count = 0;

    int count = 0;
    if (parse_test_cases(filepath, &test_cases[test_case_count], &count) != 0) {
        log_message(LOG_LVL_ERROR, "Failed to parse test cases from %s", filepath);
        return;
    }

    // Create an array to store results
    cJSON *results = cJSON_CreateObject();
    cJSON *result_array = cJSON_CreateArray();
    cJSON_AddItemToObject(results, "test_cases", result_array);

    for (int i = 0; i < count; i++) {
        execute_action(&test_cases[test_case_count + i], filepath, i, result_array);
    }

    // Write results to a JSON file
    write_results_to_file(filepath, results);

    // Free memory
    cJSON_Delete(results);

    test_case_count += count;
    if (test_case_count >= MAX_TEST_CASES) {
        log_message(LOG_LVL_ERROR, "Too many test cases, stopping");
        return;
    }

    // Move the file to the processed directory
    struct stat st = {0};
    if (stat("processed", &st) == -1) {
        mkdir("processed", 0755);
        log_message(LOG_LVL_DEBUG, "Created directory: processed");
    }

    char processed_filepath[512];
    char *filename = strrchr(filepath, '/');
    if (filename) {
        filename++; // Skip the '/' character
    } else {
        filename = (char *)filepath;
    }
    snprintf(processed_filepath, sizeof(processed_filepath), "processed/%s", filename);

    if (rename(filepath, processed_filepath) == 0) {
        log_message(LOG_LVL_DEBUG, "Moved file to %s", processed_filepath);
    } else {
        log_message(LOG_LVL_ERROR, "Failed to move file to %s", processed_filepath);
    }
}

/**
 * @brief Main entry point for the test case execution system
 *
 * This function initializes the system, sets up an `inotify` watcher to monitor the
 * `config` directory for new test case files, and processes them as they arrive.
 * It runs continuously, handling test case execution, result logging, and file management.
 * The system is designed to run as a systemd service, starting automatically at boot.
 *
 * @param argc Number of command-line arguments (not used).
 * @param argv Array of command-line arguments (not used).
 * @return int Exit code (0 for success, 1 for failure).
 */
int main(int argc, char *argv[]) {
    init_logger();
    init_action_dispatch();

    // Create an inotify instance
    int fd = inotify_init();
    if (fd < 0) {
        log_message(LOG_LVL_ERROR, "Failed to initialize inotify");
        cleanup_logger();
        return 1;
    }

    // Monitor the config directory
    int wd = inotify_add_watch(fd, "config", IN_CREATE | IN_MOVED_TO);
    if (wd < 0) {
        log_message(LOG_LVL_ERROR, "Failed to add watch for config directory");
        close(fd);
        cleanup_logger();
        return 1;
    }

    log_message(LOG_LVL_DEBUG, "Started monitoring config directory for new test case files...");

    char buffer[BUF_LEN];
    while (1) {
        int length = read(fd, buffer, BUF_LEN);
        if (length < 0) {
            log_message(LOG_LVL_ERROR, "Failed to read inotify events");
            break;
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
                        process_test_case_file(filepath);
                    }
                }
            }
            i += EVENT_SIZE + event->len;
        }
    }

    // Cleanup
    inotify_rm_watch(fd, wd);
    close(fd);
    cleanup_logger();
    return 0;
}