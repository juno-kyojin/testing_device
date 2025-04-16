/**
 * Safe Ping Plugin - Plugin an toàn để kiểm tra
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/plugin_manager.h"
#include "../include/tc.h"
#include "../include/parser_data.h"
#include "../include/log.h"

// Định nghĩa thông tin plugin
#define PLUGIN_NAME        "Safe Ping Plugin"
#define PLUGIN_DESCRIPTION "Plugin an toàn để test ping"
#define PLUGIN_VERSION     "1.0.0"
#define PLUGIN_ACTION      "ping"

// Thực thi ping test đơn giản
static int ping_execute_test(test_case_t *test_case, test_result_info_t *result, cJSON *input_params) {
    // Ghi log tới file riêng để tránh ảnh hưởng bởi lỗi trong log_message
    FILE *debug_file = fopen("/home/tobie/testing_device/var/log/safe_plugin_debug.log", "a");
    if (debug_file) {
        fprintf(debug_file, "ping_execute_test called\n");
        fclose(debug_file);
    }
    
    if (!test_case || !result) {
        return -1;
    }
    
    // Khởi tạo kết quả
    memset(result, 0, sizeof(test_result_info_t));
    snprintf(result->test_id, sizeof(result->test_id) - 1, "safe_ping");
    result->test_type = TEST_PING;
    result->status = TEST_RESULT_SUCCESS;
    
    // Thiết lập dữ liệu ping đơn giản
    result->data.ping.packets_sent = 5;
    result->data.ping.packets_received = 5;
    result->data.ping.min_rtt = 10.0;
    result->data.ping.avg_rtt = 15.0;
    result->data.ping.max_rtt = 20.0;
    result->data.ping.packet_loss = 0.0;
    
    // Thiết lập thông tin chi tiết
    snprintf(result->result_details, sizeof(result->result_details) - 1, 
            "Safe ping test completed successfully");
    
    return 0;
}

// Hàm đăng ký plugin
plugin_info_t register_plugin(void) {
    // Debug log
    FILE *debug_file = fopen("/home/tobie/testing_device/var/log/safe_plugin_debug.log", "a");
    if (debug_file) {
        fprintf(debug_file, "Safe ping plugin: register_plugin called\n");
        fclose(debug_file);
    }
    
    plugin_info_t plugin;
    memset(&plugin, 0, sizeof(plugin_info_t));
    
    // Thiết lập thông tin plugin
    strncpy(plugin.name, PLUGIN_NAME, sizeof(plugin.name) - 1);
    strncpy(plugin.description, PLUGIN_DESCRIPTION, sizeof(plugin.description) - 1);
    strncpy(plugin.version, PLUGIN_VERSION, sizeof(plugin.version) - 1);
    strncpy(plugin.action_name, PLUGIN_ACTION, sizeof(plugin.action_name) - 1);
    plugin.test_type = TEST_PING;
    
    // Khởi tạo original_action
    strncpy(plugin.original_action, PLUGIN_ACTION, sizeof(plugin.original_action) - 1);
    
    // Thiết lập hàm callback
    plugin.execute_test = ping_execute_test;
    plugin.initialize = NULL;  // Không cần hàm initialize
    plugin.cleanup = NULL;     // Không cần hàm cleanup
    
    debug_file = fopen("/home/tobie/testing_device/var/log/safe_plugin_debug.log", "a");
    if (debug_file) {
        fprintf(debug_file, "Safe ping plugin: register_plugin completed\n");
        fclose(debug_file);
    }
    
    return plugin;
}
