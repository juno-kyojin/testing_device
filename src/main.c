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

/**
 * @brief Initialize the application
 * 
 * @param log_file Path to log file
 * @return int 0 on success, -1 on failure
 */
int initialize_app(const char *log_file) {
    // Initialize logger
    set_log_level(LOG_LVL_DEBUG);
    set_log_file(log_file);
    
    // Create output directories if they don't exist
    if (!file_exists("logs")) {
        if (create_directory("logs") != 0) {
            printf("Failed to create logs directory\n");
            return -1;
        }
    }
    
    if (!file_exists("results")) {
        if (create_directory("results") != 0) {
            printf("Failed to create results directory\n");
            return -1;
        }
    }
    
    return 0;
}

/**
 * @brief Load test cases from config file
 * 
 * @param config_file Path to config file
 * @param tests Pointer to test cases array
 * @param test_count Pointer to test count variable
 * @return int 0 on success, -1 on failure
 */
int load_test_cases(const char *config_file, test_case_t **tests, int *test_count) {
    printf("Using config file: %s\n", config_file);
    
    log_message(LOG_LVL_DEBUG, "Reading test cases from %s", config_file);
    if (!read_json_test_cases(config_file, tests, test_count)) {
        log_message(LOG_LVL_ERROR, "Failed to read test cases from %s", config_file);
        printf("Failed to read test cases from %s\n", config_file);
        return -1;
    }
    
    printf("Loaded %d test cases\n", *test_count);
    return 0;
}

/**
 * @brief Execute all test cases
 * 
 * @param tests Array of test cases
 * @param test_count Number of test cases
 * @param results Array to store results
 * @return int 0 on success, -1 on failure 
 */
int execute_tests(test_case_t *tests, int test_count, test_result_info_t *results) {
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
    return 0;
}

/**
 * @brief Generate test report
 * 
 * @param results Array of test results
 * @param test_count Number of test results
 * @return int 0 on success, -1 on failure
 */
int generate_report(test_result_info_t *results, int test_count) {
    if (test_count <= 0) {
        printf("No results available to generate report\n");
        return -1;
    }
    
    char report_file[128];
    time_t now = time(NULL);
    strftime(report_file, sizeof(report_file), "results/summary_%Y%m%d_%H%M%S.json", 
             localtime(&now));
    
    if (generate_summary_report(results, test_count, report_file) == 0) {
        printf("Report generated: %s\n", report_file);
        return 0;
    } else {
        printf("Failed to generate report\n");
        return -1;
    }
}

/**
 * @brief Clean up resources
 * 
 * @param tests Test cases array
 * @param results Test results array
 * @param test_count Number of tests
 */
void cleanup(test_case_t *tests, test_result_info_t *results, int test_count) {
    if (results) {
        free(results);
    }
    
    if (tests) {
        free_test_cases(tests, test_count);
    }
}

/**
 * @brief Parse command line arguments
 * 
 * @param argc Argument count
 * @param argv Argument values
 * @param config_file Pointer to config file path
 */
void parse_arguments(int argc, char *argv[], const char **config_file) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
            *config_file = argv[++i];
        }
    }
}

int main(int argc, char *argv[]) {
    // Set up signal handlers
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    
    // Default config file path
    const char *config_file = "config/config.json";
    
    // Parse command line arguments
    parse_arguments(argc, argv, &config_file);
    
    // Initialize application
    if (initialize_app("logs/testing_device.log") != 0) {
        return EXIT_FAILURE;
    }
    
    // Load test cases
    test_case_t *tests = NULL;
    int test_count = 0;
    if (load_test_cases(config_file, &tests, &test_count) != 0) {
        return EXIT_FAILURE;
    }
    
    // Allocate memory for results
    test_result_info_t *results = (test_result_info_t*)malloc(test_count * sizeof(test_result_info_t));
    if (!results) {
        log_message(LOG_LVL_ERROR, "Failed to allocate memory for results");
        printf("Failed to allocate memory for results\n");
        free_test_cases(tests, test_count);
        return EXIT_FAILURE;
    }
    
    // Execute tests
    execute_tests(tests, test_count, results);
    
    // Print results
    print_test_results(results, test_count);
    
    // Generate report
    generate_report(results, test_count);
    
    // Clean up
    cleanup(tests, results, test_count);
    
    return EXIT_SUCCESS;
}