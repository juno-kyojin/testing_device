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

// Parse host từ file JSON
int parse_host(const char *filepath, int index, char *host, size_t host_size, cJSON *result_json) {
    // Đọc file JSON
    char *json_data;
    size_t json_size;
    if (read_file(filepath, &json_data, &json_size) != 0) {
        log_message(LOG_LVL_ERROR, "Failed to read JSON file: %s", filepath);
        return -1;
    }

    // Parse JSON
    cJSON *json = cJSON_Parse(json_data);
    if (!json) {
        log_message(LOG_LVL_ERROR, "Failed to parse JSON file: %s", filepath);
        free(json_data);
        return -1;
    }

    // Lấy mảng test_cases
    cJSON *test_cases_json = cJSON_GetObjectItem(json, "test_cases");
    if (!cJSON_IsArray(test_cases_json)) {
        log_message(LOG_LVL_ERROR, "No 'test_cases' array found in %s", filepath);
        cJSON_Delete(json);
        free(json_data);
        return -1;
    }

    // Tìm test case theo chỉ số
    cJSON *test_case_json = cJSON_GetArrayItem(test_cases_json, index);
    if (!test_case_json) {
        log_message(LOG_LVL_ERROR, "Test case at index %d not found in %s", index, filepath);
        cJSON_Delete(json);
        free(json_data);
        return -1;
    }

    // Parse params để lấy host
    cJSON *params_json = cJSON_GetObjectItem(test_case_json, "params");
    if (!params_json) {
        log_message(LOG_LVL_ERROR, "No params specified for test case %d", index);
        cJSON_Delete(json);
        free(json_data);
        return -1; // Không thêm duplicate keys, để execute_ping xử lý
    }

    cJSON *host_json = cJSON_GetObjectItem(params_json, "host");
    if (host_json && cJSON_IsString(host_json)) {
        strncpy(host, host_json->valuestring, host_size - 1);
        host[host_size - 1] = '\0';
        log_message(LOG_LVL_DEBUG, "Parsed host: %s", host);
    } else {
        log_message(LOG_LVL_ERROR, "No host specified in params for test case %d", index);
        cJSON_Delete(json);
        free(json_data);
        return -1; // Không thêm duplicate keys, để execute_ping xử lý
    }

    cJSON_ReplaceItemInObject(result_json, "host", cJSON_CreateString(host));
    cJSON_Delete(json);
    free(json_data);
    return 0;
}

// Chạy lệnh ping và thu thập kết quả
int ping(const char *host, PingResult *result) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "ping -c 5 -W 3 %s 2>&1", host);

    char buf[1024];
    FILE *fp = popen(cmd, "r");
    if (fp == NULL) {
        log_message(LOG_LVL_ERROR, "Failed to execute ping command for host: %s", host);
        strcpy(result->status, "fail");
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
        return -1;
    }

    // Xác định trạng thái pass/fail
    if (result->packets_received > 0) {
        strcpy(result->status, "pass");
    } else {
        strcpy(result->status, "fail");
    }

    return 0;
}

// Ghi log kết quả
void log_result(const PingResult *result, time_t start_time, time_t end_time) {
    log_message(LOG_LVL_DEBUG, "Ping test result: Packets received=%d, Avg time=%.3f ms, Status=%s",
                result->packets_received, result->avg_time, result->status);
    log_message(LOG_LVL_DEBUG, "Ping test completed in %ld seconds", end_time - start_time);
}

void execute_ping(TestCase *test_case, const char *filepath, int index, cJSON *result_array) {
    log_message(LOG_LVL_DEBUG, "Executing ping test");
    time_t start_time = time(NULL);

    // Tạo result_json ngay từ đầu
    cJSON *result_json = cJSON_CreateObject();
    cJSON_AddStringToObject(result_json, "host", "");
    cJSON_AddStringToObject(result_json, "status", "fail");
    cJSON_AddNumberToObject(result_json, "packets_received", 0);
    cJSON_AddNumberToObject(result_json, "packets_transmitted", 0);
    cJSON_AddNumberToObject(result_json, "avg_time_ms", 0.0);

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
        cJSON_AddItemToArray(result_array, result_json);
        return;
    }

    // Cập nhật kết quả vào result_json
    cJSON_ReplaceItemInObject(result_json, "status", cJSON_CreateString(result.status));
    cJSON_ReplaceItemInObject(result_json, "packets_received", cJSON_CreateNumber(result.packets_received));
    cJSON_ReplaceItemInObject(result_json, "packets_transmitted", cJSON_CreateNumber(result.packets_transmitted));
    cJSON_ReplaceItemInObject(result_json, "avg_time_ms", cJSON_CreateNumber(result.avg_time));

    // Thêm kết quả vào mảng
    cJSON_AddItemToArray(result_array, result_json);

    // Ghi log kết quả
    time_t end_time = time(NULL);
    log_result(&result, start_time, end_time);
}