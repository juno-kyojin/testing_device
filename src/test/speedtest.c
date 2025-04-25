// #include <stdio.h>
// #include <string.h>
// #include <stdlib.h>
// #include <unistd.h>
// #include <sys/types.h>
// #include <sys/wait.h>
// #include <time.h>
// #include "speedtest.h"
// #include "log.h"
// #include "file_process.h"
// #include "cjson/cJSON.h"

// void execute_speedtest(TestCase *test_case, const char *filepath, int index) {
//     log_message(LOG_LVL_DEBUG, "Executing speedtest test");
//     time_t start_time = time(NULL);

//     // Đọc file JSON
//     char *json_data;
//     size_t json_size;
//     if (read_file(filepath, &json_data, &json_size) != 0) {
//         log_message(LOG_LVL_ERROR, "Failed to read JSON file: %s", filepath);
//         return;
//     }

//     // Parse JSON
//     cJSON *json = cJSON_Parse(json_data);
//     if (!json) {
//         log_message(LOG_LVL_ERROR, "Failed to parse JSON file: %s", filepath);
//         free(json_data);
//         return;
//     }

//     // Lấy mảng test_cases
//     cJSON *test_cases_json = cJSON_GetObjectItem(json, "test_cases");
//     if (!cJSON_IsArray(test_cases_json)) {
//         log_message(LOG_LVL_ERROR, "No 'test_cases' array found in %s", filepath);
//         cJSON_Delete(json);
//         free(json_data);
//         return;
//     }

//     // Tìm test case theo chỉ số
//     cJSON *test_case_json = cJSON_GetArrayItem(test_cases_json, index);
//     if (!test_case_json) {
//         log_message(LOG_LVL_ERROR, "Test case at index %d not found in %s", index, filepath);
//         cJSON_Delete(json);
//         free(json_data);
//         return;
//     }

//     // Parse params để lấy server_id và location
//     int server_id = -1;
//     char location[256] = "";
//     cJSON *params_json = cJSON_GetObjectItem(test_case_json, "params");
//     if (params_json) {
//         cJSON *server_id_json = cJSON_GetObjectItem(params_json, "server_id");
//         cJSON *location_json = cJSON_GetObjectItem(params_json, "location");

//         if (server_id_json && cJSON_IsNumber(server_id_json)) {
//             server_id = server_id_json->valueint;
//             log_message(LOG_LVL_DEBUG, "Parsed server_id: %d", server_id);
//         } else {
//             log_message(LOG_LVL_ERROR, "No server_id specified in params for test case %d", index);
//             cJSON_Delete(json);
//             free(json_data);
//             return;
//         }

//         if (location_json && cJSON_IsString(location_json)) {
//             strncpy(location, location_json->valuestring, sizeof(location) - 1);
//             location[sizeof(location) - 1] = '\0';
//             log_message(LOG_LVL_DEBUG, "Parsed location: %s", location);
//         } else {
//             log_message(LOG_LVL_ERROR, "No location specified in params for test case %d", index);
//             cJSON_Delete(json);
//             free(json_data);
//             return;
//         }
//     } else {
//         log_message(LOG_LVL_ERROR, "No params specified for test case %d", index);
//         cJSON_Delete(json);
//         free(json_data);
//         return;
//     }

//     // Chạy speedtest với ID server
//     log_message(LOG_LVL_DEBUG, "Running speedtest on server ID: %d (%s)", server_id, location);

//     char cmd[512];
//     snprintf(cmd, sizeof(cmd), "speedtest-cli --server %d --json 2>&1", server_id);

//     char buf[4096] = "";
//     FILE *fp = popen(cmd, "r");
//     if (fp == NULL) {
//         log_message(LOG_LVL_ERROR, "Failed to execute speedtest command for server ID: %d", server_id);
//         cJSON_Delete(json);
//         free(json_data);
//         return;
//     }

//     // Đọc toàn bộ output từ speedtest-cli
//     size_t total_read = 0;
//     while (fgets(buf + total_read, sizeof(buf) - total_read, fp)) {
//         total_read = strlen(buf);
//         if (total_read >= sizeof(buf) - 1) {
//             log_message(LOG_LVL_ERROR, "Speedtest output too large for buffer");
//             pclose(fp);
//             cJSON_Delete(json);
//             free(json_data);
//             return;
//         }
//     }

//     int status = pclose(fp);
//     int server_failed = 0;
//     if (WEXITSTATUS(status) != 0) {
//         if (strstr(buf, "No matched servers")) {
//             log_message(LOG_LVL_WARN, "Specified server ID %d is invalid: %s", server_id, buf);
//             server_failed = 1;
//         } else {
//             log_message(LOG_LVL_ERROR, "Speedtest command failed with exit code %d: %s", WEXITSTATUS(status), buf);
//             cJSON_Delete(json);
//             free(json_data);
//             return;
//         }
//     }

//     // Nếu server không hợp lệ, chạy lại mà không chỉ định server
//     if (server_failed) {
//         log_message(LOG_LVL_DEBUG, "Falling back to automatic server selection");
//         snprintf(cmd, sizeof(cmd), "speedtest-cli --json 2>&1");
//         fp = popen(cmd, "r");
//         if (fp == NULL) {
//             log_message(LOG_LVL_ERROR, "Failed to execute speedtest command with automatic server selection");
//             cJSON_Delete(json);
//             free(json_data);
//             return;
//         }

//         total_read = 0;
//         memset(buf, 0, sizeof(buf));
//         while (fgets(buf + total_read, sizeof(buf) - total_read, fp)) {
//             total_read = strlen(buf);
//             if (total_read >= sizeof(buf) - 1) {
//                 log_message(LOG_LVL_ERROR, "Speedtest output too large for buffer");
//                 pclose(fp);
//                 cJSON_Delete(json);
//                 free(json_data);
//                 return;
//             }
//         }

//         status = pclose(fp);
//         if (WEXITSTATUS(status) != 0) {
//             log_message(LOG_LVL_ERROR, "Speedtest command with automatic server selection failed with exit code %d: %s", WEXITSTATUS(status), buf);
//             cJSON_Delete(json);
//             free(json_data);
//             return;
//         }

//         // Cập nhật location thành "Automatic Server"
//         strcpy(location, "Automatic Server");
//     }

//     double download_speed = 0.0, upload_speed = 0.0;
//     float latency = 0.0;
//     int speedtest_code = 0;

//     // Parse JSON output từ speedtest-cli
//     cJSON *result_json = cJSON_Parse(buf);
//     if (!result_json) {
//         log_message(LOG_LVL_ERROR, "Failed to parse speedtest output: %s", buf);
//         cJSON_Delete(json);
//         free(json_data);
//         return;
//     }

//     cJSON *download = cJSON_GetObjectItem(result_json, "download");
//     cJSON *upload = cJSON_GetObjectItem(result_json, "upload");
//     cJSON *ping = cJSON_GetObjectItem(result_json, "ping");
//     if (download && upload && ping && cJSON_IsNumber(download) && cJSON_IsNumber(upload) && cJSON_IsNumber(ping)) {
//         download_speed = download->valuedouble / 1e6; // Chuyển sang Mbps
//         upload_speed = upload->valuedouble / 1e6;     // Chuyển sang Mbps
//         latency = (float)ping->valuedouble;           // Latency in ms
//         speedtest_code = 1;                           // Thành công
//     } else {
//         log_message(LOG_LVL_ERROR, "Invalid speedtest output: %s", buf);
//         cJSON_Delete(result_json);
//         cJSON_Delete(json);
//         free(json_data);
//         return;
//     }

//     cJSON_Delete(result_json);

//     log_message(LOG_LVL_DEBUG, "Speedtest result: Location=%s, Code=%d, Download=%.2f Mbps, Upload=%.2f Mbps, Latency=%.2f ms",
//                 location, speedtest_code, download_speed, upload_speed, latency);

//     time_t end_time = time(NULL);
//     log_message(LOG_LVL_DEBUG, "Speedtest completed in %ld seconds", end_time - start_time);

//     cJSON_Delete(json);
//     free(json_data);
// }