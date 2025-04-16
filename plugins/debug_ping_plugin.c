/**
 * Debug Ping Plugin - Plugin đúng tên để khớp với config
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <cjson/cJSON.h>
#include "../include/plugin_manager.h"
#include "../include/tc.h"
#include "../include/parser_data.h"
#include "../include/log.h"

// Định nghĩa thông tin plugin - Phải đúng tên như trong config
#define PLUGIN_NAME        "Debug Ping Plugin"
#define PLUGIN_DESCRIPTION "Plugin thực hiện test ping để debug"
#define PLUGIN_VERSION     "1.0.0"
#define PLUGIN_ACTION      "ping"

// Ghi log thêm để debug
static void debug_log(const char *msg) {
    FILE *f = fopen("/home/tobie/testing_device/var/log/debug_ping.log", "a");
    if (f) {
        fprintf(f, "%s\n", msg);
        fclose(f);
    }
}

/**
 * @brief Thực thi ping test
 */
static int ping_execute_test(test_case_t *test_case, test_result_info_t *result, cJSON *input_params) {
    debug_log("ping_execute_test called");
    
    if (!test_case || !result) {
        debug_log("Invalid parameters");
        return -1;
    }
    
    // Khởi tạo kết quả
    memset(result, 0, sizeof(test_result_info_t));
    strncpy(result->test_id, test_case->id, sizeof(result->test_id) - 1);
    result->test_type = TEST_PING;
    result->status = TEST_RESULT_SUCCESS;
    
    // Thiết lập kết quả mẫu
    result->data.ping.packets_sent = 5;
    result->data.ping.packets_received = 5;
    result->data.ping.min_rtt = 10.0;
    result->data.ping.avg_rtt = 15.0;
    result->data.ping.max_rtt = 20.0;
    result->data.ping.packet_loss = 0.0;
    
    // Thiết lập thông tin chi tiết
    snprintf(result->result_details, sizeof(result->result_details) - 1, 
             "Debug Ping test to %s completed successfully via plugin", 
             test_case->target[0] ? test_case->target : "unknown");
    
    debug_log("ping_execute_test completed");
    return 0;
}

// Hàm đăng ký plugin
plugin_info_t register_plugin(void) {
    debug_log("register_plugin called");
    
    plugin_info_t plugin;
    memset(&plugin, 0, sizeof(plugin_info_t));
    
    // Thiết lập thông tin plugin
    strncpy(plugin.name, PLUGIN_NAME, sizeof(plugin.name) - 1);
    strncpy(plugin.description, PLUGIN_DESCRIPTION, sizeof(plugin.description) - 1);
    strncpy(plugin.version, PLUGIN_VERSION, sizeof(plugin.version) - 1);
    strncpy(plugin.action_name, PLUGIN_ACTION, sizeof(plugin.action_name) - 1);
    strncpy(plugin.original_action, PLUGIN_ACTION, sizeof(plugin.original_action) - 1);
    plugin.test_type = TEST_PING;
    
    // Thiết lập các hàm callback
    plugin.execute_test = ping_execute_test;
    plugin.initialize = NULL;
    plugin.cleanup = NULL;
    
    debug_log("register_plugin completed");
    return plugin;
}
