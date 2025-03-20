
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include "parser_data.h"
 #include "file_process.h" 
 #include "log.h"          
 #include "cjson/cJSON.h"       
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
            current_test->id[sizeof(current_test->id) - 1] = '\0'; // Đảm bảo null-terminated
            log_message(LOG_LVL_DEBUG, "Processing test case ID: %s", current_test->id);
        } else {
            log_message(LOG_LVL_WARN, "Test case at index %d has no valid ID", i);
            snprintf(current_test->id, sizeof(current_test->id), "TC%03d", i+1); // ID mặc định
            log_message(LOG_LVL_DEBUG, "Assigned default ID: %s", current_test->id);
        }
        
        // Xử lý name
        cJSON *name = cJSON_GetObjectItem(test_case_json, "name");
        if (name && cJSON_IsString(name)) {
            strncpy(current_test->name, name->valuestring, sizeof(current_test->name) - 1);
            current_test->name[sizeof(current_test->name) - 1] = '\0'; // Đảm bảo null-terminated
            log_message(LOG_LVL_DEBUG, "Test case %s name: %s", current_test->id, current_test->name);
        } else {
            log_message(LOG_LVL_WARN, "Test case %s has no valid name", current_test->id);
            snprintf(current_test->name, sizeof(current_test->name), "Unnamed Test %s", current_test->id);
        }
        
        // Xử lý description
        cJSON *description = cJSON_GetObjectItem(test_case_json, "description");
        if (description && cJSON_IsString(description)) {
            strncpy(current_test->description, description->valuestring, sizeof(current_test->description) - 1);
            current_test->description[sizeof(current_test->description) - 1] = '\0';
            log_message(LOG_LVL_DEBUG, "Test case %s description processed", current_test->id);
        } else {
            log_message(LOG_LVL_WARN, "Test case %s has no valid description", current_test->id);
            current_test->description[0] = '\0'; // Mô tả rỗng
        }
        
        // Xử lý target
        cJSON *target = cJSON_GetObjectItem(test_case_json, "target");
        if (target && cJSON_IsString(target)) {
            strncpy(current_test->target, target->valuestring, sizeof(current_test->target) - 1);
            current_test->target[sizeof(current_test->target) - 1] = '\0';
            log_message(LOG_LVL_DEBUG, "Test case %s target: %s", current_test->id, current_test->target);
        } else {
            log_message(LOG_LVL_WARN, "Test case %s has no valid target", current_test->id);
            current_test->target[0] = '\0'; // Target rỗng
        }
        
        // Xử lý timeout
        cJSON *timeout = cJSON_GetObjectItem(test_case_json, "timeout");
        if (timeout && cJSON_IsNumber(timeout)) {
            current_test->timeout = timeout->valueint;
            log_message(LOG_LVL_DEBUG, "Test case %s timeout: %d ms", current_test->id, current_test->timeout);
        } else {
            current_test->timeout = 10000; // Mặc định 10 giây (10000 ms)
            log_message(LOG_LVL_WARN, "Test case %s has no valid timeout, setting default: 10000 ms", current_test->id);
        }
        
        // Xử lý enabled
        cJSON *enabled = cJSON_GetObjectItem(test_case_json, "enabled");
        if (enabled && cJSON_IsBool(enabled)) {
            current_test->enabled = cJSON_IsTrue(enabled);
            log_message(LOG_LVL_DEBUG, "Test case %s enabled: %s", current_test->id, current_test->enabled ? "true" : "false");
        } else {
            current_test->enabled = true; // Mặc định là enabled
            log_message(LOG_LVL_WARN, "Test case %s has no valid enabled flag, enabling by default", current_test->id);
        }
        
        // Xử lý type (loại test case)
        cJSON *type = cJSON_GetObjectItem(test_case_json, "type");
        if (type && cJSON_IsString(type)) {
            const char *type_str = type->valuestring;
            if (strcmp(type_str, "ping") == 0) {
                current_test->type = TEST_PING;
                log_message(LOG_LVL_DEBUG, "Test case %s type: PING", current_test->id);
                
                // Xử lý các tham số ping nếu có
                cJSON *ping_params = cJSON_GetObjectItem(test_case_json, "ping_params");
                if (ping_params && cJSON_IsObject(ping_params)) {
                    // Đọc count
                    cJSON *count_param = cJSON_GetObjectItem(ping_params, "count");
                    if (count_param && cJSON_IsNumber(count_param)) {
                        current_test->params.ping.count = count_param->valueint;
                    } else {
                        current_test->params.ping.count = 4; // Mặc định
                    }
                    
                    // Đọc size
                    cJSON *size_param = cJSON_GetObjectItem(ping_params, "size");
                    if (size_param && cJSON_IsNumber(size_param)) {
                        current_test->params.ping.size = size_param->valueint;
                    } else {
                        current_test->params.ping.size = 64; // Mặc định
                    }
                    
                    // Đọc interval
                    cJSON *interval_param = cJSON_GetObjectItem(ping_params, "interval");
                    if (interval_param && cJSON_IsNumber(interval_param)) {
                        current_test->params.ping.interval = interval_param->valueint;
                    } else {
                        current_test->params.ping.interval = 1000; // Mặc định 1 giây
                    }
                    
                    // Đọc ipv6
                    cJSON *ipv6_param = cJSON_GetObjectItem(ping_params, "ipv6");
                    if (ipv6_param && cJSON_IsBool(ipv6_param)) {
                        current_test->params.ping.ipv6 = cJSON_IsTrue(ipv6_param);
                    } else {
                        current_test->params.ping.ipv6 = false; // Mặc định IPv4
                    }
                    
                    log_message(LOG_LVL_DEBUG, "Test case %s ping params processed", current_test->id);
                } else {
                    log_message(LOG_LVL_WARN, "Test case %s missing ping parameters, using defaults", current_test->id);
                    // Sử dụng giá trị mặc định
                    current_test->params.ping.count = 4;
                    current_test->params.ping.size = 64;
                    current_test->params.ping.interval = 1000;
                    current_test->params.ping.ipv6 = false;
                }
            }
            else if (strcmp(type_str, "throughput") == 0) {
                current_test->type = TEST_THROUGHPUT;
                log_message(LOG_LVL_DEBUG, "Test case %s type: THROUGHPUT", current_test->id);
                
                // Xử lý các tham số throughput nếu có
                cJSON *throughput_params = cJSON_GetObjectItem(test_case_json, "throughput_params");
                if (throughput_params && cJSON_IsObject(throughput_params)) {
                    // Đọc duration
                    cJSON *duration_param = cJSON_GetObjectItem(throughput_params, "duration");
                    if (duration_param && cJSON_IsNumber(duration_param)) {
                        current_test->params.throughput.duration = duration_param->valueint;
                    } else {
                        current_test->params.throughput.duration = 10; // Mặc định 10 giây
                    }
                    
                    // Đọc protocol
                    cJSON *protocol_param = cJSON_GetObjectItem(throughput_params, "protocol");
                    if (protocol_param && cJSON_IsString(protocol_param)) {
                        strncpy(current_test->params.throughput.protocol, protocol_param->valuestring, 
                                sizeof(current_test->params.throughput.protocol) - 1);
                        current_test->params.throughput.protocol[sizeof(current_test->params.throughput.protocol) - 1] = '\0';
                    } else {
                        strcpy(current_test->params.throughput.protocol, "TCP"); // Mặc định TCP
                    }
                    
                    // Đọc port
                    cJSON *port_param = cJSON_GetObjectItem(throughput_params, "port");
                    if (port_param && cJSON_IsNumber(port_param)) {
                        current_test->params.throughput.port = port_param->valueint;
                    } else {
                        current_test->params.throughput.port = 5201; // Mặc định cổng iperf3
                    }
                    
                    // Đọc buffer_size nếu có
                    cJSON *buffer_param = cJSON_GetObjectItem(throughput_params, "buffer_size");
                    if (buffer_param && cJSON_IsNumber(buffer_param)) {
                        current_test->params.throughput.buffer_size = buffer_param->valueint;
                    } else {
                        current_test->params.throughput.buffer_size = 8192; // Mặc định 8KB
                    }
                    
                    // Đọc bidirectional nếu có
                    cJSON *bidir_param = cJSON_GetObjectItem(throughput_params, "bidirectional");
                    if (bidir_param && cJSON_IsBool(bidir_param)) {
                        current_test->params.throughput.bidirectional = cJSON_IsTrue(bidir_param);
                    } else {
                        current_test->params.throughput.bidirectional = false; // Mặc định một chiều
                    }
                    
                    log_message(LOG_LVL_DEBUG, "Test case %s throughput params processed", current_test->id);
                } else {
                    log_message(LOG_LVL_WARN, "Test case %s missing throughput parameters, using defaults", current_test->id);
                    // Sử dụng giá trị mặc định
                    current_test->params.throughput.duration = 10;
                    strcpy(current_test->params.throughput.protocol, "TCP");
                    current_test->params.throughput.port = 5201;
                    current_test->params.throughput.buffer_size = 8192;
                    current_test->params.throughput.bidirectional = false;
                }
            }
            else if (strcmp(type_str, "security") == 0) {
                current_test->type = TEST_SECURITY;
                log_message(LOG_LVL_DEBUG, "Test case %s type: SECURITY", current_test->id);
                
                // Xử lý các tham số security nếu có
                cJSON *security_params = cJSON_GetObjectItem(test_case_json, "security_params");
                if (security_params && cJSON_IsObject(security_params)) {
                    // Đọc method
                    cJSON *method_param = cJSON_GetObjectItem(security_params, "method");
                    if (method_param && cJSON_IsString(method_param)) {
                        strncpy(current_test->params.security.method, method_param->valuestring, 
                                sizeof(current_test->params.security.method) - 1);
                        current_test->params.security.method[sizeof(current_test->params.security.method) - 1] = '\0';
                    } else {
                        strcpy(current_test->params.security.method, "tls_scan"); // Mặc định
                    }
                    
                    // Đọc port
                    cJSON *port_param = cJSON_GetObjectItem(security_params, "port");
                    if (port_param && cJSON_IsNumber(port_param)) {
                        current_test->params.security.port = port_param->valueint;
                    } else {
                        current_test->params.security.port = 443; // Mặc định HTTPS 
                    }
                    
                    // Đọc tls flag
                    cJSON *tls_param = cJSON_GetObjectItem(security_params, "tls");
                    if (tls_param && cJSON_IsBool(tls_param)) {
                        current_test->params.security.tls = cJSON_IsTrue(tls_param);
                    } else {
                        current_test->params.security.tls = true; // Mặc định sử dụng TLS
                    }
                    
                    log_message(LOG_LVL_DEBUG, "Test case %s security params processed", current_test->id);
                } else {
                    log_message(LOG_LVL_WARN, "Test case %s missing security parameters, using defaults", current_test->id);
                    // Sử dụng giá trị mặc định
                    strcpy(current_test->params.security.method, "tls_scan");
                    current_test->params.security.port = 443;
                    current_test->params.security.tls = true;
                }
            }
            else {
                current_test->type = TEST_OTHER;
                log_message(LOG_LVL_DEBUG, "Test case %s type: OTHER (unrecognized type: %s)", current_test->id, type_str);
            }
        } else {
            current_test->type = TEST_OTHER;
            log_message(LOG_LVL_WARN, "Test case %s has no valid type, setting to OTHER", current_test->id);
        }
        
        // Xử lý network_type (loại mạng)
        cJSON *network = cJSON_GetObjectItem(test_case_json, "network");
        if (network && cJSON_IsString(network)) {
            const char *network_str = network->valuestring;
            if (strcmp(network_str, "LAN") == 0) {
                current_test->network_type = NETWORK_LAN;
                log_message(LOG_LVL_DEBUG, "Test case %s network: LAN", current_test->id);
            } 
            else if (strcmp(network_str, "WAN") == 0) {
                current_test->network_type = NETWORK_WAN;
                log_message(LOG_LVL_DEBUG, "Test case %s network: WAN", current_test->id);
            }
            else if (strcmp(network_str, "BOTH") == 0) {
                current_test->network_type = NETWORK_BOTH;
                log_message(LOG_LVL_DEBUG, "Test case %s network: BOTH", current_test->id);
            }
            else {
                current_test->network_type = NETWORK_LAN; // Mặc định LAN
                log_message(LOG_LVL_WARN, "Test case %s has unrecognized network type: %s, setting to LAN", 
                           current_test->id, network_str);
            }
        } else {
            current_test->network_type = NETWORK_LAN; // Mặc định LAN
            log_message(LOG_LVL_WARN, "Test case %s has no valid network type, setting to LAN", current_test->id);
        }
        
        // Xử lý extra_data
        cJSON *extra_data = cJSON_GetObjectItem(test_case_json, "extra_data");
        if (extra_data) {
            // Nếu có trường extra_data, lưu dưới dạng chuỗi JSON
            char *extra_json = cJSON_PrintUnformatted(extra_data);
            if (extra_json) {
                current_test->extra_data = strdup(extra_json);
                log_message(LOG_LVL_DEBUG, "Test case %s extra_data processed", current_test->id);
                free(extra_json);
            } else {
                current_test->extra_data = NULL;
                log_message(LOG_LVL_WARN, "Failed to process extra_data for test case %s", current_test->id);
            }
        } else {
            current_test->extra_data = NULL;
        }
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