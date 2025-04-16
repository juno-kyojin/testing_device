/**
 * Example Plugin - Template cho việc tạo plugin mới
 * 
 * Cách sử dụng:
 * 1. Sao chép file này và đổi tên thành <tên_plugin>.c
 * 2. Chỉnh sửa thông tin plugin, triển khai chức năng execute_test
 * 3. Biên dịch: gcc -shared -fPIC -o <tên_plugin>.so <tên_plugin>.c -ldl
 * 4. Đặt file .so vào thư mục plugins
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h>
#include "../include/plugin_manager.h"
#include "../include/tc.h"
#include "../include/parser_data.h"
#include "../include/log.h"

// Định nghĩa thông tin plugin
#define PLUGIN_NAME        "Example Plugin"
#define PLUGIN_DESCRIPTION "Mẫu plugin để tham khảo"
#define PLUGIN_VERSION     "1.0.0"
#define PLUGIN_ACTION      "example"   // Tên action đăng ký, sẽ dùng trong file JSON

// Hàm thực thi test của plugin
static int example_execute_test(test_case_t *test_case, test_result_info_t *result, cJSON *input_params) {
    if (!test_case || !result) {
        return -1;
    }
    
    log_message(LOG_LVL_DEBUG, "Example plugin executing test");
    
    // Khởi tạo kết quả test
    memset(result, 0, sizeof(test_result_info_t));
    strncpy(result->test_id, test_case->id, sizeof(result->test_id) - 1);
    result->test_id[sizeof(result->test_id) - 1] = '\0';
    result->test_type = TEST_OTHER;  // hoặc định nghĩa loại test mới
    
    // Kiểm tra input_params
    if (input_params) {
        cJSON *param = cJSON_GetObjectItem(input_params, "example_param");
        if (param && cJSON_IsString(param)) {
            log_message(LOG_LVL_DEBUG, "Got example_param: %s", param->valuestring);
        }
    }
    
    // Thực hiện test case
    // TODO: Triển khai logic test case của bạn ở đây
    
    // Giả lập kết quả test thành công
    result->status = TEST_RESULT_SUCCESS;
    result->execution_time = 100.0;  // 100ms
    snprintf(result->result_details, sizeof(result->result_details),
             "Example test executed successfully");
    
    return 0;
}

// Hàm khởi tạo plugin (tùy chọn)
static int example_initialize(void) {
    log_message(LOG_LVL_DEBUG, "Example plugin initialized");
    return 0;
}

// Hàm giải phóng tài nguyên (tùy chọn)
static void example_cleanup(void) {
    log_message(LOG_LVL_DEBUG, "Example plugin cleaned up");
}

// Hàm đăng ký plugin (bắt buộc)
// KHÔNG thay đổi tên hàm này, plugin manager sẽ gọi hàm này khi nạp plugin
plugin_info_t register_plugin(void) {
    plugin_info_t plugin;
    memset(&plugin, 0, sizeof(plugin_info_t));
    
    // Thiết lập thông tin plugin
    strncpy(plugin.name, PLUGIN_NAME, sizeof(plugin.name) - 1);
    strncpy(plugin.description, PLUGIN_DESCRIPTION, sizeof(plugin.description) - 1);
    strncpy(plugin.version, PLUGIN_VERSION, sizeof(plugin.version) - 1);
    strncpy(plugin.action_name, PLUGIN_ACTION, sizeof(plugin.action_name) - 1);
    
    // Thiết lập các hàm callback
    plugin.execute_test = example_execute_test;
    plugin.initialize = example_initialize;
    plugin.cleanup = example_cleanup;
    
    return plugin;
}
