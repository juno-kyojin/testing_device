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

// Global flag for signal handling
static volatile int run_flag = 1;
static thread_pool_t *g_thread_pool = NULL;

// Signal handler
static void handle_signal(int sig) {
    run_flag = 0;
    printf("Received signal %d, stopping...\n", sig);
}

void print_test_results(test_result_info_t *results, int count) {
    printf("\n------ Test Results ------\n");
    for (int i = 0; i < count; i++) {
        printf("Test #%d: ID=%s, Status=%d, Time=%.2fms\n", 
               i+1, results[i].test_id, results[i].status, results[i].execution_time);
    }
    printf("-------------------------\n");
}

int main(int argc, char *argv[]) {
    // Set up signal handler
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    
    // Initialize log
    set_log_level(LOG_LVL_DEBUG);
    set_log_file("logs/testing_device.log");
    
    // Default config file
    const char *config_file = "config/config.json";
    int threads = 2;
    
    // Simple argument parsing
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
            config_file = argv[++i];
        } else if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) {
            threads = atoi(argv[++i]);
            if (threads < 1) threads = 1;
        }
    }
    
    printf("Using config file: %s\n", config_file);
    printf("Using %d threads\n", threads);
    
    // Create output directories
    mkdir("logs", 0755);
    mkdir("results", 0755);
    
    // Read test cases from config
    test_case_t *tests = NULL;
    int test_count = 0;
    
    if (!read_json_test_cases(config_file, &tests, &test_count)) {
        printf("Failed to read test cases from %s\n", config_file);
        return EXIT_FAILURE;
    }
    
    printf("Loaded %d test cases\n", test_count);
    
    // Create task queue and thread pool
    task_queue_t *queue = init_task_queue(test_count * 2);
    thread_pool_t *pool = init_thread_pool(threads, queue);
    g_thread_pool = pool;
    
    if (!queue || !pool) {
        printf("Failed to create thread pool\n");
        return EXIT_FAILURE;
    }
    
    // Add test cases to queue
    int enqueued = enqueue_test_cases(queue, tests, test_count);
    printf("Enqueued %d test cases\n", enqueued);
    
    // Monitor progress
    while (run_flag) {
        int completed = get_completed_count(pool);
        int success = get_success_count(pool);
        int failed = get_failed_count(pool);
        
        printf("Progress: %d/%d completed (%d success, %d failed)\r", 
               completed, enqueued, success, failed);
        fflush(stdout);
        
        if (completed >= enqueued) break;
        sleep(1);
    }
    
    printf("\nTests complete.\n");
    
    // Get results directly from thread pool
    int result_count = 0;
    test_result_info_t *results = get_results(pool, &result_count);
    
    if (results && result_count > 0) {
        // Print results for debugging
        print_test_results(results, result_count);
        
        // Generate report
        char report_file[128];
        time_t now = time(NULL);
        strftime(report_file, sizeof(report_file), "results/summary_%Y%m%d_%H%M%S.json", 
                 localtime(&now));
        
        generate_summary_report(results, result_count, report_file);
        printf("Report generated: %s\n", report_file);
    } else {
        printf("No results available to generate report\n");
    }
    
    // Clean up - but don't free results as they're owned by the thread pool
    stop_thread_pool(pool);
    free_task_queue(queue);
    free(tests);
    
    return EXIT_SUCCESS;
}
