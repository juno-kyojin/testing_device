#define _POSIX_C_SOURCE 200809L   /* Thêm để đảm bảo định nghĩa POSIX đầy đủ */

#include "tc.h"
#include "log.h"
#include "file_process.h"
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
static void timeout_handler(int __attribute__((unused)) signum) {
    timeout_occurred = 1;
}

/**
 * @brief Bắt đầu đếm thời gian thực thi
 */
void start_timer() {
    gettimeofday(&start_time, NULL);
}

/**
 * @brief Dừng đếm thời gian và trả về thời gian đã trôi qua (ms)
 * @return Thời gian đã trôi qua tính bằng ms
 */
float stop_timer() {
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
int set_timeout(int timeout_ms) {
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
void clear_timeout() {
    struct itimerval timer;
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;
    setitimer(ITIMER_REAL, &timer, NULL);
}

/**
 * @brief Kiểm tra xem timeout đã xảy ra chưa
 * 
 * @return int 1 nếu timeout đã xảy ra, 0 nếu chưa
 */
int is_timeout_occurred() {
    return timeout_occurred;
}

/**
 * @brief Parse kết quả ping từ output
 * 
 * @param output Chuỗi output của lệnh ping
 * @param result Con trỏ đến biến lưu kết quả
 * @return int 0 nếu thành công, -1 nếu thất bại
 */
int parse_ping_result(const char *output, ping_result_t *result) {
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
    
    // Tạo chuỗi JSON báo cáo
    char *json_content = NULL;
    size_t content_size = 0;
    
    // Tính kích thước dự kiến cho buffer
    content_size = 1024 + count * 256; // Ước tính kích thước cơ bản
    json_content = (char *)malloc(content_size);
    if (!json_content) {
        log_message(LOG_LVL_ERROR, "Failed to allocate memory for report JSON");
        return -1;
    }
    
    // Tạo báo cáo dạng JSON
    int offset = 0;
    offset += snprintf(json_content + offset, content_size - offset, "{\n  \"test_results\": [\n");
    
    for (int i = 0; i < count; i++) {
        offset += snprintf(json_content + offset, content_size - offset,
                "    {\n      \"test_id\": \"%s\",\n      \"status\": \"%s\",\n      \"details\": \"%s\"\n    }%s\n", 
                results[i].test_id, 
                test_result_status_to_string(results[i].status), 
                results[i].result_details,
                (i < count - 1) ? "," : "");
    }
    
    offset += snprintf(json_content + offset, content_size - offset, "  ]\n}\n");
    
    if (write_file(filename, json_content, offset) != 0) {
        log_message(LOG_LVL_ERROR, "Failed to write report to file %s", filename);
        free(json_content);
        return -1;
    }
    
    free(json_content);
    log_message(LOG_LVL_DEBUG, "Successfully generated report: %s", filename);
    
    return 0;
}