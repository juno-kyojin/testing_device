#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include "test/speedtest.h"
#include "core/log.h"
#include "cjson/cJSON.h"

// Hàm tìm ID server từ tên server
static int find_server_id(const char *server_name, char *server_id, size_t server_id_size) {
    char cmd[256];
    char buf[1024];
    FILE *fp;
    
    snprintf(cmd, sizeof(cmd), "speedtest-cli --list 2>&1");
    log_message(LOG_LVL_DEBUG, "Executing command to list servers: %s", cmd);
    
    fp = popen(cmd, "r");
    if (fp == NULL) {
        log_message(LOG_LVL_ERROR, "Failed to execute speedtest list command");
        return -1;
    }
    
    int found = 0;
    while (fgets(buf, sizeof(buf), fp)) {
        if (strstr(buf, server_name)) {
            char *id_start = buf;
            while (*id_start && *id_start != ')') id_start++;
            if (*id_start == ')') {
                *id_start = '\0';
                while (id_start > buf && *id_start != '(') id_start--;
                if (*id_start == '(') id_start++;
                while (*id_start == ' ') id_start++;
                strncpy(server_id, id_start, server_id_size);
                server_id[server_id_size - 1] = '\0';
                found = 1;
                break;
            }
        }
    }
    
    pclose(fp);
    
    if (!found) {
        log_message(LOG_LVL_ERROR, "Server %s not found in speedtest server list", server_name);
        return -1;
    }
    
    log_message(LOG_LVL_DEBUG, "Found server ID: %s for server name: %s", server_id, server_name);
    return 0;
}

// Hàm thực hiện kiểm tra tốc độ mạng và phân tích kết quả
static int perform_speedtest(const char *server_id, float *download_speed, float *upload_speed, float *latency,
                            char *server_name, size_t server_name_size,
                            char *server_location, size_t server_location_size,
                            char *server_country, size_t server_country_size,
                            char *isp, size_t isp_size) {
    char cmd[256];
    char buf[4096];
    FILE *fp;
    int status = -1;
    
    // Tạo lệnh speedtest với định dạng JSON
    if (server_id && strlen(server_id) > 0) {
        snprintf(cmd, sizeof(cmd), "speedtest-cli --json --secure --server %s 2>&1", server_id);
    } else {
        snprintf(cmd, sizeof(cmd), "speedtest-cli --json --secure 2>&1");
    }
    log_message(LOG_LVL_DEBUG, "Executing command: %s", cmd);
    
    fp = popen(cmd, "r");
    if (fp == NULL) {
        log_message(LOG_LVL_ERROR, "Failed to execute speedtest command");
        return -1;
    }
    
    // Đọc và phân tích kết quả
    *download_speed = 0.0;
    *upload_speed = 0.0;
    *latency = 0.0;
    
    size_t bytes_read = 0;
    char *result = buf;
    while (fgets(result, sizeof(buf) - bytes_read, fp)) {
        bytes_read = strlen(buf);
        result = buf + bytes_read;
    }
    
    log_message(LOG_LVL_DEBUG, "Successfully read %lu bytes of JSON result", (unsigned long)bytes_read);
    
    // Parse JSON result
    cJSON *json = cJSON_Parse(buf);
    if (!json) {
        log_message(LOG_LVL_ERROR, "Failed to parse speedtest JSON result: %s", cJSON_GetErrorPtr());
        pclose(fp);
        return -1;
    }
    
    // Trích xuất dữ liệu từ JSON
    cJSON *download = cJSON_GetObjectItem(json, "download");
    cJSON *upload = cJSON_GetObjectItem(json, "upload");
    cJSON *ping = cJSON_GetObjectItem(json, "ping");
    cJSON *server = cJSON_GetObjectItem(json, "server");
    cJSON *server_id_json = cJSON_GetObjectItem(server, "id");
    cJSON *server_name_json = cJSON_GetObjectItem(server, "name");
    cJSON *server_location_json = cJSON_GetObjectItem(server, "location");
    cJSON *server_country_json = cJSON_GetObjectItem(server, "country");
    cJSON *client = cJSON_GetObjectItem(json, "client");
    cJSON *isp_json = cJSON_GetObjectItem(client, "isp");
    
    if (download && cJSON_IsNumber(download)) {
        *download_speed = download->valuedouble / 1000000.0; // Chuyển từ bps sang Mbps
    }
    if (upload && cJSON_IsNumber(upload)) {
        *upload_speed = upload->valuedouble / 1000000.0; // Chuyển từ bps sang Mbps
    }
    if (ping && cJSON_IsNumber(ping)) {
        *latency = ping->valuedouble; // Độ trễ (ms)
    }
    if (server_id_json && cJSON_IsString(server_id_json)) {
        log_message(LOG_LVL_DEBUG, "Server ID for future reference: %s", server_id_json->valuestring);
    }
    if (server_name_json && cJSON_IsString(server_name_json)) {
        strncpy(server_name, server_name_json->valuestring, server_name_size);
        server_name[server_name_size - 1] = '\0';
    }
    if (server_location_json && cJSON_IsString(server_location_json)) {
        strncpy(server_location, server_location_json->valuestring, server_location_size);
        server_location[server_location_size - 1] = '\0';
    }
    if (server_country_json && cJSON_IsString(server_country_json)) {
        strncpy(server_country, server_country_json->valuestring, server_country_size);
        server_country[server_country_size - 1] = '\0';
    }
    if (isp_json && cJSON_IsString(isp_json)) {
        strncpy(isp, isp_json->valuestring, isp_size);
        isp[isp_size - 1] = '\0';
    }
    
    // Xác định trạng thái
    if (*download_speed > 0.0 && *upload_speed > 0.0) {
        status = 0; // Thành công
    } else {
        status = 1; // Thất bại
        *download_speed = 0.0;
        *upload_speed = 0.0;
        *latency = 0.0;
        log_message(LOG_LVL_ERROR, "Speedtest failed: No valid speed data received");
    }
    
    cJSON_Delete(json);
    int exit_code = pclose(fp);
    if (status == 0 && WIFEXITED(exit_code) && WEXITSTATUS(exit_code) != 0) {
        log_message(LOG_LVL_WARN, "Speedtest command exited with non-zero status %d", WEXITSTATUS(exit_code));
    }
    
    return status;
}

void execute_speedtest(Instruction *instr, TestCase *test_case) {
    log_message(LOG_LVL_DEBUG, "Executing speedtest test");
    log_message(LOG_LVL_DEBUG, "Executing instruction: speedtest");
    
    time_t start_time = time(NULL);
    
    // Tìm server để kiểm tra tốc độ
    const char *server_name = NULL;
    for (int i = 0; i < test_case->param_count; i++) {
        log_message(LOG_LVL_DEBUG, "Param %s = %s", 
                    test_case->input_params[i].key, test_case->input_params[i].value);
        if (strcmp(test_case->input_params[i].key, "server") == 0) {
            server_name = test_case->input_params[i].value;
        }
    }
    
    char server_id[32] = "";
    char selected_server_name[256] = "";
    char selected_server_location[256] = "";
    char selected_server_country[256] = "";
    char selected_isp[256] = "";
    
    if (server_name && strlen(server_name) > 0) {
        log_message(LOG_LVL_DEBUG, "Starting speedtest with server target: %s", server_name);
        if (find_server_id(server_name, server_id, sizeof(server_id)) != 0) {
            log_message(LOG_LVL_WARN, "Could not find server ID for %s, falling back to default server", server_name);
            server_id[0] = '\0';
        }
    } else {
        log_message(LOG_LVL_DEBUG, "No server specified, using default server (nearest)");
    }
    
    float download_speed = 0.0;
    float upload_speed = 0.0;
    float latency = 0.0;
    int speedtest_status = perform_speedtest(server_id[0] ? server_id : NULL, 
                                            &download_speed, &upload_speed, &latency,
                                            selected_server_name, sizeof(selected_server_name),
                                            selected_server_location, sizeof(selected_server_location),
                                            selected_server_country, sizeof(selected_server_country),
                                            selected_isp, sizeof(selected_isp));
    
    log_message(LOG_LVL_DEBUG, "Speedtest.Status = %d", speedtest_status);
    log_message(LOG_LVL_DEBUG, "Speedtest completed successfully: Server=%s (%s, %s), ISP=%s, Download=%.2f Mbps, Upload=%.2f Mbps, Latency=%.2f ms",
                selected_server_name, selected_server_location, selected_server_country, selected_isp,
                download_speed, upload_speed, latency);
    
    time_t end_time = time(NULL);
    float execution_time = (end_time - start_time) * 1000.0; 
    log_message(LOG_LVL_DEBUG, "Speedtest execution time: %.1f ms", execution_time);
}