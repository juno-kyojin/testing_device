
#define _POSIX_C_SOURCE 200809L   /* Thêm để đảm bảo định nghĩa POSIX đầy đủ */

#include "tc.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <errno.h>

// Biến đếm thời gian thực thi
static struct timeval start_time, end_time;

// Biến xử lý timeout
static volatile int timeout_occurred = 0;

/**
 * @brief Handler cho tín hiệu SIGALRM (dùng cho timeout)
 */
static void timeout_handler(int signum) {
    timeout_occurred = 1;
}

/**
 * @brief Bắt đầu đếm thời gian thực thi
 */
static void start_timer() {
    gettimeofday(&start_time, NULL);
}

/**
 * @brief Dừng đếm thời gian và trả về thời gian đã trôi qua (ms)
 * @return Thời gian đã trôi qua tính bằng ms
 */
static float stop_timer() {
    gettimeofday(&end_time, NULL);
    return (end_time.tv_sec - start_time.tv_sec) * 1000.0f + 
           (end_time.tv_usec - start_time.tv_usec) / 1000.0f;
}

/**
 * @brief Thiết lập timeout cho test
 * 
 * @param timeout_ms Thời gian timeout tính bằng ms
 * @return int 0 nếu thành công, -1 nếu thất bại
 */
static int set_timeout(int timeout_ms) {
    struct sigaction sa;
    struct itimerval timer;
    
    // Thiết lập handler cho SIGALRM
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &timeout_handler;
    if (sigaction(SIGALRM, &sa, NULL) < 0) {
        log_message(LOG_LVL_ERROR, "Failed to set signal handler for timeout");
        return -1;
    }
    
    // Thiết lập timer
    timer.it_value.tv_sec = timeout_ms / 1000;
    timer.it_value.tv_usec = (timeout_ms % 1000) * 1000;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;
    
    // Bắt đầu timer
    if (setitimer(ITIMER_REAL, &timer, NULL) < 0) {
        log_message(LOG_LVL_ERROR, "Failed to set timer for timeout");
        return -1;
    }
    
    timeout_occurred = 0;
    return 0;
}

/**
 * @brief Hủy timeout
 */
static void clear_timeout() {
    struct itimerval timer;
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;
    setitimer(ITIMER_REAL, &timer, NULL);
}

/**
 * @brief Parse kết quả ping từ output
 * 
 * @param output Chuỗi output của lệnh ping
 * @param result Con trỏ đến biến lưu kết quả
 * @return int 0 nếu thành công, -1 nếu thất bại
 */
static int parse_ping_result(const char *output, ping_result_t *result) {
    if (!output || !result) {
        return -1;
    }
    
    // Khởi tạo kết quả mặc định
    memset(result, 0, sizeof(ping_result_t));
    result->min_rtt = -1;
    result->avg_rtt = -1;
    result->max_rtt = -1;
    
    char *transmitted = strstr(output, "packets transmitted");
    char *received = strstr(output, "received");
    char *loss = strstr(output, "packet loss");
    char *rtt = strstr(output, "rtt min/avg/max");
    
    // Parse số gói tin gửi
    if (transmitted) {
        // Tìm số ngay trước "packets transmitted"
        char *num_start = transmitted;
        while (num_start > output && *(num_start-1) != '\n') {
            num_start--;
        }
        result->packets_sent = atoi(num_start);
        log_message(LOG_LVL_DEBUG, "Ping packets sent: %d", result->packets_sent);
    } else {
        log_message(LOG_LVL_WARN, "Could not find 'packets transmitted' in ping output");
    }
    
    // Parse số gói tin nhận
    if (received) {
        // Format thường là "X packets transmitted, Y received, Z% packet loss"
        // Nên phải lùi ngược từ từ khóa "received" đến dấu ","
        char *comma = received;
        while (comma > output && *comma != ',') {
            comma--;
        }
        
        if (*comma == ',') {
            // Tiến về phía trước để bỏ qua khoảng trắng
            comma++;
            while (*comma == ' ')
                comma++;
                
            // Đọc số packets received
            result->packets_received = atoi(comma);
        } else {
            // Nếu không tìm thấy dấu phẩy, thử phương pháp khác
            // Phương pháp dự phòng: Trích xuất cả dòng statistics và quét
            char *stat_line = strstr(output, "statistics");
            if (stat_line) {
                while (stat_line > output && *stat_line != '\n') {
                    stat_line--;
                }
                if (*stat_line == '\n') stat_line++;
                
                char line[256] = {0};
                char *end = strchr(stat_line, '\n');
                if (end) {
                    int len = end - stat_line;
                    strncpy(line, stat_line, len < 255 ? len : 255);
                    
                    // Quét toàn bộ dòng để tìm format "X packets transmitted, Y received"
                    int sent, recv;
                    if (sscanf(line, "%d packets transmitted, %d received", &sent, &recv) == 2) {
                        result->packets_received = recv;
                    }
                }
            }
        }
        
        log_message(LOG_LVL_DEBUG, "Ping packets received: %d", result->packets_received);
    } else {
        log_message(LOG_LVL_WARN, "Could not find 'received' in ping output");
    }
    
    // Parse tỷ lệ mất gói
    if (loss) {
        // Tìm số ngay trước "packet loss"
        char *percent_sign = strchr(loss - 10, '%');
        if (percent_sign) {
            char *num_start = percent_sign;
            while (num_start > output && *(num_start-1) != ' ') {
                num_start--;
            }
            result->packet_loss = atof(num_start);
            log_message(LOG_LVL_DEBUG, "Ping packet loss: %.1f%%", result->packet_loss);
        }
    } else if (result->packets_sent > 0) {
        // Tính tỷ lệ mất gói nếu không tìm thấy trong output
        result->packet_loss = 100.0f * (result->packets_sent - result->packets_received) / result->packets_sent;
        log_message(LOG_LVL_DEBUG, "Calculated ping packet loss: %.1f%%", result->packet_loss);
    }
    
    // Parse RTT min/avg/max
    if (rtt) {
        // Format "rtt min/avg/max/mdev = 0.042/0.054/0.074/0.014 ms"
        char *values_start = strstr(rtt, "=");
        if (values_start) {
            values_start++; // Bỏ qua dấu '='
            
            // Bỏ qua khoảng trắng
            while (*values_start == ' ') {
                values_start++;
            }
            
            // Đọc các giá trị
            if (sscanf(values_start, "%f/%f/%f", 
                      &result->min_rtt, &result->avg_rtt, &result->max_rtt) == 3) {
                log_message(LOG_LVL_DEBUG, "Ping RTT min/avg/max: %.3f/%.3f/%.3f ms", 
                           result->min_rtt, result->avg_rtt, result->max_rtt);
            } else {
                log_message(LOG_LVL_WARN, "Failed to parse RTT values: %s", values_start);
            }
        } else {
            log_message(LOG_LVL_WARN, "Could not find '=' in rtt line");
        }
    } else {
        log_message(LOG_LVL_WARN, "Could not find 'rtt min/avg/max' in ping output");
    }
    
    // QUAN TRỌNG: Đặt giá trị packets_received bằng với packets_sent nếu có RTT và packet loss = 0
    // Đây là trường hợp đặc biệt khi phân tích không tìm được số gói đã nhận
    if (result->packets_received == 0 && result->min_rtt > 0 && result->packets_sent > 0) {
        result->packets_received = result->packets_sent;
        log_message(LOG_LVL_DEBUG, "Fixed received packets count to %d based on successful RTT values", result->packets_received);
    }
    
    // Kiểm tra điều kiện thành công - nếu nhận được ít nhất 1 gói tin hoặc có RTT
    return (result->packets_received > 0 || result->min_rtt > 0) ? 0 : -1;
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
    
    // Khởi tạo kết quả
    memset(result, 0, sizeof(test_result_info_t));
    strncpy(result->test_id, test_case->id, sizeof(result->test_id) - 1);
    result->test_id[sizeof(result->test_id) - 1] = '\0';
    result->test_type = TEST_PING;
    result->status = TEST_RESULT_ERROR;
    
    // Kiểm tra target
    if (strlen(test_case->target) == 0) {
        log_message(LOG_LVL_ERROR, "Empty target for ping test case %s", test_case->id);
        snprintf(result->result_details, sizeof(result->result_details), 
                "Invalid target: empty string");
        return -1;
    }
    
    // Tạo lệnh ping
    char ping_cmd[512];
    const char *ping_cmd_base = test_case->params.ping.ipv6 ? "ping6" : "ping";
    
    // Sử dụng thông số từ test case
    snprintf(ping_cmd, sizeof(ping_cmd), 
             "%s -c %d -s %d -i %.1f %s", 
             ping_cmd_base,
             test_case->params.ping.count,
             test_case->params.ping.size,
             test_case->params.ping.interval / 1000.0f,  // Chuyển ms sang giây
             test_case->target);
    
    log_message(LOG_LVL_DEBUG, "Executing ping command: %s", ping_cmd);
    
    // Mở pipe để đọc output từ lệnh ping
    FILE *pipe = popen(ping_cmd, "r");
    if (!pipe) {
        log_message(LOG_LVL_ERROR, "Failed to execute ping command: %s", ping_cmd);
        snprintf(result->result_details, sizeof(result->result_details), 
                 "Failed to execute ping command: %s", strerror(errno));
        return -1;
    }
    
    // Đọc output
    char buffer[4096];
    memset(buffer, 0, sizeof(buffer));
    
    // Bắt đầu đếm thời gian và thiết lập timeout
    start_timer();
    if (set_timeout(test_case->timeout) != 0) {
        log_message(LOG_LVL_WARN, "Failed to set timeout for ping test");
    }
    
    // Đọc output từ pipe
    size_t bytes_read = 0;
    char *ptr = buffer;
    size_t remaining = sizeof(buffer) - 1;
    
    while (!timeout_occurred && remaining > 0) {
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
    
    // Hủy timeout và dừng đếm thời gian
    clear_timeout();
    result->execution_time = stop_timer();
    
    // Đảm bảo buffer là null-terminated
    buffer[bytes_read] = '\0';
    
    // Đóng pipe và lấy exit code
    int exit_code = pclose(pipe);
    
    // Xử lý trường hợp timeout
    if (timeout_occurred) {
        log_message(LOG_LVL_WARN, "Ping test timed out after %.1f ms", result->execution_time);
        result->status = TEST_RESULT_TIMEOUT;
        snprintf(result->result_details, sizeof(result->result_details), 
                 "Ping test to %s timed out after %.1f ms", 
                 test_case->target, result->execution_time);
        return 0;
    }
    
    // Xử lý kết quả dựa vào exit code và output
    if (WIFEXITED(exit_code)) {
        int status = WEXITSTATUS(exit_code);
        log_message(LOG_LVL_DEBUG, "Ping command exited with status %d", status);
        
        // Parse output nếu có
        if (bytes_read > 0) {
            if (parse_ping_result(buffer, &result->data.ping) == 0) {
                // Nếu parse thành công (có nhận được gói) thì đặt trạng thái SUCCESS
                result->status = TEST_RESULT_SUCCESS;
                
                // Tạo thông tin chi tiết
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
                // Nếu parse thất bại (không nhận được gói nào) thì đặt trạng thái FAILED
                result->status = TEST_RESULT_FAILED;
                snprintf(result->result_details, sizeof(result->result_details), 
                         "Ping to %s failed. All packets lost.", test_case->target);
            }
        } else {
            // Không có output
            result->status = TEST_RESULT_ERROR;
            snprintf(result->result_details, sizeof(result->result_details), 
                     "No output from ping command");
        }
    } else {
        // Lệnh ping không kết thúc đúng cách
        result->status = TEST_RESULT_ERROR;
        snprintf(result->result_details, sizeof(result->result_details), 
                 "Ping command did not exit properly");
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
    
    // Kiểm tra trạng thái enabled
    if (!test_case->enabled) {
        log_message(LOG_LVL_WARN, "Test case %s is disabled, skipping execution", test_case->id);
        
        // Khởi tạo kết quả mặc định cho test case bị disable
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
    
    // Chỉ thực thi test ping, bỏ qua các loại test khác
    int ret = -1;
    if (test_case->type == TEST_PING) {
        ret = execute_ping_test(test_case, result);
    } else {
        // Đối với các loại test khác, tạo kết quả với thông báo "not supported"
        log_message(LOG_LVL_WARN, "Only ping test is currently supported. Skipping test case %s of type %d", 
                   test_case->id, test_case->type);
        
        memset(result, 0, sizeof(test_result_info_t));
        strncpy(result->test_id, test_case->id, sizeof(result->test_id) - 1);
        result->test_id[sizeof(result->test_id) - 1] = '\0';
        result->test_type = test_case->type;
        result->status = TEST_RESULT_ERROR;
        snprintf(result->result_details, sizeof(result->result_details), 
                "Only ping test is currently supported");
        
        // Trả về 0 để không gây lỗi cho toàn bộ quy trình
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
    
    // Kiểm tra test case có được cấu hình cho loại mạng này không
    if (test_case->network_type != network_type && test_case->network_type != NETWORK_BOTH) {
        log_message(LOG_LVL_WARN, "Test case %s is not configured for network type %d", 
                   test_case->id, network_type);
        
        // Khởi tạo kết quả mặc định
        memset(result, 0, sizeof(test_result_info_t));
        strncpy(result->test_id, test_case->id, sizeof(result->test_id) - 1);
        result->test_id[sizeof(result->test_id) - 1] = '\0';
        result->test_type = test_case->type;
        result->status = TEST_RESULT_ERROR;
        snprintf(result->result_details, sizeof(result->result_details), 
                "Test case is not configured for this network type");
        
        return -1;
    }
    
    // Ta có thể cấu hình interface mạng dựa vào loại mạng
    if (network_type == NETWORK_LAN) {
        log_message(LOG_LVL_DEBUG, "Executing test case %s on LAN", test_case->id);
    } else if (network_type == NETWORK_WAN) {
        log_message(LOG_LVL_DEBUG, "Executing test case %s on WAN", test_case->id);
    }
    
    // Thực thi test case
    return execute_test_case(test_case, result);
}

/**
 * @brief Chuyển đổi trạng thái kết quả test sang chuỗi
 * 
 * @param status Trạng thái kết quả test
 * @return const char* Chuỗi mô tả trạng thái
 */
const char* test_result_status_to_string(test_result_status_t status) {
    switch (status) {
        case TEST_RESULT_SUCCESS: return "SUCCESS";
        case TEST_RESULT_FAILED: return "FAILED";
        case TEST_RESULT_TIMEOUT: return "TIMEOUT";
        case TEST_RESULT_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

/**
 * @brief Tạo báo cáo tổng hợp từ các kết quả test
 * 
 * @param results Mảng kết quả test
 * @param count Số lượng kết quả
 * @param filename Đường dẫn đến file báo cáo
 * @return int 0 nếu thành công, -1 nếu thất bại
 */
int generate_summary_report(test_result_info_t *results, int count, const char *filename) {
    if (!results || count <= 0 || !filename) {
        log_message(LOG_LVL_ERROR, "Invalid parameters for generate_summary_report");
        return -1;
    }
    
    log_message(LOG_LVL_DEBUG, "Generating summary report to %s", filename);
    
    FILE *file = fopen(filename, "w");
    if (!file) {
        log_message(LOG_LVL_ERROR, "Failed to open report file %s: %s", 
                   filename, strerror(errno));
        return -1;
    }
    
    // Tạo báo cáo dạng JSON đơn giản
    fprintf(file, "{\n  \"test_results\": [\n");
    
    for (int i = 0; i < count; i++) {
        fprintf(file, "    {\n");
        fprintf(file, "      \"test_id\": \"%s\",\n", results[i].test_id);
        fprintf(file, "      \"status\": \"%s\",\n", test_result_status_to_string(results[i].status));
        fprintf(file, "      \"details\": \"%s\"\n", results[i].result_details);
        fprintf(file, "    }%s\n", (i < count - 1) ? "," : "");
    }
    
    fprintf(file, "  ]\n}\n");
    
    fclose(file);
    log_message(LOG_LVL_DEBUG, "Successfully generated report: %s", filename);
    
    return 0;
}