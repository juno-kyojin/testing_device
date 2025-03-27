#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser_data.h"
#include "cjson/cJSON.h"
#include "tc.h"
#include "log.h"

// Biến toàn cục để lưu kết quả ping
static test_result_info_t last_ping_result;
static bool ping_executed = false;

// Thông tin test case ping hiện tại
static test_case_t current_ping;

// Stub function cho tcapi_get - sử dụng kết quả thực tế
int tcapi_get(const char *node, const char *entry, const char *attribute, char *value) {
    // Nếu chưa thực hiện ping, trả về giá trị mặc định
    if (!ping_executed) {
        if (strcmp(attribute, "Status") == 0) {
            strcpy(value, "-1");
        } else {
            strcpy(value, "N/A");
        }
        return 0;
    }
    
    // Trả về kết quả thực tế từ việc thực hiện ping
    if (strcmp(attribute, "Status") == 0) {
        // 0 = success, 1 = failed, 2 = timeout, 3 = error
        switch (last_ping_result.status) {
            case TEST_RESULT_SUCCESS:
                strcpy(value, "0");
                break;
            case TEST_RESULT_FAILED:
                strcpy(value, "1");
                break;
            case TEST_RESULT_TIMEOUT:
                strcpy(value, "2");
                break;
            case TEST_RESULT_ERROR:
            default:
                strcpy(value, "3");
        }
    } else if (strcmp(attribute, "host") == 0) {
        strcpy(value, current_ping.target);
    } else if (strcmp(attribute, "hostAddress") == 0) {
        // Sử dụng target như địa chỉ vì đó là IP/hostname mà chúng ta ping
        strcpy(value, current_ping.target);
    } else if (strcmp(attribute, "successCount") == 0) {
        sprintf(value, "%d", last_ping_result.data.ping.packets_received);
    } else if (strcmp(attribute, "failureCount") == 0) {
        int failures = last_ping_result.data.ping.packets_sent - last_ping_result.data.ping.packets_received;
        sprintf(value, "%d", failures > 0 ? failures : 0);
    } else if (strcmp(attribute, "averageResponseTime") == 0) {
        sprintf(value, "%.1f", last_ping_result.data.ping.avg_rtt);
    } else if (strcmp(attribute, "minimumResponseTime") == 0) {
        sprintf(value, "%.1f", last_ping_result.data.ping.min_rtt);
    } else if (strcmp(attribute, "maximumResponseTime") == 0) {
        sprintf(value, "%.1f", last_ping_result.data.ping.max_rtt);
    } else {
        return -1;
    }
    return 0;
}

// Stub function cho tcapi_set - được giữ lại để ghi log
int tcapi_set(const char *node, const char *entry, const char *attribute, const char *value) {
    printf("Setting %s.%s.%s = %s\n", node, entry, attribute, value);
    
    // Nếu đang thiết lập host cho ping, cập nhật target
    if (strcmp(attribute, "host") == 0) {
        strncpy(current_ping.target, value, sizeof(current_ping.target) - 1);
        current_ping.target[sizeof(current_ping.target) - 1] = '\0';
    }
    
    return 0;
}

// Function để thực hiện ping thực
int execute_instruction(const instruction_t *instruction, cJSON *input_params) {
    if (!instruction) return -1;
    
    printf("Executing instruction: %s\n", instruction->action);
    
    // Khởi tạo test case với giá trị mặc định
    memset(&current_ping, 0, sizeof(test_case_t));
    strcpy(current_ping.id, "ping_test");
    current_ping.type = TEST_PING;
    current_ping.timeout = 5000;  // 5 giây timeout
    current_ping.enabled = true;
    
    // Thiết lập tham số ping
    current_ping.params.ping.count = 5;         // 5 gói tin
    current_ping.params.ping.size = 64;         // 64 byte
    current_ping.params.ping.interval = 1000;   // 1 giây giữa các lần ping
    current_ping.params.ping.ipv6 = false;      // Sử dụng IPv4
    
    // Nếu có input_params, đọc host
    if (input_params) {
        cJSON *host = cJSON_GetObjectItem(input_params, "host");
        if (host && cJSON_IsString(host)) {
            strncpy(current_ping.target, host->valuestring, sizeof(current_ping.target) - 1);
            current_ping.target[sizeof(current_ping.target) - 1] = '\0';
        } else {
            // Mặc định ping Google DNS nếu không có host
            strcpy(current_ping.target, "8.8.8.8");
        }
    } else {
        // Mặc định ping Google DNS nếu không có input_params
        strcpy(current_ping.target, "8.8.8.8");
    }
    
    // Reset kết quả ping trước
    memset(&last_ping_result, 0, sizeof(test_result_info_t));
    ping_executed = false;
    
    // Thực hiện ping thật và lấy kết quả
    if (strcmp(instruction->action, "ping") == 0) {
        log_message(LOG_LVL_DEBUG, "Executing real ping to %s", current_ping.target);
        
        // Gọi hàm ping thực tế từ tc.c
        int result = execute_ping_test(&current_ping, &last_ping_result);
        
        if (result == 0) {
            ping_executed = true;
            log_message(LOG_LVL_DEBUG, "Ping completed with status: %d", last_ping_result.status);
            log_message(LOG_LVL_DEBUG, "Results: Sent=%d, Received=%d, Min=%.2f, Avg=%.2f, Max=%.2f",
                       last_ping_result.data.ping.packets_sent,
                       last_ping_result.data.ping.packets_received,
                       last_ping_result.data.ping.min_rtt,
                       last_ping_result.data.ping.avg_rtt,
                       last_ping_result.data.ping.max_rtt);
        } else {
            log_message(LOG_LVL_ERROR, "Ping execution failed");
            // Đặt giá trị lỗi mặc định
            last_ping_result.status = TEST_RESULT_ERROR;
            last_ping_result.data.ping.packets_sent = 0;
            last_ping_result.data.ping.packets_received = 0;
            last_ping_result.data.ping.min_rtt = 0.0f;
            last_ping_result.data.ping.avg_rtt = 0.0f;
            last_ping_result.data.ping.max_rtt = 0.0f;
            last_ping_result.data.ping.packet_loss = 100.0f;
            ping_executed = true;  // Đánh dấu là đã thử ping
        }
        
        // Log kết quả
        if (ping_executed) {
            // In thông tin về kết quả ping
            char status_str[20];
            switch(last_ping_result.status) {
                case TEST_RESULT_SUCCESS: strcpy(status_str, "SUCCESS"); break;
                case TEST_RESULT_FAILED: strcpy(status_str, "FAILED"); break; 
                case TEST_RESULT_TIMEOUT: strcpy(status_str, "TIMEOUT"); break;
                case TEST_RESULT_ERROR: strcpy(status_str, "ERROR"); break;
                default: strcpy(status_str, "UNKNOWN");
            }
            printf("Ping result: %s\n", status_str);
            printf("Details: %s\n", last_ping_result.result_details);
        }
    }
    
    return 0;
}

// Các stub functions khác không thay đổi
int tcapi_save() {
    printf("Saving configuration\n");
    return 0;
}

int ai_diagnostic_commit() {
    printf("Committing diagnostics\n");
    return 0;
}
