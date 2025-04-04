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
    // Xác định loại test dựa vào node
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
        // Node không được hỗ trợ
        strcpy(value, "N/A");
        return -1;
    }
    
    // Nếu không tìm thấy kết quả hoặc chưa thực thi, trả về giá trị mặc định
    if (!test_result || !test_result->executed) {
        if (strcmp(attribute, "Status") == 0) {
            strcpy(value, "-1");
        } else {
            strcpy(value, "N/A");
        }
        return 0;
    }
    
    // Xử lý các thuộc tính chung cho tất cả loại test
    if (strcmp(attribute, "Status") == 0) {
        // 0 = success, 1 = failed, 2 = timeout, 3 = error
        switch (test_result->result.status) {
            case TEST_RESULT_SUCCESS: strcpy(value, "0"); break;
            case TEST_RESULT_FAILED:  strcpy(value, "1"); break; 
            case TEST_RESULT_TIMEOUT: strcpy(value, "2"); break;
            case TEST_RESULT_ERROR:
            default:                  strcpy(value, "3");
        }
        return 0;
    }
    
    // Xử lý các thuộc tính riêng cho từng loại test
    if (strcmp(node, "Ping") == 0) {
        // Thuộc tính riêng của ping
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
        // Thuộc tính riêng của throughput
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
        // Thuộc tính riêng của security
        if (strcmp(attribute, "passed") == 0) {
            strcpy(value, test_result->result.data.security.passed ? "1" : "0");
        } else if (strcmp(attribute, "vulnerabilities") == 0) {
            sprintf(value, "%d", test_result->result.data.security.vulnerabilities);
        } else {
            return -1;
        }
    } else if (strcmp(node, "Speedtest") == 0) {
        // Thuộc tính riêng của speedtest
        if (strcmp(attribute, "DownloadSpeed") == 0) {
            sprintf(value, "%.2f", test_result->result.data.speedtest.download_speed);
        } else if (strcmp(attribute, "UploadSpeed") == 0) {
            sprintf(value, "%.2f", test_result->result.data.speedtest.upload_speed);
        } else if (strcmp(attribute, "Latency") == 0) {
            sprintf(value, "%.2f", test_result->result.data.speedtest.latency);
        } else if (strcmp(attribute, "ServerDetails") == 0) {
            // Trích xuất thông tin server từ result_details
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
    
    // Xử lý thiết lập tham số cho từng loại test
    if (strcmp(node, "Ping") == 0) {
        if (strcmp(attribute, "host") == 0) {
            strncpy(current_test_case.target, value, sizeof(current_test_case.target) - 1);
            current_test_case.target[sizeof(current_test_case.target) - 1] = '\0';
        }
    } else if (strcmp(node, "Throughput") == 0) {
        // Thiết lập tham số throughput
        if (strcmp(attribute, "server") == 0) {
            strncpy(current_test_case.target, value, sizeof(current_test_case.target) - 1);
            current_test_case.target[sizeof(current_test_case.target) - 1] = '\0';
        }
    } else if (strcmp(node, "Security") == 0) {
        // Thiết lập tham số security
        if (strcmp(attribute, "target") == 0) {
            strncpy(current_test_case.target, value, sizeof(current_test_case.target) - 1);
            current_test_case.target[sizeof(current_test_case.target) - 1] = '\0';
        }
    } else if (strcmp(node, "Speedtest") == 0) {
        // Thiết lập tham số speedtest
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
    
    // Thiết lập các tham số chung
    snprintf(test_case->id, sizeof(test_case->id), "%s_test", action);
    test_case->timeout = 5000;  // 5 giây timeout mặc định
    test_case->enabled = true;
    
    // Thiết lập các tham số riêng cho từng loại test
    if (strcmp(action, "ping") == 0) {
        test_case->type = TEST_PING;
        strcpy(test_case->name, "Ping Test");
        
        // Tham số ping mặc định
        test_case->params.ping.count = 5;
        test_case->params.ping.size = 64;
        test_case->params.ping.interval = 1000;
        test_case->params.ping.ipv6 = false;
        
        // Đọc host từ input_params nếu có
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
        
        // Tham số throughput mặc định
        test_case->params.throughput.duration = 10;
        strcpy(test_case->params.throughput.protocol, "TCP");
        test_case->params.throughput.port = 5201;
        test_case->params.throughput.buffer_size = 8192;
        
        // Đọc server từ input_params nếu có
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
        
        // Tham số security mặc định
        strcpy(test_case->params.security.method, "tls_scan");
        test_case->params.security.port = 443;
        test_case->params.security.tls = true;
        
        // Đọc target từ input_params nếu có
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
        
        // Tham số speedtest mặc định
        test_case->params.speedtest.timeout = 30;
        test_case->params.speedtest.use_https = true;
        
        // Đọc server từ input_params nếu có
        if (input_params) {
            cJSON *server = cJSON_GetObjectItem(input_params, "server");
            if (server && cJSON_IsString(server) && strlen(server->valuestring) > 0) {
                strncpy(test_case->target, server->valuestring, sizeof(test_case->target) - 1);
                test_case->target[sizeof(test_case->target) - 1] = '\0';
            } else {
                // Nếu không có server được chỉ định, sử dụng server mặc định
                strcpy(test_case->target, "speedtest.net");
            }
        } else {
            // Sử dụng server mặc định khi không có input_params
            strcpy(test_case->target, "speedtest.net");
        }
    }
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
    
    // Khởi tạo result
    memset(result, 0, sizeof(test_result_info_t));
    strncpy(result->test_id, test_case->id, sizeof(result->test_id) - 1);
    result->test_type = TEST_SPEEDTEST;
    result->status = TEST_RESULT_ERROR; // Mặc định là lỗi, sẽ thay đổi nếu test thành công
    
    // Kiểm tra xem speedtest-cli đã được cài đặt chưa
    if (system("which speedtest-cli > /dev/null 2>&1") != 0) {
        log_message(LOG_LVL_ERROR, "speedtest-cli not installed. Please install with: sudo apt-get install speedtest-cli");
        snprintf(result->result_details, sizeof(result->result_details), 
                "speedtest-cli not installed. Please install with: sudo apt-get install speedtest-cli");
        result->status = TEST_RESULT_ERROR;
        return -1;
    }
    
    log_message(LOG_LVL_DEBUG, "Starting speedtest with server target: %s", 
                test_case->target[0] ? test_case->target : "default");
    
    // Chuẩn bị lệnh speedtest-cli
    char command[512];
    
    // Không sử dụng target trực tiếp làm server ID, vì server phải là một ID số
    if (strcmp(test_case->target, "speedtest.net") == 0) {
        // Nếu là speedtest.net, sử dụng server gần nhất
        snprintf(command, sizeof(command), "speedtest-cli --json --secure");
        log_message(LOG_LVL_DEBUG, "Using default server (nearest)");
    } else if (atoi(test_case->target) > 0) {
        // Nếu target là một số ID hợp lệ
        snprintf(command, sizeof(command), "speedtest-cli --json --server %s --secure", 
                 test_case->target);
        log_message(LOG_LVL_DEBUG, "Using server ID: %s", test_case->target);
    } else if (test_case->target[0]) {
        // Thử tìm server gần nhất với host cụ thể (không chắc chắn sẽ hoạt động)
        log_message(LOG_LVL_WARN, "Server '%s' might not be a valid server ID, attempting to use anyway", test_case->target);
        snprintf(command, sizeof(command), "speedtest-cli --json --server %s --secure", 
                 test_case->target);
    } else {
        // Không có server được chỉ định, sử dụng server gần nhất
        snprintf(command, sizeof(command), "speedtest-cli --json --secure");
        log_message(LOG_LVL_DEBUG, "Using default server (nearest)");
    }
    
    // Bắt đầu đo thời gian thực thi
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);

    // Ghi log
    log_message(LOG_LVL_DEBUG, "Executing command: %s", command);
    
    // Mở pipe để đọc output trực tiếp từ lệnh
    FILE *pipe = popen(command, "r");
    if (!pipe) {
        log_message(LOG_LVL_ERROR, "Failed to open pipe for speedtest-cli");
        snprintf(result->result_details, sizeof(result->result_details), 
                "Failed to execute speedtest-cli command: %s", strerror(errno));
        result->status = TEST_RESULT_ERROR;
        return -1;
    }
    
    // Đọc output trực tiếp từ pipe vào buffer
    char json_buffer[8192] = {0};
    size_t bytes_read = fread(json_buffer, 1, sizeof(json_buffer) - 1, pipe);
    
    // Đóng pipe và lấy exit code
    int exit_status = pclose(pipe);
    
    // Dừng đo thời gian
    gettimeofday(&end_time, NULL);
    result->execution_time = (end_time.tv_sec - start_time.tv_sec) * 1000.0f + 
                            (end_time.tv_usec - start_time.tv_usec) / 1000.0f;
    
    log_message(LOG_LVL_DEBUG, "Speedtest execution time: %.1f ms", result->execution_time);
    
    if (exit_status != 0) {
        log_message(LOG_LVL_ERROR, "speedtest-cli execution failed with code %d", exit_status);
        
        if (bytes_read > 0) {
            // Có thể có thông báo lỗi trong output, giới hạn kích thước
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
        
        // Thử chạy một lệnh đơn giản hơn, và lấy kết quả trực tiếp
        log_message(LOG_LVL_DEBUG, "Attempting basic speedtest-cli command for diagnostic");
        
        FILE *simple_pipe = popen("speedtest-cli --simple", "r");
        if (simple_pipe) {
            char simple_result[1024] = {0};
            size_t simple_bytes = fread(simple_result, 1, sizeof(simple_result) - 1, simple_pipe);
            pclose(simple_pipe);
            
            if (simple_bytes > 0) {
                log_message(LOG_LVL_DEBUG, "Simple speedtest result: %s", simple_result);
                
                // Giới hạn kích thước thông tin kết quả đơn giản
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
    // Parse JSON kết quả
    cJSON *json = cJSON_Parse(json_buffer);
    if (!json) {
        log_message(LOG_LVL_ERROR, "Failed to parse speedtest JSON result: %s", cJSON_GetErrorPtr());
        log_message(LOG_LVL_DEBUG, "Raw JSON content (first 200 chars): %.200s", json_buffer);
        snprintf(result->result_details, sizeof(result->result_details), 
                "Failed to parse speedtest JSON result");
        result->status = TEST_RESULT_ERROR;
        return -1;
    }
    
    // Lấy download, upload speed và ping
    cJSON *download = cJSON_GetObjectItem(json, "download");
    cJSON *upload = cJSON_GetObjectItem(json, "upload");
    cJSON *ping = cJSON_GetObjectItem(json, "ping");
    
    if (download && cJSON_IsNumber(download)) {
        // speedtest-cli trả về bit/s, chuyển đổi sang Mbps
        result->data.speedtest.download_speed = download->valuedouble / 1000000.0;
    } else {
        log_message(LOG_LVL_WARN, "Download speed not found in JSON result");
    }
    
    if (upload && cJSON_IsNumber(upload)) {
        // speedtest-cli trả về bit/s, chuyển đổi sang Mbps
        result->data.speedtest.upload_speed = upload->valuedouble / 1000000.0;
    } else {
        log_message(LOG_LVL_WARN, "Upload speed not found in JSON result");
    }
    
    if (ping && cJSON_IsNumber(ping)) {
        result->data.speedtest.latency = ping->valuedouble;
    } else {
        log_message(LOG_LVL_WARN, "Ping/latency not found in JSON result");
    }
    
    // Lấy thông tin thêm về server
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
                
                // Lưu lại server ID cho lần sau
                log_message(LOG_LVL_DEBUG, "Server ID for future reference: %s", serverId->valuestring);
            } else {
                snprintf(server_info, sizeof(server_info), "%s (%s, %s)", 
                        name->valuestring, host->valuestring, country->valuestring);
            }
        }
    }
    
    // Cập nhật kết quả
    snprintf(result->result_details, sizeof(result->result_details), 
            "Speedtest completed with server %s. Download: %.2f Mbps, Upload: %.2f Mbps, Latency: %.2f ms", 
            server_info,
            result->data.speedtest.download_speed,
            result->data.speedtest.upload_speed,
            result->data.speedtest.latency);
    
    // Giải phóng bộ nhớ
    cJSON_Delete(json);
    
    // Kiểm tra nếu có kết quả hợp lệ
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

// Function thực thi test case dựa trên instruction và input parameters
int execute_instruction(const instruction_t *instruction, cJSON *input_params) {
    if (!instruction) return -1;
    
    log_message(LOG_LVL_DEBUG, "Executing instruction: %s", instruction->action);
    
    // Reset kết quả trước khi thực thi
    num_test_results = 0;
    
    // Khởi tạo test case dựa trên action
    init_test_case(&current_test_case, instruction->action, input_params);
    
    // Kiểm tra xem có target hợp lệ không
    bool has_valid_target = (strlen(current_test_case.target) > 0);
    
    // Lấy hoặc tạo test result cho action hiện tại
    test_action_result_t *test_result = find_test_result(instruction->action);
    if (!test_result) {
        log_message(LOG_LVL_ERROR, "Failed to allocate memory for test result");
        return -1;
    }
    
    // Reset kết quả test
    memset(&test_result->result, 0, sizeof(test_result_info_t));
    test_result->executed = false;
    
    // Xử lý các loại action khác nhau
    if (strcmp(instruction->action, "ping") == 0) {
        if (has_valid_target) {
            log_message(LOG_LVL_DEBUG, "Executing ping test to %s", current_test_case.target);
            
            // Gọi hàm ping thực tế
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
        
        // Gọi hàm thực thi speedtest
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
    
    // In kết quả thực thi vào log thay vì stdout
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
