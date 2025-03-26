#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser_data.h"
#include "cjson/cJSON.h"

// Stub function cho tcapi_get 
int tcapi_get(const char *node, const char *entry, const char *attribute, char *value) {
    // Triển khai đơn giản trả về giá trị mặc định cho mỗi thuộc tính
    if (strcmp(attribute, "Status") == 0) {
        strcpy(value, "0");
    } else if (strcmp(attribute, "host") == 0) {
        strcpy(value, "google.com");
    } else if (strcmp(attribute, "hostAddress") == 0) {
        strcpy(value, "8.8.8.8");
    } else if (strcmp(attribute, "successCount") == 0) {
        strcpy(value, "4");
    } else if (strcmp(attribute, "failureCount") == 0) {
        strcpy(value, "0");
    } else if (strcmp(attribute, "averageResponseTime") == 0) {
        strcpy(value, "12.5");
    } else if (strcmp(attribute, "minimumResponseTime") == 0) {
        strcpy(value, "10.2");
    } else if (strcmp(attribute, "maximumResponseTime") == 0) {
        strcpy(value, "15.8");
    } else {
        return -1;
    }
    return 0;
}

// Stub function cho tcapi_set
int tcapi_set(const char *node, const char *entry, const char *attribute, const char *value) {
    // Đơn giản chỉ log và trả về thành công
    printf("Setting %s.%s.%s = %s\n", node, entry, attribute, value);
    return 0;
}

// Stub function cho execute_instruction
int execute_instruction(const instruction_t *instruction, cJSON *input_params) {
    if (!instruction) return -1;
    
    printf("Executing instruction: %s\n", instruction->action);
    
    // Nếu có input_params, đọc host
    if (input_params) {
        cJSON *host = cJSON_GetObjectItem(input_params, "host");
        if (host && cJSON_IsString(host)) {
            // Đặt giá trị host
            tcapi_set(instruction->node_name, instruction->sub_node, "host", host->valuestring);
        }
    }
    
    // Giả định thực thi thành công
    tcapi_set(instruction->node_name, instruction->sub_node, "Status", "0");
    tcapi_set(instruction->node_name, instruction->sub_node, "successCount", "4");
    tcapi_set(instruction->node_name, instruction->sub_node, "failureCount", "0");
    tcapi_set(instruction->node_name, instruction->sub_node, "averageResponseTime", "12.5");
    tcapi_set(instruction->node_name, instruction->sub_node, "minimumResponseTime", "10.2");
    tcapi_set(instruction->node_name, instruction->sub_node, "maximumResponseTime", "15.8");
    
    return 0;
}

// Các stub functions khác nếu cần
int tcapi_save() {
    printf("Saving configuration\n");
    return 0;
}

int ai_diagnostic_commit() {
    printf("Committing diagnostics\n");
    return 0;
}
