/**
 * Minimal Ping Plugin - Plugin đơn giản để test
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/plugin_manager.h"
#include "../include/tc.h"
#include "../include/parser_data.h"
#include "../include/log.h"

// Cấu trúc plugin đơn giản
#define PLUGIN_NAME        "Minimal Ping Plugin"
#define PLUGIN_DESCRIPTION "Plugin đơn giản để test hệ thống plugin"
#define PLUGIN_VERSION     "1.0.0"
#define PLUGIN_ACTION      "ping"

// Ghi log để debug
static void debug_log(const char *msg) {
    FILE *log = fopen("/home/tobie/testing_device/var/log/minimal_plugin.log", "a");
    if (log) {
        fprintf(log, "%s\n", msg);
        fclose(log);
    }
}

// Thực thi ping test đơn giản
static int ping_execute_test(test_case_t *test_case, test_result_info_t *result, cJSON *input_params) {
    debug_log("ping_execute_test called");
    
    if (!test_case || !result) {
        debug_log("invalid parameters");
        return -1;
    }
    
    // Khởi tạo kết quả đơn giản
    memset(result, 0, sizeof(test_result_info_t));
    strncpy(result->test_id, test_case->id, sizeof(result->test_id) - 1);
    result->test_type = TEST_PING;
    result->status = TEST_RESULT_SUCCESS;
    
    // Giả lập kết quả ping
    result->data.ping.packets_sent = 5;
    result->data.ping.packets_received = 5;
    result->data.ping.min_rtt = 10.0;
    result->data.ping.avg_rtt = 15.0;
    result->data.ping.max_rtt = 20.0;
    result->data.ping.packet_loss = 0.0;
    
    snprintf(result->result_details, sizeof(result->result_details) - 1, 
             "Minimal ping test to %s completed successfully", 
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
    plugin.initialize = NULL;  // Không cần khởi tạo
    plugin.cleanup = NULL;     // Không cần giải phóng
    
    debug_log("register_plugin completed");
    return plugin;
}
