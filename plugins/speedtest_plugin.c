/**
 * Speedtest Plugin - Plugin thực hiện test tốc độ mạng
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <stdarg.h>
#include <time.h>
#include <cjson/cJSON.h>
#include "../include/plugin_manager.h"
#include "../include/tc.h"
#include "../include/parser_data.h"
// Không bao gồm log.h trực tiếp để tránh phụ thuộc vào hàm log_message

// Định nghĩa thông tin plugin
#define PLUGIN_NAME        "Speedtest Plugin"  // Phải trùng với tên trong config
#define PLUGIN_DESCRIPTION "Plugin thực hiện test tốc độ mạng internet"
#define PLUGIN_VERSION     "1.0.0"
#define PLUGIN_ACTION      "speedtest"

// Định nghĩa các mức log giống như trong log.h
#define LOCAL_LOG_LVL_ERROR 1
#define LOCAL_LOG_LVL_WARN  2
#define LOCAL_LOG_LVL_DEBUG 3

// Triển khai hàm log cục bộ để tránh phụ thuộc vào log_message
static void local_log_message(int level, const char *format, ...) {
    va_list args;
    va_start(args, format);
    
    FILE *log_file = fopen("/home/tobie/testing_device/var/log/speedtest_plugin.log", "a");
    if (log_file) {
        char timestamp[32];
        time_t now;
        
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
        
        // Chuyển level thành string tương ứng
        const char *level_str = "UNKNOWN";
        switch (level) {
            case LOCAL_LOG_LVL_ERROR: level_str = "ERROR"; break;
            case LOCAL_LOG_LVL_WARN:  level_str = "WARN"; break;
            case LOCAL_LOG_LVL_DEBUG: level_str = "DEBUG"; break;
        }
        
        fprintf(log_file, "%s %s: ", timestamp, level_str);
        vfprintf(log_file, format, args);
        fprintf(log_file, "\n");
        fclose(log_file);
    }
    
    va_end(args);
}

/**
 * @brief Thực thi speedtest
 */
static int speedtest_execute_test(test_case_t *test_case, test_result_info_t *result, cJSON *input_params) {
    if (!test_case || !result) {
        local_log_message(LOCAL_LOG_LVL_ERROR, "Invalid parameters for speedtest");
        return -1;
    }
    
    // Đảm bảo test_case có thông tin server
    if (strlen(test_case->target) == 0 && input_params) {
        cJSON *server = cJSON_GetObjectItem(input_params, "server");
        if (server && cJSON_IsString(server)) {
            strncpy(test_case->target, server->valuestring, sizeof(test_case->target) - 1);
            test_case->target[sizeof(test_case->target) - 1] = '\0';
            local_log_message(LOCAL_LOG_LVL_DEBUG, "Using server from input_params: %s", test_case->target);
        } else {
            // Mặc định sử dụng speedtest.net
            strcpy(test_case->target, "speedtest.net");
        }
    }
    
    memset(result, 0, sizeof(test_result_info_t));
    strncpy(result->test_id, test_case->id, sizeof(result->test_id) - 1);
    result->test_id[sizeof(result->test_id) - 1] = '\0';
    result->test_type = TEST_SPEEDTEST;
    result->status = TEST_RESULT_ERROR;
    
    if (system("which speedtest-cli > /dev/null 2>&1") != 0) {
        local_log_message(LOCAL_LOG_LVL_ERROR, "speedtest-cli not installed. Please install with: sudo apt-get install speedtest-cli");
        snprintf(result->result_details, sizeof(result->result_details), 
                "speedtest-cli not installed. Please install with: sudo apt-get install speedtest-cli");
        result->status = TEST_RESULT_ERROR;
        return -1;
    }
    
    local_log_message(LOCAL_LOG_LVL_DEBUG, "Starting speedtest with server target: %s", 
                test_case->target[0] ? test_case->target : "default");
    
    char command[512];
    
    if (strcmp(test_case->target, "speedtest.net") == 0) {
        snprintf(command, sizeof(command), "speedtest-cli --json --secure");
        local_log_message(LOCAL_LOG_LVL_DEBUG, "Using default server (nearest)");
    } else if (atoi(test_case->target) > 0) {
        snprintf(command, sizeof(command), "speedtest-cli --json --server %s --secure", 
                 test_case->target);
        local_log_message(LOCAL_LOG_LVL_DEBUG, "Using server ID: %s", test_case->target);
    } else if (test_case->target[0]) {
        local_log_message(LOCAL_LOG_LVL_WARN, "Server '%s' might not be a valid server ID, attempting to use anyway", test_case->target);
        snprintf(command, sizeof(command), "speedtest-cli --json --server %s --secure", 
                 test_case->target);
    } else {
        snprintf(command, sizeof(command), "speedtest-cli --json --secure");
        local_log_message(LOCAL_LOG_LVL_DEBUG, "Using default server (nearest)");
    }
    
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);

    local_log_message(LOCAL_LOG_LVL_DEBUG, "Executing command: %s", command);
    
    FILE *pipe = popen(command, "r");
    if (!pipe) {
        local_log_message(LOCAL_LOG_LVL_ERROR, "Failed to open pipe for speedtest-cli");
        snprintf(result->result_details, sizeof(result->result_details), 
                "Failed to execute speedtest-cli command: %s", strerror(errno));
        result->status = TEST_RESULT_ERROR;
        return -1;
    }
    
    char json_buffer[8192] = {0};
    size_t bytes_read = fread(json_buffer, 1, sizeof(json_buffer) - 1, pipe);
    
    int exit_status = pclose(pipe);
    
    gettimeofday(&end_time, NULL);
    result->execution_time = (end_time.tv_sec - start_time.tv_sec) * 1000.0f + 
                            (end_time.tv_usec - start_time.tv_usec) / 1000.0f;
    
    local_log_message(LOCAL_LOG_LVL_DEBUG, "Speedtest execution time: %.1f ms", result->execution_time);
    
    if (exit_status != 0) {
        local_log_message(LOCAL_LOG_LVL_ERROR, "speedtest-cli execution failed with code %d", exit_status);
        
        if (bytes_read > 0) {
            char short_error[900] = {0};
            strncpy(short_error, json_buffer, sizeof(short_error) - 1);
            short_error[sizeof(short_error) - 1] = '\0';
            
            snprintf(result->result_details, sizeof(result->result_details), 
                    "Speedtest execution failed: %s", short_error);
        } else {
            snprintf(result->result_details, sizeof(result->result_details), 
                    "Speedtest execution failed with code %d", exit_status);
        }
        
        result->status = TEST_RESULT_ERROR;
        return -1;
    }
    
    if (bytes_read == 0) {
        local_log_message(LOCAL_LOG_LVL_ERROR, "Empty result from speedtest");
        
        // Thử với lệnh đơn giản
        FILE *simple_pipe = popen("speedtest-cli --simple", "r");
        if (simple_pipe) {
            char simple_result[1024] = {0};
            size_t simple_bytes = fread(simple_result, 1, sizeof(simple_result) - 1, simple_pipe);
            pclose(simple_pipe);
            
            if (simple_bytes > 0) {
                local_log_message(LOCAL_LOG_LVL_DEBUG, "Simple speedtest result: %s", simple_result);
                
                char short_result[900] = {0};
                strncpy(short_result, simple_result, sizeof(short_result) - 1);
                short_result[sizeof(short_result) - 1] = '\0';
                
                snprintf(result->result_details, sizeof(result->result_details), 
                        "JSON result empty. Simple test: %s", short_result);
            } else {
                snprintf(result->result_details, sizeof(result->result_details), 
                        "Empty result from speedtest");
            }
        } else {
            snprintf(result->result_details, sizeof(result->result_details), 
                    "Empty result from speedtest");
        }
        
        result->status = TEST_RESULT_ERROR;
        return -1;
    }
    
    local_log_message(LOCAL_LOG_LVL_DEBUG, "Successfully read %lu bytes of JSON result", (unsigned long)bytes_read);
    cJSON *json = cJSON_Parse(json_buffer);
    if (!json) {
        local_log_message(LOCAL_LOG_LVL_ERROR, "Failed to parse speedtest JSON result");
        local_log_message(LOCAL_LOG_LVL_DEBUG, "Raw JSON content (first 200 chars): %.200s", json_buffer);
        snprintf(result->result_details, sizeof(result->result_details), 
                "Failed to parse speedtest JSON result");
        result->status = TEST_RESULT_ERROR;
        return -1;
    }
    
    // Parse results
    cJSON *download = cJSON_GetObjectItem(json, "download");
    cJSON *upload = cJSON_GetObjectItem(json, "upload");
    cJSON *ping = cJSON_GetObjectItem(json, "ping");
    
    if (download && cJSON_IsNumber(download)) {
        result->data.speedtest.download_speed = download->valuedouble / 1000000.0;
    } else {
        local_log_message(LOCAL_LOG_LVL_WARN, "Download speed not found in JSON result");
    }
    
    if (upload && cJSON_IsNumber(upload)) {
        result->data.speedtest.upload_speed = upload->valuedouble / 1000000.0;
    } else {
        local_log_message(LOCAL_LOG_LVL_WARN, "Upload speed not found in JSON result");
    }
    
    if (ping && cJSON_IsNumber(ping)) {
        result->data.speedtest.latency = ping->valuedouble;
    } else {
        local_log_message(LOCAL_LOG_LVL_WARN, "Ping/latency not found in JSON result");
    }
    
    cJSON *server = cJSON_GetObjectItem(json, "server");
    char server_info[256] = "Unknown server";
    if (server && cJSON_IsObject(server)) {
        cJSON *host = cJSON_GetObjectItem(server, "host");
        cJSON *name = cJSON_GetObjectItem(server, "name");
        cJSON *country = cJSON_GetObjectItem(server, "country");
        cJSON *serverId = cJSON_GetObjectItem(server, "id");
        
        if (host && cJSON_IsString(host) && 
            name && cJSON_IsString(name) && 
            country && cJSON_IsString(country)) {
            
            if (serverId && cJSON_IsString(serverId)) {
                snprintf(server_info, sizeof(server_info), "%s (%s, %s) [ID: %s]", 
                        name->valuestring, host->valuestring, country->valuestring, serverId->valuestring);
                
                local_log_message(LOCAL_LOG_LVL_DEBUG, "Server ID for future reference: %s", serverId->valuestring);
            } else {
                snprintf(server_info, sizeof(server_info), "%s (%s, %s)", 
                        name->valuestring, host->valuestring, country->valuestring);
            }
        }
    }
    
    snprintf(result->result_details, sizeof(result->result_details), 
            "Speedtest completed with server %s. Download: %.2f Mbps, Upload: %.2f Mbps, Latency: %.2f ms", 
            server_info,
            result->data.speedtest.download_speed,
            result->data.speedtest.upload_speed,
            result->data.speedtest.latency);
    
    cJSON_Delete(json);
    
    if (result->data.speedtest.download_speed > 0 || 
        result->data.speedtest.upload_speed > 0) {
        result->status = TEST_RESULT_SUCCESS;
        local_log_message(LOCAL_LOG_LVL_DEBUG, "Plugin: Speedtest completed successfully: Download=%.2f Mbps, Upload=%.2f Mbps, Latency=%.2f ms", 
                  result->data.speedtest.download_speed,
                  result->data.speedtest.upload_speed,
                  result->data.speedtest.latency);
    } else {
        result->status = TEST_RESULT_FAILED;
        local_log_message(LOCAL_LOG_LVL_ERROR, "Plugin: Speedtest failed to get valid results");
    }
    
    return 0;
}

// Hàm khởi tạo plugin
static int speedtest_initialize(void) {
    // Tạo log file mới
    FILE *log_file = fopen("/home/tobie/testing_device/var/log/speedtest_plugin.log", "w");
    if (log_file) {
        fprintf(log_file, "[INIT] Speedtest plugin initialized\n");
        fclose(log_file);
    }
    return 0;
}

// Hàm giải phóng tài nguyên
static void speedtest_cleanup(void) {
    FILE *log_file = fopen("/home/tobie/testing_device/var/log/speedtest_plugin.log", "a");
    if (log_file) {
        fprintf(log_file, "[CLEANUP] Speedtest plugin cleaned up\n");
        fclose(log_file);
    }
}

// Hàm đăng ký plugin
plugin_info_t register_plugin(void) {
    plugin_info_t plugin;
    memset(&plugin, 0, sizeof(plugin_info_t));
    
    // Thiết lập thông tin plugin
    strncpy(plugin.name, PLUGIN_NAME, sizeof(plugin.name) - 1);
    strncpy(plugin.description, PLUGIN_DESCRIPTION, sizeof(plugin.description) - 1);
    strncpy(plugin.version, PLUGIN_VERSION, sizeof(plugin.version) - 1);
    strncpy(plugin.action_name, PLUGIN_ACTION, sizeof(plugin.action_name) - 1);
    strncpy(plugin.original_action, PLUGIN_ACTION, sizeof(plugin.original_action) - 1);
    plugin.test_type = TEST_SPEEDTEST;
    
    // Thiết lập các hàm callback
    plugin.execute_test = speedtest_execute_test;
    plugin.initialize = speedtest_initialize;
    plugin.cleanup = speedtest_cleanup;
    
    // Ghi log
    FILE *log_file = fopen("/home/tobie/testing_device/var/log/speedtest_plugin.log", "a");
    if (log_file) {
        fprintf(log_file, "[REGISTER] Speedtest plugin registered\n");
        fclose(log_file);
    }
    
    return plugin;
}
