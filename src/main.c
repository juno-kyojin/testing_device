<<<<<<< Updated upstream
=======
// src/main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser_data.h"
#include "tc.h"
#include "cjson/cJSON.h"
#include "log.h"
#include "file_process.h"

#define CONFIG_PATH "config/config.json"

int main() {
    instruction_t *instructions = NULL;
    int count = 0;

    log_message(LOG_LVL_DEBUG, "Starting test program");

    // Đọc và parse file JSON từ config
    char *json_content = NULL;
    size_t content_size = 0;
    if (read_file(CONFIG_PATH, &json_content, &content_size) != 0) {
        log_message(LOG_LVL_ERROR, "Failed to read JSON file %s", CONFIG_PATH);
        printf("{\"error\": \"Failed to load config\"}\n");
        return -1;
    }

    // Parse JSON content thành instructions (sử dụng tên hàm mới)
    if (!parse_json_instruction(json_content, &instructions, &count)) {
        log_message(LOG_LVL_ERROR, "Failed to parse JSON content from %s", CONFIG_PATH);
        free(json_content);
        printf("{\"error\": \"Failed to load config\"}\n");
        return -1;
    }

    log_message(LOG_LVL_DEBUG, "Parsed %d instructions from %s", count, CONFIG_PATH);

    // Parse JSON để lấy input_params
    cJSON *root = cJSON_Parse(json_content);
    if (!root) {
        log_message(LOG_LVL_ERROR, "Failed to parse JSON content");
        free(json_content);
        free_instructions(instructions, count);
        return -1;
    }
    cJSON *input_params = cJSON_GetObjectItem(root, "input_params");

    // Thực thi instruction
    for (int i = 0; i < count; i++) {
        if (strcmp(instructions[i].action, "ping") == 0) {
            log_message(LOG_LVL_DEBUG, "Executing ping test");

            // Tạo JSON để lưu kết quả
            cJSON *result = cJSON_CreateObject();
            cJSON *attributes = cJSON_CreateObject();
            cJSON_AddItemToObject(result, "results", attributes);

            // Thực thi test case
            execute_instruction(&instructions[i], input_params);

            // Lấy kết quả từ tcapi_get
            for (int j = 0; j < instructions[i].attr_count; j++) {
                attribute_t *attr = &instructions[i].attributes[j];
                if (strcmp(attr->execute, "yes") == 0) {
                    char buffer[128];
                    if (tcapi_get(instructions[i].node_name, instructions[i].sub_node, attr->private_name, buffer) == 0) {
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

            // In kết quả ra stdout
            char *result_str = cJSON_PrintUnformatted(result);
            printf("%s\n", result_str);
            free(result_str);
            cJSON_Delete(result);
            break; // Chỉ thực thi action "ping" đầu tiên (nếu có nhiều action)
        }
    }

    // Giải phóng bộ nhớ
    cJSON_Delete(root);
    free(json_content);
    free_instructions(instructions, count);

    log_message(LOG_LVL_DEBUG, "Test program completed");
    return 0;
}
>>>>>>> Stashed changes
