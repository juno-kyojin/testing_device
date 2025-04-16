/**
 * Ping Plugin - Plugin thực hiện test ping
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>
#include <cjson/cJSON.h>
#include "../include/plugin_manager.h"
#include "../include/tc.h"
#include "../include/parser_data.h"
#include "../include/log.h"

// Định nghĩa thông tin plugin
#define PLUGIN_NAME        "Ping Test Plugin"
#define PLUGIN_DESCRIPTION "Plugin thực hiện test ping"
#define PLUGIN_VERSION     "1.0.0"
#define PLUGIN_ACTION      "ping"

// Nếu log_message không thể truy cập, dùng phiên bản cục bộ
static void local_log_message(int level, const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    FILE* log_file = fopen("/home/tobie/testing_device/var/log/plugin.log", "a");
    if (log_file) {
        // Simplify timestamp generation to avoid segfault
        char timestamp[32];
        time_t now;
        
        // Get current time safely
        if (time(&now) != (time_t)-1) {
            struct tm tm_buf;
            struct tm* tm_info = localtime_r(&now, &tm_buf);
            
            if (tm_info) {
                strftime(timestamp, sizeof(timestamp), "[%Y-%m-%d %H:%M:%S]", tm_info);
            } else {
                strcpy(timestamp, "[TIME ERROR]");
            }
        } else {
            strcpy(timestamp, "[TIME ERROR]");
        }
        
        fprintf(log_file, "%s [%d]: ", timestamp, level);
        vfprintf(log_file, format, args);
        fprintf(log_file, "\n");
        fclose(log_file);
    }
    
    va_end(args);
}

// Phân tích kết quả ping trong trường hợp hàm parse_ping_result không khả dụng
static int local_parse_ping_result(const char *output, ping_result_t *result) {
    if (!output || !result) {
        local_log_message(1, "Invalid parameters for parse_ping_result");
        return -1;
    }
    
    memset(result, 0, sizeof(ping_result_t));
    
    // Mẫu để trích xuất số gói tin
    char *packets_line = strstr(output, "packets transmitted");
    if (packets_line) {
        sscanf(packets_line - 10, "%d packets transmitted, %d received", 
               &result->packets_sent, &result->packets_received);
    }
    
    // Mẫu để trích xuất tỷ lệ mất gói
    char *loss_line = strstr(output, "% packet loss");
    if (loss_line) {
        sscanf(loss_line - 10, "%f%%", &result->packet_loss);
    }
    
    // Mẫu để trích xuất RTT
    char *rtt_line = strstr(output, "min/avg/max");
    if (rtt_line) {
        char *values = strstr(rtt_line, "=");
        if (values) {
            sscanf(values + 1, "%f/%f/%f", 
                   &result->min_rtt, &result->avg_rtt, &result->max_rtt);
        }
    }
    
    return (result->packets_sent > 0) ? 0 : -1;
}

/**
 * @brief Thực thi ping test
 */
static int ping_execute_test(test_case_t *test_case, test_result_info_t *result, cJSON *input_params) {
    if (!test_case || !result) {
        local_log_message(1, "Invalid parameters for ping test");
        return -1;
    }
    
    // Đảm bảo test_case có thông tin target
    if (strlen(test_case->target) == 0 && input_params) {
        cJSON *host = cJSON_GetObjectItem(input_params, "host");
        if (host && cJSON_IsString(host) && strlen(host->valuestring) > 0) {
            strncpy(test_case->target, host->valuestring, sizeof(test_case->target) - 1);
            test_case->target[sizeof(test_case->target) - 1] = '\0';
            local_log_message(0, "Using host from input_params: %s", test_case->target);
        }
    }
    
    memset(result, 0, sizeof(test_result_info_t));
    strncpy(result->test_id, test_case->id, sizeof(result->test_id) - 1);
    result->test_id[sizeof(result->test_id) - 1] = '\0';
    result->test_type = TEST_PING;
    result->status = TEST_RESULT_ERROR;
    
    if (strlen(test_case->target) == 0) {
        local_log_message(1, "Empty target for ping test case %s", test_case->id);
        snprintf(result->result_details, sizeof(result->result_details), 
                "Invalid target: empty string");
        return -1;
    }
    
    // Thiết lập tham số ping mặc định nếu chưa có
    if (test_case->params.ping.count <= 0) {
        test_case->params.ping.count = 5;
    }
    
    if (test_case->params.ping.size <= 0) {
        test_case->params.ping.size = 64;
    }
    
    if (test_case->params.ping.interval <= 0) {
        test_case->params.ping.interval = 1000;
    }
    
    char ping_cmd[512];
    const char *ping_cmd_base = test_case->params.ping.ipv6 ? "ping6" : "ping";
    
    snprintf(ping_cmd, sizeof(ping_cmd), 
             "%s -c %d -s %d -i %.1f %s", 
             ping_cmd_base,
             test_case->params.ping.count,
             test_case->params.ping.size,
             test_case->params.ping.interval / 1000.0f,
             test_case->target);
    
    local_log_message(0, "Executing ping command: %s", ping_cmd);
    
    FILE *pipe = popen(ping_cmd, "r");
    if (!pipe) {
        local_log_message(1, "Failed to execute ping command: %s", ping_cmd);
        snprintf(result->result_details, sizeof(result->result_details), 
                 "Failed to execute ping command: %s", strerror(errno));
        return -1;
    }
    
    char buffer[4096];
    memset(buffer, 0, sizeof(buffer));
    
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);
    
    size_t bytes_read = 0;
    char *ptr = buffer;
    size_t remaining = sizeof(buffer) - 1;
    
    while (remaining > 0) {
        size_t count = fread(ptr, 1, remaining, pipe);
        if (count <= 0) {
            if (feof(pipe)) {
                break;
            }
            if (ferror(pipe) && errno != EINTR) {
                local_log_message(1, "Error reading from pipe: %s", strerror(errno));
                break;
            }
        } else {
            ptr += count;
            remaining -= count;
            bytes_read += count;
        }
    }
    
    gettimeofday(&end_time, NULL);
    result->execution_time = (end_time.tv_sec - start_time.tv_sec) * 1000.0f + 
                            (end_time.tv_usec - start_time.tv_usec) / 1000.0f;
    
    buffer[bytes_read] = '\0';
    
    int exit_code = pclose(pipe);
    
    if (bytes_read > 0) {
        // Thử dùng parse_ping_result từ thư viện chính trước
        int parse_result = -1;
        
        // Nếu có thể truy cập hàm parse_ping_result
        if (parse_ping_result != NULL) {
            parse_result = parse_ping_result(buffer, &result->data.ping);
        } else {
            // Nếu không, dùng phiên bản cục bộ
            parse_result = local_parse_ping_result(buffer, &result->data.ping);
        }
        
        if (parse_result == 0) {
            result->status = TEST_RESULT_SUCCESS;
            
            snprintf(result->result_details, sizeof(result->result_details), 
                     "Ping to %s completed. Packets: %d/%d, Loss: %.1f%%, RTT min/avg/max: %.3f/%.3f/%.3f ms", 
                     test_case->target, 
                     result->data.ping.packets_received, 
                     result->data.ping.packets_sent,
                     result->data.ping.packet_loss, 
                     result->data.ping.min_rtt, 
                     result->data.ping.avg_rtt, 
                     result->data.ping.max_rtt);
        } else {
            result->status = TEST_RESULT_FAILED;
            snprintf(result->result_details, sizeof(result->result_details), 
                     "Ping to %s failed. All packets lost.", test_case->target);
        }
    } else {
        result->status = TEST_RESULT_ERROR;
        snprintf(result->result_details, sizeof(result->result_details), 
                 "No output from ping command");
    }
    
    return 0;
}

// Hàm khởi tạo plugin (tùy chọn)
static int ping_initialize(void) {
    local_log_message(0, "Ping plugin initialized");
    return 0;
}

// Hàm giải phóng tài nguyên (tùy chọn)
static void ping_cleanup(void) {
    local_log_message(0, "Ping plugin cleaned up");
}

// Hàm đăng ký plugin (bắt buộc)
plugin_info_t register_plugin(void) {
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
    
    return plugin;
}
