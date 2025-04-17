#include "app_config.h"
#include "test_executor.h"
#include "log.h"

int main(int argc, char *argv[]) {
    if (app_config_init() != 0) {
        log_message(LOG_LVL_ERROR, "Failed to initialize application configuration");
        return 1;
    }

    if (execute_tests("config/wan.json") != 0) {
        log_message(LOG_LVL_ERROR, "Failed to execute tests");
        app_config_cleanup();
        return 1;
    }

    app_config_cleanup();
    return 0;
}