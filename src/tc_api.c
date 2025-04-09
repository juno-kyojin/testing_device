#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>  
#include "parser_data.h"
#include "cjson/cJSON.h"
#include "tc.h"
#include "log.h"

// Định nghĩa số lượng tối đa test cases có thể xử lý cùng lúc
#define MAX_TEST_RESULTS 5

// Biến toàn cục để lưu kết quả nhiều loại test
typedef struct {
    char action_name[32];             // Tên action (ping, throughput, security...)
    test_result_info_t result;        // Kết quả test
    bool executed;                    // Đã thực thi hay chưa
} test_action_result_t;

static test_action_result_t test_results[MAX_TEST_RESULTS];
static int num_test_results = 0;

// Thông tin test case hiện tại
static test_case_t current_test_case;

// Hàm tìm kiếm kết quả test theo tên action
static test_action_result_t* find_test_result(const char *action) {
    for (int i = 0; i < num_test_results; i++) {
        if (strcmp(test_results[i].action_name, action) == 0) {
            return &test_results[i];
        }
    }
    // Nếu không tìm thấy và còn chỗ, tạo mới
    if (num_test_results < MAX_TEST_RESULTS) {
        int idx = num_test_results++;
        strncpy(test_results[idx].action_name, action, sizeof(test_results[idx].action_name) - 1);
        test_results[idx].action_name[sizeof(test_results[idx].action_name) - 1] = '\0';
        test_results[idx].executed = false;
        memset(&test_results[idx].result, 0, sizeof(test_result_info_t));
        return &test_results[idx];
    }
    return NULL;
}

// Stub function cho tcapi_get - sử dụng kết quả thực tế
int tcapi_get(const char *node, const char *entry, const char *attribute, char *value) {
    test_action_result_t *test_result = NULL;
    
    if (strcmp(node, "Ping") == 0) {
        test_result = find_test_result("ping");
    } else if (strcmp(node, "Throughput") == 0) {
        test_result = find_test_result("throughput");
    } else if (strcmp(node, "Security") == 0) {
        test_result = find_test_result("security");
    } else if (strcmp(node, "Speedtest") == 0) {
        test_result = find_test_result("speedtest");
    } else {
        strcpy(value, "N/A");
        return -1;
    }
    
    if (!test_result || !test_result->executed) {
        if (strcmp(attribute, "Status") == 0) {
            strcpy(value, "-1");
        } else {
            strcpy(value, "N/A");
        }
        return 0;
    }
    
    if (strcmp(attribute, "Status") == 0) {
        switch (test_result->result.status) {
            case TEST_RESULT_SUCCESS: strcpy(value, "0"); break;
            case TEST_RESULT_FAILED:  strcpy(value, "1"); break; 
            case TEST_RESULT_TIMEOUT: strcpy(value, "2"); break;
            case TEST_RESULT_ERROR:
            default:                  strcpy(value, "3");
        }
        return 0;
    }
    
    if (strcmp(node, "Ping") == 0) {
        if (strcmp(attribute, "host") == 0) {
            strcpy(value, current_test_case.target);
        } else if (strcmp(attribute, "hostAddress") == 0) {
            strcpy(value, current_test_case.target);
        } else if (strcmp(attribute, "successCount") == 0) {
            sprintf(value, "%d", test_result->result.data.ping.packets_received);
        } else if (strcmp(attribute, "failureCount") == 0) {
            int failures = test_result->result.data.ping.packets_sent - 
                           test_result->result.data.ping.packets_received;
            sprintf(value, "%d", failures > 0 ? failures : 0);
        } else if (strcmp(attribute, "averageResponseTime") == 0) {
            sprintf(value, "%.1f", test_result->result.data.ping.avg_rtt);
        } else if (strcmp(attribute, "minimumResponseTime") == 0) {
            sprintf(value, "%.1f", test_result->result.data.ping.min_rtt);
        } else if (strcmp(attribute, "maximumResponseTime") == 0) {
            sprintf(value, "%.1f", test_result->result.data.ping.max_rtt);
        } else if (strcmp(attribute, "packetLoss") == 0) {
            sprintf(value, "%.1f", test_result->result.data.ping.packet_loss);
        } else if (strcmp(attribute, "Details") == 0) {
            strcpy(value, test_result->result.result_details);
        } else {
            return -1;
        }
    } else if (strcmp(node, "Throughput") == 0) {
        if (strcmp(attribute, "bandwidth") == 0) {
            sprintf(value, "%.2f", test_result->result.data.throughput.bandwidth);
        } else if (strcmp(attribute, "jitter") == 0) {
            sprintf(value, "%d", test_result->result.data.throughput.jitter);
        } else if (strcmp(attribute, "packetLoss") == 0) {
            sprintf(value, "%d", test_result->result.data.throughput.packet_loss);
        } else {
            return -1;
        }
    } else if (strcmp(node, "Security") == 0) {
        if (strcmp(attribute, "passed") == 0) {
            strcpy(value, test_result->result.data.security.passed ? "1" : "0");
        } else if (strcmp(attribute, "vulnerabilities") == 0) {
            sprintf(value, "%d", test_result->result.data.security.vulnerabilities);
        } else {
            return -1;
        }
    } else if (strcmp(node, "Speedtest") == 0) {
        if (strcmp(attribute, "DownloadSpeed") == 0) {
            sprintf(value, "%.2f", test_result->result.data.speedtest.download_speed);
        } else if (strcmp(attribute, "UploadSpeed") == 0) {
            sprintf(value, "%.2f", test_result->result.data.speedtest.upload_speed);
        } else if (strcmp(attribute, "Latency") == 0) {
            sprintf(value, "%.2f", test_result->result.data.speedtest.latency);
        } else if (strcmp(attribute, "ServerDetails") == 0) {
            char server_info[256] = "";
            if (sscanf(test_result->result.result_details, 
                    "Speedtest completed with server %255[^.]", server_info) == 1) {
                strcpy(value, server_info);
            } else {
                strcpy(value, "Unknown server");
            }
        } else if (strcmp(attribute, "ExecutionTime") == 0) {
            sprintf(value, "%.1f", test_result->result.execution_time);
        } else if (strcmp(attribute, "Details") == 0) {
            strcpy(value, test_result->result.result_details);
        } else {
            return -1;
        }
    }
    
    return 0;
}

// Stub function cho tcapi_set - được giữ lại để ghi log
int tcapi_set(const char *node, const char *entry, const char *attribute, const char *value) {
    log_message(LOG_LVL_DEBUG, "Setting %s.%s.%s = %s", node, entry, attribute, value);
    
    if (strcmp(node, "Ping") == 0) {
        if (strcmp(attribute, "host") == 0) {
            strncpy(current_test_case.target, value, sizeof(current_test_case.target) - 1);
            current_test_case.target[sizeof(current_test_case.target) - 1] = '\0';
        }
    } else if (strcmp(node, "Throughput") == 0) {
        if (strcmp(attribute, "server") == 0) {
            strncpy(current_test_case.target, value, sizeof(current_test_case.target) - 1);
            current_test_case.target[sizeof(current_test_case.target) - 1] = '\0';
        }
    } else if (strcmp(node, "Security") == 0) {
        if (strcmp(attribute, "target") == 0) {
            strncpy(current_test_case.target, value, sizeof(current_test_case.target) - 1);
            current_test_case.target[sizeof(current_test_case.target) - 1] = '\0';
        }
    } else if (strcmp(node, "Speedtest") == 0) {
        if (strcmp(attribute, "server") == 0) {
            strncpy(current_test_case.target, value, sizeof(current_test_case.target) - 1);
            current_test_case.target[sizeof(current_test_case.target) - 1] = '\0';
        }
    }
    
    return 0;
}

// Khởi tạo test case dựa trên đối tượng cJSON input_params
static void init_test_case(test_case_t *test_case, const char *action, cJSON *input_params) {
    memset(test_case, 0, sizeof(test_case_t));
    
    snprintf(test_case->id, sizeof(test_case->id), "%s_test", action);
    test_case->timeout = 5000;
    test_case->enabled = true;
    
    if (strcmp(action, "ping") == 0) {
        test_case->type = TEST_PING;
        strcpy(test_case->name, "Ping Test");
        
        test_case->params.ping.count = 5;
        test_case->params.ping.size = 64;
        test_case->params.ping.interval = 1000;
        test_case->params.ping.ipv6 = false;
        
        if (input_params) {
            cJSON *host = cJSON_GetObjectItem(input_params, "host");
            if (host && cJSON_IsString(host) && strlen(host->valuestring) > 0) {
                strncpy(test_case->target, host->valuestring, sizeof(test_case->target) - 1);
                test_case->target[sizeof(test_case->target) - 1] = '\0';
            }
        }
    } else if (strcmp(action, "throughput") == 0) {
        test_case->type = TEST_THROUGHPUT;
        strcpy(test_case->name, "Throughput Test");
        
        test_case->params.throughput.duration = 10;
        strcpy(test_case->params.throughput.protocol, "TCP");
        test_case->params.throughput.port = 5201;
        test_case->params.throughput.buffer_size = 8192;
        
        if (input_params) {
            cJSON *server = cJSON_GetObjectItem(input_params, "server");
            if (server && cJSON_IsString(server) && strlen(server->valuestring) > 0) {
                strncpy(test_case->target, server->valuestring, sizeof(test_case->target) - 1);
                test_case->target[sizeof(test_case->target) - 1] = '\0';
            }
        }
    } else if (strcmp(action, "security") == 0) {
        test_case->type = TEST_SECURITY;
        strcpy(test_case->name, "Security Test");
        
        strcpy(test_case->params.security.method, "tls_scan");
        test_case->params.security.port = 443;
        test_case->params.security.tls = true;
        
        if (input_params) {
            cJSON *target = cJSON_GetObjectItem(input_params, "target");
            if (target && cJSON_IsString(target) && strlen(target->valuestring) > 0) {
                strncpy(test_case->target, target->valuestring, sizeof(test_case->target) - 1);
                test_case->target[sizeof(test_case->target) - 1] = '\0';
            }
        }
    } else if (strcmp(action, "speedtest") == 0) {
        test_case->type = TEST_SPEEDTEST;
        strcpy(test_case->name, "Speedtest Test");
        
        test_case->params.speedtest.timeout = 30;
        test_case->params.speedtest.use_https = true;
        
        if (input_params) {
            cJSON *server = cJSON_GetObjectItem(input_params, "server");
            if (server && cJSON_IsString(server) && strlen(server->valuestring) > 0) {
                strncpy(test_case->target, server->valuestring, sizeof(test_case->target) - 1);
                test_case->target[sizeof(test_case->target) - 1] = '\0';
            } else {
                strcpy(test_case->target, "speedtest.net");
            }
        } else {
            strcpy(test_case->target, "speedtest.net");
        }
    }
}

/**
 * @brief Thực thi ping test
 * 
 * @param test_case Con trỏ đến test case
 * @param result Con trỏ đến biến lưu kết quả
 * @return int 0 nếu thành công, -1 nếu thất bại
 */
int execute_ping_test(test_case_t *test_case, test_result_info_t *result) {
    if (!test_case || !result || test_case->type != TEST_PING) {
        log_message(LOG_LVL_ERROR, "Invalid parameters for ping test");
        return -1;
    }
    
    memset(result, 0, sizeof(test_result_info_t));
    strncpy(result->test_id, test_case->id, sizeof(result->test_id) - 1);
    result->test_id[sizeof(result->test_id) - 1] = '\0';
    result->test_type = TEST_PING;
    result->status = TEST_RESULT_ERROR;
    
    if (strlen(test_case->target) == 0) {
        log_message(LOG_LVL_ERROR, "Empty target for ping test case %s", test_case->id);
        snprintf(result->result_details, sizeof(result->result_details), 
                "Invalid target: empty string");
        return -1;
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
    
    log_message(LOG_LVL_DEBUG, "Executing ping command: %s", ping_cmd);
    
    FILE *pipe = popen(ping_cmd, "r");
    if (!pipe) {
        log_message(LOG_LVL_ERROR, "Failed to execute ping command: %s", ping_cmd);
        snprintf(result->result_details, sizeof(result->result_details), 
                 "Failed to execute ping command: %s", strerror(errno));
        return -1;
    }
    
    char buffer[4096];
    memset(buffer, 0, sizeof(buffer));
    
    start_timer();
    if (set_timeout(test_case->timeout) != 0) {
        log_message(LOG_LVL_WARN, "Failed to set timeout for ping test");
    }
    
    size_t bytes_read = 0;
    char *ptr = buffer;
    size_t remaining = sizeof(buffer) - 1;
    
    while (!is_timeout_occurred() && remaining > 0) {
        size_t count = fread(ptr, 1, remaining, pipe);
        if (count <= 0) {
            if (feof(pipe)) {
                break;
            }
            if (ferror(pipe) && errno != EINTR) {
                log_message(LOG_LVL_ERROR, "Error reading from pipe: %s", strerror(errno));
                break;
            }
        } else {
            ptr += count;
            remaining -= count;
            bytes_read += count;
        }
    }
    
    clear_timeout();
    result->execution_time = stop_timer();
    
    buffer[bytes_read] = '\0';
    
    int exit_code = pclose(pipe);
    
    if (is_timeout_occurred()) {
        log_message(LOG_LVL_WARN, "Ping test timed out after %.1f ms", result->execution_time);
        result->status = TEST_RESULT_TIMEOUT;
        snprintf(result->result_details, sizeof(result->result_details), 
                 "Ping test to %s timed out after %.1f ms", 
                 test_case->target, result->execution_time);
        return 0;
    }
    
    if (WIFEXITED(exit_code)) {
        int status = WEXITSTATUS(exit_code);
        log_message(LOG_LVL_DEBUG, "Ping command exited with status %d", status);
        
        if (bytes_read > 0) {
            if (parse_ping_result(buffer, &result->data.ping) == 0) {
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
    } else {
        result->status = TEST_RESULT_ERROR;
        snprintf(result->result_details, sizeof(result->result_details), 
                 "Ping command did not exit properly");
    }
    
    return 0;
}

/**
 * @brief Thực thi speedtest test thực tế
 * 
 * @param test_case Con trỏ đến test case
 * @param result Con trỏ đến biến lưu kết quả
 * @return int 0 nếu thành công, -1 nếu thất bại
 */
int execute_speedtest_test(test_case_t *test_case, test_result_info_t *result) {
    if (!test_case || !result) {
        log_message(LOG_LVL_ERROR, "Invalid parameters for execute_speedtest_test");
        return -1;
    }
    
    memset(result, 0, sizeof(test_result_info_t));
    strncpy(result->test_id, test_case->id, sizeof(result->test_id) - 1);
    result->test_type = TEST_SPEEDTEST;
    result->status = TEST_RESULT_ERROR;
    
    if (system("which speedtest-cli > /dev/null 2>&1") != 0) {
        log_message(LOG_LVL_ERROR, "speedtest-cli not installed. Please install with: sudo apt-get install speedtest-cli");
        snprintf(result->result_details, sizeof(result->result_details), 
                "speedtest-cli not installed. Please install with: sudo apt-get install speedtest-cli");
        result->status = TEST_RESULT_ERROR;
        return -1;
    }
    
    log_message(LOG_LVL_DEBUG, "Starting speedtest with server target: %s", 
                test_case->target[0] ? test_case->target : "default");
    
    char command[512];
    
    if (strcmp(test_case->target, "speedtest.net") == 0) {
        snprintf(command, sizeof(command), "speedtest-cli --json --secure");
        log_message(LOG_LVL_DEBUG, "Using default server (nearest)");
    } else if (atoi(test_case->target) > 0) {
        snprintf(command, sizeof(command), "speedtest-cli --json --server %s --secure", 
                 test_case->target);
        log_message(LOG_LVL_DEBUG, "Using server ID: %s", test_case->target);
    } else if (test_case->target[0]) {
        log_message(LOG_LVL_WARN, "Server '%s' might not be a valid server ID, attempting to use anyway", test_case->target);
        snprintf(command, sizeof(command), "speedtest-cli --json --server %s --secure", 
                 test_case->target);
    } else {
        snprintf(command, sizeof(command), "speedtest-cli --json --secure");
        log_message(LOG_LVL_DEBUG, "Using default server (nearest)");
    }
    
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);

    log_message(LOG_LVL_DEBUG, "Executing command: %s", command);
    
    FILE *pipe = popen(command, "r");
    if (!pipe) {
        log_message(LOG_LVL_ERROR, "Failed to open pipe for speedtest-cli");
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
    
    log_message(LOG_LVL_DEBUG, "Speedtest execution time: %.1f ms", result->execution_time);
    
    if (exit_status != 0) {
        log_message(LOG_LVL_ERROR, "speedtest-cli execution failed with code %d", exit_status);
        
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
        log_message(LOG_LVL_ERROR, "Empty result from speedtest");
        
        log_message(LOG_LVL_DEBUG, "Attempting basic speedtest-cli command for diagnostic");
        
        FILE *simple_pipe = popen("speedtest-cli --simple", "r");
        if (simple_pipe) {
            char simple_result[1024] = {0};
            size_t simple_bytes = fread(simple_result, 1, sizeof(simple_result) - 1, simple_pipe);
            pclose(simple_pipe);
            
            if (simple_bytes > 0) {
                log_message(LOG_LVL_DEBUG, "Simple speedtest result: %s", simple_result);
                
                char short_result[900] = {0};
                strncpy(short_result, simple_result, sizeof(short_result) - 1);
                short_result[sizeof(short_result) - 1] = '\0';
                
                snprintf(result->result_details, sizeof(result->result_details), 
                        "JSON result empty. Try running 'speedtest-cli' manually. Simple test: %s", short_result);
            } else {
                snprintf(result->result_details, sizeof(result->result_details), 
                        "Empty result from speedtest. Try running 'speedtest-cli' manually");
            }
        } else {
            snprintf(result->result_details, sizeof(result->result_details), 
                    "Empty result from speedtest. Try running 'speedtest-cli' manually");
        }
        
        result->status = TEST_RESULT_ERROR;
        return -1;
    }
    
    log_message(LOG_LVL_DEBUG, "Successfully read %lu bytes of JSON result", (unsigned long)bytes_read);       
    cJSON *json = cJSON_Parse(json_buffer);
    if (!json) {
        log_message(LOG_LVL_ERROR, "Failed to parse speedtest JSON result: %s", cJSON_GetErrorPtr());
        log_message(LOG_LVL_DEBUG, "Raw JSON content (first 200 chars): %.200s", json_buffer);
        snprintf(result->result_details, sizeof(result->result_details), 
                "Failed to parse speedtest JSON result");
        result->status = TEST_RESULT_ERROR;
        return -1;
    }
    
    cJSON *download = cJSON_GetObjectItem(json, "download");
    cJSON *upload = cJSON_GetObjectItem(json, "upload");
    cJSON *ping = cJSON_GetObjectItem(json, "ping");
    
    if (download && cJSON_IsNumber(download)) {
        result->data.speedtest.download_speed = download->valuedouble / 1000000.0;
    } else {
        log_message(LOG_LVL_WARN, "Download speed not found in JSON result");
    }
    
    if (upload && cJSON_IsNumber(upload)) {
        result->data.speedtest.upload_speed = upload->valuedouble / 1000000.0;
    } else {
        log_message(LOG_LVL_WARN, "Upload speed not found in JSON result");
    }
    
    if (ping && cJSON_IsNumber(ping)) {
        result->data.speedtest.latency = ping->valuedouble;
    } else {
        log_message(LOG_LVL_WARN, "Ping/latency not found in JSON result");
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
                
                log_message(LOG_LVL_DEBUG, "Server ID for future reference: %s", serverId->valuestring);
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
        log_message(LOG_LVL_DEBUG, "Speedtest completed successfully: Download=%.2f Mbps, Upload=%.2f Mbps, Latency=%.2f ms", 
                  result->data.speedtest.download_speed,
                  result->data.speedtest.upload_speed,
                  result->data.speedtest.latency);
    } else {
        result->status = TEST_RESULT_FAILED;
        log_message(LOG_LVL_ERROR, "Speedtest failed to get valid results");
    }
    
    return 0;
}

/**
 * @brief Thực thi test case
 * 
 * @param test_case Con trỏ đến test case
 * @param result Con trỏ đến biến lưu kết quả
 * @return int 0 nếu thành công, -1 nếu thất bại
 */
int execute_test_case(test_case_t *test_case, test_result_info_t *result) {
    if (!test_case || !result) {
        log_message(LOG_LVL_ERROR, "Invalid parameters for execute_test_case");
        return -1;
    }
    
    if (!test_case->enabled) {
        log_message(LOG_LVL_WARN, "Test case %s is disabled, skipping execution", test_case->id);
        
        memset(result, 0, sizeof(test_result_info_t));
        strncpy(result->test_id, test_case->id, sizeof(result->test_id) - 1);
        result->test_id[sizeof(result->test_id) - 1] = '\0';
        result->test_type = test_case->type;
        result->status = TEST_RESULT_ERROR;
        snprintf(result->result_details, sizeof(result->result_details), 
                "Test case is disabled");
        
        return 0;
    }
    
    log_message(LOG_LVL_DEBUG, "Executing test case %s (%s)", test_case->id, test_case->name);
    
    int ret = -1;
    if (test_case->type == TEST_PING) {
        ret = execute_ping_test(test_case, result);
    } else if (test_case->type == TEST_SPEEDTEST) {
        ret = execute_speedtest_test(test_case, result);
    } else if (test_case->type == TEST_THROUGHPUT) {
        log_message(LOG_LVL_WARN, "Throughput test is not implemented yet. Skipping test case %s", test_case->id);
        
        memset(result, 0, sizeof(test_result_info_t));
        strncpy(result->test_id, test_case->id, sizeof(result->test_id) - 1);
        result->test_id[sizeof(result->test_id) - 1] = '\0';
        result->test_type = test_case->type;
        result->status = TEST_RESULT_ERROR;
        snprintf(result->result_details, sizeof(result->result_details), 
                "Throughput test is not implemented yet");
        
        return 0;
    } else if (test_case->type == TEST_SECURITY) {
        log_message(LOG_LVL_WARN, "Security test is not implemented yet. Skipping test case %s", test_case->id);
        
        memset(result, 0, sizeof(test_result_info_t));
        strncpy(result->test_id, test_case->id, sizeof(result->test_id) - 1);
        result->test_id[sizeof(result->test_id) - 1] = '\0';
        result->test_type = test_case->type;
        result->status = TEST_RESULT_ERROR;
        snprintf(result->result_details, sizeof(result->result_details), 
                "Security test is not implemented yet");
        
        return 0;
    } else {
        log_message(LOG_LVL_WARN, "Test type %d is not supported. Skipping test case %s", 
                   test_case->type, test_case->id);
        
        memset(result, 0, sizeof(test_result_info_t));
        strncpy(result->test_id, test_case->id, sizeof(result->test_id) - 1);
        result->test_id[sizeof(result->test_id) - 1] = '\0';
        result->test_type = test_case->type;
        result->status = TEST_RESULT_ERROR;
        snprintf(result->result_details, sizeof(result->result_details), 
                "Test type is not supported");
        
        return 0;
    }
    
    if (ret == 0) {
        log_message(LOG_LVL_DEBUG, "Test case %s completed with status: %d", 
                   test_case->id, result->status);
    } else {
        log_message(LOG_LVL_ERROR, "Failed to execute test case %s", test_case->id);
    }
    
    return ret;
}

/**
 * @brief Thực thi test case dựa trên loại mạng
 * 
 * @param test_case Con trỏ đến test case
 * @param network_type Loại mạng để thực thi (LAN hoặc WAN)
 * @param result Con trỏ đến biến lưu kết quả
 * @return int 0 nếu thành công, -1 nếu thất bại
 */
int execute_test_case_by_network(test_case_t *test_case, network_type_t network_type, test_result_info_t *result) {
    if (!test_case || !result) {
        log_message(LOG_LVL_ERROR, "Invalid parameters for execute_test_case_by_network");
        return -1;
    }
    
    if (test_case->network_type != network_type && test_case->network_type != NETWORK_BOTH) {
        log_message(LOG_LVL_WARN, "Test case %s is not configured for network type %d", 
                   test_case->id, network_type);
        
        memset(result, 0, sizeof(test_result_info_t));
        strncpy(result->test_id, test_case->id, sizeof(result->test_id) - 1);
        result->test_id[sizeof(result->test_id) - 1] = '\0';
        result->test_type = test_case->type;
        result->status = TEST_RESULT_ERROR;
        snprintf(result->result_details, sizeof(result->result_details), 
                "Test case is not configured for this network type");
        
        return -1;
    }
    
    if (network_type == NETWORK_LAN) {
        log_message(LOG_LVL_DEBUG, "Executing test case %s on LAN", test_case->id);
    } else if (network_type == NETWORK_WAN) {
        log_message(LOG_LVL_DEBUG, "Executing test case %s on WAN", test_case->id);
    }
    
    return execute_test_case(test_case, result);
}

// Function thực thi test case dựa trên instruction và input parameters
int execute_instruction(const instruction_t *instruction, cJSON *input_params) {
    if (!instruction) return -1;
    
    log_message(LOG_LVL_DEBUG, "Executing instruction: %s", instruction->action);
    
    num_test_results = 0;
    
    init_test_case(&current_test_case, instruction->action, input_params);
    
    bool has_valid_target = (strlen(current_test_case.target) > 0);
    
    test_action_result_t *test_result = find_test_result(instruction->action);
    if (!test_result) {
        log_message(LOG_LVL_ERROR, "Failed to allocate memory for test result");
        return -1;
    }
    
    memset(&test_result->result, 0, sizeof(test_result_info_t));
    test_result->executed = false;
    
    if (strcmp(instruction->action, "ping") == 0) {
        if (has_valid_target) {
            log_message(LOG_LVL_DEBUG, "Executing ping test to %s", current_test_case.target);
            
            int result = execute_ping_test(&current_test_case, &test_result->result);
            
            if (result == 0) {
                test_result->executed = true;
                log_message(LOG_LVL_DEBUG, "Ping completed with status: %d", test_result->result.status);
            } else {
                log_message(LOG_LVL_ERROR, "Ping execution failed");
                test_result->result.status = TEST_RESULT_ERROR;
                test_result->executed = true;
            }
        } else {
            log_message(LOG_LVL_WARN, "Ping skipped: No valid host specified");
            test_result->result.status = TEST_RESULT_ERROR;
            snprintf(test_result->result.result_details, sizeof(test_result->result.result_details), 
                    "No valid host specified for ping");
            test_result->executed = true;
        }
    } else if (strcmp(instruction->action, "throughput") == 0) {
        log_message(LOG_LVL_WARN, "Throughput test skipped: Not implemented");
        test_result->result.status = TEST_RESULT_ERROR;
        snprintf(test_result->result.result_details, sizeof(test_result->result.result_details), 
                "Throughput test is not implemented yet");
        test_result->executed = true;
    } else if (strcmp(instruction->action, "security") == 0) {
        log_message(LOG_LVL_WARN, "Security test skipped: Not implemented");
        test_result->result.status = TEST_RESULT_ERROR;
        snprintf(test_result->result.result_details, sizeof(test_result->result.result_details), 
                "Security test is not implemented yet");
        test_result->executed = true;
    } else if (strcmp(instruction->action, "speedtest") == 0) {
        log_message(LOG_LVL_DEBUG, "Executing speedtest with server %s", current_test_case.target);
        
        int result = execute_speedtest_test(&current_test_case, &test_result->result);
        
        if (result == 0) {
            test_result->executed = true;
            log_message(LOG_LVL_DEBUG, "Speedtest completed with status: %d", test_result->result.status);
        } else {
            log_message(LOG_LVL_ERROR, "Speedtest execution failed");
            test_result->result.status = TEST_RESULT_ERROR;
            test_result->executed = true;
        }
    } else {
        log_message(LOG_LVL_WARN, "Execution skipped: Unknown action %s", instruction->action);
        test_result->result.status = TEST_RESULT_ERROR;
        snprintf(test_result->result.result_details, sizeof(test_result->result.result_details), 
                "Unknown action: %s", instruction->action);
        test_result->executed = true;
    }
    
    if (test_result->executed) {
        log_message(LOG_LVL_DEBUG, "Test result: %s", test_result_status_to_string(test_result->result.status));
        log_message(LOG_LVL_DEBUG, "Details: %s", test_result->result.result_details);
    }
    
    return 0;
}

// Các stub functions khác cũng sửa tương tự
int tcapi_save() {
    log_message(LOG_LVL_DEBUG, "Saving configuration");
    return 0;
}

int ai_diagnostic_commit() {
    log_message(LOG_LVL_DEBUG, "Committing diagnostics");
    return 0;
}
