/**
 * Simple Ping Plugin - Plugin đơn giản để debug
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../include/plugin_manager.h"
#include "../include/tc.h"
#include "../include/parser_data.h"

// Định nghĩa thông tin plugin
#define PLUGIN_NAME        "Simple Ping Test Plugin"
#define PLUGIN_DESCRIPTION "Plugin đơn giản để debug"
#define PLUGIN_VERSION     "1.0.0"
#define PLUGIN_ACTION      "ping"

// Hàm đơn giản để ghi log vào file để debug
static void debug_log(const char *message) {
    FILE *log_file = fopen("/home/tobie/testing_device/var/log/simple_plugin.log", "a");
    if (log_file) {
        fprintf(log_file, "[DEBUG] %s\n", message);
        fclose(log_file);
    }
}

/**
 * @brief Thực thi ping test đơn giản
 */
static int ping_execute_test(test_case_t *test_case, test_result_info_t *result, cJSON *input_params) {
    // Ghi log để debug
    debug_log("ping_execute_test called");
    
    if (!test_case || !result) {
        debug_log("Invalid parameters");
        return -1;
    }
    
    // Ghi log về thông số đầu vào
    char debug_buf[256];
    snprintf(debug_buf, sizeof(debug_buf), "Target: %s", test_case->target);
    debug_log(debug_buf);
    
    // Khởi tạo kết quả với dữ liệu đơn giản để tránh segfault
    memset(result, 0, sizeof(test_result_info_t));
    strncpy(result->test_id, test_case->id, sizeof(result->test_id) - 1);
    result->test_type = TEST_PING;
    result->status = TEST_RESULT_SUCCESS; // Luôn trả về thành công
    
    // Thiết lập kết quả mẫu
    result->data.ping.packets_sent = 5;
    result->data.ping.packets_received = 5;
    result->data.ping.min_rtt = 10.0;
    result->data.ping.avg_rtt = 15.0;
    result->data.ping.max_rtt = 20.0;
    result->data.ping.packet_loss = 0.0;
    
    snprintf(result->result_details, sizeof(result->result_details), 
             "Simple ping test to %s completed successfully", 
             test_case->target[0] ? test_case->target : "unknown");
    
    debug_log("ping_execute_test completed successfully");
    return 0;
}

// Hàm khởi tạo plugin đơn giản
static int ping_initialize(void) {
    debug_log("Simple ping plugin initialized");
    return 0;
}

// Hàm giải phóng tài nguyên
static void ping_cleanup(void) {
    debug_log("Simple ping plugin cleaned up");
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
    plugin.test_type = TEST_PING;
    
    // Thiết lập các hàm callback
    plugin.execute_test = ping_execute_test;
    plugin.initialize = ping_initialize;
    plugin.cleanup = ping_cleanup;
    
    debug_log("register_plugin completed");
    return plugin;
}
