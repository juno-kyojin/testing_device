/**
 * @file test_case_handler.c
 * @brief Implementation of test case handling utilities
 *
 * This file implements utilities for handling test case files in the test case execution
 * system. It parses test case files, executes the test cases, writes the results to a JSON
 * file, and moves the processed files to the `processed` directory to prevent reprocessing.
 *
 * @author [junokyojin]
 * @date 2025-04-23
 * @see test_case_handler.h
 * @see parser.h
 * @see action.h
 * @see file_process.h
 * @see log.h
 */
#include "test_case_handler.h"
#include "parser.h"
#include "action.h"
#include "file_process.h"
#include "log.h"
#include <sys/stat.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

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
 * @brief Move a test case file to the processed directory
 *
 * This function moves a test case file to the `processed` directory to prevent reprocessing.
 * If the `processed` directory does not exist, it is created automatically with appropriate
 * permissions.
 *
 * @param filepath Path to the test case file to move (e.g., "config/ping.json").
 * @return None
 */
static void moveToProcessedDir(const char *filepath) {
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
 * @brief Write test case results to a JSON file with a summary of failures
 *
 * This function writes the test case results to a JSON file in the `result` directory,
 * including a summary of the test execution. The summary contains the total number of
 * test cases, the number of passed test cases, the failure percentage, and a breakdown
 * of failure reasons with their counts. Only failed test cases are listed in detail,
 * with their host and failure reason. The file name includes a timestamp to prevent
 * overwriting previous results. If the `result` directory does not exist, it is created
 * automatically with appropriate permissions.
 *
 * @param filepath Path to the original test case file (e.g., "config/ping.json").
 * @param results Pointer to the `cJSON` object containing the test case results.
 * @param total_test_cases Total number of test cases (including pass and fail).
 * @return None
 */
static void writeTestResults(const char *filepath, cJSON *results, int total_test_cases) {
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

    // Remove the .json extension from the filename
    char base_filename[256];
    strncpy(base_filename, filename, sizeof(base_filename) - 1);
    base_filename[sizeof(base_filename) - 1] = '\0';
    char *dot = strrchr(base_filename, '.');
    if (dot && strcmp(dot, ".json") == 0) {
        *dot = '\0'; // Truncate at the .json extension
    }

    // Create the result file name: result/<base_filename>_<timestamp>.json
    char result_filepath[512];
    snprintf(result_filepath, sizeof(result_filepath), "result/%s_%s.json", base_filename, timestamp);

    // Calculate summary
    cJSON *failed_test_cases_array = cJSON_GetObjectItem(results, "failed_test_cases");
    int failed = cJSON_GetArraySize(failed_test_cases_array); // Number of failed test cases
    int passed = total_test_cases - failed; // Number of passed test cases

    // Create an object to store failure reasons
    cJSON *failure_reasons = cJSON_CreateObject();

    // Count failure reasons
    for (int i = 0; i < failed; i++) {
        cJSON *test_case = cJSON_GetArrayItem(failed_test_cases_array, i);
        cJSON *fail_reason = cJSON_GetObjectItem(test_case, "fail_reason");
        if (fail_reason && cJSON_IsString(fail_reason)) {
            const char *reason = fail_reason->valuestring;
            cJSON *reason_count = cJSON_GetObjectItem(failure_reasons, reason);
            if (reason_count) {
                reason_count->valueint++;
            } else {
                cJSON_AddNumberToObject(failure_reasons, reason, 1);
            }
        }
    }

    // Calculate failure percentage
    double fail_percentage = (total_test_cases > 0) ? ((double)failed / total_test_cases) * 100.0 : 0.0;

    // Create summary object
    cJSON *summary = cJSON_CreateObject();
    cJSON_AddNumberToObject(summary, "total_test_cases", total_test_cases);
    cJSON_AddNumberToObject(summary, "passed", passed);
    cJSON_AddNumberToObject(summary, "fail_percentage", fail_percentage);
    cJSON_AddItemToObject(summary, "failure_reasons", failure_reasons);

    // Add summary to results
    cJSON_AddItemToObject(results, "summary", summary);

    // Convert results to JSON string
    char *json_str = cJSON_Print(results);
    if (!json_str) {
        log_message(LOG_LVL_ERROR, "Failed to print results to JSON string");
        return;
    }

    // Write JSON string to file using write_file from file_process.h
    if (write_file(result_filepath, json_str, strlen(json_str)) != 0) {
        free(json_str);
        return;
    }

    free(json_str);
}

void processTestCaseFile(const char *filepath) {
    TestCase test_cases[MAX_TEST_CASES];

    // Create an array to store failed test cases
    cJSON *results = cJSON_CreateObject();
    cJSON *result_array = cJSON_CreateArray();
    cJSON_AddItemToObject(results, "failed_test_cases", result_array);

    // Parse file test case
    int count = 0;
    if (parse_test_cases(filepath, test_cases, &count, results) != 0) {
        // If parsing fails, log the failure with a reason
        cJSON *result_json = cJSON_CreateObject();
        cJSON_AddStringToObject(result_json, "service", "unknown");
        cJSON_AddStringToObject(result_json, "action", "unknown");
        cJSON_AddStringToObject(result_json, "host", "");
        cJSON_AddStringToObject(result_json, "status", "fail");
        if (!cJSON_GetObjectItem(result_json, "fail_reason")) {
            cJSON_AddStringToObject(result_json, "fail_reason", "Unknown error");
        }
        cJSON_AddItemToArray(result_array, result_json);

        // Write results with total_test_cases = 0
        writeTestResults(filepath, results, 0);
    } else {
        // If parsing succeeds, execute the test cases
        for (int i = 0; i < count; i++) {
            execute_action(&test_cases[i], filepath, i, result_array);
        }

        // Write results with the total test case count
        writeTestResults(filepath, results, count);
    }

    // Free memory
    cJSON_Delete(results);

    // Move the file to the processed directory
    moveToProcessedDir(filepath);
}