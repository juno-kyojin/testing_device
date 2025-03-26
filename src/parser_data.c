#include "parser_data.h"
#include "file_process.h"
#include "cjson/cJSON.h"
#include <stdlib.h>
#include <string.h>
#include <log.h>
bool read_json_test_cases(const char *json_file, test_case_t **test_cases, int *count) {
    char *json_str = NULL;
    size_t size;
    // Đọc nội dung file JSON vào chuỗi
    if (read_file(json_file, &json_str, &size) != 0) {
        return false;
    }

    // Phân tích chuỗi JSON bằng cJSON
    cJSON *root = cJSON_Parse(json_str);
    free(json_str); // Giải phóng chuỗi sau khi phân tích
    if (!root) {
        return false;
    }

    // Lấy số lượng phần tử trong mảng JSON
    int array_size = cJSON_GetArraySize(root);
    *test_cases = (test_case_t *)malloc(array_size * sizeof(test_case_t));
    if (!*test_cases) {
        cJSON_Delete(root);
        return false;
    }

    // Chuyển đổi từng phần tử JSON thành test_case_t
    *count = 0;
    cJSON *item;
    cJSON_ArrayForEach(item, root) {
        test_case_t *tc = &(*test_cases)[*count];
        if (json_to_test_case(item, tc)) {
            (*count)++;
        }
    }

    cJSON_Delete(root); // Giải phóng đối tượng cJSON
    return true;
}

bool json_to_test_case(cJSON *item, test_case_t *test_case) {
    // Lấy các trường cơ bản từ JSON
    cJSON *id = cJSON_GetObjectItem(item, "id");
    cJSON *type = cJSON_GetObjectItem(item, "type");
    cJSON *network_type = cJSON_GetObjectItem(item, "network_type");
    cJSON *name = cJSON_GetObjectItem(item, "name");
    cJSON *description = cJSON_GetObjectItem(item, "description");
    cJSON *target = cJSON_GetObjectItem(item, "target");
    cJSON *timeout = cJSON_GetObjectItem(item, "timeout");
    cJSON *enabled = cJSON_GetObjectItem(item, "enabled");
    cJSON *params = cJSON_GetObjectItem(item, "params");

    // Kiểm tra các trường bắt buộc
    if (!id || !type || !network_type || !name || !description || !target || !timeout || !enabled || !params) {
        return false;
    }

    // Gán giá trị vào cấu trúc test_case_t
    strncpy(test_case->id, id->valuestring, sizeof(test_case->id) - 1);
    test_case->type = (test_type_t)type->valueint;
    test_case->network_type = (network_type_t)network_type->valueint;
    strncpy(test_case->name, name->valuestring, sizeof(test_case->name) - 1);
    strncpy(test_case->description, description->valuestring, sizeof(test_case->description) - 1);
    strncpy(test_case->target, target->valuestring, sizeof(test_case->target) - 1);
    test_case->timeout = timeout->valueint;
    test_case->enabled = enabled->valueint != 0;

    // Xử lý tham số params theo loại test
    switch (test_case->type) {
        case TEST_PING:
            test_case->params.ping.count = cJSON_GetObjectItem(params, "count")->valueint;
            test_case->params.ping.size = cJSON_GetObjectItem(params, "size")->valueint;
            break;
        case TEST_THROUGHPUT:
            test_case->params.throughput.duration = cJSON_GetObjectItem(params, "duration")->valueint;
            strncpy(test_case->params.throughput.protocol, cJSON_GetObjectItem(params, "protocol")->valuestring, sizeof(test_case->params.throughput.protocol) - 1);
            break;
        case TEST_VLAN:
            test_case->params.vlan.vlan_id = cJSON_GetObjectItem(params, "vlan_id")->valueint;
            break;
        case TEST_SECURITY:
            strncpy(test_case->params.security.method, cJSON_GetObjectItem(params, "method")->valuestring, sizeof(test_case->params.security.method) - 1);
            break;
        default:
            break; // Có thể mở rộng cho các loại test khác
    }

    test_case->extra_data = NULL; // Chưa xử lý extra_data
    test_case->extra_data_size = 0;
    return true;
}

void free_test_cases(test_case_t *test_cases, int count) {
    if (test_cases) {
        for (int i = 0; i < count; i++) {
            if (test_cases[i].extra_data) {
                free(test_cases[i].extra_data);
            }
        }
        free(test_cases);
    }
}

bool filter_test_cases_by_network(const test_case_t *test_cases, int count, network_type_t network_type, test_case_t **filtered_test_cases, int *filtered_count) {
    // Đếm số lượng test case phù hợp
    int filtered_size = 0;
    for (int i = 0; i < count; i++) {
        if (test_cases[i].network_type == network_type || test_cases[i].network_type == NETWORK_BOTH) {
            filtered_size++;
        }
    }

    // Cấp phát bộ nhớ cho mảng lọc
    *filtered_test_cases = (test_case_t *)malloc(filtered_size * sizeof(test_case_t));
    if (!*filtered_test_cases) {
        return false;
    }

    // Sao chép các test case phù hợp
    *filtered_count = 0;
    for (int i = 0; i < count; i++) {
        if (test_cases[i].network_type == network_type || test_cases[i].network_type == NETWORK_BOTH) {
            memcpy(&(*filtered_test_cases)[*filtered_count], &test_cases[i], sizeof(test_case_t));
            (*filtered_count)++;
        }
    }

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

/**
 * @brief Phân tích nội dung JSON trực tiếp thành instruction
 * 
 * @param json_content Chuỗi JSON chứa instruction
 * @param instructions Con trỏ đến mảng instructions
 * @param count Con trỏ đến biến lưu số lượng instructions
 * @return true nếu thành công, false nếu thất bại
 */
bool parse_json_instruction(const char *json_content, instruction_t **instructions, int *count) {
    if (!json_content || !instructions || !count) {
        log_message(LOG_LVL_ERROR, "Invalid parameters for parse_json_instruction");
        return false;
    }

    cJSON *root = cJSON_Parse(json_content);
    if (!root) {
        log_message(LOG_LVL_ERROR, "Failed to parse JSON: %s", cJSON_GetErrorPtr());
        return false;
    }

    *count = 1; // Giả sử chỉ có 1 instruction
    *instructions = (instruction_t *)malloc(*count * sizeof(instruction_t));
    if (!(*instructions)) {
        cJSON_Delete(root);
        return false;
    }
    memset(*instructions, 0, *count * sizeof(instruction_t));
    instruction_t *instr = &(*instructions)[0];

    // Xử lý action
    cJSON *action = cJSON_GetObjectItem(root, "action");
    if (action && cJSON_IsString(action)) {
        strncpy(instr->action, action->valuestring, sizeof(instr->action) - 1);
    }

    // Giá trị mặc định nếu không có đầy đủ thông tin
    instr->type = ACTION_DIAGNOSTIC;
    strcpy(instr->set_func, "tcapi_set");
    strcpy(instr->get_func, "tcapi_get");
    strcpy(instr->commit_func, "ai_diagnostic_commit");
    strcpy(instr->save_func, "tcapi_save");
    strcpy(instr->node_type, "single");
    strcpy(instr->node_name, "Ping");
    strcpy(instr->sub_node, "Entry");
    instr->max_entry = 1;

    // Nếu có attributes thì parse, không thì dùng mặc định
    cJSON *attributes = cJSON_GetObjectItem(root, "attributes");
    if (attributes && cJSON_IsArray(attributes)) {
        instr->attr_count = cJSON_GetArraySize(attributes);
        instr->attributes = (attribute_t *)malloc(instr->attr_count * sizeof(attribute_t));
        // Parse attributes như trước
        for (int j = 0; j < instr->attr_count; j++) {
            cJSON *attr_json = cJSON_GetArrayItem(attributes, j);
            attribute_t *attr = &instr->attributes[j];
            cJSON *public_name = cJSON_GetObjectItem(attr_json, "public_attr_name");
            if (public_name) strncpy(attr->public_name, public_name->valuestring, sizeof(attr->public_name) - 1);
            cJSON *private_name = cJSON_GetObjectItem(attr_json, "private_attr_name");
            if (private_name) strncpy(attr->private_name, private_name->valuestring, sizeof(attr->private_name) - 1);
            cJSON *attr_type = cJSON_GetObjectItem(attr_json, "attr_type");
            if (attr_type) strncpy(attr->attr_type, attr_type->valuestring, sizeof(attr->attr_type) - 1);
            cJSON *attr_execute = cJSON_GetObjectItem(attr_json, "attr_execute");
            if (attr_execute) strncpy(attr->execute, attr_execute->valuestring, sizeof(attr->execute) - 1);
        }
    } else {
        // Mặc định attributes cho ping
        instr->attr_count = 8;
        instr->attributes = (attribute_t *)malloc(instr->attr_count * sizeof(attribute_t));
        strcpy(instr->attributes[0].public_name, "pingCode"); strcpy(instr->attributes[0].private_name, "Status"); strcpy(instr->attributes[0].attr_type, "int"); strcpy(instr->attributes[0].execute, "yes");
        strcpy(instr->attributes[1].public_name, "host"); strcpy(instr->attributes[1].private_name, "host"); strcpy(instr->attributes[1].attr_type, "string"); strcpy(instr->attributes[1].execute, "yes");
        strcpy(instr->attributes[2].public_name, "hostAddress"); strcpy(instr->attributes[2].private_name, "hostAddress"); strcpy(instr->attributes[2].attr_type, "string"); strcpy(instr->attributes[2].execute, "yes");
        strcpy(instr->attributes[3].public_name, "successCount"); strcpy(instr->attributes[3].private_name, "successCount"); strcpy(instr->attributes[3].attr_type, "int"); strcpy(instr->attributes[3].execute, "yes");
        strcpy(instr->attributes[4].public_name, "failureCount"); strcpy(instr->attributes[4].private_name, "failureCount"); strcpy(instr->attributes[4].attr_type, "int"); strcpy(instr->attributes[4].execute, "yes");
        strcpy(instr->attributes[5].public_name, "averageResponseTime"); strcpy(instr->attributes[5].private_name, "averageResponseTime"); strcpy(instr->attributes[5].attr_type, "float"); strcpy(instr->attributes[5].execute, "yes");
        strcpy(instr->attributes[6].public_name, "minimumResponseTime"); strcpy(instr->attributes[6].private_name, "minimumResponseTime"); strcpy(instr->attributes[6].attr_type, "float"); strcpy(instr->attributes[6].execute, "yes");
        strcpy(instr->attributes[7].public_name, "maximumResponseTime"); strcpy(instr->attributes[7].private_name, "maximumResponseTime"); strcpy(instr->attributes[7].attr_type, "float"); strcpy(instr->attributes[7].execute, "yes");
    }

    cJSON_Delete(root);
    return true;
}

/**
 * @brief Giải phóng bộ nhớ của mảng instructions
 * 
 * @param instructions Mảng instructions cần giải phóng
 * @param count Số lượng instructions
 */
void free_instructions(instruction_t *instructions, int count) {
    if (!instructions) {
        return;
    }
    
    log_message(LOG_LVL_DEBUG, "Freeing memory for %d instructions", count);
    
    for (int i = 0; i < count; i++) {
        if (instructions[i].attributes) {
            free(instructions[i].attributes);
            instructions[i].attributes = NULL;
        }
    }
    
    free(instructions);
}