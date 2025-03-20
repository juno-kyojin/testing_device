/**
 * @file main.c
 * @brief Main program for network testing device
 * @date 2025-03-20
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <signal.h>

#include "parser_data.h"
#include "log.h"
#include "file_process.h"
#include "tc.h"

// Global flag for signal handling
static volatile int run_flag = 1;

// Signal handler
static void handle_signal(int sig) {
    run_flag = 0;
    printf("Received signal %d, stopping...\n", sig);
}

/**
 * @brief Print test results to console
 * 
 * @param results Array of test results
 * @param count Number of results
 */
void print_test_results(test_result_info_t *results, int count) {
    printf("\n------ Test Results ------\n");
    for (int i = 0; i < count; i++) {
        printf("Test #%d: ID=%s, Status=%s, Time=%.2fms\n", 
               i+1, results[i].test_id, 
               test_result_status_to_string(results[i].status), 
               results[i].execution_time);
        printf("  Details: %s\n", results[i].result_details);
    }
    printf("-------------------------\n");
}

int main(int argc, char *argv[]) {
    // Set up signal handlers
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    
    // Initialize logger
    set_log_level(LOG_LVL_DEBUG);
    set_log_file("logs/testing_device.log");
    
    // Default config file path
    const char *config_file = "config/config.json";
    
    // Simple argument parsing
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
            config_file = argv[++i];
        }
    }
    
    printf("Using config file: %s\n", config_file);
    
    // Create output directories if they don't exist
    if (!file_exists("logs")) {
        create_directory("logs");
    }
    if (!file_exists("results")) {
        create_directory("results");
    }
    
    // Read test cases from config file
    test_case_t *tests = NULL;
    int test_count = 0;
    
    log_message(LOG_LVL_DEBUG, "Reading test cases from %s", config_file);
    if (!read_json_test_cases(config_file, &tests, &test_count)) {
        log_message(LOG_LVL_ERROR, "Failed to read test cases from %s", config_file);
        printf("Failed to read test cases from %s\n", config_file);
        return EXIT_FAILURE;
    }
    
    printf("Loaded %d test cases\n", test_count);
    
    // Allocate memory for results
    test_result_info_t *results = (test_result_info_t*)malloc(test_count * sizeof(test_result_info_t));
    if (!results) {
        log_message(LOG_LVL_ERROR, "Failed to allocate memory for results");
        printf("Failed to allocate memory for results\n");
        free(tests);
        return EXIT_FAILURE;
    }
    
    // Track execution statistics
    int success_count = 0;
    int failed_count = 0;
    
    printf("Executing test cases...\n");
    
    // Execute each test case sequentially
    for (int i = 0; i < test_count && run_flag; i++) {
        printf("Running test case %d/%d: %s (%s)\n", 
               i+1, test_count, tests[i].id, tests[i].name);
        
        // Execute the test case
        int ret = execute_test_case(&tests[i], &results[i]);
        
        // Update statistics
        if (ret == 0) {
            if (results[i].status == TEST_RESULT_SUCCESS) {
                printf("  Result: SUCCESS\n");
                success_count++;
            } else {
                printf("  Result: %s\n", test_result_status_to_string(results[i].status));
                failed_count++;
            }
        } else {
            printf("  Result: EXECUTION FAILED\n");
            failed_count++;
        }
        
        // Show progress
        printf("Progress: %d/%d completed (%d success, %d failed)\n",
               i+1, test_count, success_count, failed_count);
    }
    
    printf("\nTests complete.\n");
    
    // Print detailed results
    print_test_results(results, test_count);
    
    // Generate summary report
    if (test_count > 0) {
        char report_file[128];
        time_t now = time(NULL);
        strftime(report_file, sizeof(report_file), "results/summary_%Y%m%d_%H%M%S.json", 
                 localtime(&now));
        
        if (generate_summary_report(results, test_count, report_file) == 0) {
            printf("Report generated: %s\n", report_file);
        } else {
            printf("Failed to generate report\n");
        }
    } else {
        printf("No results available to generate report\n");
    }
    
    // Cleanup resources
    free(results);
    free_test_cases(tests, test_count);
    
    return EXIT_SUCCESS;
}