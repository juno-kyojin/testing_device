#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <signal.h>

#include "parser_option.h"
#include "parser_data.h"
#include "log.h"
#include "file_process.h"
#include "tc.h"
#include "packet_process.h"
#include "get_data.h"

// Global variables for graceful termination
static volatile sig_atomic_t run_flag = 1;
static thread_pool_t *g_thread_pool = NULL;

/**
 * @brief Signal handler for graceful termination
 * 
 * @param sig Signal number
 */
static void signal_handler(int sig) {
    static int signal_count = 0;
    
    signal_count++;
    log_message(LOG_LVL_WARN, "Received signal %d, terminating program... (attempt %d)", sig, signal_count);
    
    // Set the run flag to false to stop the main loop
    run_flag = 0;
    
    // If we receive multiple signals, force exit
    if (signal_count >= 2) {
        log_message(LOG_LVL_ERROR, "Forced exit after multiple interrupt signals");
        
        // Clean up thread pool if it exists
        if (g_thread_pool) {
            stop_thread_pool(g_thread_pool);
            g_thread_pool = NULL;
        }
        
        // Force exit if we get more than one signal
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Set up signal handlers
 */
static void setup_signal_handlers() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signal_handler;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
}

/**
 * @brief Check if a directory exists, create if not
 * 
 * @param directory Directory path
 * @return int 0 if successful, -1 on failure
 */
static int ensure_directory_exists(const char *directory) {
    struct stat st = {0};
    if (stat(directory, &st) == -1) {
        // Try to create the directory
        if (mkdir(directory, 0755) == -1) {
            log_message(LOG_LVL_ERROR, "Failed to create directory: %s", directory);
            return -1;
        }
        log_message(LOG_LVL_DEBUG, "Created directory: %s", directory);
    }
    return 0;
}

/**
 * @brief Generate a timestamp string for file naming
 * 
 * @param buffer Buffer to store the timestamp string
 * @param size Size of buffer
 */
static void generate_timestamp(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(buffer, size, "%Y%m%d_%H%M%S", tm_info);
}

int main(int argc, char *argv[]) {
    cmd_options_t options;
    test_case_t *test_cases = NULL;
    test_case_t *filtered_test_cases = NULL;
    int test_count = 0;
    int filtered_count = 0;
    test_result_info_t *results = NULL;
    int ret = EXIT_SUCCESS;
    task_queue_t *task_queue = NULL;
    thread_pool_t *thread_pool = NULL;
    connection_info_t *conn = NULL;

    // Setup signal handlers for graceful termination
    setup_signal_handlers();

    // Parse command line arguments
    if (parse_command_line(argc, argv, &options) != 0) {
        show_usage(argv[0]);
        return EXIT_FAILURE;
    }

    // Initialize logger
    set_log_file(options.log_file);
    set_log_level(options.log_level);
    init_logger();
    
    log_message(LOG_LVL_DEBUG, "Started testing device application");

    if (options.verbose) {
        print_options(&options);
    }

    // Create output directory if it doesn't exist
    if (ensure_directory_exists(options.output_directory) != 0) {
        log_message(LOG_LVL_ERROR, "Failed to create output directory");
        ret = EXIT_FAILURE;
        goto cleanup;
    }

    // Load test cases from file
    char test_cases_file[256];
    strncpy(test_cases_file, options.config_file, sizeof(test_cases_file) - 1);
    test_cases_file[sizeof(test_cases_file) - 1] = '\0';

    if (access(test_cases_file, F_OK) != 0) {
        log_message(LOG_LVL_ERROR, "Test cases file not found: %s", test_cases_file);
        ret = EXIT_FAILURE;
        goto cleanup;
    }

    log_message(LOG_LVL_DEBUG, "Reading test cases from file: %s", test_cases_file);
    if (!read_json_test_cases(test_cases_file, &test_cases, &test_count)) {
        log_message(LOG_LVL_ERROR, "Failed to read test cases");
        ret = EXIT_FAILURE;
        goto cleanup;
    }
    log_message(LOG_LVL_DEBUG, "Read %d test cases", test_count);

    // Filter test cases based on network type
    if (!filter_test_cases_by_network(test_cases, test_count, options.network_type, 
                                      &filtered_test_cases, &filtered_count)) {
        log_message(LOG_LVL_ERROR, "Failed to filter test cases by network type");
        ret = EXIT_FAILURE;
        goto cleanup;
    }
    log_message(LOG_LVL_DEBUG, "Filtered to %d test cases for network type %s", 
                filtered_count, network_type_to_string(options.network_type));

    // Create task queue and thread pool
    task_queue = init_task_queue(filtered_count * 2);  // Make queue twice the size of test cases
    if (!task_queue) {
        log_message(LOG_LVL_ERROR, "Failed to initialize task queue");
        ret = EXIT_FAILURE;
        goto cleanup;
    }

    thread_pool = init_thread_pool(options.thread_count, task_queue);
    if (!thread_pool) {
        log_message(LOG_LVL_ERROR, "Failed to initialize thread pool");
        ret = EXIT_FAILURE;
        goto cleanup;
    }
    g_thread_pool = thread_pool;  // Store in global for signal handler

    // Allocate memory for results
    results = (test_result_info_t *)calloc(filtered_count, sizeof(test_result_info_t));
    if (!results) {
        log_message(LOG_LVL_ERROR, "Failed to allocate memory for results");
        ret = EXIT_FAILURE;
        goto cleanup;
    }

    // Check if we need to connect to a remote PC
    if (strlen(options.connection_host) > 0 && strlen(options.username) > 0) {
        log_message(LOG_LVL_DEBUG, "Attempting to connect to %s:%d", 
                    options.connection_host, options.connection_port);
        conn = init_connection(options.connection_host, options.connection_port, 
                              options.username, options.password);
        if (conn) {
            log_message(LOG_LVL_DEBUG, "Connected to remote PC");
            
            // Example: Receive network configuration if needed
            network_config_t network_config;
            if (receive_network_config_from_pc(conn, &network_config) == 0) {
                log_message(LOG_LVL_DEBUG, "Received network configuration");
            }
        } else {
            log_message(LOG_LVL_WARN, "Failed to connect to remote PC, continuing in local mode");
        }
    }

    // Add test cases to the queue
    int enqueued = enqueue_test_cases_to_queue(task_queue, filtered_test_cases, filtered_count);
    log_message(LOG_LVL_DEBUG, "Enqueued %d test cases", enqueued);

    if (enqueued <= 0) {
        log_message(LOG_LVL_ERROR, "No test cases were enqueued, exiting");
        ret = EXIT_FAILURE;
        goto cleanup;
    }

    // Execute test cases
    log_message(LOG_LVL_DEBUG, "Starting test execution with %d threads", options.thread_count);
    
    // Monitor progress
    int last_completed = 0;
    while (run_flag) {
        int completed = get_completed_task_count(thread_pool);
        int success = get_success_task_count(thread_pool);
        int failed = get_failed_task_count(thread_pool);
        
        // Report progress if something changed
        if (completed > last_completed) {
            log_message(LOG_LVL_DEBUG, "Progress: %d/%d completed, %d success, %d failed", 
                       completed, enqueued, success, failed);
            last_completed = completed;
            
            // If connected to PC, send progress update
            if (conn) {
                send_progress_to_pc(conn, completed, enqueued, success, failed);
            }
        }
        
        // Check if all tasks are completed
        if (completed >= enqueued) {
            log_message(LOG_LVL_DEBUG, "All test cases completed");
            break;
        }
        
        // Sleep to avoid CPU spinning
        sleep(1);
    }

    // Process results
    log_message(LOG_LVL_DEBUG, "Processing results");
    
    // Generate timestamp for output files
    char timestamp[32];
    generate_timestamp(timestamp, sizeof(timestamp));
    
    // Generate summary report
    char summary_file[512];
    snprintf(summary_file, sizeof(summary_file), "%s/summary_%s.json", 
             options.output_directory, timestamp);
    
    if (generate_summary_report(results, filtered_count, summary_file) == 0) {
        log_message(LOG_LVL_DEBUG, "Generated summary report: %s", summary_file);
        
        // Send summary report to PC if connected
        if (conn) {
            send_compressed_results_to_pc(conn, summary_file);
        }
    } else {
        log_message(LOG_LVL_ERROR, "Failed to generate summary report");
    }

    log_message(LOG_LVL_DEBUG, "Test execution completed successfully");

cleanup:
    log_message(LOG_LVL_DEBUG, "Cleaning up resources");
    
    // Free results
    if (results) {
        free(results);
    }
    
    // Stop thread pool
    if (thread_pool) {
        stop_thread_pool(thread_pool);
        g_thread_pool = NULL;
    }
    
    // Free task queue
    if (task_queue) {
        free_task_queue(task_queue);
    }
    
    // Free test cases
    if (filtered_test_cases && filtered_test_cases != test_cases) {
        free_test_cases(filtered_test_cases, filtered_count);
    }
    
    if (test_cases) {
        free_test_cases(test_cases, test_count);
    }
    
    // Close connection to PC
    if (conn) {
        close_connection(conn);
    }
    
    // Clean up logger
    cleanup_logger();
    
    return ret;
}
