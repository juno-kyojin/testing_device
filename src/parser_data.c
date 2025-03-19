/**
 * @file parser_data.c
 * @brief Triển khai các hàm xử lý file JSON test case
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include "parser_data.h"
 #include "file_process.h"  // Sử dụng cho việc đọc/ghi file
 #include "log.h"           // Sử dụng cho việc ghi log
 #include "cjson/cJSON.h"         // Thư viện xử lý JSON
 
 bool parse_json_content(const char *json_content, test_case_t **test_cases, int *count) {
     if (!json_content || !test_cases || !count) {
         log_message(LOG_LVL_ERROR, "Invalid parameters for parse_json_content");
         return false;
     }
     
     // Log bắt đầu parse JSON
     log_message(LOG_LVL_DEBUG, "Starting JSON parsing");
     
     cJSON *root = cJSON_Parse(json_content);
     if (!root) {
         log_message(LOG_LVL_ERROR, "Failed to parse JSON: %s", cJSON_GetErrorPtr());
         return false;
     }
     
     // Xử lý JSON như triển khai trước đó
     cJSON *test_cases_array = cJSON_GetObjectItem(root, "test_cases");
     if (!test_cases_array || !cJSON_IsArray(test_cases_array)) {
         log_message(LOG_LVL_ERROR, "JSON doesn't contain 'test_cases' array");
         cJSON_Delete(root);
         return false;
     }
     
     *count = cJSON_GetArraySize(test_cases_array);
     if (*count <= 0) {
         log_message(LOG_LVL_ERROR, "No test cases found in JSON");
         cJSON_Delete(root);
         return false;
     }
     
     log_message(LOG_LVL_DEBUG, "Found %d test cases in JSON", *count);
     
     // Cấp phát bộ nhớ cho mảng test cases
     *test_cases = (test_case_t *)malloc(*count * sizeof(test_case_t));
     if (!(*test_cases)) {
         log_message(LOG_LVL_ERROR, "Memory allocation failed for test cases");
         cJSON_Delete(root);
         return false;
     }
     memset(*test_cases, 0, *count * sizeof(test_case_t));
     
     // Xử lý từng test case
     for (int i = 0; i < *count; i++) {
         cJSON *test_case_json = cJSON_GetArrayItem(test_cases_array, i);
         test_case_t *current_test = &((*test_cases)[i]);
         
         // Xử lý ID
         cJSON *id = cJSON_GetObjectItem(test_case_json, "id");
         if (id && cJSON_IsString(id)) {
             strncpy(current_test->id, id->valuestring, sizeof(current_test->id) - 1);
             log_message(LOG_LVL_DEBUG, "Processing test case ID: %s", current_test->id);
         } else {
             log_message(LOG_LVL_WARN, "Test case at index %d has no valid ID", i);
         }
         
         // Xử lý name và các thuộc tính khác tương tự như triển khai trước đó
         // ...
         
         // Ví dụ:
         cJSON *name = cJSON_GetObjectItem(test_case_json, "name");
         if (name && cJSON_IsString(name)) {
             strncpy(current_test->name, name->valuestring, sizeof(current_test->name) - 1);
         }
         
         // Tiếp tục xử lý các trường khác...
     }
     
     cJSON_Delete(root);
     log_message(LOG_LVL_DEBUG, "Completed parsing JSON test cases");
     return true;
 }
 
 bool read_json_test_cases(const char *json_file, test_case_t **test_cases, int *count) {
     if (!json_file || !test_cases || !count) {
         log_message(LOG_LVL_ERROR, "Invalid parameters for read_json_test_cases");
         return false;
     }
     
     log_message(LOG_LVL_DEBUG, "Reading JSON file: %s", json_file);
     
     // Kiểm tra file tồn tại
     if (!file_exists(json_file)) {
         log_message(LOG_LVL_ERROR, "JSON file does not exist: %s", json_file);
         return false;
     }
     
     // Đọc nội dung file JSON sử dụng file_process.c
     char *json_content = NULL;
     size_t content_size = 0;
     
     if (read_file(json_file, &json_content, &content_size) != 0) {
         log_message(LOG_LVL_ERROR, "Failed to read JSON file: %s", json_file);
         return false;
     }
     
     log_message(LOG_LVL_DEBUG, "Successfully read %lu bytes from JSON file", 
                (unsigned long)content_size);
     
     // Parse nội dung JSON
     bool result = parse_json_content(json_content, test_cases, count);
     
     // Giải phóng bộ nhớ
     free(json_content);
     
     if (result) {
         log_message(LOG_LVL_DEBUG, "Successfully parsed %d test cases from %s", 
                    *count, json_file);
     }
     
     return result;
 }
 
 // Triển khai các hàm khác từ parser_data.h...
 
 bool test_cases_to_json(const test_case_t *test_cases, int count, char *json_buffer, size_t buffer_size) {
     if (!test_cases || count <= 0 || !json_buffer || buffer_size <= 0) {
         log_message(LOG_LVL_ERROR, "Invalid parameters for test_cases_to_json");
         return false;
     }
     
     log_message(LOG_LVL_DEBUG, "Converting %d test cases to JSON", count);
     
     cJSON *root = cJSON_CreateObject();
     if (!root) {
         log_message(LOG_LVL_ERROR, "Failed to create JSON object");
         return false;
     }
     
     cJSON *test_cases_array = cJSON_CreateArray();
     if (!test_cases_array) {
         log_message(LOG_LVL_ERROR, "Failed to create JSON array");
         cJSON_Delete(root);
         return false;
     }
     
     // Thêm mảng test_cases vào root
     cJSON_AddItemToObject(root, "test_cases", test_cases_array);
     
     // Chuyển đổi từng test case thành JSON
     for (int i = 0; i < count; i++) {
         const test_case_t *tc = &test_cases[i];
         cJSON *test_case_json = cJSON_CreateObject();
         
         if (!test_case_json) {
             log_message(LOG_LVL_ERROR, "Failed to create JSON object for test case %d", i);
             cJSON_Delete(root);
             return false;
         }
         
         // Thêm các trường của test case
         cJSON_AddStringToObject(test_case_json, "id", tc->id);
         cJSON_AddStringToObject(test_case_json, "name", tc->name);
         cJSON_AddStringToObject(test_case_json, "description", tc->description);
         cJSON_AddStringToObject(test_case_json, "target", tc->target);
         cJSON_AddNumberToObject(test_case_json, "timeout", tc->timeout);
         cJSON_AddBoolToObject(test_case_json, "enabled", tc->enabled);
         
         // Thêm loại test
         switch (tc->type) {
             case TEST_PING:
                 cJSON_AddStringToObject(test_case_json, "type", "ping");
                 
                 // Thêm tham số ping
                 cJSON *ping_params = cJSON_CreateObject();
                 cJSON_AddNumberToObject(ping_params, "count", tc->params.ping.count);
                 cJSON_AddNumberToObject(ping_params, "size", tc->params.ping.size);
                 cJSON_AddNumberToObject(ping_params, "interval", tc->params.ping.interval);
                 cJSON_AddBoolToObject(ping_params, "ipv6", tc->params.ping.ipv6);
                 cJSON_AddItemToObject(test_case_json, "ping_params", ping_params);
                 break;
                 
             case TEST_THROUGHPUT:
                 cJSON_AddStringToObject(test_case_json, "type", "throughput");
                 
                 // Thêm tham số throughput
                 cJSON *throughput_params = cJSON_CreateObject();
                 cJSON_AddNumberToObject(throughput_params, "duration", tc->params.throughput.duration);
                 cJSON_AddStringToObject(throughput_params, "protocol", tc->params.throughput.protocol);
                 cJSON_AddNumberToObject(throughput_params, "port", tc->params.throughput.port);
                 cJSON_AddItemToObject(test_case_json, "throughput_params", throughput_params);
                 break;
                 
             case TEST_SECURITY:
                 cJSON_AddStringToObject(test_case_json, "type", "security");
                 
                 // Thêm tham số security
                 cJSON *security_params = cJSON_CreateObject();
                 cJSON_AddStringToObject(security_params, "method", tc->params.security.method);
                 cJSON_AddNumberToObject(security_params, "port", tc->params.security.port);
                 cJSON_AddItemToObject(test_case_json, "security_params", security_params);
                 break;
                 
             default:
                 cJSON_AddStringToObject(test_case_json, "type", "other");
                 break;
         }
         
         // Thêm loại mạng
         switch (tc->network_type) {
             case NETWORK_LAN:
                 cJSON_AddStringToObject(test_case_json, "network", "LAN");
                 break;
             case NETWORK_WAN:
                 cJSON_AddStringToObject(test_case_json, "network", "WAN");
                 break;
             case NETWORK_BOTH:
                 cJSON_AddStringToObject(test_case_json, "network", "BOTH");
                 break;
         }
         
         // Thêm test_case_json vào mảng
         cJSON_AddItemToArray(test_cases_array, test_case_json);
     }
     
     // Convert JSON structure to string
     char *json_str = cJSON_PrintUnformatted(root);
     if (!json_str) {
         log_message(LOG_LVL_ERROR, "Failed to convert JSON to string");
         cJSON_Delete(root);
         return false;
     }
     
     // Kiểm tra kích thước
     if (strlen(json_str) >= buffer_size) {
         log_message(LOG_LVL_ERROR, "JSON string too large for buffer");
         free(json_str);
         cJSON_Delete(root);
         return false;
     }
     
     // Copy vào buffer
     strcpy(json_buffer, json_str);
     
     // Clean up
     free(json_str);
     cJSON_Delete(root);
     
     log_message(LOG_LVL_DEBUG, "Successfully converted test cases to JSON");
     return true;
 }
 
 void free_test_cases(test_case_t *test_cases, int count) {
     if (!test_cases) {
         return;
     }
     
     log_message(LOG_LVL_DEBUG, "Freeing memory for %d test cases", count);
     
     for (int i = 0; i < count; i++) {
         if (test_cases[i].extra_data) {
             free(test_cases[i].extra_data);
             test_cases[i].extra_data = NULL;
         }
     }
     
     free(test_cases);
 }  