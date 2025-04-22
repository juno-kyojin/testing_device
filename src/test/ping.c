#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include "test/ping.h"
#include "core/log.h"

// Hàm thực hiện lệnh ping thực tế và phân tích kết quả
static int perform_real_ping(const char *host, char *host_address, size_t host_address_size,
                            int *success_count, int *failure_count,
                            float *avg_time, float *min_time, float *max_time,
                            float *jitter, float *packet_loss) {
    char cmd[256];
    char buf[1024];
    FILE *fp;
    int status = -1;
    int packets_received = 0;
    int packets_transmitted = 0;
    
    // Tạo lệnh ping với 5 gói tin, timeout 3 giây
    snprintf(cmd, sizeof(cmd), "ping -c 5 -W 3 %s 2>&1", host);
    log_message(LOG_LVL_DEBUG, "Executing real ping command: %s", cmd);
    
    // Thực thi lệnh ping
    fp = popen(cmd, "r");
    if (fp == NULL) {
        log_message(LOG_LVL_ERROR, "Failed to execute ping command");
        return -1;
    }
    
    // Đọc và phân tích kết quả
    *avg_time = 0.0;
    *min_time = 0.0;
    *max_time = 0.0;
    *jitter = 0.0;
    *packet_loss = 100.0;
    *success_count = 0;
    *failure_count = 0;
    host_address[0] = '\0';
    
    while (fgets(buf, sizeof(buf), fp)) {
        // Trích xuất địa chỉ IP của host (hostAddress)
        if (strstr(buf, "PING") && strstr(buf, host)) {
            char *ip_start = strstr(buf, "(");
            char *ip_end = strstr(buf, ")");
            if (ip_start && ip_end) {
                ip_start++;
                *ip_end = '\0';
                strncpy(host_address, ip_start, host_address_size);
                host_address[host_address_size - 1] = '\0';
            }
        }
        
        // Phân tích dòng chứa thông tin số gói tin gửi/nhận
        if (strstr(buf, "packets transmitted")) {
            sscanf(buf, "%d packets transmitted, %d %*[^,]", 
                  &packets_transmitted, &packets_received);
            
            if (packets_transmitted > 0) {
                *packet_loss = 100.0 * (packets_transmitted - packets_received) / packets_transmitted;
                *success_count = packets_received;
                *failure_count = packets_transmitted - packets_received;
            }
            
            log_message(LOG_LVL_DEBUG, "Packets: %d sent, %d received, %.1f%% loss", 
                       packets_transmitted, packets_received, *packet_loss);
        }
        
        // Phân tích dòng chứa thông tin thời gian
        if (strstr(buf, "rtt min/avg/max/mdev")) {
            sscanf(buf, "%*[^=]= %f/%f/%f/%f", min_time, avg_time, max_time, jitter);
            log_message(LOG_LVL_DEBUG, "RTT: min=%.3f ms, avg=%.3f ms, max=%.3f ms, jitter=%.3f ms", 
                        *min_time, *avg_time, *max_time, *jitter);
        }
    }
    
    // Xác định trạng thái dựa trên kết quả thực tế
    if (packets_received > 0) {
        status = 0; // Thành công nếu nhận được ít nhất 1 gói tin
    } else {
        status = 1; // Thất bại nếu không nhận được gói tin nào
        *avg_time = 0.0;
        *min_time = 0.0;
        *max_time = 0.0;
        *jitter = 0.0;
    }
    
    int exit_code = pclose(fp);
    if (status == 0 && WIFEXITED(exit_code) && WEXITSTATUS(exit_code) != 0) {
        log_message(LOG_LVL_WARN, "Ping command exited with non-zero status %d", WEXITSTATUS(exit_code));
    }
    
    return status;
}

void execute_ping(Instruction *instr, TestCase *test_case) {
    log_message(LOG_LVL_DEBUG, "Executing ping test");
    time_t start_time = time(NULL);

    // Tìm host để ping
    const char *host = NULL;
    for (int i = 0; i < test_case->param_count; i++) {
        log_message(LOG_LVL_DEBUG, "Param %s = %s", 
                    test_case->input_params[i].key, test_case->input_params[i].value);
        if (strcmp(test_case->input_params[i].key, "host") == 0) {
            host = test_case->input_params[i].value;
        }
    }
    
    if (host == NULL) {
        log_message(LOG_LVL_ERROR, "No host specified for ping test");
        return;
    }
    
    log_message(LOG_LVL_DEBUG, "Performing real ping to %s", host);
    
    char host_address[256] = "";
    int success_count = 0;
    int failure_count = 0;
    float avg_time = 0.0;
    float min_time = 0.0;
    float max_time = 0.0;
    float jitter = 0.0;
    float packet_loss = 0.0;
    
    int ping_status = perform_real_ping(host, host_address, sizeof(host_address),
                                        &success_count, &failure_count,
                                        &avg_time, &min_time, &max_time,
                                        &jitter, &packet_loss);
    
    // Log kết quả mà không lưu trữ
    log_message(LOG_LVL_DEBUG, "Ping.Status = %d", ping_status);
    log_message(LOG_LVL_DEBUG, "Ping.host = %s", host);
    log_message(LOG_LVL_DEBUG, "Ping.hostAddress = %s", host_address);
    log_message(LOG_LVL_DEBUG, "Ping.successCount = %d", success_count);
    log_message(LOG_LVL_DEBUG, "Ping.failureCount = %d", failure_count);
    log_message(LOG_LVL_DEBUG, "Ping.averageResponseTime = %.3f ms", avg_time);
    log_message(LOG_LVL_DEBUG, "Ping.minimumResponseTime = %.3f ms", min_time);
    log_message(LOG_LVL_DEBUG, "Ping.maximumResponseTime = %.3f ms", max_time);
    log_message(LOG_LVL_DEBUG, "Ping.jitter = %.3f ms", jitter);
    log_message(LOG_LVL_DEBUG, "Ping.packetLoss = %.1f%%", packet_loss);
    
    time_t end_time = time(NULL);
    log_message(LOG_LVL_DEBUG, "Ping test completed in %ld seconds", end_time - start_time);
}