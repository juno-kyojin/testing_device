/**
 * @file ping.c
 * @brief Implementation of the ping test case handler
 *
 * This file implements the handler for the "ping" service, which executes ping
 * test cases by sending ICMP packets to a specified host and recording the results.
 * The handler processes test cases from a JSON file, executes the `ping` command,
 * and stores the results (e.g., packets received, packet loss, average latency)
 * in a JSON array. It is part of the test case execution system and is registered
 * with the action dispatch mechanism.
 *
 * @author [junokyojin]
 * @date 2025-04-23
 * @see ping.h
 * @see action.h
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include "ping.h"
#include "log.h"
#include "file_process.h"
#include "cjson/cJSON.h"

int parse_host(const char *filepath, int index, char *host, size_t host_size, cJSON *result_json) {
    // Đọc file JSON
    char *json_data;
    size_t json_size;
    if (read_file(filepath, &json_data, &json_size) != 0) {
        log_message(LOG_LVL_ERROR, "Failed to read JSON file: %s", filepath);
        cJSON_AddStringToObject(result_json, "fail_reason", "Failed to read JSON file");
        return -1;
    }

    // Parse JSON
    cJSON *json = cJSON_Parse(json_data);
    if (!json) {
        log_message(LOG_LVL_ERROR, "Failed to parse JSON file: %s", filepath);
        cJSON_AddStringToObject(result_json, "fail_reason", "Failed to parse JSON file");
        free(json_data);
        return -1;
    }

    // Lấy mảng test_cases
    cJSON *test_cases_json = cJSON_GetObjectItem(json, "test_cases");
    if (!cJSON_IsArray(test_cases_json)) {
        log_message(LOG_LVL_ERROR, "No 'test_cases' array found in %s", filepath);
        cJSON_AddStringToObject(result_json, "fail_reason", "No test cases array found");
        cJSON_Delete(json);
        free(json_data);
        return -1;
    }

    // Tìm test case theo chỉ số
    cJSON *test_case_json = cJSON_GetArrayItem(test_cases_json, index);
    if (!test_case_json) {
        log_message(LOG_LVL_ERROR, "Test case at index %d not found in %s", index, filepath);
        cJSON_AddStringToObject(result_json, "fail_reason", "Test case not found at specified index");
        cJSON_Delete(json);
        free(json_data);
        return -1;
    }

    // Parse params để lấy host
    cJSON *params_json = cJSON_GetObjectItem(test_case_json, "params");
    if (!params_json) {
        log_message(LOG_LVL_ERROR, "No params specified for test case %d", index);
        cJSON_AddStringToObject(result_json, "fail_reason", "Missing host parameter");
        cJSON_Delete(json);
        free(json_data);
        return -1;
    }

    cJSON *host_json = cJSON_GetObjectItem(params_json, "host");
    if (host_json && cJSON_IsString(host_json)) {
        strncpy(host, host_json->valuestring, host_size - 1);
        host[host_size - 1] = '\0';
        log_message(LOG_LVL_DEBUG, "Parsed host: %s", host);
    } else {
        log_message(LOG_LVL_ERROR, "No host specified in params for test case %d", index);
        cJSON_AddStringToObject(result_json, "fail_reason", "Missing host parameter");
        cJSON_Delete(json);
        free(json_data);
        return -1;
    }

    cJSON_ReplaceItemInObject(result_json, "host", cJSON_CreateString(host));
    cJSON_Delete(json);
    free(json_data);
    return 0;
}

int ping(const char *host, PingResult *result) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "ping -c 5 -W 3 %s 2>&1", host);

    char buf[1024];
    FILE *fp = popen(cmd, "r");
    if (fp == NULL) {
        log_message(LOG_LVL_ERROR, "Failed to execute ping command for host: %s", host);
        strcpy(result->status, "fail");
        result->packet_loss = 100.0; // 100% mất gói
        return -1;
    }

    result->packets_received = 0;
    result->packets_transmitted = 0;
    result->avg_time = 0.0;
    while (fgets(buf, sizeof(buf), fp)) {
        if (strstr(buf, "packets transmitted")) {
            sscanf(buf, "%d packets transmitted, %d", &result->packets_transmitted, &result->packets_received);
        }
        if (strstr(buf, "rtt min/avg/max/mdev")) {
            sscanf(buf, "%*[^=]= %*f/%f", &result->avg_time);
        }
    }

    int status = pclose(fp);
    if (WEXITSTATUS(status) != 0) {
        log_message(LOG_LVL_ERROR, "Ping command failed with exit code %d", WEXITSTATUS(status));
        strcpy(result->status, "fail");
        result->packet_loss = 100.0; // 100% mất gói
        return -1;
    }

    // Tính tỷ lệ mất gói
    if (result->packets_transmitted > 0) {
        result->packet_loss = ((float)(result->packets_transmitted - result->packets_received) / result->packets_transmitted) * 100.0;
    } else {
        result->packet_loss = 100.0; // Nếu không gửi được gói nào, coi như 100% mất
    }

    // Xác định trạng thái pass/partial/fail
    if (result->packets_received == result->packets_transmitted) {
        strcpy(result->status, "pass"); // 100% gói tin được nhận
    } else if (result->packets_received > 0) {
        strcpy(result->status, "partial"); // Có gói tin được nhận, nhưng không phải tất cả
    } else {
        strcpy(result->status, "fail"); // Không có gói tin nào được nhận
    }

    return 0;
}

void log_result(const PingResult *result, time_t start_time, time_t end_time) {
    log_message(LOG_LVL_DEBUG, "Ping test result: Packets received=%d/%d, Packet loss=%.2f%%, Avg time=%.3f ms, Status=%s",
                result->packets_received, result->packets_transmitted, result->packet_loss, result->avg_time, result->status);
    log_message(LOG_LVL_DEBUG, "Ping test completed in %ld seconds", end_time - start_time);
}

void execute_ping(TestCase *test_case, const char *filepath, int index, cJSON *result_array) {
    log_message(LOG_LVL_DEBUG, "Executing ping test");
    time_t start_time = time(NULL);

    // Tạo result_json ngay từ đầu
    cJSON *result_json = cJSON_CreateObject();
    cJSON_AddStringToObject(result_json, "service", test_case->service);
    cJSON_AddStringToObject(result_json, "action", test_case->action[0] ? test_case->action : "default");
    cJSON_AddStringToObject(result_json, "host", "");
    cJSON_AddStringToObject(result_json, "status", "fail");

    // Parse host từ file JSON
    char host[256] = "";
    if (parse_host(filepath, index, host, sizeof(host), result_json) != 0) {
        cJSON_AddItemToArray(result_array, result_json);
        return;
    }

    // Chạy lệnh ping
    PingResult result;
    log_message(LOG_LVL_DEBUG, "Pinging host: %s", host);
    if (ping(host, &result) != 0) {
        cJSON_AddStringToObject(result_json, "fail_reason", "Failed to execute ping command");
        cJSON_AddItemToArray(result_array, result_json);
        return;
    }

    // Cập nhật kết quả vào result_json
    cJSON_ReplaceItemInObject(result_json, "status", cJSON_CreateString(result.status));
    if (strcmp(result.status, "pass") != 0) {
        // Nếu không phải pass, giữ lại fail_reason nếu có hoặc thêm lý do mặc định
        if (!cJSON_GetObjectItem(result_json, "fail_reason")) {
            if (strcmp(result.status, "partial") == 0) {
                cJSON_AddStringToObject(result_json, "fail_reason", "Partial packet loss");
            } else {
                cJSON_AddStringToObject(result_json, "fail_reason", "No packets received");
            }
        }
        // Chỉ thêm vào result_array nếu test case fail
        cJSON_AddItemToArray(result_array, result_json);
    } else {
        // Nếu pass, không thêm vào result_array
        cJSON_Delete(result_json);
    }

    // Ghi log kết quả
    time_t end_time = time(NULL);
    log_result(&result, start_time, end_time);
}