#include <stdio.h>
#include "config/app_config.h"
#include "test/test_executor.h"
#include "action/action_registry.h"
#include "core/log.h"

int main(int argc, char *argv[]) {
    // Check if there are any command-line arguments
    if (argc < 2) {
        log_message(LOG_LVL_ERROR, "No test configuration files provided. Usage: %s <config_file1> [config_file2 ...]", argv[0]);
        return 1;
    }

    // Initialize configuration from config.json
    if (app_config_init() != 0) {
        log_message(LOG_LVL_ERROR, "Failed to initialize application configuration");
        return 1;
    }

    // Initialize the action dispatch table
    init_action_dispatch();

    // Execute test cases from all provided configuration files
    for (int i = 1; i < argc; i++) {
        log_message(LOG_LVL_DEBUG, "Processing test configuration file: %s", argv[i]);
        if (execute_tests(argv[i]) != 0) {
            log_message(LOG_LVL_ERROR, "Failed to execute tests from %s", argv[i]);
            app_config_cleanup();
            return 1;
        }
    }

    // Clean up resources
    app_config_cleanup();
    return 0;
}