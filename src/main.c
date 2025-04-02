// src/main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser_data.h"
#include "tc.h"
#include "cjson/cJSON.h"
#include "log.h"
#include "file_process.h"

// Cho phép chọn file config thông qua biến môi trường hoặc tham số dòng lệnh
#define DEFAULT_CONFIG_PATH "config/config.json"

// Khai báo các hàm trong tc_stubs.c để tránh cảnh báo implicit declaration
int execute_instruction(const instruction_t *instruction, cJSON *input_params);
int tcapi_get(const char *node, const char *entry, const char *attribute, char *value);

// Hàm trợ giúp để xác định node name cho mỗi action
const char* get_node_name_for_action(const char* action) {
    if (strcmp(action, "ping") == 0) return "Ping";
    if (strcmp(action, "throughput") == 0) return "Throughput";
    if (strcmp(action, "security") == 0) return "Security";
    if (strcmp(action, "speedtest") == 0) return "Speedtest";
    return "Unknown";
}

int main(int argc, char *argv[]) {
    instruction_t *instructions = NULL;
    int count = 0;
    
    // Xác định đường dẫn config file
    const char* config_path = DEFAULT_CONFIG_PATH;
    
    // Kiểm tra xem có tham số dòng lệnh chỉ định file config hay không
    if (argc > 1) {
        config_path = argv[1];
    }
    
    // Kiểm tra biến môi trường CONFIG_FILE nếu không có tham số dòng lệnh
    const char* env_config = getenv("CONFIG_FILE");
    if (env_config != NULL && argc <= 1) {
        config_path = env_config;
    }

    log_message(LOG_LVL_DEBUG, "Starting test program with config: %s", config_path);

    // Đọc và parse file JSON từ config
    char *json_content = NULL;
    size_t content_size = 0;
    if (read_file(config_path, &json_content, &content_size) != 0) {
        log_message(LOG_LVL_ERROR, "Failed to read JSON file %s", config_path);
        printf("{\"error\": \"Failed to load config\"}\n");
        return -1;
    }

    // Parse JSON content thành instructions
    if (!parse_json_instruction(json_content, &instructions, &count)) {
        log_message(LOG_LVL_ERROR, "Failed to parse JSON content from %s", config_path);
        free(json_content);
        printf("{\"error\": \"Failed to load config\"}\n");
        return -1;
    }

    log_message(LOG_LVL_DEBUG, "Parsed %d instructions from %s", count, config_path);

    // Parse JSON để kiểm tra execute_all
    cJSON *root = cJSON_Parse(json_content);
    bool execute_all = false;
    if (root) {
        cJSON *execute_all_json = cJSON_GetObjectItem(root, "execute_all");
        if (execute_all_json && cJSON_IsBool(execute_all_json)) {
            execute_all = cJSON_IsTrue(execute_all_json);
        }
    }

    // Kết quả tổng hợp từ tất cả các test cases
    cJSON *all_results = cJSON_CreateObject();
    cJSON *test_results = cJSON_CreateArray();
    cJSON_AddItemToObject(all_results, "test_results", test_results);

    // Thực thi các instruction
    for (int i = 0; i < count; i++) {
        const char* action = instructions[i].action;
        const char* node_name = get_node_name_for_action(action);
        
        log_message(LOG_LVL_DEBUG, "Executing %s test", action);

        // Tạo JSON để lưu kết quả cho test case này
        cJSON *result = cJSON_CreateObject();
        cJSON *attributes = cJSON_CreateObject();
        cJSON_AddStringToObject(result, "action", action);
        cJSON_AddItemToObject(result, "results", attributes);

        // Parse JSON để lấy input_params cho test case này
        cJSON *input_params = NULL;
        if (root) {
            // Nếu là file đa test case
            cJSON *test_cases = cJSON_GetObjectItem(root, "test_cases");
            if (test_cases && cJSON_IsArray(test_cases) && i < cJSON_GetArraySize(test_cases)) {
                cJSON *test_case = cJSON_GetArrayItem(test_cases, i);
                input_params = cJSON_GetObjectItem(test_case, "input_params");
            } else {
                // Nếu là file đơn test case
                input_params = cJSON_GetObjectItem(root, "input_params");
            }
        }

        // Thực thi test case
        execute_instruction(&instructions[i], input_params);

        // Lấy kết quả từ tcapi_get
        for (int j = 0; j < instructions[i].attr_count; j++) {
            attribute_t *attr = &instructions[i].attributes[j];
            if (strcmp(attr->execute, "yes") == 0) {
                char buffer[128];
                if (tcapi_get(node_name, instructions[i].sub_node, attr->private_name, buffer) == 0) {
                    if (strcmp(attr->attr_type, "int") == 0) {
                        cJSON_AddNumberToObject(attributes, attr->public_name, atoi(buffer));
                    } else if (strcmp(attr->attr_type, "float") == 0) {
                        cJSON_AddNumberToObject(attributes, attr->public_name, atof(buffer));
                    } else {
                        cJSON_AddStringToObject(attributes, attr->public_name, buffer);
                    }
                }
            }
        }

        // Thêm kết quả của test case này vào mảng kết quả
        cJSON_AddItemToArray(test_results, result);
        
        // Nếu không thực thi tất cả, chỉ thực thi test case đầu tiên
        if (!execute_all) {
            break;
        }
    }

    // In kết quả tổng hợp ra stdout
    char *result_str = cJSON_PrintUnformatted(all_results);
    printf("%s\n", result_str);
    free(result_str);

    // Giải phóng bộ nhớ
    cJSON_Delete(all_results);
    if (root) cJSON_Delete(root);
    free(json_content);
    free_instructions(instructions, count);

    log_message(LOG_LVL_DEBUG, "Test program completed");
    return 0;
}